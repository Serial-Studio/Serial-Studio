/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include "edbee/models/change.h"

namespace edbee {

class TextDocSelState;
class TextEditorController;
class TextRangeSet;

/// Move the caret / and selection commands
class EDBEE_EXPORT SelectionChange : public ControllerChange
{
public:

    SelectionChange( TextEditorController* controller );
    virtual ~SelectionChange();

    virtual void giveTextRangeSet( TextRangeSet* rangeSet );
    virtual TextRangeSet* takeRangeSet();

    virtual void execute(TextDocument* document);
    virtual void revert(TextDocument* document);

    virtual bool giveAndMerge(TextDocument *document, Change* textChange );
    TextRangeSet* rangeSet() { return rangeSet_; }

    virtual QString toString();

protected:
    void notifyChange();

private:

    TextRangeSet* rangeSet_;            ///< This rangeset contains the new ranges OR the old ranges, depending on the state on the undo-stack (This is scary but saves us the storage of a compelte rangeSet)
};


} // edbee
