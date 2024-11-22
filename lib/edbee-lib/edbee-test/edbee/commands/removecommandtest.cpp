/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "removecommandtest.h"

#include "edbee/commands/removecommand.h"
#include "edbee/models/chardocument/chartextdocument.h"
#include "edbee/models/texteditorconfig.h"

#include "edbee/debug.h"

namespace edbee {


RemoveCommandTest::RemoveCommandTest()
    : command_(0)
{
}


/// test the remove command operation
void RemoveCommandTest::init()
{
    command_ = new RemoveCommand( RemoveCommand::RemoveChar, RemoveCommand::Left );
}


/// the clean operation
void RemoveCommandTest::clean()
{
    delete command_;
}


/// tests the smart backspace operation
void RemoveCommandTest::testSmartBackspace()
{
    CharTextDocument doc;
    doc.config()->setUseTabChar(false);

    // simple spaces test
    doc.config()->setIndentSize(2);
    //===========0123456789
    doc.setText("       abcd");
    testEqual( command_->smartBackspace(&doc,0), 0 );
    testEqual( command_->smartBackspace(&doc,1), 0 );
    testEqual( command_->smartBackspace(&doc,2), 0 );
    testEqual( command_->smartBackspace(&doc,3), 2 );
    testEqual( command_->smartBackspace(&doc,4), 2 );
    testEqual( command_->smartBackspace(&doc,5), 4 );
    testEqual( command_->smartBackspace(&doc,6), 4 );
    testEqual( command_->smartBackspace(&doc,7), 6 );
    testEqual( command_->smartBackspace(&doc,8), 7 );   // this is the 'a'
    testEqual( command_->smartBackspace(&doc,9), 8 );   // this is the 'b'


    // extra tabs test
    doc.config()->setIndentSize(4);
    doc.setText("\t \t abcd");
    testEqual( command_->smartBackspace(&doc,0), 0 );   // 0 => 0
    testEqual( command_->smartBackspace(&doc,1), 0 );   // "\t" => 0
    testEqual( command_->smartBackspace(&doc,2), 1 );   // "\t " Directly at the first column which is after the tab char
    testEqual( command_->smartBackspace(&doc,3), 1 );   // "\t \t" Directly at the first column which is after the tab char
    testEqual( command_->smartBackspace(&doc,4), 3 );   // "\t \t "
}



} // edbee
