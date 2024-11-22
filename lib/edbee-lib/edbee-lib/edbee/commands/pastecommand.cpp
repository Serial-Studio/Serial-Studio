/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "pastecommand.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QStringList>

#include "edbee/commands/copycommand.h"
#include "edbee/models/changes/mergablechangegroup.h"
#include "edbee/models/textbuffer.h"
#include "edbee/models/change.h"
#include "edbee/models/textdocument.h"
#include "edbee/models/textrange.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/views/textselection.h"

#include "edbee/debug.h"

namespace edbee {


/// Default (blank) constructor
PasteCommand::PasteCommand()
{
}


/// Returns the command id for coalescing this operation
int PasteCommand::commandId()
{
    return CoalesceId_Paste;
}


/// Execute the paste command
/// @param controller the controller context this operation is executed for
void PasteCommand::execute(TextEditorController* controller)
{
    QClipboard* clipboard     = QApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();
    QString text              = clipboard->text();

    TextDocument* doc    = controller->textDocument();
    TextRangeSet* sel    = controller->textSelection();

    // a line-based paste
    if( mimeData && mimeData->hasFormat( CopyCommand::EDBEE_TEXT_TYPE ) ) {
        TextRangeSet newRanges(doc);
        for( int i=0,cnt=sel->rangeCount(); i<cnt; ++i ) {
            TextRange& range = sel->range(i);
            int line   = doc->lineFromOffset( range.min() );
            int offset = doc->offsetFromLine(line);
            newRanges.addRange(offset,offset);
        }

        controller->replaceRangeSet( newRanges, text, commandId() );
        return;

    // normal paste
    } else {
        if( !text.isEmpty() ) {
            int lineCount = text.count("\n") + 1;
            int rangeCount = controller->textSelection()->rangeCount();

            // multi-line/caret copy paste
            if( lineCount == rangeCount ) {
                QStringList texts = text.split("\n");
                controller->replaceRangeSet( *controller->textSelection(), texts, commandId() );

            // this the normal operation
            } else {
                controller->replaceSelection(text,commandId());
            }
        }
        return;
    }
}


/// Converst the command to a string (for fetching the commandname)
QString PasteCommand::toString()
{
    return "PasteCommand";
}


} // edbee
