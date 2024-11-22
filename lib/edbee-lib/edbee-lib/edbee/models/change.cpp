/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "change.h"

#include "edbee/models/textdocument.h"
#include "edbee/models/changes/textchange.h"

#include "edbee/debug.h"

namespace edbee {

/// a virtual empty destructor
Change::~Change()
{
}


/// this method reverts the given operation
void Change::revert(TextDocument*)
{
    Q_ASSERT(false);
}


/// Gives the change and merges it if possible. This method should return false if the change couldn't be merged.
/// When the method returns true the ownership of the given textchange is transfered to this class.
/// @param document the document this change is for
/// @param textChange the textchange
/// @return true if the merge succeeded. The textChange ownership is only tranfered if true if returend
bool Change::giveAndMerge(TextDocument* document, Change* textChange)
{
    Q_UNUSED(document)
    Q_UNUSED(textChange )
    return false;
}


/// This method should return true if the change can be reverted
bool Change::canUndo()
{
    return false;
}


/// This  flag is used to mark this stack item as non-persistence requirable
/// The default behaviour is that every textchange requires persistence. It is also possible to
/// have certain changes that do not require persitence but should be placed on the undo stack
bool Change::isPersistenceRequired()
{
    return true;
}


/// A text command can belong to a controller/view
/// When it's a view only command. The undo only applies only to this view
/// warning a DOCUMENT change may NEVER return a controllerContext!!
TextEditorController*Change::controllerContext()
{
    return nullptr;
}


/// this method can be used to check if the given change is a document change
bool Change::isDocumentChange()
{
     return controllerContext() == nullptr;
}


/// This method returns true if this change is a group change. When an object is group change
/// it should be inherited by TextChangeGroup
bool Change::isGroup()
{
    return false;
}


//--------------------------------------------------------------


/// Empty change doesn't do anything
bool EmptyDocumentChange::isPersistenceRequired()
{
    return false;
}


/// does nothing
void EmptyDocumentChange::execute(TextDocument*)
{
}

/// does nothing
void EmptyDocumentChange::revert(TextDocument*)
{
}

/// returns the name of the textchange
QString EmptyDocumentChange::toString()
{
     return "EmptyDocumentTextChange";
}


//--------------------------------------------------------------


/// A controller specific textcommand. Warning you should NOT modify the textdocument!
/// @param controller the controller this change is for
ControllerChange::ControllerChange(TextEditorController* controller)
    : controllerRef_( controller )
{
}


/// A text command can belong to a controller/view
/// When it's a view only command. The undo only applies only to this view
TextEditorController* ControllerChange::controllerContext()
{
    return controllerRef_;
}


/// returns the controller
TextEditorController* ControllerChange::controller()
{
    return controllerRef_;
}


//--------------------------------------------------------------


/// default contructor
/// @param controller the controller this groups belongs to
ChangeGroup::ChangeGroup(TextEditorController* controller)
    : ControllerChange( controller )

{
}


/// The destructor deletes all added textchanges
ChangeGroup::~ChangeGroup()
{
    qDeleteAll(changeList_);
    changeList_.clear();
}


/// A group change is a group change, so this method returns true :)
bool ChangeGroup::isGroup()
{
    return true;
}


/// This method is called it the group is discardable. A discardable group will be optimized away if
/// the group is empty, or if there's a single item in the group. A none-discardable group will
/// always remain
bool ChangeGroup::isDiscardable()
{
    return true;
}

/// This method is called if the group is closed and is added to the stack
/// Default implementation is to do nothing
void ChangeGroup::groupClosed()
{
}


/// executes this command group
/// @param document the document the document to execute this for
void ChangeGroup::execute(TextDocument* document)
{
    Q_UNUSED(document)
    for( int i=0,cnt=size(); i<cnt; ++i ) {
        at(i)->execute(document);
    }
}


/// Reverts the command gorup
void ChangeGroup::revert(TextDocument* document)
{
    Q_UNUSED(document)
    for( int i=size()-1; i>=0; --i ) {
        at(i)->revert(document);
    }
}


/// When another group is given, a merge is performed
bool ChangeGroup::giveAndMerge(TextDocument *document, Change *textChange)
{
    if( textChange->isGroup()) {

        // move all changes from the given change-group to the new one
        ChangeGroup* inGroup = dynamic_cast<ChangeGroup*>(textChange);
        for( int i=0, cnt=inGroup->size(); i<cnt; ++i ) {
            giveChange( document, inGroup->at(i));
        }
        while( inGroup->takeLast());
        delete textChange;

        return true;
    }
    return false;
}


/// This method flattens the undo-group by expanding all subgroups to local groups
void ChangeGroup::flatten()
{
    for( int idx=0; idx < changeList_.size(); ++idx ) {
        ChangeGroup* group = dynamic_cast<ChangeGroup*>( changeList_[idx] );
        if( group ) {
            changeList_.removeAt(idx);
            for( int i=0; i<group->size(); ++i ) {
                ChangeGroup* subGroup = dynamic_cast<ChangeGroup*>( group->changeList_[i] );
                if( subGroup ) {
                    subGroup->flatten();   // flatten the group
                    delete subGroup;
                    if( i >= group->size() ) break; // just make sure empty groups go right (in theory empty groups aren't possible)
                }
                changeList_.insert( idx+i, group->changeList_[i] );
            }
            group->changeList_.clear();
            delete group;
        }
    }
}

void ChangeGroup::giveChange(TextDocument *doc, Change *change)
{
    Q_UNUSED(doc)
    changeList_.append(change);
}

Change *ChangeGroup::at(int idx)
{
    Q_ASSERT(idx < changeList_.size());
    return changeList_.at(idx);
}

Change *ChangeGroup::take(int idx)
{
    Change* change = changeList_.at(idx);
    changeList_.removeAt(idx);
    return change;
}

int ChangeGroup::size()
{
    return changeList_.size();
}

void ChangeGroup::clear(bool performDelete)
{
    if( performDelete ) qDeleteAll(changeList_);
    changeList_.clear();
}


/// This method returns the last change in the change group
/// @return the last textchange
Change* ChangeGroup::last()
{
    if( size() == 0 ) { return nullptr; }
    return at(size()-1);
}


/// Takes the ownership of the last element and removes it from the stack
/// @return the last textchange
Change* ChangeGroup::takeLast()
{
    if( size() == 0 ) { return nullptr; }
    return take(size()-1);
}


/// The total number of items in the list (excluding the group items)
/// @return the number of items recussive (iterating) all groups
int ChangeGroup::recursiveSize()
{
    int itemCount = 0;
    for( int i=0,cnt=changeList_.size(); i<cnt; ++i ) {
        ChangeGroup* group = dynamic_cast<ChangeGroup*>(changeList_[i]);
        if( group ) {
            itemCount += group->size();
        } else {
            ++itemCount;
        }
    }
    return itemCount;
}


/// if this commandgroup only contains commands for a single controller context
/// Then this context is returned else 0 is returned
TextEditorController* ChangeGroup::controllerContext()
{
    TextEditorController* context = nullptr;
    for( int i=size()-1; i>=0; --i ) {
        TextEditorController* commandContext = at(i)->controllerContext();

        // multiple context in 1 group means it's a 'hard' undo
        if( commandContext == nullptr ) return nullptr;  /// 0 is always 0!
        if( commandContext && context && commandContext != context ) { return nullptr; }
        if( !context && commandContext ) {
            context = commandContext;
        }
    }
    return context;
}


/// Converts this change group to a string
QString ChangeGroup::toString()
{
    QString s;
    s = QStringLiteral("%1/%2").arg(size()).arg(recursiveSize());
//    for( int i=0,cnt=changeList_.size(); i<cnt; ++i ) {
//        s.append( changeList_.at(i)->toString() );
//        s.append(",");
//    }
//    s.remove(s.length()-1);

    QString extra;
    for( int i=0,cnt=size(); i<cnt; ++i ) {
        extra.append( QStringLiteral(" - %1: ").arg(i));
        extra.append( at(i)->toString() );
        extra.append("\n");
    }

    return QStringLiteral("ChangeGroup(%1)\n%2").arg(s).arg(extra);
}



} // edbee
