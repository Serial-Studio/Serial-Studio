/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "texteditorcommand.h"

#include "edbee/debug.h"

namespace edbee {


/// The default constructor
TextEditorCommand::TextEditorCommand()
{
}


/// A blank virtual destructor
TextEditorCommand::~TextEditorCommand()
{

}

bool TextEditorCommand::readonly()
{
    return false;
}


} // edbee
