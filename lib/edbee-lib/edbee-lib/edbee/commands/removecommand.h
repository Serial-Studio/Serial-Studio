/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include "edbee/texteditorcommand.h"

namespace edbee {

class TextDocument;
class TextRangeSet;

/// A delete command. This is a backspace or a delete operation
///
/// The delete behaviour is pretty obvious, it just deletes :)
///
/// The behaviour of the RemoveCharLeft depends on several factors:
///
/// When the editor has enabled useTabChar_is enabled, backspace is very simple, it simply
/// deletes the previous character or the selection
///
/// When spaces are used for tabs the behaviour is different. If the caret is left of
/// the first non-space character, it will move 1 column to the left, depending on the tabsize
///
class EDBEE_EXPORT RemoveCommand : public TextEditorCommand
{
public:
    enum RemoveMode {
        RemoveChar,                     ///< Remove a single character
        RemoveWord,                     ///< Remove a single word
        RemoveLine                      ///< Remove a line
    };

    enum Direction {
        Left,                           ///< Remove the item to the left
        Right                           ///< Remove the item to ther right
    };


    RemoveCommand( int removeMode, int direction );

    int coalesceId() const;
    int smartBackspace( TextDocument* doc, int caret );

    void rangesForRemoveChar( TextEditorController* controller, TextRangeSet* ranges );
    void rangesForRemoveWord( TextEditorController* controller, TextRangeSet* ranges );
    void rangesForRemoveLine( TextEditorController* controller, TextRangeSet* ranges );

    virtual void execute( TextEditorController* controller ) override;
    virtual QString toString() override;

private:
    int directionSign() const;


    int removeMode_;        ///< The remove mode
    int direction_;         ///< The remove direction
};


} // edbee
