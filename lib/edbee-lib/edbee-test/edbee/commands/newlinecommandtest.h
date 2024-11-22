/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/util/test.h"

namespace edbee {

class TextDocument;
class NewlineCommand;
class TextEditorController;
class TextEditorConfig;
class TextEditorWidget;


/// test the newline command
class NewlineCommandTest : public edbee::test::TestCase
{
    Q_OBJECT
public:
    NewlineCommandTest();

private slots:

    void init();
    void clean();
    void testCalculateSmartIndent_useSpaces();
    void testCalculateSmartIndent_useTabs();

private:
    TextDocument* doc();
    TextEditorConfig* config();
    TextEditorController* controller();

    TextEditorWidget* widget_;
    NewlineCommand* command_;
};


} // edbee

DECLARE_TEST(edbee::NewlineCommandTest);

