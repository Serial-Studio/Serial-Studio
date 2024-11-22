/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "duplicatecommandtest.h"

#include "edbee/models/textdocument.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/texteditorwidget.h"
#include "edbee/views/textselection.h"

#include "edbee/debug.h"

namespace edbee {


DuplicateCommandTest::DuplicateCommandTest()
    : widget_(0)
{
}

/// Initialization for every test case
void DuplicateCommandTest::init()
{
    widget_ = new TextEditorWidget();
}


/// cleanup the testcase
void DuplicateCommandTest::clean()
{
    delete widget_;
}


/// tests the line duplication for the last line (a line without newline)
void DuplicateCommandTest::testLastLineDuplication()
{
    // construct the initial situation
    doc()->setText("duplication test");
    sel()->setRange(1,1);

    // test the initial situation
    testEqual( doc()->text(),"duplication test" );
    testEqual( sel()->rangesAsString(), "1>1" );

    // execute the duplication command
    ctrl()->executeCommand("duplicate");
    testEqual( doc()->text(), "duplication test\nduplication test" );
    testEqual( sel()->rangesAsString(), "18>18" );
}


/// test the duplicate selection operation
void DuplicateCommandTest::testDuplicateSelection()
{
    // construct the initial situation
    doc()->setText("abcdef");                       // a[bcd]ef
    sel()->setRange(1,4);

    // test the initial situation
    testEqual( sel()->rangesAsString(), "1>4" );

    // execute the duplication command
    ctrl()->executeCommand("duplicate");
    testEqual( doc()->text(), "abcdbcdef" );       // abcd[bcd]ef
    testEqual( sel()->rangesAsString(), "4>7" );
}


/// A special case is the duplication with multiple carets on the same line
/// Tests: a|bc[d]ef => abcdef#a|bcd[d]ef
void DuplicateCommandTest::testDuplicateMultipleCaretsOnSingleLine1()
{
    doc()->setText("abcdef");                       // a|bc[d]ef
    sel()->setRange(1,1);
    sel()->addRange(3,4);

    // execute the duplication command
    // a|bc[d]ef => abcdef#a|bcd[d]ef
    ctrl()->executeCommand("duplicate");
    testEqual( doc()->text(), "abcdef\nabcddef" );
    testEqual( sel()->rangesAsString(), "8>8,11>12");

}


/// This method test duplication with 2 line-duplication carets on a single line
/// Tests: a[b]c[d]ef => ab[b]cd[d]ef => abb[b]cdd[d]ef
void DuplicateCommandTest::testDuplicateMultipleCaretsOnSingleLine2()
{
    doc()->setText("abcdef");
    sel()->setRange(1,2);
    sel()->addRange(3,4);

    // execute the duplication command
    // a[b]c[d]ef => ab[b]cd[d]ef => abb[b]cdd[d]ef
    ctrl()->executeCommand("duplicate");

    testEqual( doc()->text(), "abbcddef" );
    testEqual( sel()->rangesAsString(), "2>3,5>6");


    // a[b]c[d]ef => ab[b]cd[d]ef => abb[b]cdd[d]ef
    ctrl()->executeCommand("duplicate");

    testEqual( doc()->text(), "abbbcdddef" );
    testEqual( sel()->rangesAsString(), "3>4,7>8");

}


/// This method test duplication with 2 line-duplication carets on a single line
/// Tests: a|bc|def
void DuplicateCommandTest::testDuplicateMultipleCaretsOnSingleLine3()
{
    doc()->setText("abcdef");
    sel()->setRange(1,1);
    sel()->addRange(3,3);

    // execute the duplication command
    // a|bc|def => abcdef#abcdef#a|bc|def
    ctrl()->executeCommand("duplicate");
    testEqual( doc()->text(), "abcdef\nabcdef\nabcdef" );
    testEqual( sel()->rangesAsString(), "15>15,17>17");


    // make sure the second time it also works (This will crash when removing hack #73 from the duplicate command)
    ctrl()->executeCommand("duplicate");
    testEqual( doc()->text(), "abcdef\nabcdef\nabcdef\nabcdef\nabcdef" );
    testEqual( sel()->rangesAsString(), "29>29,31>31");
}


/// Returns the widget
TextEditorWidget* DuplicateCommandTest::widget() const
{
    return widget_;
}


/// returns the controller
TextEditorController* DuplicateCommandTest::ctrl() const
{
    return widget_->controller();
}


/// returns the document
TextDocument*DuplicateCommandTest::doc() const
{
    return widget_->textDocument();
}


/// returns the text selection
TextSelection*DuplicateCommandTest::sel() const
{
    return widget_->textSelection();
}


} // edbee
