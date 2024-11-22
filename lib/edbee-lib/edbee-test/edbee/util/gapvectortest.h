/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/util/test.h"

namespace edbee {


class GapVectorTest : public edbee::test::TestCase
{
    Q_OBJECT
    
private slots:

    void testMoveGap();
    void testResize();
    void testReplace();

    void testCopyRange();

    void testIssue141();

    void testIssueLineDataVector();

};

} // edbee

DECLARE_TEST(edbee::GapVectorTest);
