/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/util/test.h"

namespace edbee {

/// Contains tests of the utility functions
class UtilTest : public edbee::test::TestCase
{
    Q_OBJECT

private slots:
    void testConvertTabsToSpaces();
    void testTabColumnOffsets();

};


} // edbee

DECLARE_TEST(edbee::UtilTest);
