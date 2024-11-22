/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "rangesetlineiteratortest.h"

#include "edbee/models/chardocument/chartextdocument.h"
#include "edbee/models/textrange.h"
#include "edbee/util/rangesetlineiterator.h"

#include "edbee/debug.h"

namespace edbee {

/// basic test for line iteration
void RangeSetLineIteratorTest::testBasicIteration()
{
    CharTextDocument doc;
    doc.setText( QStringLiteral("a1|b2|c3|d4|e5|f6").replace("|","\n"));
    TextRangeSet ranges(&doc);
    ranges.addRange(0,0);   // line: 0
    ranges.addRange(1,1);   // line: 0
    ranges.addRange(2,7);   // line: 0-2
    ranges.addRange(12,15); // line: 4-5

    RangeSetLineIterator itr(&ranges);

    testTrue( itr.hasNext() );
    testEqual( itr.next(), 0 );
    testTrue( itr.hasNext() );
    testEqual( itr.next(), 1 );
    testTrue( itr.hasNext() );
    testEqual( itr.next(), 2 );
    testTrue( itr.hasNext() );
    testEqual( itr.next(), 4 );
    testTrue( itr.hasNext() );
    testEqual( itr.next(), 5 );
    testFalse( itr.hasNext() );
    testEqual( itr.next(), -1 );
}


} // edbee
