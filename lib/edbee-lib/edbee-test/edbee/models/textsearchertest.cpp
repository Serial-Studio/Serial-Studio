/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textsearchertest.h"


#include "edbee/models/chardocument/chartextdocument.h"
#include "edbee/models/textdocument.h"
#include "edbee/models/textrange.h"
#include "edbee/models/textsearcher.h"

#include "edbee/debug.h"


namespace edbee {

/// The search operation
TextSearcherTest::TextSearcherTest()
    : doc_(0)
    , searcher_(0)
    , ranges_(0)

{
}


/// Initializes the test for the next test
void TextSearcherTest::init()
{
    doc_ = createFixtureDocument();
    searcher_ = new TextSearcher( doc_ );
    ranges_ = new TextRangeSet( doc_ );
    ranges_->addRange(0,0);     // set the caret at the start
}


/// cleans up the testing environment
void TextSearcherTest::clean()
{
    delete ranges_;
    delete searcher_;
    delete doc_;
}


/// Tests the initial state
void TextSearcherTest::testInitialState()
{
    testEqual( ranges_->rangeCount(), 1 );  // test thte test

    testEqual( searcher_->searchTerm(), "" );
    testEqual( searcher_->isWrapAroundEnabled(), true );
    testEqual( searcher_->isReverse(), false );
    testTrue( searcher_->syntax() == TextSearcher::SyntaxPlainString );
}


/// Tests the find next operation
void TextSearcherTest::testFindNextRange()
{
    // searching next without search-term should result in not found
    testTrue( searcher_->findNextRange( ranges_ ).isEmpty() );

    // test if standard searching wors
    searcher_->setSearchTerm("test");
    testEqual( searcher_->findNextRange( ranges_ ).toString(), "10>14" );

    // make sure the previous call didn't change the textrange
    testEqual( searcher_->findNextRange( ranges_ ).toString(), "10>14" );

    searcher_->setSearchTerm("not-findable");
    testEqual( searcher_->findNextRange( ranges_ ).toString(), "0>0" );

}


/// Tests the findnext operation
void TextSearcherTest::testFindNext()
{
    searcher_->setSearchTerm("test");

    testTrue( searcher_->findNext( ranges_ ) );
    testEqual( ranges_->rangesAsString(), "10>14" );
    testTrue( searcher_->findNext( ranges_ ) );
    testEqual( ranges_->rangesAsString(), "24>28" );

    // tests wrap around
    searcher_->setWrapAround(true);
    testTrue( searcher_->findNext( ranges_ ) );
    testEqual( ranges_->rangesAsString(), "10>14" );
    testTrue( searcher_->findNext( ranges_ ) );
    testEqual( ranges_->rangesAsString(), "24>28" );

    // finding next when wrap-around is disabled should fail
    searcher_->setWrapAround(false);
    testFalse( searcher_->findNext( ranges_ ) );
    testEqual( ranges_->rangesAsString(), "24>28" );    // and the selection shouldn't change

}


/// Tests the findPrv operation
void TextSearcherTest::testFindPrev()
{
    searcher_->setSearchTerm("test");

    testTrue( searcher_->findPrev( ranges_ ) );
    testEqual( ranges_->rangesAsString(), "24>28" );

    testTrue( searcher_->findPrev( ranges_ ) );
    testEqual( ranges_->rangesAsString(), "10>14" );

    // when wrap-around is disabled it should't wrap backwards
    searcher_->setWrapAround(false);
    testFalse( searcher_->findPrev( ranges_ ) );
    testEqual( ranges_->rangesAsString(), "10>14" );
}


/// Test the select next operation
void TextSearcherTest::testSelectNext()
{
    searcher_->setSearchTerm("test");
    testTrue( searcher_->selectNext( ranges_ ) );
    testEqual( ranges_->rangesAsString(), "10>14" );

    testTrue( searcher_->selectNext( ranges_ ) );
    testEqual( ranges_->rangesAsString(), "10>14,24>28" );

    testTrue( searcher_->selectNext( ranges_ ) );           // searching again with wrap-around enabled results in true
    testEqual( ranges_->rangesAsString(), "10>14,24>28" );  // but shouldn't make any changes to the selection

}


/// Tests the select previous operation
void TextSearcherTest::testSelectPrev()
{
    searcher_->setSearchTerm("test");
    testTrue( searcher_->selectPrev( ranges_ ) );
    testEqual( ranges_->rangesAsString(), "24>28" );

    testTrue( searcher_->selectPrev( ranges_ ) );
    testEqual( ranges_->rangesAsString(), "10>14,24>28" );

    testTrue( searcher_->selectPrev( ranges_ ) );           // searching again with wrap-around enabled results in true
    testEqual( ranges_->rangesAsString(), "10>14,24>28" );  // but shouldn't make any changes to the selection
}


/// Tests the selection of all items in a text
void TextSearcherTest::testSelectAll()
{
    searcher_->setSearchTerm("non-exsiting");
    testFalse( searcher_->selectAll( ranges_ ) );
    testEqual( ranges_->rangesAsString(), "0>0" );


    searcher_->setSearchTerm("test");
    testTrue( searcher_->selectAll( ranges_ ) );
    testEqual( ranges_->rangesAsString(), "10>14,24>28" );


}


/// Creates the basic fixture
TextDocument* TextSearcherTest::createFixtureDocument()
{
    CharTextDocument* doc = new CharTextDocument();
    doc->setText(
    //01234567890123456789
     "This is a test doc.\n"
     "for testing the new\n"  // 20
     "search api. It is a\n"  // 40
     "nice detail to know\n"  // 60
     "that every line is \n"  // 80
     "twenty chars long. \n"  // 100
     "It's makes it much \n"  // 120
     "nicer to calculate \n"  // 140
     "the positions. :)"      // 160
    );
    return doc;
}


} // edbee
