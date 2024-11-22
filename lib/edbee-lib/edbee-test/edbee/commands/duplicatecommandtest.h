/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/util/test.h"

namespace edbee {

class TextEditorWidget;
class TextEditorController;
class TextDocument;
class TextSelection;


/// Tests the duplication command
class DuplicateCommandTest : public edbee::test::TestCase
{
    Q_OBJECT
public:
    DuplicateCommandTest();

private slots:

    void init();
    void clean();

    void testLastLineDuplication();
    void testDuplicateSelection();
    void testDuplicateMultipleCaretsOnSingleLine1();
    void testDuplicateMultipleCaretsOnSingleLine2();
    void testDuplicateMultipleCaretsOnSingleLine3();

private:

    TextEditorWidget* widget() const;
    TextEditorController* ctrl() const;
    TextDocument* doc() const;
    TextSelection* sel() const;

    TextEditorWidget* widget_;
};


} // edbee

DECLARE_TEST(edbee::DuplicateCommandTest);
