/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"


#include "edbee/texteditorcommand.h"


namespace edbee {

class TextRange;

/// Inserts a newline.
/// When  smarttab is enabled it also inser leading tabs/spaces
class EDBEE_EXPORT NewlineCommand : public TextEditorCommand
{
public:
    enum NewLineType {
        NormalNewline = 0,
        AddLineBefore = 1,
        AddLineAfter = 2
    };


    NewlineCommand( NewLineType method );

    QString calculateSmartIndent( TextEditorController* controller, TextRange& range );

    virtual void executeNormalNewline( TextEditorController* controller );

    virtual void executeSpecialNewline( TextEditorController* controller, bool nextLine );

    virtual void execute( TextEditorController* controller ) override;
    virtual QString toString() override;

private:
    NewLineType newLineType_;       ///< The current newline type
};


} // edbee
