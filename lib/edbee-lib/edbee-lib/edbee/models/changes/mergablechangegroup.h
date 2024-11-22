/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include "edbee/models/change.h"

namespace edbee {

class AbstractRangedChange;
class LineDataListChange;
class TextChange;
class TextRangeSet;

/// A special mergable group textchange. Used by the editor  to merge editing operation together..
/// I hate the name, but currently don't know a better name of this class :)
///
/// This is a complete rewrite of the earlier implementation. This implemenation
/// simply adds all changes to a group. It tries to compress the changes that are compressable
/// This group has 1 previous selection and 1 next selection. All selection changes are 'removed' and only the last and first state is stored.
class EDBEE_EXPORT MergableChangeGroup : public ChangeGroup
{
public:
    MergableChangeGroup( TextEditorController* controller );
    virtual ~MergableChangeGroup();

    // this change cannot be optimized away
    virtual bool isDiscardable();
    virtual void groupClosed();

    virtual void execute(TextDocument* document);
    virtual void revert(TextDocument* document);

private:
    void addOffsetDeltaToChanges( QList<AbstractRangedChange*>& changes, int fromIndex, int delta );
    int findInsertIndexForOffset( QList<AbstractRangedChange*>& changes, int offset );
    int mergeChange( QList<AbstractRangedChange*>& changes, TextDocument* doc, AbstractRangedChange* newChange, int& delta );
    void inverseMergeRemainingOverlappingChanges( QList<AbstractRangedChange*>& changes, TextDocument* doc, int mergedAtIndex, int orgStartOffset, int orgEndOffset , int delta);

    void giveChangeToList(  QList<AbstractRangedChange*>& changes, TextDocument* doc, AbstractRangedChange* change );
    void giveAndMergeChangeToList(  QList<AbstractRangedChange*>& changes, TextDocument* doc, AbstractRangedChange* change );

//TODO:     void giveAbstractRangedTextChange( TextDocument* doc, QList<AbstractRangedTextChange* changeList>& changes, AbstractRangedTextChange* change );

public:
    void giveSingleTextChange( TextDocument* doc, TextChange* change);
    void giveLineDataListTextChange( TextDocument* doc, LineDataListChange* change );

    virtual void giveChange( TextDocument* doc, Change* change );
    virtual Change* at( int idx );
    virtual Change* take( int idx );
    virtual int size();
    virtual void clear(bool performDelete=true);

    /// This method tries to merge the given change with the other change
    /// The textChange supplied with this method should NOT have been executed yet.
    /// It's the choice of this merge operation if the execution is required
    /// @param textChange the textchange to merge
    /// @return true if the merge has been successfull. False if nothing has been merged and executed
    virtual bool giveAndMerge( TextDocument* document, Change* textChange );

    virtual QString toString();
    QString toSingleTextChangeTestString();

    void moveChangesFromGroup( TextDocument* doc, ChangeGroup* group);

protected:

    bool mergeAsGroup( TextDocument* document, Change* textChange );
    bool mergeAsSelection( TextDocument* document, Change* textChange );

    void compressTextChanges( TextDocument* document );
    void compressChanges( TextDocument* document );

private:

    QList<AbstractRangedChange*> textChangeList_;           ///< The list of textchanges
    QList<LineDataListChange*> lineDataTextChangeList_;     ///<The list with liendata text changes
    QList<Change*> miscChangeList_;                         ///< Other textchanges


    TextRangeSet* previousSelection_;
    TextRangeSet* newSelection_;

};

} // edbee
