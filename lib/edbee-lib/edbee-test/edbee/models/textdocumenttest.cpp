/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textdocumenttest.h"

#include <QStringList>
#include <QDebug>

#include "edbee/models/chardocument/chartextdocument.h"
#include "edbee/models/textbuffer.h"
#include "edbee/models/textlinedata.h"
#include "edbee/models/textrange.h"

#include "edbee/debug.h"

namespace edbee {

/// test the documentbuffer
/// The marker list is a list of markers, with the end marker closed with a -1
#define testBuffer( buf, txt, markers ) \
do { \
    testEqual( buf->text(), txt );  \
    QString lineOffsets = buf->lineOffsetsAsString(); \
    testEqual( lineOffsets, markers ); \
} while(false)


/// Test the line data handling of line data
void TextDocumentTest::testLineData()
{
    CharTextDocument doc;
    TextBuffer* buf = doc.buffer();
    buf->appendText("aaa\nbbb\nccc");
    testTrue( doc.getLineData( 0, 0 ) == 0 );
    testTrue( doc.getLineData( 1, 0 ) == 0 );
    testTrue( doc.getLineData( 2, 0 ) == 0 );

    // set an item at line 1
    doc.giveLineData( 1, 0, new QStringTextLineData("test") );
    testTrue( doc.getLineData( 0, 0 ) == 0 );
    testTrue( doc.getLineData( 1, 0 ) != 0 );
    testTrue( doc.getLineData( 2, 0 ) == 0 );

    QStringTextLineData* data = dynamic_cast<QStringTextLineData*>( doc.getLineData(1,0) );
    testEqual( data->value(), "test" );
    data->setValue("new-test");

    // inserting a line should 'shift' the data to the next line
    buf->replaceText(1,0, "\n");
    testBuffer( buf, "a\naa\nbbb\nccc","0,2,5,9");
    testTrue( doc.getLineData( 0, 0 ) == 0 );
    testTrue( doc.getLineData( 1, 0 ) == 0 );    
    testTrue( doc.getLineData( 2, 0 ) != 0 );

    Q_ASSERT(doc.getLineData( 1, 0 )==0);
    Q_ASSERT(doc.getLineData( 2, 0 ));

    data = dynamic_cast<QStringTextLineData*>( doc.getLineData(2,0) );
    Q_ASSERT(data);
    testEqual( data->value(), "new-test" );

    // removing a line should 'shift' the data to the previous line
    buf->replaceText(0,4,"");
    testBuffer( buf, "\nbbb\nccc","0,1,5");
    testTrue( doc.getLineData( 0, 0 ) == 0 );
    testTrue( doc.getLineData( 1, 0 ) != 0 );
    testTrue( doc.getLineData( 2, 0 ) == 0 );

    // replacing a line with a new line should remove the field
    buf->replaceText(0,3,"\n");
    testBuffer( buf, "\nb\nccc","0,1,3");
    testTrue( doc.getLineData( 0, 0 ) == 0 );
    testTrue( doc.getLineData( 1, 0 ) == 0 );

    // remove all items
    buf->replaceText(0,100,"");
    testBuffer( buf, "","0");
    testTrue( doc.getLineData( 0, 0 ) == 0 );
}


/// adds a simple replacement
/// a) "a[b]cd" => "aX[]cd"
/// b) "a[X]c[d] => "aR[]cS[]"
void TextDocumentTest::testReplaceRangeSet_simple()
{
    CharTextDocument doc;
    doc.append("abcd");

    // create the ranges
    TextRangeSet ranges(&doc);
    ranges.addRange(1,2);

    // execute the replace, this should also move the ranges
    doc.replaceRangeSet( ranges, "X" );
    testEqual( doc.text(), "aXcd");
    testEqual( ranges.rangesAsString(), "2>2");

    // next test multiple carets
    /// b) "a[X]b[c]d => "aR[]bSd
    ranges.setRange(1,2);
    ranges.addRange(3,4);
    doc.replaceRangeSet( ranges, QStringLiteral("R,S").split(",") );
    testEqual( doc.text(), "aRcS");
    testEqual( ranges.rangesAsString(), "2>2,4>4");
}


/// Test the replace rangeset operation
/// test =>  "a[bc]de[fg]h" => "aX|deY|h
void TextDocumentTest::testReplaceRangeSet_sizeDiff()
{
    CharTextDocument doc;
    doc.append("abcdefgh");

    // create the ranges
    TextRangeSet ranges(&doc);
    ranges.addRange(1,3);
    ranges.addRange(5,7);

    // execute the replace, this should move the ranges
    doc.replaceRangeSet( ranges, QStringLiteral("X,Y").split(",") );
    testEqual( doc.text(), "aXdeYh");
}


/// Implements a simple text insert with multiple ranges
/// a|b|cd => aX|bY|cd
void TextDocumentTest::testReplaceRangeSet_simpleInsert()
{
    CharTextDocument doc;
    doc.append("abcd");

    // create the ranges
    TextRangeSet ranges(&doc);
    ranges.addRange(1,1);
    ranges.addRange(2,2);

    doc.replaceRangeSet( ranges, QStringLiteral("X,Y").split(",") );
    testEqual( doc.text(), "aXbYcd");
    testEqual( ranges.rangesAsString(), "2>2,4>4" );

}


/// Tests a delete operation
/// a[1]b2c[3]d4 =>  a|b2c|d4
void TextDocumentTest::testReplaceRangeSet_delete()
{
    CharTextDocument doc;
    doc.append("a1b2c3d4");

    // create the ranges
    TextRangeSet ranges(&doc);
    ranges.addRange(1,2);
    ranges.addRange(5,6);

    doc.replaceRangeSet( ranges, "" );
    testEqual( doc.text(), "ab2cd4" );
    testEqual( ranges.rangesAsString(), "1>1,4>4" );
}



} // edbee
