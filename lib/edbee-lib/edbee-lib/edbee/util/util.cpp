/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "util.h"

#include <QString>

#include "edbee/debug.h"

namespace edbee {


/// Converst all tabs to sapces of the given string, using the current tab/indent settings
/// It converts "\t" to spaces.
///
/// @param str the string where to convert the tabs to space
/// @param tabSize the size of a single tab. This needs to be at least 1
/// @return A string with all tabs converted to spaces
QString Util::convertTabsToSpaces(const QString& str, int tabSize )
{
    Q_ASSERT(tabSize > 0);

    QString result;
    result.reserve( str.length() );

    // append all characters to the result
    for( int i=0,cnt=str.size(); i<cnt; ++i ) {
        QChar c = str.at(i);

        // when a tab character is used it is converted to the correct column
        if( c == '\t' ) {
            int amount = tabSize - result.length() % tabSize;
            result.append( QStringLiteral(" ").repeated(amount) );
        } else {
            result.append(c);
        }
    }
    return result;
}


/// This method returns all tab-column offsets of the given string
/// @param str the string to convert
/// @param tabSize the tab size to use for conversion
/// @return a vector with the character-offset in the given string that contains the given tab-column
QList<int> Util::tabColumnOffsets(const QString& str, int tabSize)
{
    // build the resut (column 0 is always available)
    QList<int> offsets;
    offsets.push_back( 0);

    int column = 0;

    // iterate over all characters
    for( int offset=0,cnt=str.size(); offset<cnt; ++offset ) {
        QChar c = str.at(offset);

        // when a tab character is found, we need to jump to the next column
        if( c == '\t' ) {
            int amount = tabSize - column % tabSize;
            column += amount;
        } else {
            ++column;
        }
        // when we've reached another tab-column, we add the column
        if( column % tabSize == 0 ) {
           offsets.push_back( offset+1 );
        }
    }
    return offsets;
}



} // edbee
