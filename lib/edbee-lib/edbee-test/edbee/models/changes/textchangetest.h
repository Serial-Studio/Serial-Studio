/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/util/test.h"

namespace edbee {

/// performs some tests on the single textchanges
class TextChangeTest : public edbee::test::TestCase
{
    Q_OBJECT

private slots:
    void testBoundaryMethods();
    void testMerge1();
    void testMerge2();
    void testMerge3();
    void testMerge4();
    void testMerge5();

    void testMerge6_splitMerge();
    void testMerge7_splitMergeInvert();
};

} // edbee

DECLARE_TEST(edbee::TextChangeTest);
