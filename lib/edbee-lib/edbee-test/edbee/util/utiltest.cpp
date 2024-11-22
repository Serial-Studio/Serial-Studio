/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "utiltest.h"

#include <QList>

#include "edbee/util/util.h"

#include "edbee/debug.h"

namespace edbee {


/// tests if the convert Tabs to spaces works correctly
void UtilTest::testConvertTabsToSpaces()
{
    // some basic test (no conversion happens
    testEqual( Util().convertTabsToSpaces("",4), "" );
    testEqual( Util().convertTabsToSpaces("test",4), "test" );
    testEqual( Util().convertTabsToSpaces(" ",4), " " );

    // no test if the basics work
    testEqual( Util().convertTabsToSpaces("\t",1), " " );
    testEqual( Util().convertTabsToSpaces("\t",2), "  " );
    testEqual( Util().convertTabsToSpaces("\t",4), "    " );

    // now more complex tests
    testEqual( Util().convertTabsToSpaces("\t ",4), QStringLiteral(" ").repeated(5) );
    testEqual( Util().convertTabsToSpaces(" \t",4), QStringLiteral(" ").repeated(4) );
    testEqual( Util().convertTabsToSpaces("  \t",4), QStringLiteral(" ").repeated(4) );
    testEqual( Util().convertTabsToSpaces("\t  \t",4), QStringLiteral(" ").repeated(8) );

    // when using a tab character in the middle of the line it should work
    testEqual( Util().convertTabsToSpaces("12345\t9",4), QStringLiteral("12345   9") );
}


/// helper function to convert a QVector<int> to a string list
static QString str( const QList<int>& list )
{
    QString result;
    foreach( int val, list ) {
        if( !result.isEmpty() ) { result.append(","); }
        result.append( QString::number(val) );
    }
    return result;
}


/// Tests the calculation of tab-column offests
void UtilTest::testTabColumnOffsets()
{
    testEqual( str(Util().tabColumnOffsets("",4)), "0" );
    testEqual( str(Util().tabColumnOffsets("  ",4)), "0" );
    testEqual( str(Util().tabColumnOffsets("    ",4)), "0,4" );
    testEqual( str(Util().tabColumnOffsets("\t",4)), "0,1" );
    testEqual( str(Util().tabColumnOffsets("  \t",4)), "0,3" );
    testEqual( str(Util().tabColumnOffsets("  \t \t",4)), "0,3,5" );
    testEqual( str(Util().tabColumnOffsets("\t\t\t",4)), "0,1,2,3" );
}


} // edbee
