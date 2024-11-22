/**
 * Copyright 2011-2014 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "factorykeymap.h"

#include "edbee/models/texteditorkeymap.h"

#include "edbee/debug.h"


namespace edbee {

#define add(key,value) km->add(key,value)

/// This method adds all factory keys to the texteditor keymap
/// This method does NOT clear the existing items.
/// @param km the command manager
void FactoryKeyMap::fill( TextEditorKeyMap* km )
{
    // add(  command,  keyname/sequence )
    add( "goto_next_char", "move_to_next_char" );
    add( "goto_prev_char", "move_to_previous_char" );
    add( "goto_next_word", "move_to_next_word" );
    add( "goto_prev_word", "move_to_previous_word" );
    add( "goto_bol", "move_to_start_of_line" );
    add( "goto_bol", "move_to_start_of_block");
    add( "goto_eol", "move_to_end_of_line" );
    add( "goto_eol", "move_to_end_of_block");
    add( "goto_next_line", "move_to_next_line" );
    add( "goto_prev_line", "move_to_previous_line" );
    add( "goto_bof", "move_to_start_of_document" );
    add( "goto_eof", "move_to_end_of_document" );
    add( "goto_page_down", "move_to_next_page" );
    add( "goto_page_up", "move_to_previous_page" );

    // selection
    add( "sel_next_char", "select_next_char" );
    add( "sel_prev_char", "select_previous_char" );
    add( "sel_next_word", "select_next_word" );
    add( "sel_prev_word", "select_previous_word" );
    add( "sel_to_bol", "select_start_of_line" );
    add( "sel_to_eol", "select_end_of_line" );
    add( "sel_to_next_line", "select_next_line" );
    add( "sel_to_prev_Line", "select_previous_line" );
    add( "sel_to_bof", "select_start_of_document" );
    add( "sel_to_eof", "select_end_of_document" );
    add( "sel_page_down", "select_next_page" );
    add( "sel_page_up", "select_previous_page" );

    add( "sel_all", "select_all" );
    //add("sel_word", "Ctrl+D" );	// is superseeded by 'find_under_expand'
    add( "sel_line", "Ctrl+L" );
    add( "sel_prev_line", "Ctrl+Shift+L" );
    add( "add_caret_prev_line", "Meta+shift+Up" );
    add( "add_caret_next_line", "Meta+shift+Down" );
    add( "sel_reset", "Escape" );

    // TODO: We need to build in support for alternative keymaps per platform
    add( "add_caret_prev_line", "Ctrl+Alt+Up" );
    add( "add_caret_next_line", "Ctrl+Alt+Down" );

    // line entry
    add( "ins_newline", "Enter" );
    add( "ins_newline", "Return" );
    add( "ins_newline", "Shift+Enter" );
    add( "ins_newline", "Shift+Return" );
    add( "ins_newline_before", "Ctrl+Shift+Return" );
    add( "ins_newline_before", "Ctrl+Shift+Enter" );
    add( "ins_newline_after", "Ctrl+Enter" );
    add( "ins_newline_after", "Ctrl+Return" );

    // deletion left
    add( "del_left", "Backspace" );
    add( "del_left", "Shift+Backspace" );
    add( "del_word_left", "Alt+Backspace" );
    add( "del_line_left", "Ctrl+Backspace" );

    // deletion right
    add( "del_right", "Delete" );
    add( "del_word_right", "Alt+Delete" );
    add( "del_line_right", "Ctrl+Delete" );

    // tab entry
    add( "tab", "Tab" );
//    add( "tab_back", "Backtab" );  (Workaround for issue #72)
    add( "tab_back", "Shift+Tab" );
    add( "tab_back", "Shift+Backtab" );
    add( "indent", "Ctrl+]" );
    add( "outdent", "Ctrl+[" );

    // special entry
    add( "duplicate", "Ctrl+Shift+D" );
    add( "toggle_comment", "Ctrl+/" );
    add( "toggle_block_comment", "Ctrl+Meta+/" );

    // undo / redo commands
    add( "undo", "undo" );
    add( "redo", "redo" );
    add( "soft_undo", "Ctrl+U" );
    add( "soft_redo", "Ctrl+Shift+U" );

    // clipboard operations
    add( "copy", "Ctrl+Insert" );
    add( "cut", "Shift+Delete" );
    add( "paste", "Shift+Insert" );
    
    add( "copy", "copy" );
    add( "cut", "cut" );
    add( "paste", "paste" );

    // debug commands
    add( "debug_dump_scopes", "Ctrl+Shift+X,S" );
    add( "debug_rebuild_scopes", "Ctrl+Shift+X,R" );
    add( "debug_dump_undo_stack", "Ctrl+Shift+X,U" );
    add( "debug_dump_character_codes", "Ctrl+Shift+X,C" );

    // find commands
    add( "find_use_sel", "Ctrl+E" );
    add( "find_next_match", "find_next" );
    add( "find_prev_match", "find_previous" );
    add( "sel_next_match", "Meta+S" );
    add( "sel_prev_match", "Meta+Shift+S" );
    add( "sel_all_matches", "Ctrl+Shift+Meta+A" );
    add( "select_under_expand", "Ctrl+D" );
    add( "select_all_under", "Alt+F3" );

    // move lines
    add( "move_lines_up", "Ctrl+Meta+Up");
    add( "move_lines_down", "Ctrl+Meta+Down");

    // toggle readonly
    add( "toggle_readonly", "Ctrl+R");

}


} // edbee
