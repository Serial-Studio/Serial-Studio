#pragma once

#include "edbee/exports.h"

#include "edbee/texteditorcommand.h"

namespace edbee {

/// moves
class EDBEE_EXPORT MoveLineCommand : public TextEditorCommand
{
public:
    MoveLineCommand( int direction );
    virtual ~MoveLineCommand();

    virtual void execute( TextEditorController* controller ) override;
    virtual QString toString() override;

private:
    int direction_;
};

}
