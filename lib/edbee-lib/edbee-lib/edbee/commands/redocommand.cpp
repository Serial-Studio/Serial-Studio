/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "redocommand.h"

#include "edbee/texteditorcontroller.h"

#include "edbee/debug.h"

namespace edbee {

RedoCommand::RedoCommand( bool soft )
    : soft_(soft)
{
}

void RedoCommand::execute( TextEditorController* controller)
{
    controller->redo(soft_);
}

QString RedoCommand::toString()
{
    return QStringLiteral("RedoCommand(%1)").arg( soft_ ? "soft" : "hard" );
}

bool RedoCommand::readonly()
{
    return soft_;
}


} // edbee
