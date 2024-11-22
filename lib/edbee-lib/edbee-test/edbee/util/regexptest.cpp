/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "regexptest.h"

#include "edbee/util/regexp.h"

#include "edbee/debug.h"

namespace edbee {

void RegExpTest::testRegExp()
{
    RegExp* regExp = new RegExp( "a+(b*)c*");
    int idx = regExp->indexIn("xxxaaabccccddddd");
    testEqual( idx, 3 );
    testEqual( regExp->pos(0), 3 );
    testEqual( regExp->len(0), 8 );

    testEqual( regExp->pos(1), 6 ); // the b is at the 6th position
    testEqual( regExp->len(1), 1 );

    testEqual( regExp->pos(2), -1 ); // the b is at the 6th position
    testEqual( regExp->len(2), -1 );


//    qlog_info() << "pos-0: " << regExp->pos(0);
//    qlog_info() << "pos-1: " << regExp->pos(1);
//    qlog_info() << "pos-2: " << regExp->pos(2);


    delete regExp;
}


} // edbee
