/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QList>

class QString;

namespace edbee {

/// A global utiltity class.
/// The purpose of this class is to put 'global' function that don't quite fit on other places
///
/// You can use this class like this:   Util().converTabsToSpaces()
class EDBEE_EXPORT Util {
public:
    QString convertTabsToSpaces( const QString& str, int tabSize );
    QList<int> tabColumnOffsets( const QString& str, int tabSize );


    /// This method calculates 2 intersections between 2 ranges.
    /// @param exclusive if it is exclusive then an overlap will not be included. In other words:
    ///			- Inclusive:  end < begin
    ///			- Exclusive:  end <= begin
    /// @param resultBegin a pointer to the variable receiving the result
    /// @param resultEnd a pointer to the variable receiving the result
    /// @return false => no overlap, true => overlap
    template<typename T>
    bool intersection( T begin1, T end1, T begin2, T end2, bool exclusive=false, T* resultBegin=0, T* resultEnd=0 )
    {
        if( exclusive ) {
            if( end1 <= begin2 ) return false;
            if( end2 <= begin1 ) return false;
        } else {
            if( end1 < begin2 ) return false;
            if( end2 < begin1 ) return false;
        }

        // assign the result
        if( resultBegin ) { *resultBegin = qMax(begin1,begin2);  }
        if( resultEnd ) { *resultEnd = qMin(end1,end2); }
        return true;
    }
};

} // edbee
