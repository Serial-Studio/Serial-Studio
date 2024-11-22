/**
 * Copyright 2011-2020 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "factorycommandmap.h"

#include "edbee/commands/commentcommand.h"
#include "edbee/commands/copycommand.h"
#include "edbee/commands/cutcommand.h"
#include "edbee/commands/debugcommand.h"
#include "edbee/commands/duplicatecommand.h"
#include "edbee/commands/findcommand.h"
#include "edbee/commands/pastecommand.h"
#include "edbee/commands/movelinecommand.h"
#include "edbee/commands/newlinecommand.h"
#include "edbee/commands/redocommand.h"
#include "edbee/commands/removecommand.h"
#include "edbee/commands/replaceselectioncommand.h"
#include "edbee/commands/selectioncommand.h"
#include "edbee/commands/tabcommand.h"
#include "edbee/commands/togglereadonlycommand.h"
#include "edbee/commands/undocommand.h"
#include "edbee/models/texteditorcommandmap.h"

#include "edbee/debug.h"


namespace edbee {


#define give(key,command) cm->give(key,command)

/// This method adds all factory keys to the texteditor keymap
/// This method does NOT clear the existing items.
/// @param km the command manager
void FactoryCommandMap::fill(TextEditorCommandMap* cm)
{
    give( "goto_next_char", new SelectionCommand( SelectionCommand::MoveCaretsOrDeselect, 1 ) );
    give( "goto_prev_char", new SelectionCommand( SelectionCommand::MoveCaretsOrDeselect, -1 ) );
    give( "goto_next_word", new SelectionCommand( SelectionCommand::MoveCaretByWord, 1 ) );
    give( "goto_prev_word", new SelectionCommand( SelectionCommand::MoveCaretByWord, -1 ) );
    give( "goto_bol", new SelectionCommand( SelectionCommand::MoveCaretToLineBoundary, -1 ) );
    give( "goto_eol", new SelectionCommand( SelectionCommand::MoveCaretToLineBoundary, 1 ) );
    give( "goto_next_line", new SelectionCommand( SelectionCommand::MoveCaretByLine, 1 ) );
    give( "goto_prev_line", new SelectionCommand( SelectionCommand::MoveCaretByLine, -1 ) );
    give( "goto_bof", new SelectionCommand( SelectionCommand::MoveCaretToDocumentBegin ) );
    give( "goto_eof", new SelectionCommand( SelectionCommand::MoveCaretToDocumentEnd ) );
    give( "goto_page_down", new SelectionCommand( SelectionCommand::MoveCaretByPage, 1 ));
    give( "goto_page_up", new SelectionCommand( SelectionCommand::MoveCaretByPage, -1 ));

    // selection
    give( "sel_next_char", new SelectionCommand( SelectionCommand::MoveCaretByCharacter, 1, true ) );
    give( "sel_prev_char", new SelectionCommand( SelectionCommand::MoveCaretByCharacter, -1, true ) );
    give( "sel_next_word", new SelectionCommand( SelectionCommand::MoveCaretByWord, 1, true ) );
    give( "sel_prev_word", new SelectionCommand( SelectionCommand::MoveCaretByWord, -1, true ) );
    give( "sel_to_bol", new SelectionCommand( SelectionCommand::MoveCaretToLineBoundary, -1, true) );
    give( "sel_to_eol", new SelectionCommand( SelectionCommand::MoveCaretToLineBoundary, 1, true ) );
    give( "sel_to_next_line", new SelectionCommand( SelectionCommand::MoveCaretByLine, 1, true ) );
    give( "sel_to_prev_Line", new SelectionCommand( SelectionCommand::MoveCaretByLine, -1, true ) );
    give( "sel_to_bof", new SelectionCommand( SelectionCommand::MoveCaretToDocumentBegin, -1, true ) );
    give( "sel_to_eof", new SelectionCommand( SelectionCommand::MoveCaretToDocumentEnd, 1, true ) );
    give( "sel_page_down", new SelectionCommand( SelectionCommand::MoveCaretByPage, 1, true ));
    give( "sel_page_up", new SelectionCommand( SelectionCommand::MoveCaretByPage, -1, true ));

    give( "sel_all", new SelectionCommand( SelectionCommand::SelectAll ) );
    give( "sel_word", new SelectionCommand( SelectionCommand::SelectWord ));
    give( "sel_line", new SelectionCommand( SelectionCommand::SelectFullLine, 1 ));
    give( "sel_prev_line", new SelectionCommand( SelectionCommand::SelectFullLine, -1 ));
    give( "add_caret_prev_line", new SelectionCommand( SelectionCommand::AddCaretByLine, -1));
    give( "add_caret_next_line", new SelectionCommand( SelectionCommand::AddCaretByLine, 1));
    give( "sel_reset", new SelectionCommand( SelectionCommand::ResetSelection ) );

    // line entry
    give( "ins_newline", new NewlineCommand( NewlineCommand::NormalNewline ) );
    give( "ins_newline_before", new NewlineCommand( NewlineCommand::AddLineBefore ) );
    give( "ins_newline_after", new NewlineCommand( NewlineCommand::AddLineAfter ) );

    // remove text at the left
    give( "del_left", new RemoveCommand( RemoveCommand::RemoveChar, RemoveCommand::Left ) );
    give( "del_word_left", new RemoveCommand( RemoveCommand::RemoveWord, RemoveCommand::Left ) );
    give( "del_line_left", new RemoveCommand( RemoveCommand::RemoveLine, RemoveCommand::Left ) );

    // remove text at the rigth
    give( "del_right", new RemoveCommand( RemoveCommand::RemoveChar, RemoveCommand::Right ) );
    give( "del_word_right", new RemoveCommand( RemoveCommand::RemoveWord, RemoveCommand::Right ) );
    give( "del_line_right", new RemoveCommand( RemoveCommand::RemoveLine, RemoveCommand::Right ) );

    // special text entry commands
    give( "duplicate", new DuplicateCommand() );
    give( "toggle_comment", new CommentCommand( false) );
    give( "toggle_block_comment", new CommentCommand( true) );

    // tab entry
    //give( "tab", new ReplaceSelectionCommand( "\t", CoalesceId_InsertTab ));
    /// TODO: add a backtab action here
    //set( QKeySequence( Qt::Key_BackTab ), new )
    give( "tab", new TabCommand( TabCommand::Forward, true ));
    give( "tab_back", new TabCommand( TabCommand::Backward, true ));

    give( "indent", new TabCommand( TabCommand::Forward, false));
    give( "outdent", new TabCommand( TabCommand::Backward, false ));

    // undo / redo comamnds
    give( "undo", new UndoCommand() );
    give( "redo", new RedoCommand() );
    give( "soft_undo", new UndoCommand(true) );
    give( "soft_redo", new RedoCommand(true) );

    // clipboard operations
    give( "copy", new CopyCommand( ) );
    give( "cut", new CutCommand() );
    give( "paste", new PasteCommand() );

    // debug commands
    give( "debug_dump_scopes", new DebugCommand( DebugCommand::DumpScopes ) );
    give( "debug_rebuild_scopes", new DebugCommand( DebugCommand::RebuildScopes ) );
    give( "debug_dump_undo_stack", new DebugCommand( DebugCommand::DumpUndoStack ) );
    give( "debug_dump_character_codes", new DebugCommand( DebugCommand::DumpCharacterCodes ) );

    // find items
    give( "find_use_sel", new FindCommand( FindCommand::UseSelectionForFind ) );
    give( "find_next_match", new FindCommand( FindCommand::FindNextMatch) );
    give( "find_prev_match", new FindCommand( FindCommand::FindPreviousMatch) );
    give( "sel_next_match", new FindCommand( FindCommand::SelectNextMatch ));
    give( "sel_prev_match", new FindCommand( FindCommand::SelectPreviousMatch));
    give( "sel_all_matches", new FindCommand( FindCommand::SelectAllMatches ));

    give( "select_under_expand", new FindCommand( FindCommand::SelectUnderExpand ));
    give( "select_all_under", new FindCommand( FindCommand::SelectAllUnder ));

    give( "move_lines_up", new MoveLineCommand(-1));
    give( "move_lines_down", new MoveLineCommand(1));

    give( "toggle_readonly", new ToggleReadonlyCommand());
}


}
