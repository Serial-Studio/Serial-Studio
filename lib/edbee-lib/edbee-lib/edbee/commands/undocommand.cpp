/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "undocommand.h"

#include "edbee/texteditorcontroller.h"

#include "edbee/debug.h"

namespace edbee {

UndoCommand::UndoCommand(bool soft)
    : soft_(soft)
{
}

void UndoCommand::execute( TextEditorController* controller)
{
    controller->undo(soft_);
}

QString UndoCommand::toString()
{
    return QStringLiteral("UndoCommand(%1)").arg( soft_ ? "soft" : "hard" );
}

bool UndoCommand::readonly()
{
    return soft_;
}


} // edbee
