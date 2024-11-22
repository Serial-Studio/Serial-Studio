/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "cutcommand.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>

#include "edbee/commands/copycommand.h"

#include "edbee/models/textdocument.h"
#include "edbee/models/change.h"
#include "edbee/models/textrange.h"
#include "edbee/models/textundostack.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/views/textselection.h"

#include "edbee/debug.h"

namespace edbee {


/// Performs the cut command
/// @param controller the controller context
void CutCommand::execute(TextEditorController* controller)
{
    QClipboard *clipboard = QApplication::clipboard();
    TextRangeSet* sel = controller->textSelection();

    // get the selected text
    QString str = sel->getSelectedText();
    if( !str.isEmpty() ) {
        clipboard->setText( str );
        controller->replaceSelection( "", 0);
        return;

    // perform a full-lines cut
    } else {

        // fetch the selected lines
        TextRangeSet newSel( *sel );
        newSel.expandToFullLines(1);
        str = newSel.getSelectedText();

        // we only coalesce if 1 range is available
        int coalesceId = ( sel->rangeCount() != 1) ? 0 : CoalesceId_CutLine;

        // when the previous command was a cut
        // and there's a line on the stack, we need to expand the line
        if( controller->textDocument()->textUndoStack()->lastCoalesceIdAtCurrentLevel() == CoalesceId_CutLine ) {
            QClipboard* clipboard     = QApplication::clipboard();
            const QMimeData* mimeData = clipboard->mimeData();
            if( mimeData->hasFormat( CopyCommand::EDBEE_TEXT_TYPE ) ) {
                str = mimeData->text() + str;
            }
        }

        // set the new clipboard data
        QMimeData* mimeData = new QMimeData();
        mimeData->setText( str );
        mimeData->setData( CopyCommand::EDBEE_TEXT_TYPE, "line" );
        clipboard->setMimeData( mimeData );
        //delete mimeData;

        // remove the selection
        controller->replaceRangeSet( newSel, "", coalesceId );
        return;
    }
}


/// Converts this command to a string
QString CutCommand::toString()
{
    return "CutCommand";
}

} // edbee
