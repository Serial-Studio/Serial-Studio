/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/util/test.h"

namespace edbee {

/// A test for testing the rangeset line iterator
class RangeSetLineIteratorTest  : public edbee::test::TestCase
{
    Q_OBJECT

private slots:
    void testBasicIteration();
};

} // edbee

DECLARE_TEST(edbee::RangeSetLineIteratorTest);
