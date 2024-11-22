/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textundostack.h"

#include "edbee/models/change.h"
#include "edbee/texteditorcommand.h"
#include "edbee/models/textdocument.h"

#include "edbee/debug.h"

//#define DUMP_UNDO_STACK

namespace edbee {


/// Constructs the main undostack
TextUndoStack::TextUndoStack( TextDocument* doc, QObject *parent)
    : QObject(parent)
    , documentRef_(doc)
    , changeList_()
    , changeIndex_(0)
    , controllerIndexMap_()
    , persistedIndex_(0)        // defaults to 0 because a blank/new document is perstisted (doesn't need to be saved)
    , undoGroupStack_()
    , lastCoalesceIdStack_()
    , collectionEnabled_(true)
    , undoRunning_(false)
    , redoRunning_(false)
{
    lastCoalesceIdStack_.push(0);
}


/// the undo stack items
TextUndoStack::~TextUndoStack()
{
    disconnect();   // disconnect so no signals are emitted
    clearHistoryLists();
    controllerIndexMap_.clear();
}


/// deletes all created objects
void TextUndoStack::clearHistoryLists()
{
    qDeleteAll(changeList_);
    changeList_.clear();
    qDeleteAll( undoGroupStack_ );
    undoGroupStack_.clear();
    lastCoalesceIdStack_.clear();
}


/// clears both stacks
void TextUndoStack::clear()
{
    clearHistoryLists();
    lastCoalesceIdStack_.push(0);   // always 1 item

    // point all the indices to BLANK
    changeIndex_ = 0;
    setPersistedIndex(-1); // clearing the undo-stack must result in a non-saved state
    QMapIterator<TextEditorController*, int> itr(controllerIndexMap_);
    while (itr.hasNext()) {
        itr.next();
        controllerIndexMap_.insert( itr.key(), 0 );
    }
}


/// Registers acontroller for it's own view pointer
void TextUndoStack::registerContoller(TextEditorController* controller)
{
    controllerIndexMap_.insert(controller,changeIndex_);
}


/// Unregisters the given controller
void TextUndoStack::unregisterController(TextEditorController* controller)
{
    controllerIndexMap_.remove(controller);
}


/// Checks if the given controller is registered
bool TextUndoStack::isControllerRegistered(TextEditorController *controller)
{
    return controllerIndexMap_.contains(controller);
}


/// Starts an undo group (or increase nest-level-counter)
void TextUndoStack::beginUndoGroup( ChangeGroup* group )
{
//    if( !group ) { group = new TextChangeGroup(controller); }
    undoGroupStack_.append( group );
    lastCoalesceIdStack_.append(0);
    emit undoGroupStarted( group );
}


/// This method returns the current active group
/// @return the current undo stack item or 0
ChangeGroup* TextUndoStack::currentGroup()
{
    if( undoGroupStack_.isEmpty() ) { return 0; }
    return undoGroupStack_.last();
}


/// Ends the undogroup
/// @param coalesceId the coalesceId to use
/// @param flatten should the textchange groups be flattened
void TextUndoStack::endUndoGroup(int coalesceId, bool flatten )
{
    Q_ASSERT(!undoGroupStack_.isEmpty());
    if( undoGroupStack_.isEmpty() ) return;
    ChangeGroup* group = undoGroupStack_.pop();
    lastCoalesceIdStack_.pop();

    if( group ) {
        group->groupClosed();   // close the group

        // flatten the group?
        if( flatten ) {
            group->flatten();
        }
        // multiple-changes or not discardable
        if( !group->isDiscardable() || group->size() > 1 ) {
            Change* newChange = giveChange( group , coalesceId );
            bool merged = newChange && newChange != group;
            emit undoGroupEnded( coalesceId, merged, ActionEnd );

        // a single change (don't give the group, just the single change)
        } else if( group->size() == 1 ) {
            Change* change = group->takeLast();
            Change* newChange = giveChange( change, coalesceId );
            bool merged = newChange && newChange != change;
            delete group;
            emit undoGroupEnded( coalesceId, merged, ActionUngrouped );
        // empty group, can be optimized away
        } else {
            delete group;
            emit undoGroupEnded( coalesceId, false, ActionFullDiscard );
        }
    }
}


/// Ends the undo-group and discards all content
void TextUndoStack::endUndoGroupAndDiscard()
{
    Q_ASSERT(!undoGroupStack_.isEmpty());
    if( undoGroupStack_.isEmpty() ) return;
    ChangeGroup* group = undoGroupStack_.pop();
    lastCoalesceIdStack_.pop();

    if( group ) {
        group->groupClosed();   // close the group
        delete group;
        emit undoGroupEnded( 0, false, ActionFullDiscard );
    }
}


/// returns the number of stacked undo-groups
int TextUndoStack::undoGroupLevel()
{
    return undoGroupStack_.size();
}


/// returns the last coalesceId at the given level
int TextUndoStack::lastCoalesceIdAtCurrentLevel()
{
    return lastCoalesceIdStack_[ lastCoalesceIdStack_.size()-1 ];
}


/// Sets the last coalesceId at the current level
void TextUndoStack::setLastCoalesceIdAtCurrentLevel(int id)
{
    lastCoalesceIdStack_[ lastCoalesceIdStack_.size()-1 ] = id;
}


/// Resets all coalsceIds
void TextUndoStack::resetAllLastCoalesceIds()
{
    for( int i=0, cnt=lastCoalesceIdStack_.size(); i<cnt; ++i ) {
        lastCoalesceIdStack_[i]=0;
    }
}


/// This method gives the command and tries to coalesce the command if possible
/// Warning when a change is MERGED, the original change is deleted!!! And the original pointer is invalid!!!!
///
/// @param change the change that's added
/// @param coalesceId the coalseceId to use for this text change.
////
/// @return the merged Change
///        The return value is 0, if the change was discared
///        The return value == change if the change was appended to the stack
///        The return value != change if the change was merged  (previous stack item is returned)
Change* TextUndoStack::giveChange(Change* change, int coalesceId )
{
    // when undo-collection is disabled. don't record the change
    if( !collectionEnabled_ ) {
        delete change;
        return 0;
    }

    ChangeGroup* group = undoGroupStack_.isEmpty() ? 0 : undoGroupStack_.top();

    int lastCoalesceId = lastCoalesceIdAtCurrentLevel();
    bool merge = lastCoalesceId && lastCoalesceIdStack_.top() == coalesceId && coalesceId != CoalesceId_None;

    setLastCoalesceIdAtCurrentLevel(coalesceId);
    if( coalesceId == CoalesceId_ForceMerge ) { merge = true; }

    // still an undogroup active ?
    //----------------------------
    if( group ) {
        //TextEditorController* controller = change->controllerContext();
        // try to merge the operation
        if( merge ) {
            Change* lastChange = group->last();
            if( lastChange && lastChange->giveAndMerge( documentRef_, change ) ) {
                return lastChange;
            }
        }

        group->giveChange( document(), change);
        // TODO: giveChange to a group can also merge a change. This can have implications
        //   on the change. At the moment I simply return the group. I doubt this is correct
        //   for the moment it's the best solution for this problem
        return group;

    // normal operation
    //-----------------
    } else {
        // get the optional controller context
        TextEditorController* controller = change->controllerContext();
        clearRedo( controller );

        // try to merge the operation
        if( merge ) {
            Change* lastChange = this->findUndoChange(controller);
            if( lastChange && lastChange->giveAndMerge( documentRef_, change ) ) {
                //emit changeMerged( lastChange, change );
                dumpStackInternal();
                return lastChange;
            }
        }

        // execut the operation
        changeList_.append(change);

        // increase the pointers
        // controller-only change, only update the controller pointer
        if( controller ) {
            controllerIndexMap_[controller] = changeList_.size();

        // model-change update the model-pointer and ALL controller pointers
        } else {
            setChangeIndex( changeList_.size() );
            QMap<TextEditorController*, int>::iterator i;
            for (i = controllerIndexMap_.begin(); i != controllerIndexMap_.end(); ++i) {
                i.value() = changeIndex_;
            }
        }
        emit changeAdded( change );

    }
    dumpStackInternal();
    return change;
}


/// Should check if a undo operation can be performed.
/// When no controller is given a document-undo is checkd
/// else a view specific soft undo is tested
bool TextUndoStack::canUndo(TextEditorController* controller)
{
    return findUndoChange(controller) != 0;
}


/// Should check if a redo operation can be performed.
/// When no controller is given a document-redo is checkd
/// else a view specific soft redo is tested
bool TextUndoStack::canRedo(TextEditorController* controller)
{
    return findRedoChange(controller) != 0;
}


/// performs an undo operation
/// @param controller this method is called
void TextUndoStack::undo(TextEditorController* controller, bool controllerOnly )
{
    Q_ASSERT(!undoRunning_);
    undoRunning_ = true;
    resetAllLastCoalesceIds();

    Change* changeToUndo = this->findUndoChange( controller );
    // only for the current controller
    if( changeToUndo && changeToUndo->controllerContext() ) {
        Q_ASSERT(changeToUndo->controllerContext() == controller );
        undoControllerChange( controller );

    // else we need to undo ALL other controller changes
    } else if( !controllerOnly ) {
        undoDocumentChange( );
    }
    dumpStackInternal();

    undoRunning_ = false;
}


/// performs the redo operation for the given controller
/// @param controller the controller to execute the redo for
/// @param controllerOnly undo controller undo's only?
void TextUndoStack::redo(TextEditorController* controller, bool controllerOnly )
{
    Q_ASSERT(!redoRunning_);
    redoRunning_ = true;
    resetAllLastCoalesceIds();

    Change* changeToRedo = findRedoChange( controller );
    if( changeToRedo && changeToRedo->controllerContext() ) {
        Q_ASSERT(changeToRedo->controllerContext() == controller );
        redoControllerChange( controller );

    } else if( !controllerOnly ){
        redoDocumentChange();

    }
    dumpStackInternal();
    redoRunning_ = false;
}


/// returns the number of changes on the stack
int TextUndoStack::size()
{
    return changeList_.size();
}


/// returns the change at the given index
Change* TextUndoStack::at(int idx)
{
    return changeList_.at(idx);
}


/// This method return the index that's active for the given controller
/// The currentIndex points directly AFTER the last placed item on the stack
int TextUndoStack::currentIndex(TextEditorController* controller)
{
    if( controller ) {
        Q_ASSERT( controllerIndexMap_.contains(controller) );
        return controllerIndexMap_.value(controller);
    } else {
        return changeIndex_;
    }

}


/// This method returns the current redo change. This maybe the change for the given context or the document change
/// depending on what comes first
/// @param controller the controller this change is for
/// @return the new text change (0 if there aren't any redo items availabe for the given context). When a controller is given the document change is ALSO returned
Change* TextUndoStack::findRedoChange(TextEditorController* controller)
{
    int redoIndex = findRedoIndex( currentIndex(controller), controller );
    if( redoIndex < changeList_.size() ) {
        return at( redoIndex );
    }
    return 0;
}


/// This method returns the last index for a given controller
/// The lastIndex points to the index on the stack
/// @return the index on the stack or -1 if the item isn't on the undo-stack
int TextUndoStack::lastIndex(TextEditorController *controller)
{
    return currentIndex( controller ) - 1;
}


/// Returns last textchange on the undo stack
/// @param controller the controller
/// @return the last change (with optional the controller context) 0 if no last change
Change* TextUndoStack::last(TextEditorController* controller)
{
    int idx = lastIndex(controller);
    if( idx < 0 ) { return  0; }
    return at(idx);
}


/// The number of doc-'undo' items on the stack
int TextUndoStack::sizeInDocChanges()
{
    int count = 0;
    foreach( Change* change, changeList_ ) {
        if( change->isDocumentChange() ) { ++count;}
    }
    return count;
}


/// This method returns the current doc change index
int TextUndoStack::currentIndexInDocChanges()
{
    int index = 0;
    for( int i=0; i < changeIndex_; ++i ) {
        if( changeList_.at(i)->isDocumentChange() ) { ++index; }
    }
    return index;
}


/// This method returns the last change (TOS) for the given controller
Change* TextUndoStack::findUndoChange(TextEditorController* controller)
{
    int undoIndex = this->findUndoIndex( currentIndex(controller), controller );
    if( undoIndex >= 0 ) {
        return changeList_.at(undoIndex);
    }
    return 0;
}


/// Marks the current documentIndex as persisted
void TextUndoStack::setPersisted(bool enabled)
{
    setPersistedIndex( enabled ? currentIndex() : -1);
}


/// returns true if the undo-stack is on a persisted index
bool TextUndoStack::isPersisted()
{
    int curIdx = currentIndex();
    if( persistedIndex_ == curIdx ) { return true; }
    if( persistedIndex_ < 0 ) { return false;}

    // check if all changes are non-persistable
    int startIdx = qMax( 0, qMin( curIdx, persistedIndex_ ));
    int endIdx   = qMin( qMax( curIdx, persistedIndex_ ), changeList_.size());

    for( int idx=startIdx; idx<endIdx; ++idx ) {
        Change* change = changeList_.at(idx);

        // only check document changes
        if( change->isDocumentChange() ) {
            if( change->isPersistenceRequired() ) {
                return false;
            }
        }
    }
    return true;
}


/// This method returns the current persisted index
int TextUndoStack::persistedIndex()
{
    return persistedIndex_;
}


/// makes a dump of the current stack
QString TextUndoStack::dumpStack()
{
    QString result;
    result += "\n";
    result +="UndoStack\n";
    result += "=====================\n";

    bool end=true;
    for( int i=changeList_.size(); i >=0 ; --i ) {
        QString str;
        QString changeName;
        QString controllerName;
        QString pointers;

        /// build the stack overview
        if( end ) {
            changeName= "^^^^^^^^^^^^^^^^^";
        } else {
            Change* change= at(i);
            TextEditorController* controller = change->controllerContext();
            changeName = change->toString();
            if( controller ) {
                controllerName = QString::number((quintptr)controller,16);
            }
        }

        // next append the pointers
        QMap<TextEditorController*, int>::iterator itr;
        for( itr = controllerIndexMap_.begin(); itr != controllerIndexMap_.end(); ++itr ) {
            if( itr.value() == i ) {
                pointers.append( QStringLiteral("<=(%1) ").arg( QString::number((quintptr)itr.key(),16) ) );
            }
        }
        if( changeIndex_ == i ) {
            pointers.append( QStringLiteral("<=(DOC) " ));
        }
        if( persistedIndex_ == i ){
            pointers.append( QStringLiteral("<=(P) " ));
        }

        result += QStringLiteral("-|%1:%2| %3\n").arg(changeName,-30).arg(controllerName,10).arg(pointers);
        end=false;
    }
    return result;
}


/// Dumps the internal stack
void TextUndoStack::dumpStackInternal()
{
#ifdef DUMP_UNDO_STACK
    qDebug().noquote() << this->dumpStack();
#endif
}


/// This method finds the next redo-item
/// @param index the index to search it from
/// @param controller the controller context
/// @return the next change index. Or changeList_.size() if there's no next change
int TextUndoStack::findRedoIndex(int index, TextEditorController* controller)
{
    if( index < changeList_.size() ) {
        int size = changeList_.size();
        for( ; index < size; ++index ) {
            Change* change = at(index);
            TextEditorController* context = change->controllerContext();
            if( context == 0 || context == controller ) { return index; }
        }
    }
    return changeList_.size();
}


/// This method finds the index of the given stackitem from the given index
/// @param  index the previous index
/// @param controller the controller context
/// @return the previous index -1, if there's no previous index
int TextUndoStack::findUndoIndex( int index, TextEditorController* controller)
{
    if( index > 0 ) {
        for( --index; index >= 0; --index ) {
            Change* change = at(index);
            TextEditorController* context = change->controllerContext();
            if( context == 0 || context == controller ) { return index; }
        }
    }
    return -1;
}


/// Clears all redo's for the given controller
void TextUndoStack::clearRedo(TextEditorController* controller)
{
    // view specific undo
    if( controller ) {      
        int idx = changeIndex_;
        if( controllerIndexMap_.contains(controller) ) {
            idx = this->controllerIndexMap_.value(controller);
        } else {
            Q_ASSERT(false && "The current controller isn't registered with the undostack!");    // warning view isn't registered!
        }

        // remove all items from the stack AFTER the given index
        for( int i=changeList_.size()-1; i >= idx; --i ) {
            if( changeList_.at(i)->controllerContext() == controller ) {
                delete changeList_.takeAt(i);
            }
        }

    } else {
        if( currentIndex(0) < persistedIndex() ) {
            setPersistedIndex(-1);
        }

        // find the next redo-document operation
        int redoIndex = findRedoIndex( currentIndex(0), 0);
        for( int i=changeList_.size()-1; i >= redoIndex; --i ) {
            delete changeList_.takeAt(i);
        }

        // remove all items from the stack AFTER the given index
//        for( int i=changeList_.size()-1; i >= changeIndex_; --i ) {
//            delete changeList_.takeAt(i);
//        }
    }
}


/// This method undos the given controller change. This method does NOT undo document changes
/// @param controller the controller to undo the change form
/// @return true if a change has been undone.
bool TextUndoStack::undoControllerChange(TextEditorController* controller)
{
    Q_ASSERT(controller);
    int changeIdx = controllerIndexMap_.value(controller)-1;    // -1, because the index is directly AFTER the item
    Change* changeToUndo = findUndoChange( controller );

    if( changeToUndo && changeToUndo->controllerContext() ) {
        Q_ASSERT( changeToUndo->controllerContext() == controller );
        changeToUndo->revert( documentRef_ );
        controllerIndexMap_[controller] = findUndoIndex( changeIdx, controller )+1;
        emit undoExecuted( changeToUndo );
        return true;
    }
    return false;
}


/// Performs an document-change undo.
void TextUndoStack::undoDocumentChange()
{
    // first make sure ALL controller-changes are back to the document change-level
    QMap<TextEditorController*, int>::iterator itr;
    for( itr = controllerIndexMap_.begin(); itr != controllerIndexMap_.end(); ++itr) {
        TextEditorController* controller = itr.key();
        while( undoControllerChange(controller) ) {};  // undo all controller operations
    }

    // next perform an undo of the document operations
    Change* change = findUndoChange();
    if( change ) {
        Q_ASSERT( change->controllerContext() == 0 );
        change->revert( documentRef_ );

        // next move the pointers to first 'related' change
        for( itr = controllerIndexMap_.begin(); itr != controllerIndexMap_.end(); ++itr) {
            TextEditorController* controller = itr.key();
            controllerIndexMap_[controller] = findUndoIndex( changeIndex_-1, controller ) + 1; // +1 because the pointer points AFTER the found index
        }

        // and finally move the document pointer
        setChangeIndex( findUndoIndex( changeIndex_-1 ) + 1 );  // +1 because the pointer points AFTER the found index
        emit undoExecuted( change);
    }

}


/// undo's the given controller change
bool TextUndoStack::redoControllerChange(TextEditorController *controller)
{
    Q_ASSERT(controller);
    int changeIdx = findRedoIndex( currentIndex(controller), controller);
    Change* changeToRedo = findRedoChange( controller );
    if( changeToRedo && changeToRedo->controllerContext() ) {
        Q_ASSERT( changeToRedo->controllerContext() == controller );
        changeToRedo->execute( documentRef_ );

        // move the pointer to the next
        controllerIndexMap_[controller] = changeIdx+1;
        emit redoExecuted( changeToRedo );
    }
    return false;
}


/// redo a document change
void TextUndoStack::redoDocumentChange()
{
    // first find the redo operation
    int redoIndex = findRedoIndex( currentIndex(0), 0 );
    Change* redo = findRedoChange(0);
    if( redo ) { ++redoIndex; }

    // first move all controller redo-operation to the given redo found redo location
    QMap<TextEditorController*, int>::iterator itr;
    for( itr = controllerIndexMap_.begin(); itr != controllerIndexMap_.end(); ++itr) {
        TextEditorController* controller = itr.key();
        while( redoControllerChange(controller) ) {};  // undo all controller operations
        itr.value() = redoIndex;    // move the position after the DOC operation
    }

    // is there a document redo? execute it
    if( redo ) {
        redo->execute(documentRef_);
        setChangeIndex(redoIndex);
        emit redoExecuted( redo );
    }
}


/// Sets the persisted in dex and fires a signal
void TextUndoStack::setPersistedIndex(int index)
{
    bool oldPersisted = isPersisted();
    persistedIndex_ = index;
    bool persisted = isPersisted();
    if( oldPersisted != persisted ) {
        emit persistedChanged( persisted );
    }
}


/// When setting the change-index we sometimes must emit a persisted change
void TextUndoStack::setChangeIndex(int index)
{
    if( index != changeIndex_ ) {
        bool oldPersisted= isPersisted();
        changeIndex_ = index;
        bool persisted = isPersisted();
        if( oldPersisted != persisted ) {
            emit persistedChanged( persisted );
        }

    }
}


} // edbee
