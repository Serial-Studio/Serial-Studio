/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "removecommand.h"

#include "edbee/models/changes/mergablechangegroup.h"
#include "edbee/models/textdocument.h"
#include "edbee/models/texteditorconfig.h"
#include "edbee/models/textrange.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/views/textselection.h"
#include "edbee/util/util.h"

#include "edbee/debug.h"


namespace edbee {


/// The remove command constructor
/// @param removeMode the mode for removal
/// @param direction the direction for the removal
RemoveCommand::RemoveCommand( int removeMode, int direction  )
    : removeMode_( removeMode )
    , direction_( direction )
{
}


/// This method returns the coalesceId to use
/// Currently all commands in the same direction will get the same coalesceId
/// @return the coalesceId to use
int RemoveCommand::coalesceId() const
{
    return CoalesceId_Remove + ( direction_ == Left ? 1 : 2 );
}


/// Performs a smart backspace by adjusting the textrange so the backspace leads to the start of a column
///
/// @param controller to perform the smartbackspace for
/// @param caret the current caret position
///
/// @return the new caret position
int RemoveCommand::smartBackspace(TextDocument* doc, int caret )
{
    TextBuffer* buf = doc->buffer();
    TextEditorConfig* config = doc->config();

    // find the line column position
    int line = doc->lineFromOffset( caret );
    int lineStartOffset = doc->offsetFromLine(line);
    int lineEndOffset = qMin( caret,doc->offsetFromLine(line+1)-1 );

    // searches for the start of the current line
    int firstNoneWhitespaceCharPos = buf->findCharPosWithinRangeOrClamp( lineStartOffset, 1, config->whitespaceWithoutNewline(), false, lineStartOffset, lineEndOffset );

    // only when the caret if before a characer
    if( caret <= firstNoneWhitespaceCharPos && lineStartOffset < firstNoneWhitespaceCharPos ) {

        // retrieve the whitespace-start-part of the line
        QString linePrefix = doc->textPart( lineStartOffset, firstNoneWhitespaceCharPos-lineStartOffset );
        QList<int> lineColumnOffsets = Util().tabColumnOffsets( linePrefix, config->indentSize() );


        // when we're exactly at a columnOffset, we need to get the previous
        int lastColumnOffset = lineColumnOffsets.last() + lineStartOffset;
        if( lastColumnOffset == caret) {
            lastColumnOffset = lineStartOffset + ( lineColumnOffsets.size() > 1 ?  lineColumnOffsets.at( lineColumnOffsets.size()-2 ): 0 );
        }

        // we need to got the given number of characters
        return qMax( 0, caret - ( caret - lastColumnOffset ) );

    }
    return qMax( 0, caret-1 );
}


/// Changes the ranges so one character on the left is removed
/// This method can switch to smart-backspace mode, so backspace moves to the previous column at the start of the line
///
/// @param controller the active controller
/// @param ranges (in/out) the ranges to modify for deletion
void RemoveCommand::rangesForRemoveChar(TextEditorController* controller, TextRangeSet* ranges)
{
    // there's already a selection, just delete that one
    if( ranges->hasSelection() ) { return; }

    // when there isn't a selection we need to 'smart-move' the caret

    // when a tab char is used (or we're deleting right) the operation is pretty simple, just delete the character on the left
    if( direction_ == Right || controller->textDocument()->config()->useTabChar() ) {
        ranges->moveCarets( directionSign() );

    // in the case of spaces, we need to some smart stuff :)
    } else {
        for( int i=0,cnt=ranges->rangeCount(); i<cnt; ++i ) {
            TextRange& range = ranges->range(i);
            range.minVar() = smartBackspace( controller->textDocument(), range.min() );
        }
    }
}


/// Changes the ranges so one word at the left is removed
///
/// @param controller the active controller
/// @param ranges (in/out) the ranges to modify so it spans a word
void RemoveCommand::rangesForRemoveWord(TextEditorController* controller, TextRangeSet* ranges)
{
    // there's already a selection, just delete that one
    if( ranges->hasSelection() ) { return; }

    TextEditorConfig* config = controller->textDocument()->config();
    ranges->moveCaretsByCharGroup( directionSign(), config->whitespaceWithoutNewline(), config->charGroups() );
}


/// Changes the ranges so one line at the left is removed
///
/// @param controller the active controller
/// @param ranges (in/out) the ranges to modify so it spans a line
void RemoveCommand::rangesForRemoveLine(TextEditorController* controller, TextRangeSet* ranges)
{
    TextDocument* doc = controller->textDocument();

    // process all carets
    int offset = direction_ == Left ? -1 : 0;
    for( int rangeIdx=ranges->rangeCount()-1; rangeIdx >= 0; --rangeIdx ) {
        TextRange& range = ranges->range(rangeIdx);
        QChar chr = doc->charAtOrNull( range.caret() + offset );
        if( chr == '\n' ) {
            range.moveCaret(doc,directionSign());
        } else {            
            range.moveCaretToLineBoundary(doc, directionSign(), controller->textDocument()->config()->whitespaceWithoutNewline());
        }
    }


}


/// Performs the remove command
///
/// @param controller the active controller
void RemoveCommand::execute(TextEditorController* controller)
{
    TextSelection* sel = controller->textSelection();
    TextRangeSet* ranges = new TextRangeSet( *sel );

    int coalesceId = this->coalesceId();

    // depending on the delete mode we need to expand the selection
    ranges->beginChanges();
    switch( removeMode_ ) {
        case RemoveChar:
            rangesForRemoveChar( controller, ranges );
            break;
        case RemoveWord:
            rangesForRemoveWord( controller, ranges );
            break;
        case RemoveLine:
            rangesForRemoveLine( controller, ranges );
            break;
        default:
            Q_ASSERT(false);
    }
    ranges->endChanges();

    // when there still isn't a selection, simply delete/ignore this command
    if( !ranges->hasSelection() ) {
        delete ranges;
        return;
    }

    // use the simple replacerangeset function
    TextDocument* doc = controller->textDocument();
    doc->beginChanges( controller );
    doc->replaceRangeSet( *ranges, "" );
    doc->giveSelection( controller, ranges );
    doc->endChanges(coalesceId);
    emit controller->backspacePressed();
}


/// Converts the command to a string
QString RemoveCommand::toString()
{
    // build the mode string
    QString mode;
    switch( removeMode_ ) {
        case RemoveChar: mode = "Char"; break;
        case RemoveWord: mode = "Word"; break;
        case RemoveLine: mode = "Line"; break;
        default: mode = "Unkown!";
    }
    // build the direction string
    QString dir = direction_ == Left ? "Left" : "Right";

    // next returnt he string
    return QStringLiteral("RemoveCommand(%1,%2)").arg(mode).arg(dir);
}


/// This method returns the direction sign.
/// (It seems negative enumeration-values are not allowed on all compilers: http://stackoverflow.com/questions/159034/are-c-enums-signed-or-unsigned )
/// @return -1 if left direction, 1 on the right direction.
int RemoveCommand::directionSign() const
{
    return direction_ == Left ? -1 : 1;
}


} // edbee
