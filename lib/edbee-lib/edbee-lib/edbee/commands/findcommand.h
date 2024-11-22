/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include "edbee/texteditorcommand.h"

namespace edbee {

class EDBEE_EXPORT FindCommand : public TextEditorCommand
{
public:

    enum FindType {
        UseSelectionForFind,        //< uses the current selection for find
        FindNextMatch,              //< finds the next match
        FindPreviousMatch,          //< finds the previous match
        SelectNextMatch,            //< adds the next match to the selection
        SelectPreviousMatch,        //< adds the previous match to the selection
        SelectAllMatches,           //< select all matches

        SelectUnderExpand,          ///< A smart selection: when there's no selection the current word is used and selected, else the next word is selected
        SelectAllUnder              ///< A smart selection: when there's no selection the current word is used, and all occurences in the document are selected

    };


    FindCommand( FindType findType );

    virtual void execute( TextEditorController* controller ) override;
    virtual QString toString() override;
    virtual bool readonly() override;

private:



    FindType findType_;     ///< the current find-type

};

} // edbee
