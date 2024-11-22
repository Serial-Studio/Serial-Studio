/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "replaceselectioncommand.h"

#include "edbee/texteditorcontroller.h"

#include "edbee/debug.h"

namespace edbee {


/// The constructor of the replaceSelection command
/// @param text the text to replace
/// @param coalesceId the coalesceId top merge undo operations
ReplaceSelectionCommand::ReplaceSelectionCommand( const QString& text, int coalesceId )
    : text_(text)
    , coalesceId_( coalesceId )
{
}


/// Executes the replace selection command
/// @param controller the controller context this command runs ons
void ReplaceSelectionCommand::execute(TextEditorController* controller)
{
    controller->replaceSelection( text_ , coalesceId_ );
}


/// Converts the current command to a string
QString ReplaceSelectionCommand::toString()
{
    return QStringLiteral("ReplaceSelectionCommand");
}


} // edbee
