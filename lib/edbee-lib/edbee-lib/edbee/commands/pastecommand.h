/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include "edbee/texteditorcommand.h"

namespace edbee {

class TextEditorController;

/// Executes a paste command
/// This usually simply pastes the text at the given location.
/// There are two special cases:
/// - there's a line cutting/pasing mode to copy complete lines
/// - when the number of carets is equal to the number of lines, every line is pasted at a caret
class EDBEE_EXPORT PasteCommand : public TextEditorCommand
{
public:
    PasteCommand();

    virtual int commandId();

    virtual void execute( TextEditorController* controller ) override;
    virtual QString toString() override;
};

} // edbee
