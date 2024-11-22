/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QString>
#include "edbee/texteditorcommand.h"

namespace edbee {

class EDBEE_EXPORT RedoCommand : public TextEditorCommand
{
public:
    RedoCommand(bool soft=false);

    /// This method should return the command identifier
    virtual int commandId() { return CoalesceId_None; }

    virtual void execute(TextEditorController* controller) override;
    virtual QString toString() override;
    virtual bool readonly() override;

private:
    bool soft_;
};

} // edbee
