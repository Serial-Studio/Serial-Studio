/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QString>

namespace edbee {

class TextEditorController;

/// Predefined coalesce/command ids
///
/// Coalescing is the merging of several undoable operations.
/// Only commands with the same ID will be merged
///
/// Every commands has got 16 bits to add subtype information (and generate another unique-number this way)
enum CoalesceId {
    CoalesceId_ForceMerge    = -1,           ///< FORCE coalescing
    CoalesceId_None          = 0,            ///< NO coalescing

    CoalesceId_Selection     = 1 << 16,      ///< Selection coalescing  (we use 2 bytes for sub-types)

    CoalesceId_TextChanges   = 2 << 16,      ///< A textchange
    CoalesceId_AppendChar    = 3 << 16,      ///< Appending of a character

    CoalesceId_Remove        = 4 << 16,      ///< Removing of text

    CoalesceId_InsertNewLine = 6 << 16,
    CoalesceId_Indent        = 7 << 16,
    CoalesceId_CutLine       = 8 << 16,
    CoalesceId_Paste         = 9 << 16,
    CoalesceId_Duplicate     = 10 << 16,
    CoalesceId_MoveLine      = 11 << 16,

    CoalesceId_UserDefined   = 100 << 16
};



/// This is the abstract base class for a textcommand
///
/// A textcommand is an object/class that's used for executing an operation
/// Though the name is 'command' these commands are NOT used on the undo stack. The undostack makes use of TextChange objects!
///
/// These commands can be placed in the TextEditorCommandMap with a given name
/// By linking this command-name in the Keymap you can register keyboard operations this way
class EDBEE_EXPORT TextEditorCommand {
public:
    TextEditorCommand();
    virtual ~TextEditorCommand();

    /// This method should execute the given command
    virtual void execute( TextEditorController* controller ) = 0;

    /// should return the description of the command
    virtual QString toString() = 0;

    /// Tis method should return true if it's executable in readonly mode
    virtual bool readonly();
};

} // edbee
