/**
 * Copyright 2011-2020 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once
#include "edbee/exports.h"
#include "edbee/texteditorcommand.h"


namespace edbee {

class ToggleReadonlyCommand : public TextEditorCommand
{
public:
    ToggleReadonlyCommand();

    virtual void execute(TextEditorController* controller) override;
    virtual QString toString() override;
    virtual bool readonly() override;

};

} // edbee

