/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

namespace edbee {

class TextRangeSet;

/// a rangeset line iterator. For iterating over ALL distinct lines
/// used in a rangeset.
///
/// Usage sample:
/// @code{.cpp}
///
/// RangeSetLineIterator itr( controller->textSelection() )
/// while( itr.hasNext() ) {
///     qDebug() << "Line: " << itr.next();
/// }
///
/// @endcode
class EDBEE_EXPORT RangeSetLineIterator {
public:
    RangeSetLineIterator( TextRangeSet* rangeSet );

    bool hasNext() const;
    int next();

private:
    void findNextLine();

private:
    TextRangeSet* rangeSetRef_;     ///< a reference to the range sets
    int rangeIndex_;                ///< the current range index
    int rangeEndLine_;              ///< the last line of the current range
    int curLine_;                   ///< The current line number (this value is -1 if there's no current line)


};

} // edbee
