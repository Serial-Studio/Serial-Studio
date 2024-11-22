/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "lineoffsetvectortest.h"

#include "edbee/models/textbuffer.h"
#include "edbee/util/lineoffsetvector.h"

#include "edbee/debug.h"


namespace edbee {

#define testLov(v,expected) testEqual(v.toUnitTestString(),expected)


void LineOffsetVectorTest::testMoveDeltaToIndex()
{
    LineOffsetVector v;
    v.initForUnitTesting( 3, 2, 2,4,6,8,-1);
    testLov(v,"2,4[3>6,8");

    // move to the left
    v.moveDeltaToIndex(1);
    testLov(v,"2[3>1,6,8");

    // move to the right
    v.moveDeltaToIndex(2);
    testLov(v,"2,4[3>6,8");

    // one step further
    v.moveDeltaToIndex(3);
    testLov(v,"2,4,9[3>8");

    // moving to the left 2 steps should force the delta-0 rule (2>1)
    v.moveDeltaToIndex(1);
    testLov(v,"2[0>4,9,11");
}


void LineOffsetVectorTest::testChangeOffsetDelta()
{
    LineOffsetVector v;
    v.initForUnitTesting( 3, 2, 2,4,6,8,-1);
    testLov(v,"2,4[3>6,8");

    // test the same location
    v.changeOffsetDelta(2,4);
    testLov(v,"2,4[7>6,8");

    // test an offset before the current location
    v.changeOffsetDelta(1,-1);
    testLov(v,"2[6>-2,6,8");

    // test an offset after the current location
    v.changeOffsetDelta(3,-2);
    testLov(v,"2,4,12[4>8");

}


#define V_TEXT_REPLACED(offset,lengthIn,str) do {\
    QString qstr(str); \
    TextBufferChange change(&v, offset, lengthIn, qstr.data(), qstr.length() ); \
    v.applyChange(change); \
} while(false)


void LineOffsetVectorTest::testTextReplaced()
{
    LineOffsetVector v;
    V_TEXT_REPLACED(0,0,"a\nb\nc\nd\ne");
    testLov(v,"0,2,4,6,8[0>");

    // next insert a newline
    V_TEXT_REPLACED(3,0,"\n");
    testLov(v,"0,2,4[1>4,6,8");

    v.moveDeltaToIndex(0);
    testLov(v,"[0>0,2,4,5,7,9");

    V_TEXT_REPLACED(0,0,"\n");
    testLov(v,"0,1[1>2,4,5,7,9");


    V_TEXT_REPLACED(0,11,"");
    testLov(v,"0[0>");

}

void LineOffsetVectorTest::testFindLineFromOffset()
{
    LineOffsetVector v;

    // offset 0,4
    v.initForUnitTesting(0,0, 0,4,-1);
    testEqual( v.findLineFromOffset(0), 0 );
    testEqual( v.findLineFromOffset(1), 0 );
    testEqual( v.findLineFromOffset(2), 0 );
    testEqual( v.findLineFromOffset(3), 0 );
    testEqual( v.findLineFromOffset(4), 1 );


    // offsets: 0,4
    v.initForUnitTesting(2,1, 0,2,-1);
    testEqual( v.findLineFromOffset(0), 0 );
    testEqual( v.findLineFromOffset(1), 0 );
    testEqual( v.findLineFromOffset(2), 0 );
    testEqual( v.findLineFromOffset(3), 0 );
    testEqual( v.findLineFromOffset(4), 1 );


}


} // edbee
