/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "linedatalistchangetest.h"

#include "edbee/models/changes/linedatalistchange.h"
#include "edbee/models/chardocument/chartextdocument.h"
#include "edbee/models/textlinedata.h"

#include "edbee/debug.h"

namespace edbee {

static const int TEST_FIELD_INDEX = PredefinedFieldCount;

typedef BasicTextLineData<QString> TestLineData;

LineDataListChangeTest::LineDataListChangeTest()
    : doc_(nullptr)
{
}

/// constructs the basic textdocument
void LineDataListChangeTest::init()
{
    doc_ = new CharTextDocument();
    doc_->setLineDataFieldsPerLine( TEST_FIELD_INDEX + 1 );
    doc_->setText("1\n2\n3");
    doc_->giveLineData( 0, TEST_FIELD_INDEX, new TestLineData("a") );
    doc_->giveLineData( 1, TEST_FIELD_INDEX, new TestLineData("b") );
    doc_->giveLineData( 2, TEST_FIELD_INDEX, new TestLineData("c") );
}



/// cleans the data
void LineDataListChangeTest::clean()
{
    qDeleteAll(changeList_);
    changeList_.clear();
    delete doc_;
    doc_ = nullptr;
}


/// Test the execution of the data
/// At the moment only the grow/shrink is tested
void LineDataListChangeTest::testExecute()
{
    // test inserting
    testEqual( manager()->length(), 3 );
    createChange(1, 0, 1)->execute( doc_ );
    testEqual( manager()->length(), 4 );


    // test replace
    createChange(0, 2, 3)->execute( doc_ );
    testEqual( manager()->length(), 5 );

    // test delete
    createChange(1, 3, 0)->execute( doc_ );
    testEqual( manager()->length(), 2 );
}


/// Placing a growing textchange under the previous textchange and merge
void LineDataListChangeTest::testMerge_growBelow()
{
    LineDataListChange* change1 = createChange(1, 0, 1);
    change1->execute(doc_);
    testEqual( data2str(change1), "" );

    LineDataListChange* change2 = createChange(2, 0, 1);
    change2->execute(doc_);
    testEqual( data2str(change2), "" );

    testTrue( change1->giveAndMerge( doc_, change2 ) );
    takeChange(change2);
    testEqual( change1->offset (), 1);
    testEqual( change1->storedLength(), 0);
    testEqual( change1->docLength(), 2);
    testEqual( data2str(change1), "" );
}


/// Placing a growing textchange above the previous textchange and merge
void LineDataListChangeTest::testMerge_growAbove()
{
    LineDataListChange* change1 = createChange(2, 0, 1);
    change1->execute(doc_);
    testEqual( data2str(change1), "" );

    LineDataListChange* change2 = createChange(2, 0, 1);    // previous test was wong, inserting at 1 and 2 doesn't merge!!
    change2->execute(doc_);
    testEqual( data2str(change2), "" );

    testTrue( change1->giveAndMerge( doc_, change2 ) );

    takeChange(change2);
    testEqual( change1->offset (), 2);
    testEqual( change1->storedLength(), 0);
    testEqual( change1->docLength(), 2);
    testEqual( data2str(change1), "" );
}


/// Placing a shrinking textchange under the previous textchange and merge
void LineDataListChangeTest::testMerge_shrinkBelow()
{
    LineDataListChange* change1 = createChange(1, 1, 0);
    change1->execute(doc_);
    testEqual( data2str(change1), "b" );

    LineDataListChange* change2 = createChange(1, 1, 0);
    change2->execute(doc_);
    testEqual( data2str(change2), "c" );

    testTrue( change1->giveAndMerge( doc_, change2 ) );
    takeChange(change2);
    testEqual( change1->offset (), 1);
    testEqual( change1->storedLength(), 2);
    testEqual( change1->docLength(), 0);
    testEqual( data2str(change1), "bc" );
}


/// Placing a shrinking textchange above the previous textchange and merge
void LineDataListChangeTest::testMerge_shrinkAbove()
{
    LineDataListChange* change1 = createChange(1, 1, 0);
    change1->execute(doc_);
    testEqual( data2str(change1), "b" );

    LineDataListChange* change2 = createChange(0, 1, 0);
    change2->execute(doc_);
    testEqual( data2str(change2), "a" );

    testTrue( change1->giveAndMerge( doc_, change2 ) );
    takeChange(change2);
    testEqual( change1->offset (), 0);
    testEqual( change1->storedLength(), 2);
    testEqual( change1->docLength(), 0);
    testEqual( data2str(change1), "ab" );
}


/// returns the line data manager
TextLineDataManager* LineDataListChangeTest::manager()
{
    return doc_->lineDataManager();
}


/// creates a line change
/// @param line the start line that's changed
/// @param length the new number of lines
/// @param newLength the new number of line
LineDataListChange* LineDataListChangeTest::createChange(int line, int length, int newLength)
{
    LineDataListChange* result = new LineDataListChange( manager(), line, length, newLength );
    changeList_.append(result);
    return result;
}


/// Takes the given change (and remove it from the delete lsit )
/// @param change the change to take
LineDataListChange* LineDataListChangeTest::takeChange(LineDataListChange* change)
{
    return changeList_.takeAt( changeList_.indexOf(change) );
}


/// A static helper function to convert the line-data of the given textchange
/// to single string that's testable. 0's are converted to dots
QString LineDataListChangeTest::data2str( LineDataListChange* change )
{
    QString result;
    TextLineDataList** list = change->oldListList();
    for( int i=0,cnt=change->oldListListLength(); i<cnt; ++i ) {
        if( list[i] ) {
            TextLineData* lineData = list[i]->at(manager(),TEST_FIELD_INDEX);
            if( lineData ) {
                TestLineData* testLineData = dynamic_cast<TestLineData*>(lineData);
                if( testLineData ) {
                    result.append( testLineData->value() );
                }
            } else {
                result.append(".");
            }
        } else {
            result.append(".");
        }
    }
    return result;
}


/// A static helper function to convert the line-data of the given textchange
/// to single string that's testable. 0's are converted to dots
QString LineDataListChangeTest::data2ptr( LineDataListChange* change )
{
    QString result;
    TextLineDataList** list = change->oldListList();
    for( int i=0,cnt=change->oldListListLength(); i<cnt; ++i ) {
        if( list[i] ) {
            TextLineData* lineData = list[i]->at(manager(),TEST_FIELD_INDEX);
            if( lineData ) {
                result.append( QStringLiteral("").asprintf("%8p", static_cast<void*>(lineData)) );
            } else {
                result.append(".");
            }
        }
        result.append(",");
    }
    return result;
}


} // edbee
