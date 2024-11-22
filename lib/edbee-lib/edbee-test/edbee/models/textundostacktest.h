/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/util/test.h"

namespace edbee {

class TextUndoStackTest : public edbee::test::TestCase
{
Q_OBJECT

private slots:
    void testMultiCaretUndoIssue196();
    void testClearUndoStackCrashIssue24();
    void testClearUndoStackShouldnotUnregisterTheControllerIssue24();

};

} // edbee

DECLARE_TEST(edbee::TextUndoStackTest);

