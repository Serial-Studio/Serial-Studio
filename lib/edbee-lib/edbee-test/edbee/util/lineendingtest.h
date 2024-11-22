/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/util/test.h"

namespace edbee {


class LineEndingTest : public edbee::test::TestCase
{
    Q_OBJECT

private slots:
    void testDetect();


};

} // edbee

DECLARE_TEST(edbee::LineEndingTest);

