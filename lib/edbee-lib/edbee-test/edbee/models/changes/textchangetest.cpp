/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textchangetest.h"

#include "edbee/models/changes/textchange.h"

#include "edbee/debug.h"

namespace edbee {


/// Test the overlapped by function
void TextChangeTest::testBoundaryMethods()
{
    // INSERT TESTS
    TextChange s1(4,4,"");    //      [    ]
    TextChange s2(0,2,"");    // [  ]
    TextChange s3(5,2,"");    //       [  ]
    TextChange s4(1,2,"");    //  [  ]
    TextChange s5(2,2,"");    //    [  ]

    // test non overlap touch
    testFalse( s1.isOverlappedBy(&s2) );
    testFalse( s1.isTouchedBy(&s2) );
    testFalse( s2.isOverlappedBy(&s1) );
    testFalse( s2.isTouchedBy(&s1) );

    // test overlap (fully included)
    testTrue( s1.isOverlappedBy(&s3) );
    testFalse( s1.isTouchedBy(&s3) );
    testTrue( s3.isOverlappedBy(&s1) );
    testFalse( s3.isTouchedBy(&s1) );

    // test overlap not fully included
    testTrue( s4.isOverlappedBy(&s5));
    testFalse( s4.isTouchedBy(&s5));
    testTrue( s5.isOverlappedBy(&s4));
    testFalse( s5.isTouchedBy(&s5));

    // test touching items
    testTrue( s2.isTouchedBy(&s5));
    testFalse( s2.isOverlappedBy(&s5));
    testTrue( s5.isTouchedBy(&s2));
    testFalse( s5.isOverlappedBy(&s2));

    // BACKSPACE TEST
    TextChange d1(0,0,"Y");
    TextChange d2(1,0,"X");
    testFalse( d1.isTouchedBy(&d2) );
    testFalse( d1.isOverlappedBy(&d2) );
    testTrue( d2.isTouchedBy(&d1) );
    testFalse( d2.isOverlappedBy(&d1) );


    TextChange d3(0,0,"X");
    TextChange d4(0,0,"Y");
    testTrue( d3.isTouchedBy(&d4) );
    testFalse( d3.isOverlappedBy(&d4) );
    testTrue( d4.isTouchedBy(&d3) );
    testFalse( d4.isOverlappedBy(&d3) );
}


/// MERGE 1
/// - a[bc]defgh  =(x)=> axdefgh   (A) 1:1:bc
/// - a[xd]efgh   =(y)=> ayefgh    (B) 1:1:xd
/// = This should be merge into:   (M) 1:1:bcd        =>  bc + d
void TextChangeTest::testMerge1()
{
    TextChange a(1,1,"bc");
    TextChange* b = new TextChange(1,1,"xd");

    testTrue( a.giveAndMerge(0,b) );
    testEqual( a.offset(), 1 );
    testEqual( a.docLength(), 1 );
    testEqual( a.storedText(), "bcd" );
}


/// MERGE 2
/// - a[bc]defgh   =(xyz)=> axyzdefgh (A) 1:3:bc
/// - [axyz]defgh  =(q)=>   qdefgh    (B) 0:1:axyz
/// = This should be merge into:      (M) 0:1:abc     =>  a + bc
void TextChangeTest::testMerge2()
{
    TextChange a(1,3,"bc");
    TextChange* b = new TextChange(0,1,"axyz");

    testTrue( a.giveAndMerge(0,b) );
    testEqual( a.offset(), 0 );
    testEqual( a.docLength(), 1 );
    testEqual( a.storedText(), "abc" );
}


/// MERGE 3
/// - a[bc]defgh  =(x)=> axdefgh   (A) 1:1:bc
/// - [axdef]gh   =(y)=> ygh       (B) 0:1:axdef
/// = This should be merge into:   (M) 0:1:abcdef    =>  a + bc + def
void TextChangeTest::testMerge3()
{
    TextChange a(1,1,"bc");
    TextChange* b = new TextChange(0,1,"axdef");

    testTrue( a.giveAndMerge(0,b) );
    testEqual( a.offset(), 0 );
    testEqual( a.docLength(), 1 );
    testEqual( a.storedText(), "abcdef" );
}


/// MERGE 4
/// - a[b]cdefgh     =(wxyz)=> awxyzcdefgh  (A) 1:4:b
/// - awx[yzc]defgh  =(q)=>    awxqdefgh    (B) 3:1:yzc
///= This should be merge into:             (M) 1:3:bc    =>  b + c
void TextChangeTest::testMerge4()
{
    TextChange a(1,4,"b");
    TextChange* b = new TextChange(3,1,"yzc");

    testTrue( a.giveAndMerge(0,b) );
    testEqual( a.offset(), 1 );
    testEqual( a.docLength(), 3 );
    testEqual( a.storedText(), "bc" );
}

/// MERGE 5
/// - a[b]cdefgh     =(wxyz)=> awxyzcdefgh   (A) 1:4:b
/// - awx[y]zcdefgh  =(q)=>    awxqzcdefgh   (B) 3:1:y
/// = This should be merge into:             (M) 1:4:b     =>  b
void TextChangeTest::testMerge5()
{
    TextChange a(1,4,"b");
    TextChange* b = new TextChange(3,1,"y");

    testTrue( a.giveAndMerge(0,b) );
    testEqual( a.offset(), 1 );
    testEqual( a.docLength(), 4 );
    testEqual( a.storedText(), "b" );
}


/// A special split-merge operation
/// Which means the second change was placed OVER the previous one on both side
/// (A) 1:0:BC
/// (B) 0:0:AD
/// (M) 0:0:ABCD
void TextChangeTest::testMerge6_splitMerge()
{
    TextChange a(1,0,"bc");
    TextChange* b = new TextChange(0,0,"ad");
    testTrue( a.giveAndMerge(0,b) );
    testEqual( a.offset(), 0 );
    testEqual( a.docLength(), 0 );
    testEqual( a.storedText(), "abcd" );
}


/// A special split-merge
/// Which means the second change was placed OVER the previous one on both side
/// (A) 1:1:
/// (B) 0:0:axb
/// (M) 0:2:ab
///
/// - mergeResult("1:1:", "0:2:axb"), "0:2:ab");
///  (text = a[x]b[y]e)
///
///
void TextChangeTest::testMerge7_splitMergeInvert()
{
    {
        TextChange a(1,1,"") ;
        TextChange* b = new TextChange(0,0,"axb");
        testTrue( a.giveAndMerge(0,b));
        testEqual( a.offset(),0 );
        testEqual( a.docLength(),0 );
        testEqual( a.storedText(), "ab" );
    }

    {
        TextChange a(0,1,"") ;
        TextChange* b = new TextChange(0,0,"xab");
        testTrue( a.giveAndMerge(0,b));
        testEqual( a.offset(),0 );
        testEqual( a.docLength(),0 );
        testEqual( a.storedText(), "ab" );
    }


    {
        TextChange a(0,2,"") ;                                       // |abcd => [xy]abcd             (0:2:)
        TextChange* b = new TextChange(1,0,"yab");             // x[yab]cd  => xcd              (1:0:yab)
        testTrue( a.giveAndMerge(0,b));                                    // =>                            (0:1:ab)
        testEqual( a.offset(),0 );
        testEqual( a.docLength(),1 );
        testEqual( a.storedText(), "ab" );
    }

    {
        TextChange a(1,3,"bc") ;                                     // a[bc]def     => aXYZdef        (1:3:bc)
        TextChange* b = new TextChange(4,1,"de");              // aXYZ[de]f    => aXYZ?f         (4:1:de)
        testTrue( a.giveAndMerge(0,b));                                    // =>                             (1:4:bcde)
        testEqual( a.offset(),1 );
        testEqual( a.docLength(),4 );
        testEqual( a.storedText(), "bcde" );
    }

    {
        TextChange a(1,4,"bc") ;                                     // a[bc]def     => aKLMNdef       (1:4:bc)
        TextChange* b = new TextChange(2,2,"L");               // aK[L]MNdef  => aKxyMNdef       (2:2:L)
        testTrue( a.giveAndMerge(0,b));                                    // =>                             (1:5:bc)
        testEqual( a.offset(),1 );
        testEqual( a.docLength(),5 );
        testEqual( a.storedText(), "bc" );
    }

}


} // edbee
