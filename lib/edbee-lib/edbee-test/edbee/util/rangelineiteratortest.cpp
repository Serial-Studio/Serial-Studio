/**
 * Copyright 2011-2014 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "rangelineiteratortest.h"

#include "edbee/models/chardocument/chartextdocument.h"
#include "edbee/models/textrange.h"
#include "edbee/util/rangelineiterator.h"

#include "edbee/debug.h"

namespace edbee {

/// tests for basic line iteration
void RangeLineIteratorTest::testBasicIteration()
{
    CharTextDocument doc;
    doc.setText( QStringLiteral("a1|b2|c3|d4|e5|f6").replace("|","\n"));
    TextRange range(4,15);

    RangeLineIterator itr( &doc, range);
    testTrue( itr.hasNext() );
    testEqual( itr.next(), 1 );
    testTrue( itr.hasNext() );
    testEqual( itr.next(), 2 );
    testTrue( itr.hasNext() );
    testEqual( itr.next(), 3 );
    testTrue( itr.hasNext() );
    testEqual( itr.next(), 4 );
    testTrue( itr.hasNext() );
    testEqual( itr.next(), 5 );
    testFalse( itr.hasNext() );

}


/// Test a single-line iteration.
/// This should also work and return a single line
void RangeLineIteratorTest::testSingleLineIteration()
{
    CharTextDocument doc;
    doc.setText( QStringLiteral("line1|line").replace("|","\n"));
    TextRange range(1,1);

    RangeLineIterator itr( &doc, range);
    testTrue( itr.hasNext() );
    testEqual( itr.next(), 0 );

}




} // edbee
