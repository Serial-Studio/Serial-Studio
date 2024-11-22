/**
 * Copyright 2011-2014 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/util/test.h"

namespace edbee {

/// performs testing of the RangeLine Iteratopr
class RangeLineIteratorTest : public edbee::test::TestCase
{
    Q_OBJECT

private slots:
    void testBasicIteration();
    void testSingleLineIteration();
};

} // edbee

DECLARE_TEST(edbee::RangeLineIteratorTest);
