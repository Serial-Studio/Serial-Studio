/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include "edbee/models/textrange.h"

namespace edbee {

class TextEditorController;

/// The class textselection is a RangeSet that is used
/// by the view of the document
///
/// The textselection is placed in the view directory because it closely
/// relates to the view of the textdocument
///
/// For instance moving line/up and line/down isn an operation that isn't possible on the document
/// side. Because we support variable font widts..
/// Also 'remembering' the caret screen-x and -y positions isn't relevant for a docuemnt. That is
/// stuff that needs to be placed in the view side
class EDBEE_EXPORT TextSelection : public TextRangeSet
{
public:
    TextSelection( TextEditorController* controller );
    TextSelection( const TextSelection& selection );
    virtual ~TextSelection();

    static void moveCaretsByLine( TextEditorController* controller, TextRangeSet* rangeSet, int amount );
    static void moveCaretsByPage( TextEditorController* controller, TextRangeSet* rangeSet, int amount );
    static void addRangesByLine( TextEditorController* controller, TextRangeSet* rangeSet, int amount );


    virtual

  // getters
    TextEditorController* textEditorController() const;

protected:
    virtual void processChangesIfRequired( bool joinBorders=false );
    virtual void processChangesIfRequiredKeepCaretCache( bool joinBorders=false );


private:

    TextEditorController* textControllerRef_;         ///< A reference to the controller


};


} // edbee
