/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "gapvectortest.h"

#include "edbee/util/gapvector.h"

#include "edbee/debug.h"

namespace edbee {

#define testContent( vector, expected ) testEqual( vector.getUnitTestString(), expected )

/// test the gap movement
void GapVectorTest::testMoveGap()
{
    QCharGapVector v("ABCD", 3 );
    testEqual( v.length(), 4 );
    testEqual( v.capacity(), 7 );
    testContent( v, "ABCD[___>");

    v.moveGapTo(0);
    testContent( v, "[___>ABCD");

    v.moveGapTo(2);
    testContent( v, "AB[___>CD");

    v.moveGapTo(4);
    testContent( v, "ABCD[___>");

    v.moveGapTo(2);
    testContent( v, "AB[___>CD");

    v.moveGapTo(3);
    testContent( v, "ABC[___>D");

    // moving the to far should 'grow' the left part (now gives an assert failure!)
//    v.moveGapTo(6);
//    testContent( v, "ABCDBC[_>");

}

void GapVectorTest::testResize()
{
    QCharGapVector v("ABCD", 0 );
    testContent( v, "ABCD[>" );
    v.resize(8);
    testContent( v, "ABCD[____>" );

    v.moveGapTo( 2 );
    testContent( v, "AB[____>CD" );

    v.resize(10);
    testContent( v, "ABCD[______>" );

    // reset the size (start new test)
    v.clear();
    v.init("AB",1);
    v.setGrowSize(2);
    testContent( v, "AB[_>" );
    v.moveGapTo(1);
    testContent( v, "A[_>B" );
    v.replaceString(1,0,"XY");
    testContent( v, "AXY[__>B");

}


void GapVectorTest::testReplace()
{
    QCharGapVector v(4);
    v.setGrowSize(2);
    testContent( v, "[____>" );

    v.replaceString(0,0, "AB" );
    testContent( v, "AB[__>" );
    testEqual(v.length(),2);

    v.replaceString(0,0, "X" );
    testContent( v, "X[_>AB");
    testEqual(v.length(),3);

    v.replaceString(0,1, "Y" );
    testContent( v, "Y[_>AB");

    v.replaceString(0,1, "XYZ" );
    testContent( v, "XYZ[__>AB");

    testEqual(v.length(),5);
    testEqual(v.capacity(),7);

    v.replaceString(0, 5, "Q" );
    testContent( v, "Q[______>");

    v.replaceString(0, 1, "Blaat" );
    testContent( v, "Blaat[__>");


    v.init("abc|def|ghi",4);
    testContent( v, "abc|def|ghi[____>");

    v.replaceString(4,0,"1");   // insert 1
    testContent( v, "abc|1[___>def|ghi");

    v.replaceString(5,1,"");   // insert 1
    testContent( v, "abc|1[____>ef|ghi");


    v.init("abcdef|ghijklm",2);
    testContent(v,"abcdef|ghijklm[__>");
    v.replaceString(1,0,"1");
    testContent(v,"a1[_>bcdef|ghijklm");


    v.replaceString(3,0,"2");
    testContent(v,"a1b2[>cdef|ghijklm");

}


void GapVectorTest::testCopyRange()
{
    QCharGapVector v("ABCD",2);
    v.moveGapTo(2);
    testContent( v, "AB[__>CD" );

    // copying should work 'over' the gap
    QString str(3,'x');
    v.copyRange( str.data(), 0, 3 );
    testEqual( str, "ABC" );
    testContent( v, "AB[__>CD" );

    str.fill('x',2);
    v.copyRange( str.data(), 2, 2 );
    testEqual( str, "CD" );
    testContent( v, "AB[__>CD" );

    str.fill('x',1);
    v.copyRange( str.data(), 2, 1 );
    testEqual( str, "C" );
    testContent( v, "AB[__>CD" );

    str.fill('x',1);
    v.copyRange( str.data(), 3, 1 );
    testEqual( str, "D" );
    testContent( v, "AB[__>CD" );

    str.fill('x',1);
    v.copyRange( str.data(), 0, 1 );
    testEqual( str, "A" );
    testContent( v, "AB[__>CD" );

    str.fill('x',1);
    v.replaceString(2,1,"");
    testContent( v, "AB[___>D" );
    v.copyRange( str.data(), 2, 1 );
    testEqual( str, "D" );

}

void GapVectorTest::testIssue141()
{
    QCharGapVector v("036",1);
    testContent( v, "036[_>" );

    // append a new line
    v.replaceString(1,0,"2");
    testContent( v, "02[>36" );
    /*
    // Before
    1|1|22|33
    0 2 4  7

    // change

    1|1|22|33
    XXXX         => aa|bb|

    // after
    aa|bb|22|33
    0  3  6  9
    */

    // next append the new text
    v.replaceString(1,2, "XY");
    testContent( v, "0X[>Y6" );

}

void GapVectorTest::testIssueLineDataVector()
{
    // This is the issue decribed as gapvector test
    QCharGapVector v("ABCD",2);
    testContent( v, "ABCD[__>");

    v.fill(1,0,'X',1);

//    v.replaceString(1,0,"X");
    testContent( v, "AX[_>BCD");

    v.replaceString(1,1,"");
    testContent( v, "A[__>BCD");
}


} // edbee
