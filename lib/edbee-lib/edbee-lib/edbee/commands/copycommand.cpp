/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "copycommand.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>

#include "edbee/texteditorcontroller.h"
#include "edbee/views/textselection.h"

#include "edbee/debug.h"


namespace edbee {

/// An internal mime-type, used for identifying special copy/paste operations of edbee
const QString CopyCommand::EDBEE_TEXT_TYPE("application/vnd.edbee.text-type");


/// Copies the current selection to the clipboard
void CopyCommand::execute(TextEditorController* controller)
{
    QClipboard* clipboard = QApplication::clipboard();
    TextSelection* sel = controller->textSelection();

    // get the selected text
    QString str = sel->getSelectedText();
    if( !str.isEmpty() ) {
        clipboard->setText( str );

    // perform a full-lines copy.. The full line of the caret's position is copied
    } else {

        str = sel->getSelectedTextExpandedToFullLines();

        // we don't want to detect a memory leak because Qt is going to delete our mime data
        edbee::pause_memleak_detection(true);
        QMimeData* mimeData = new QMimeData();
        edbee::pause_memleak_detection(false);
        mimeData->setText( str );
        mimeData->setData( EDBEE_TEXT_TYPE, "line" );
        clipboard->setMimeData( mimeData );
//        delete mimeData;
    }
}


/// Convers this command to string
QString CopyCommand::toString()
{
    return "CopyCommand";
}

bool CopyCommand::readonly()
{
    return true;
}

} // edbee
