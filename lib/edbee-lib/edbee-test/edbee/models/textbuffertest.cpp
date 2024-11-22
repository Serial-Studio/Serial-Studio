/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textbuffertest.h"

#include "edbee/models/textbuffer.h"
#include "edbee/models/chardocument/chartextbuffer.h"
#include "edbee/models/chardocument/chartextdocument.h"

#include "edbee/debug.h"

namespace edbee {


/// Asserts the buffer
/// The marker list is a list of markers, with the end marker closed with a -1
#define testBuffer( buf, txt, markers ) \
do { \
    testEqual( buf->text(), txt );  \
    QString lineOffsets = buf->lineOffsetsAsString(); \
    testEqual( lineOffsets, markers ); \
} while(false)


/// This method tests the line from offset method
void TextBufferTest::testlineFromOffset()
{
    CharTextDocument doc;
    TextBuffer* buf = doc.buffer();

    // build an initial doc
    buf->appendText("a1\nb2\nc3\n\nd5");
    testBuffer( buf, "a1\nb2\nc3\n\nd5", "0,3,6,9,10" );

    testEqual( buf->lineFromOffset(0), 0 );
    testEqual( buf->lineFromOffset(1), 0 );
    testEqual( buf->lineFromOffset(2), 0 );
    testEqual( buf->lineFromOffset(3), 1 );


}

/// Tests if the column from offset and line works correctly
void TextBufferTest::testColumnFromOffsetAndLine()
{
    CharTextDocument doc;
    TextBuffer* buf = doc.buffer();
    buf->appendText("a1\nbb2\nccc3\n\nd5");

    // basic test
    testEqual( buf->columnFromOffsetAndLine( 0, -1  ), 0 );
    testEqual( buf->columnFromOffsetAndLine( 1, -1  ), 1 );
    testEqual( buf->columnFromOffsetAndLine( 2, -1  ), 2 );
    testEqual( buf->columnFromOffsetAndLine( 3, -1  ), 0 );

    // when supplying a line offset it should work
    testEqual( buf->columnFromOffsetAndLine( 0, 0  ), 0 );
    testEqual( buf->columnFromOffsetAndLine( 2, 0  ), 2 );
    testEqual( buf->columnFromOffsetAndLine( 3, 0  ), 3 );
    testEqual( buf->columnFromOffsetAndLine( 4, 0  ), 3 );

    // another line then 0 should work
    testEqual( buf->columnFromOffsetAndLine( 3, 1  ), 0 );
    testEqual( buf->columnFromOffsetAndLine( 4, 1  ), 1 );
    testEqual( buf->columnFromOffsetAndLine( 5, 1  ), 2 );

    // it should sill clamb between the boundries
    testEqual( buf->columnFromOffsetAndLine( 2, 1  ), 0 );
    testEqual( buf->columnFromOffsetAndLine( 5, 1  ), 2 );
    testEqual( buf->columnFromOffsetAndLine( 6, 1  ), 3 );
    testEqual( buf->columnFromOffsetAndLine( 7, 1  ), 4 );
    testEqual( buf->columnFromOffsetAndLine( 8, 1  ), 4 );

    // line overflow should return 0
    testEqual( buf->columnFromOffsetAndLine( 0, 100  ), 0 );
}


/// This method tests the replace text method
void TextBufferTest::testReplaceText()
{
    // first test. An empty document should be empty!
    CharTextDocument doc;
    TextBuffer* buf = doc.buffer();
    testBuffer( buf, "", "0" );

    // test 'global inserting
    //-----------------------

        // Test inserting of plain text
        buf->replaceText(0,0,"Test");
        testBuffer( buf, "Test", "0" );

        // Test the insertation of another string
        buf->replaceText(0,0,"A");
        testBuffer( buf, "ATest", "0" );

        // Test the replacement of a string
        buf->replaceText(0,2,"XX");
        testBuffer( buf, "XXest", "0" );

        buf->replaceText(1,3,"Y");
        testBuffer( buf, "XYt", "0" );

        // replacement over the end should be possible
        buf->replaceText(0,100,"Zz.");
        testBuffer( buf, "Zz.", "0" );

        // appending to the end should be possible even at a bigger offset
        buf->replaceText(10,0,"X");
        testBuffer( buf, "Zz.X", "0" );

        // append over the end from the middle should work
        buf->replaceText(1,100,"abc");
        testBuffer( buf, "Zabc", "0" );

        // inserting a single character
        buf->replaceText(1,0,"z");
        testBuffer( buf, "Zzabc", "0" );

        // replacing a single
        buf->replaceText(1,1,"Y");
        testBuffer( buf, "ZYabc", "0" );

    // test the new line markers
    //--------------------------

        buf->replaceText(0,buf->length(), "");
        testBuffer( buf, "", "0" );

        // are newlines detected correctly on an insert?
        buf->replaceText(0,buf->length(), "1\n2");
        testBuffer( buf, "1\n2","0,2");

        // inserting a text should update newline markers after the insertation point
        buf->replaceText(1,0,"a");
        testBuffer( buf, "1a\n2","0,3");

        // inserting a text at the 0th pos should change the 0 pos
        buf->replaceText(0,0,"b");
        testBuffer( buf, "b1a\n2","0,4");

        // we should be able to add a newline
        buf->replaceText(0,3,"rick\n");
        testBuffer( buf, "rick\n\n2","0,5,6");

        // replacing text should fix newlines
//        LineOffsetVector::debug = true;
        buf->replaceText(5,100,"a\nb\nc");
        testBuffer( buf, "rick\na\nb\nc","0,5,7,9");
//        LineOffsetVector::debug = false;

        // inserting a text should change the newlines
        buf->replaceText(0,5,"");
        testBuffer( buf, "a\nb\nc","0,2,4");

        // inserting a text should change the newlines
        buf->replaceText(0,10,"");
        testBuffer( buf, "","0");
}


/// This method tests the finchar pos within range function
void TextBufferTest::testFindCharPosWithinRange()
{
    CharTextDocument doc;
    TextBuffer* buf = doc.buffer();
    buf->appendText("aaa\naa bb cc\n a \n ");
    QString strA = "a";
    QString strC = "c";
    QString strAll = "abc \n";

    // test invalid search values
    testEqual( buf->findCharPos(-1, -1, strAll, true), -1 );
    testEqual( buf->findCharPos(200, -1, strAll, true), -1 );

    // test moving out of left and right border
    testEqual( buf->findCharPos(0, 1, strAll, true), 0 );
    testEqual( buf->findCharPos(0, -1, strAll, false), -1 );
    testEqual( buf->findCharPos(5, -1, strAll, false), -1 );
    testEqual( buf->findCharPos(0, 1, strAll, false), -1 );

    testEqual( buf->findCharPos(2, 1, strA, true), 2 );
    testEqual( buf->findCharPos(2, 2, strA, true), 4 );
    testEqual( buf->findCharPos(2, 4, strA, true), 14 );
    testEqual( buf->findCharPos(2, -1, strA, true), 2 );
    testEqual( buf->findCharPos(2, -2, strA, true), 1 );
    testEqual( buf->findCharPos(2, -3, strA, true), 0 );
    testEqual( buf->findCharPos(2, -4, strA, true), -1 );

    testEqual( buf->findCharPos(2, 1, strC, true), 10 );
    testEqual( buf->findCharPos(2, 2, strC, true), 11 );


    testEqual( buf->findCharPosOrClamp(5, -1, "X", true), 0 );
    testEqual( buf->findCharPosOrClamp(5, 1, "X", true), buf->length() );

    testEqual( buf->findCharPosWithinRangeOrClamp(5, -1, "X", true, 2, 8), 2 );
    testEqual( buf->findCharPosWithinRangeOrClamp(5, 1, "X", true, 2, 8), 8 );

}


/// This method is for testing the line function
void TextBufferTest::testLine()
{
    CharTextDocument doc;
    TextBuffer* buf = doc.buffer();
    buf->appendText("aaa\nbbb\nccc\nddd");

    testEqual( buf->line(0), "aaa\n");
    testEqual( buf->lineWithoutNewline(0), "aaa" );


    testEqual( buf->line(3), "ddd");
    testEqual( buf->lineWithoutNewline(3), "ddd" );
}


/// test method test the working of the lineoffsetvector. (Which fgot corrupted with certain replaces)
void TextBufferTest::testReplaceIssue141()
{
    CharTextDocument doc;
    TextBuffer* buf = doc.buffer();
//    CharTextBuffer* charBuf = dynamic_cast<CharTextBuffer*>(buf);
    buf->appendText("11\n22\n33");
    testBuffer( buf, "11\n22\n33", "0,3,6" );

    // append a new line
    buf->replaceText(1,0,"\n");
    testBuffer( buf, "1\n1\n22\n33", "0,2,4,7" );

    // // Before
    // 1|1|22|33
    // 0 2 4  7
    //
    // // change
    //
    // 1|1|22|33
    // XXXX         => aa|bb|
    //
    // // after
    // aa|bb|22|33
    // 0  3  6  9

    // next append the new text
    buf->replaceText(0,4,"aa\nbb\n");
    testBuffer( buf, "aa\nbb\n22\n33", "0,3,6,9" );
}


} // edbee
