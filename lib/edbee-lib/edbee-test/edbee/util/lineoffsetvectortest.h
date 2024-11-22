/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/util/test.h"

namespace edbee {

class LineOffsetVector;

class LineOffsetVectorTest : public edbee::test::TestCase
{
    Q_OBJECT
    
private slots:

    void testMoveDeltaToIndex();
    void testChangeOffsetDelta();
    void testTextReplaced();
    void testFindLineFromOffset();

};


} // edbee

DECLARE_TEST(edbee::LineOffsetVectorTest);

