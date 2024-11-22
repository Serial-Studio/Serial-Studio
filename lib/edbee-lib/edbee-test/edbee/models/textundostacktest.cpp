/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textundostacktest.h"

#include "edbee/commands/removecommand.h"
#include "edbee/models/change.h"
#include "edbee/models/textdocument.h"
#include "edbee/models/textrange.h"
#include "edbee/models/textundostack.h"
#include "edbee/views/textselection.h"
#include "edbee/texteditorwidget.h"
#include "edbee/texteditorcontroller.h"

#include "edbee/debug.h"

namespace edbee {

///  The following problem occures
///
///    1a|2b|3c4d
///
///  Press delete 3 times
///
///    1a|b|c4d                (1x)
///    1a||4d     =>  1a|4d    (2x)
///    1a|d                    (3x)
///    1a|                     (4x) (extra check)
///

void TextUndoStackTest::testMultiCaretUndoIssue196()
{
    TextEditorWidget widget;
    TextDocument* doc = widget.textDocument();
    TextEditorController* controller = widget.controller();
//    TextUndoStack* undoStack = doc->textUndoStack();

    controller->replace(0,0,"1a2b3c4d",0);                  // 1a|2b|3c4d
    controller->moveCaretToOffset(2,false);
    controller->addCaretAtOffset(4);

    testEqual( doc->text(), "1a2b3c4d" );
    testEqual( controller->textSelection()->rangesAsString(), "2>2,4>4");

    RemoveCommand del( RemoveCommand::RemoveChar, RemoveCommand::Right );

    del.execute(controller);
    testEqual( doc->text(), "1abc4d" );                          // 1a|b|c4d
    testEqual( controller->textSelection()->rangesAsString(), "2>2,3>3");

    del.execute(controller);
    testEqual( controller->textSelection()->rangesAsString(), "2>2");
    testEqual( doc->text(), "1a4d" );                       // 1a||4d



    del.execute(controller);
    testEqual( doc->text(), "1ad" );
    testEqual( controller->textSelection()->rangesAsString(), "2>2");

    del.execute(controller);
    testEqual( doc->text(), "1a" );
    testEqual( controller->textSelection()->rangesAsString(), "2>2");

    del.execute(controller);
    testEqual( doc->text(), "1a" );
    testEqual( controller->textSelection()->rangesAsString(), "2>2");

//qlog_info() << "STACK: ---------------------------------------";
//qlog_info() << doc->textUndoStack()->dumpStack();
//qlog_info() << "----------------------------------------------";

    controller->undo();

    testEqual( doc->text(), "1a2b3c4d" );
    testEqual( controller->textSelection()->rangesAsString(), "2>2,4>4");


/*

==== after 1 delete ===

  1a|2b|3c4d =>  1a|b|c4d


 UndoStack
 =====================
 "-|Complex::TextChangeGroup(3/3)
 - 0: SelectionTextChange
 - 1: SingleTextChange:2:0:2
 - 2: SingleTextChange:3:0:3

==== after 2 deletes ===

  1a|b|c4d =>  1a|4d


UndoStack
=====================
"-|Complex::TextChangeGroup(3/3)
 - 0: SelectionTextChange
 - 1: SingleTextChange:2:0:2b         (2b en 3c is verwijderd... Dit is nog goed!)
 - 2: SingleTextChange:2:0:3c

 ==== after 3 deletes ===

  1a|4d =>  1a|d

  // NEW STACK ITEM REQUIRED!!!!

 UndoStack
 =====================
 "-|Complex::TextChangeGroup(3/3)
 - 0: SelectionTextChange
 - 1: SingleTextChange:2:0:2b4
 - 2: SingleTextChange:1:0:3c          <= hier zou de 4 achter moeten staan!!! (Dit is nooit te bepalen, omdat je niet weet welke carets verdwenen zijn)

*/


}

void TextUndoStackTest::testClearUndoStackCrashIssue24()
{
    TextEditorWidget widget;
    TextDocument* doc = widget.textDocument();
    TextEditorController* controller = widget.controller();

    controller->replace(0,0,"1a2b3c4d",0);
    testEqual(doc->text(),"1a2b3c4d");

    // clear the undo stack
    doc->textUndoStack()->clear();
    testEqual(doc->text(),"1a2b3c4d");

    // move the caret (this seems to crash the undostack)
    controller->moveCaretTo(0,4,false);
}


void TextUndoStackTest::testClearUndoStackShouldnotUnregisterTheControllerIssue24()
{
    TextEditorWidget widget;
    TextDocument* doc = widget.textDocument();
    TextEditorController* controller = widget.controller();

    testTrue( doc->textUndoStack()->isControllerRegistered(controller));

    // clearing the undo stack should NOT unregister a controller
    doc->textUndoStack()->clear();

    testTrue( doc->textUndoStack()->isControllerRegistered(controller));
}


} // edbee
