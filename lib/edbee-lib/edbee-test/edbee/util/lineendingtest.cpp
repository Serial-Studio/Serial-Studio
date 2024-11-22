/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "lineendingtest.h"

#include "edbee/util/lineending.h"

#include "edbee/debug.h"

namespace edbee {

void LineEndingTest::testDetect()
{
    testEqual( LineEnding::detect("aaa\nbb\nccc")->type(), LineEnding::UnixType );
    testEqual( LineEnding::detect("aaa\rbb\rccc")->type(), LineEnding::MacClassicType );
    testEqual( LineEnding::detect("aaa\r\nbb\r\nccc")->type(), LineEnding::WindowsType );

    testEqual( LineEnding::detect("aaa\nbb\r\nccc\nddd")->type(), LineEnding::UnixType );
    testEqual( LineEnding::detect("aaa\rbb\nccc\rddd")->type(), LineEnding::MacClassicType );
    testEqual( LineEnding::detect("aaa\r\nbb\nccc\r\nddd")->type(), LineEnding::WindowsType );

    // multiple types (prefered type is unix)
    testEqual( LineEnding::detect("aaa\rbb\nccc\r\nddd")->type(), LineEnding::UnixType);


    testTrue( LineEnding::detect("aaaaa") == 0 );
    testEqual( LineEnding::detect("aaaaa", LineEnding::get( LineEnding::UnixType ) )->type(), LineEnding::UnixType );
}



} // edbee
