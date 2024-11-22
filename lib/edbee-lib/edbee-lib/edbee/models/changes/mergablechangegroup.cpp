/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "mergablechangegroup.h"

#include "edbee/models/changes/abstractrangedchange.h"
#include "edbee/models/changes/selectionchange.h"
#include "edbee/models/changes/textchange.h"
#include "edbee/models/changes/linedatalistchange.h"
#include "edbee/models/textrange.h"
#include "edbee/views/textselection.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/util/util.h"

#include "edbee/debug.h"

namespace edbee {


/// The default complex textchange constructor
MergableChangeGroup::MergableChangeGroup(TextEditorController* controller)
    : ChangeGroup(controller)
    , previousSelection_(0)
    , newSelection_(0)
{
    if( controller ) {
       previousSelection_ = new TextRangeSet( *( controller->textSelection() ) );
    }
}


/// The default destructor
MergableChangeGroup::~MergableChangeGroup()
{
    clear(true);
    delete newSelection_;
    delete previousSelection_;
}


/// default not discardable
bool MergableChangeGroup::isDiscardable()
{
    return false;
}


/// the group is closed, we must 'store' the selection
void MergableChangeGroup::groupClosed()
{
    if( controller() ) {
        delete newSelection_;
        newSelection_ = new TextRangeSet( *(controller()->textSelection() ) ) ;
    }
}


/// Executes this textchange
/// @param document the document to execute this operation for
void MergableChangeGroup::execute(TextDocument* document)
{
    ChangeGroup::execute( document );
    if( newSelection_ ) {
        TextRangeSet* currentSelection = dynamic_cast<TextRangeSet*>( controller()->textSelection() );
        *currentSelection = *newSelection_ ;
    }
}


/// this method is called to revert the operation
/// reverts the given operation
void MergableChangeGroup::revert( TextDocument* document )
{
    ChangeGroup::revert(document);
    if( previousSelection_) {
        TextRangeSet* currentSelection = dynamic_cast<TextRangeSet*>( controller()->textSelection() );
        *currentSelection = *previousSelection_;
    }
}


/// This method adds the given delta to the changes
void MergableChangeGroup::addOffsetDeltaToChanges(QList<AbstractRangedChange*>& changes, int fromIndex, int delta)
{
    for(int i = fromIndex; i<changes.size(); ++i ) {
        AbstractRangedChange* s2 = changes.at(i);
        s2->addOffset( delta );
    }
}


/// This method finds the insert index for the given offset
/// @param offset the offset of the change
/// @return the inertindex used for inserting the data
int MergableChangeGroup::findInsertIndexForOffset(QList<AbstractRangedChange*>& changes, int offset)
{
    int insertIndex = 0;
    for( int i=0,cnt=changes.size(); i<cnt; ++i ){
        AbstractRangedChange* change = changes.at(i);
        if( change->offset() < offset ) {
            insertIndex = i+1;
        }
    }
    return insertIndex;
}


/// This method tries to merge the given textchagne
/// @param newChange the new change to merge
/// @param delta (out) the delta applied to this change
/// @return the merged index or -1 if not merged!
int MergableChangeGroup::mergeChange(QList<AbstractRangedChange*>& changes, TextDocument* doc, AbstractRangedChange* newChange, int& delta)
{
    for( int i=0,cnt=changes.size(); i<cnt; ++i ){
        AbstractRangedChange* change = changes.at(i);

        // we need the previous length and new-length to know how the delta is changed of the other items
        int prevNewLength = change->docLength();
        int prevContentLength = change->storedLength();

        // try to merge it
        if( change->giveAndMerge( doc, newChange ) ) {

            // apply the delta (newLength and length is reversed when in undo state!)
            delta += (change->docLength()-prevNewLength) - (change->storedLength()-prevContentLength);
            return i;
        }
    }
    return -1;
}


/// When a normal merge was succesful (with mergeTextChange) we need to inverseMerge the other overlapping textchanges
/// @param mergedAtIndex the index where the first merge was succeeded
/// @param orgStartOffset the offset of the orgingal merged textchange
/// @param orgEndoOffset the end offset of the original merged textchange
/// @param delta the current delta used for offset calculating
void MergableChangeGroup::inverseMergeRemainingOverlappingChanges(QList<AbstractRangedChange*>& changes, TextDocument* doc, int mergedAtIndex, int orgStartOffset, int orgEndOffset, int delta)
{
    AbstractRangedChange* mergedChange = changes.at(mergedAtIndex);
    for( int i=mergedAtIndex+1; i<changes.size(); ++i ) {
        AbstractRangedChange* nextChange= changes.at(i);

        // only perform merging if the change overlapped a previous change
        if( nextChange->offset() < orgEndOffset && orgStartOffset < (nextChange->offset() + nextChange->docLength())   ) {

            // take the delta of the previous change before the merge
            int tmpDelta = mergedChange->storedLength() - mergedChange->docLength() + delta;

            // alter the delta, so we find the correct merge index
            nextChange->addOffset(tmpDelta);

            // notice the 'inversion of the merge. We apply the merged change to the next change
            if( nextChange->giveAndMerge( doc, mergedChange) ) {
                mergedChange = nextChange;
                changes.removeAt(mergedAtIndex);
                --i;
            } else {
                // remove the temporary delta
                nextChange->addOffset(-tmpDelta);
                break;  // cannot merge, we're done
            }
        } else {
            break;
        }
    }
}


/// gives the given textchange to the merge routine
void MergableChangeGroup::giveChangeToList(QList<AbstractRangedChange*>& changes, TextDocument* doc, AbstractRangedChange* newChange)
{
    //qlog_info() << "giveSingleTextChange" << newChange->toString();
    // remember the orginal ranges so we know which changes are affected by this new change
    int orgStartOffset = newChange->offset();
    int orgEndOffset = newChange->offset() + newChange->storedLength();

    // some variables to remebmer
    int addDeltaFromIndex = size();         // From which change index should we add delta?!
    int delta = 0;

    // First try to merge this new change
    int mergedAtIndex = mergeChange( changes, doc, newChange, delta );

    // when we could merge the change,
    // we need to try to merge it with next overlapping changes
    if( mergedAtIndex >= 0 ) {

        inverseMergeRemainingOverlappingChanges( changes, doc, mergedAtIndex, orgStartOffset, orgEndOffset, delta );
        addDeltaFromIndex = mergedAtIndex + 1;

    // not merged? then we need to add the change at the given index
    } else {

        // find the insert index
        int insertIndex = findInsertIndexForOffset( changes, newChange->offset() );

        // just insert change
        textChangeList_.insert( insertIndex, newChange);
        addDeltaFromIndex = insertIndex+1;

        // apply the delta (newLength and length is reversed when in undo state!)
        delta += newChange->storedLength() - newChange->docLength();
    }

    // next apply the delta to the following change
    addOffsetDeltaToChanges( changes, addDeltaFromIndex, delta);
}


/// Gives a single textchange
void MergableChangeGroup::giveSingleTextChange(TextDocument* doc, TextChange* change)
{
    giveChangeToList( textChangeList_, doc, change );
}


/// gives a line data list text change
void MergableChangeGroup::giveLineDataListTextChange(TextDocument* doc, LineDataListChange* newChange)
{
    Q_UNUSED(doc);
    this->lineDataTextChangeList_.append(newChange);    // no merge for the moment
/* TODO:  Work in progress below. This algorithm seems identical to the one used with SingleTextChanges
 *  we need to abstract the interfaces and create a general merge-ranges operation
 *
    // try to merge the change
    int mergeIndex = -1;
    int delta = 0;
    int addDeltaIndex = lineDataTextChangeList_.size();

    for( int i=0, cnt=lineDataTextChangeList_.size(); i<cnt; ++i ) {
        LineDataListTextChange* change = lineDataTextChangeList_.at(i);

        // we need the previous length and new-length to know how the delta is changed of the other items
        int prevLength = change->length();
        int prevNewLength = change->newLength();

        if( change->giveAndMerge( doc, newChange ) ) {
            mergeIndex = i;
            delta += (change->length()-prevLength) - (change->newLength()-prevNewLength);
            break;
        }
    }

    // when merge succeeded, add the delta index
    if( mergeIndex >= 0 ) {
        addDeltaIndex = mergeIndex+1;
    }

    // when not merged, find the insert index
    if( mergeIndex < 0 ) {
        // STILL a TODO
        lineDataTextChangeList_.append(newChange);
    }

    // next add delta offsets from the given index
    for(int i = addDeltaIndex,cnt=lineDataTextChangeList_.size(); i<cnt; ++i ) {
        LineDataListTextChange* s2 = lineDataTextChangeList_.at(i);
        s2->addDeltaToLine(delta);
    }
*/

}


/// Gives the change
void MergableChangeGroup::giveChange(TextDocument* doc, Change* change)
{
    // a single text change
    TextChange* textChange = dynamic_cast<TextChange*>(change);
    if( textChange ) {
        //textChangeList_.append(textChange);
        giveSingleTextChange( doc, textChange);
        return;
    }

    // a list text change
    LineDataListChange* lineDataTextChange = dynamic_cast<LineDataListChange*>(change);
    if( lineDataTextChange ) {
        giveLineDataListTextChange(doc,lineDataTextChange);
        return;
    }

    // a selection change simply is moved to the new selection object
    SelectionChange* selectionChange = dynamic_cast<SelectionChange*>(change);
    if( selectionChange ) {
        delete newSelection_;
        newSelection_ = selectionChange->takeRangeSet();

        /// we can simply delete the change, the ComplexTextChange automatically records the last change selection on the undoGroupEnd
        delete change;
        return;
    }

    // a group changes, just add all the groups
    ChangeGroup* group = dynamic_cast<ChangeGroup*>(change);
    if( group ) {
        moveChangesFromGroup(doc,group);
        delete group;
        return;
    }


    // other changes are (currently) added to a misch change list. And are pretty scary for now :)
    miscChangeList_.append(change);
}


///returns the textchange at the given index
Change* MergableChangeGroup::at(int idx)
{
    // plain text changes
    if( idx < textChangeList_.size() ) {
        return textChangeList_.at(idx);
    }
    // line-data changes
    idx -= textChangeList_.size();
    if( idx < lineDataTextChangeList_.size() ) {
        return lineDataTextChangeList_.at(idx);
    }
    // other changes
    idx -= lineDataTextChangeList_.size();
    Q_ASSERT(idx < miscChangeList_.size() );
    return miscChangeList_.at(idx);
}


/// Takes the given item
Change*MergableChangeGroup::take(int idx)
{
    // plain text changes
    if( idx < textChangeList_.size() ) {
        return textChangeList_.takeAt(idx);
    }
    // line-data changes
    idx -= textChangeList_.size();
    if( idx < lineDataTextChangeList_.size() ) {
        return lineDataTextChangeList_.takeAt(idx);
    }
    // other changes
    idx -= lineDataTextChangeList_.size();
    Q_ASSERT(idx < miscChangeList_.size() );
    return miscChangeList_.takeAt(idx);
}


/// returns the number of elements
int MergableChangeGroup::size()
{
    return textChangeList_.size() + lineDataTextChangeList_.size() + miscChangeList_.size();
}


/// clears all items
void MergableChangeGroup::clear(bool performDelete)
{
    // delete
    if( performDelete ) {
        qDeleteAll(textChangeList_);
        qDeleteAll(lineDataTextChangeList_);
        qDeleteAll(miscChangeList_);
    }

    // clear the changes
    textChangeList_.clear();
    lineDataTextChangeList_.clear();
    miscChangeList_.clear();
}


/// this method tries to merge the textchange with this text change
bool MergableChangeGroup::giveAndMerge(TextDocument* document, Change* textChange)
{
    giveChange( document, textChange );
    return true;
}



/// Converts this textchange to a textual representation
QString MergableChangeGroup::toString()
{
    return QStringLiteral("Complex::%1").arg(ChangeGroup::toString());
}


/// Converts the textchangeList as as string
/// The format is the following:
/// <offset>:<length>:<str>,...
///
/// sample:
/// 0:2:ABC,1:2:QW
QString MergableChangeGroup::toSingleTextChangeTestString()
{
    QString result;
    foreach( AbstractRangedChange* abstractChange, textChangeList_ ) {
        TextChange* change = dynamic_cast<TextChange*>(abstractChange);
        if( change ) {
            if( !result.isEmpty() ) result.append(",");
            result.append( QStringLiteral("%1:%2:%3").arg(change->offset()).arg(change->docLength()).arg(change->storedText()) );
        }
    }
    return result;
}


/// Moves all textchanges from the given group to this group
/// @param group the group to move the selection from
void MergableChangeGroup::moveChangesFromGroup( TextDocument* doc, ChangeGroup* group )
{
//qlog_info() << "moveChangeFromGroup **************** (MERGE)";
//qlog_info() << "A:" << this->toString();
//qlog_info() << "B:" << group->toString();

    // process all changes
    for( int i=0,cnt=group->size(); i<cnt; ++i ) {

        Change* change = group->at(i);

        // handle nested groups
        if( change->isGroup() ) {
            ChangeGroup* childGroup = dynamic_cast<ChangeGroup*>( change );
            if( childGroup ) {
                moveChangesFromGroup( doc, childGroup );
                delete childGroup;
                continue;
            }
        }

        // merge the the change
        giveChange( doc, change );
    }
    group->clear(false); // no delete we've taken the ownership

//qlog_info() << "=1>:" << this->toString();

    // next we need to compress the changes (merging changes that are next to eachother)
    compressChanges(doc);
//qlog_info() << "************************";
//qlog_info() << "=2>:" << this->toString();

    // when it's a complex text change, also move the selection to this group
    MergableChangeGroup* complexGroup = dynamic_cast<MergableChangeGroup*>(group);
    if( complexGroup ) {
        qSwap( newSelection_, complexGroup->newSelection_ );
    }
}



/// merges the given textchange as a group.
/// @param document the document this merge is for
/// @param textChange the textchange to merge
/// @return true if the textchange was merged as group.
bool MergableChangeGroup::mergeAsGroup(TextDocument* document, Change* textChange )
{
    Q_UNUSED(document);

    // make sure it's a group
    if( !textChange->isGroup() ) { return false; }
    ChangeGroup* group = dynamic_cast<ChangeGroup*>(textChange);
    if( !group ) { return false; }

    // move all changes from the other group to this group
    moveChangesFromGroup(document, group);

    delete textChange;
    return true;
}


// merge the text selection
/// @param document the document this merge is for
/// @param textChange the textchange to merge
/// @return true if the textchange was merged as group.
bool MergableChangeGroup::mergeAsSelection(TextDocument* document, Change* textChange)
{
    Q_UNUSED(document);
    SelectionChange* selectionChange = dynamic_cast<SelectionChange*>(textChange);
    if( !selectionChange ) { return false; }

    // simply take the selection from the textchange and make this the new selection
    delete newSelection_;
    newSelection_ = selectionChange->takeRangeSet();
    delete textChange;
    return true;
}


/// Compresses the textchanges
/// @param document the document to merge the changes for
void MergableChangeGroup::compressTextChanges(TextDocument* document)
{
    // compress single text changes
    for( int i=0; i<textChangeList_.size(); ++i ) {
        AbstractRangedChange* change1 = textChangeList_.at(i);

        // find the next text change
        for( int j=i+1; j<textChangeList_.size(); ++j ) {
            AbstractRangedChange* change2 = textChangeList_.at(j);
            if( change1->giveAndMerge( document, change2 ) ) {
                textChangeList_.takeAt(j);  // just take it, the item is already deleted by the merge
                --j;
            }
        } // for j
    } // for i
}


/// This is going to be the magic method that's going to merge all changes
/// @param document the document to merge the changes for
void MergableChangeGroup::compressChanges(TextDocument* document)
{
    compressTextChanges(document);
}


} // edbee
