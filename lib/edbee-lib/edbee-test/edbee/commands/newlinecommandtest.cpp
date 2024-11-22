/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "newlinecommandtest.h"

#include "edbee/commands/newlinecommand.h"
#include "edbee/models/textdocument.h"
#include "edbee/models/texteditorconfig.h"
#include "edbee/models/textrange.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/texteditorwidget.h"

#include "edbee/debug.h"

namespace edbee {


NewlineCommandTest::NewlineCommandTest()
    : widget_(0)
    , command_(0)
{
}


/// initializes the test
void NewlineCommandTest::init()
{
    widget_ = new TextEditorWidget();
    command_ = new NewlineCommand( NewlineCommand::NormalNewline);
}


/// cleansup the test
void NewlineCommandTest::clean()
{
    delete command_;
    delete widget_;
}


/// tests the calculate smart indent
void NewlineCommandTest::testCalculateSmartIndent_useSpaces()
{
    // config
    config()->setUseTabChar(false);
    config()->setSmartTab(true);
    config()->setIndentSize(2);

    /// no indent
    doc()->setText("No indent");
    TextRange range(0,0);
    testEqual( command_->calculateSmartIndent( controller(), range ), QString() );

    range.set(4,4);
    testEqual( command_->calculateSmartIndent( controller(), range ), QString() );

    // pressing enter at the start of line should stay on the start of the line
    range.set(0,0);
    doc()->setText("  One Indent");
    testEqual( command_->calculateSmartIndent( controller(), range ), QString() );

    // pressing enter before the start of the indent should stay at the samen column
    range.set(1,1);
    testEqual( command_->calculateSmartIndent( controller(), range ), QStringLiteral(" ") );

    // pressing enter at the indent level should stay at the same level
    range.set(2,2);
    testEqual( command_->calculateSmartIndent( controller(), range ), QStringLiteral(" ").repeated(2) );

    // pressing enter in the middle of the line should indent to the start of the first nonespace
    range.set(4,4);
    testEqual( command_->calculateSmartIndent( controller(), range ), QStringLiteral(" ").repeated(2) );

    // pressing enter at the end of the document should indent to the start of the first nonespace
    range.set( doc()->length(),doc()->length());
    testEqual( command_->calculateSmartIndent( controller(), range ), QStringLiteral(" ").repeated(2) );

    // it also should work correctly if the buffer contains a tab
    config()->setIndentSize(4);
    doc()->setText(" \tTest");      // this is equal to 4 spaces
    range.set(2,2);
    testEqual( command_->calculateSmartIndent( controller(), range ), QStringLiteral(" ").repeated(4) );
    range.set(5,5);
    testEqual( command_->calculateSmartIndent( controller(), range ), QStringLiteral(" ").repeated(4) );


    // make sure multi line (offset > 0 works correctly)
    config()->setIndentSize(4);
    doc()->setText("  Line1\n  Line2");
    range.set(10,10);
    testEqual( command_->calculateSmartIndent( controller(), range ), QStringLiteral(" ").repeated(2) );
}


/// test the smart indent with tabs
void NewlineCommandTest::testCalculateSmartIndent_useTabs()
{
    // config
    config()->setUseTabChar(true);
    config()->setSmartTab(true);
    config()->setIndentSize(2);

    /// no indent
    doc()->setText("No indent");
    TextRange range(0,0);
    testEqual( command_->calculateSmartIndent( controller(), range ), QString() );

    range.set(4,4);
    testEqual( command_->calculateSmartIndent( controller(), range ), QString() );

    // pressing enter at the start of line should stay on the start of the line
    range.set(0,0);
    doc()->setText("  One Indent");
    testEqual( command_->calculateSmartIndent( controller(), range ), QString() );

    // pressing enter before the start of the indent should stay at the samen column
    range.set(1,1);
    testEqual( command_->calculateSmartIndent( controller(), range ), QStringLiteral(" ") );

    // pressing enter at the indent level should stay at the same level
    range.set(2,2);
    testEqual( command_->calculateSmartIndent( controller(), range ), QStringLiteral("\t") );

    // pressing enter in the middle of the line should indent to the start of the first nonespace
    range.set(4,4);
    testEqual( command_->calculateSmartIndent( controller(), range ), QStringLiteral("\t") );

    // pressing enter at the end of the document should indent to the start of the first nonespace
    range.set( doc()->length(),doc()->length());
    testEqual( command_->calculateSmartIndent( controller(), range ), QStringLiteral("\t") );

    // it also should work correctly if the buffer contains a tab
    config()->setIndentSize(4);
    doc()->setText(" \tTest");      // this is equal to 4 spaces
    range.set(2,2);
    testEqual( command_->calculateSmartIndent( controller(), range ), QStringLiteral("\t") );
    range.set(5,5);
    testEqual( command_->calculateSmartIndent( controller(), range ), QStringLiteral("\t") );

    doc()->setText("\t Test");      // this is equal to 5 spaces
    range.set(2,2);
    testEqual( command_->calculateSmartIndent( controller(), range ), QStringLiteral("\t ") );
    range.set(5,5);
    testEqual( command_->calculateSmartIndent( controller(), range ), QStringLiteral("\t ") );

}


/// returns the textdocument
TextDocument* NewlineCommandTest::doc()
{
    return widget_->textDocument();
}


/// Returnst the texteditor configuration
TextEditorConfig* NewlineCommandTest::config()
{
    return widget_->config();
}


/// returns the texteditor controller
TextEditorController* NewlineCommandTest::controller()
{
    return widget_->controller();
}


} // edbee
