/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "tabcommand.h"

#include "edbee/texteditorcontroller.h"
#include "edbee/models/changes/mergablechangegroup.h"
#include "edbee/models/texteditorconfig.h"
#include "edbee/models/textdocument.h"
#include "edbee/views/textselection.h"

#include "edbee/debug.h"

namespace edbee {


/// Constructs the tabcommand
/// @param direction the direction of the tab (Forward or Backward)
/// @param insertTab should we use a tab character?
TabCommand::TabCommand(TabCommand::Direction direction, bool insertTab)
    : dir_( direction )
    , insertTab_( insertTab )
{
}


/// indents or outdents in the given controller
/// @parma controller the controller
void TabCommand::indent( TextEditorController* controller )
{
    TextDocument* doc  = controller->textDocument();
    TextSelection* sel = controller->textSelection();
    int tabSize = doc->config()->indentSize();
    bool useTab = false; //doc->config()->useTabChar();
    QString tab = useTab ? "\t" : QStringLiteral(" ").repeated(tabSize);

    controller->beginUndoGroup( new MergableChangeGroup( controller ) );

    sel->beginChanges();

    int lastLine = -1;
    for( int i=0, cnt = sel->rangeCount(); i<cnt; ++i ) {
        TextRange& range = sel->range(i);

        // calculate the start and endline
        int startLine = doc->lineFromOffset( range.min() );
        int endLine   = doc->lineFromOffset( range.max() );

        // when the last line is blank, we do NOT want to indent it (fixes #61)
        if( startLine!=endLine && doc->columnFromOffsetAndLine(range.max(),endLine) == 0 ) {
            --endLine;
        }

        // make sure we at least indent one line
        if( startLine <= lastLine ) startLine = lastLine + 1;

        // now indent all lines
        for( int line=startLine; line<=endLine; ++line ) {
            int offset = doc->offsetFromLine(line);

            // insert-tab
            if( dir_ == Forward ) {
                doc->replace(offset,0, tab );
                sel->changeSpatial( offset, 0, tab.length(), true );

            // remove tab
            } else {
                int offsetNextLine = doc->offsetFromLine(line+1)-1;
                int endOffset = offset;
                for( int j=0; j<tabSize; ++j ) {
                    int off = offset + j;
                    if(off >= doc->length()) break;
                    QChar chr = doc->charAt(off);

                    if( chr != ' ' && chr != '\t' ) break;
                    if( off >= offsetNextLine ) break;
                    ++endOffset;
                    if( chr == '\t' ) break;
                }
                if( offset != (endOffset-offset)) {
                    doc->replace( offset, endOffset-offset, "" );
                    sel->changeSpatial( offset, endOffset-offset, 0, true );
                }
            }

        }
        lastLine = endLine;
    }

    sel->endChanges();

    controller->endUndoGroup( CoalesceId_Indent + dir_, true );
}


/// TODO: Refactor :)
void TabCommand::execute(TextEditorController* controller)
{
    if( !insertTab_ ) {
        indent(controller);
        return;
    }

    // retrieve the settings
    TextDocument* doc  = controller->textDocument();
    TextSelection* sel = controller->textSelection();
    int tabSize = doc->config()->indentSize();
    bool useTab = doc->config()->useTabChar();
    QString ws  = doc->config()->whitespaces();

    // check if the changes are multi-line
    bool multiLine = false;
    bool atStartOfLine = true;
    for( int i=0, cnt = sel->rangeCount(); i<cnt && !multiLine; ++i ) {
        TextRange& range = sel->range(i);
        multiLine = multiLine || doc->lineFromOffset(range.anchor()) != doc->lineFromOffset(range.caret());

        // no selection? we need to check if we are at the start of the line
        if( !range.hasSelection() && atStartOfLine ) {
            int pos = range.caret();
            int startPos = doc->offsetFromLine( doc->lineFromOffset( pos ) );
            --pos;  // look hat the character before the pos

            //Q_ASSERT(startPos <= pos);
            if( pos >= startPos ) {
                while(pos>startPos && atStartOfLine ) {
                    QChar chr = doc->charAt(pos);
                    if( !ws.contains(chr) ) {
                        atStartOfLine = false;
                    }
                    --pos;
                }
            }

        } else {
            atStartOfLine = false;
        }
    }

    // a Line-moving operation
    if( multiLine || ( atStartOfLine && dir_ == Backward ) ) {
        indent( controller );

    // a character inserting operation
    } else {
        if( dir_ == Forward) {
            if( useTab ) {
                controller->replaceSelection("\t", CoalesceId_Indent + SubCoalesceId_Indent_InsertTab );
            } else {

                // we need to 'calculate' the number of spaces to add (per caret)
                QStringList texts;
                for( int i=0, cnt = sel->rangeCount(); i<cnt; ++i ) {
                    TextRange& range = sel->range(i);
                    int offset = range.min();
                    int col = doc->columnFromOffsetAndLine(offset);

                    // calculate the number of spaces depending on the column
                    int spaces = tabSize - col%tabSize;
                    if( !spaces ) spaces = tabSize;

                    // append the text
                    texts.push_back( QStringLiteral(" ").repeated(spaces) );
                }

                controller->replaceSelection( texts, CoalesceId_Indent + SubCoalesceId_Indent_InsertSpaces  );
            }
        }
    }
}


/// Returs the name of this command
QString TabCommand::toString()
{
    if( dir_ == Backward ) return "TabCommand(back)";
    return "TabCommand(fwd)";
}

} // edbee
