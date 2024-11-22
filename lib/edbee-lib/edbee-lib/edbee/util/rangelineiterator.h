/**
 * Copyright 2011-2014 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"


namespace edbee {


class TextDocument;
class TextRange;

/// Implements a line iterator for a single range
/// It iterates over all affected lines that are inside the given textrange
///
/// Warning you should not change the document in such way that the linecount changes
/// Doing this will result in incorrect behavior of this iterator
///
/// Usage sample:
/// @code{.cpp}
///
/// RangeLineIterator itr( &controller->textSelection().range(0) )
/// while( itr.hasNext() ) {
///     qDebug() << "Line: " << itr.next();
/// }
///
/// @endcode
class EDBEE_EXPORT RangeLineIterator {
public:
    RangeLineIterator( TextDocument* doc, const TextRange& range );
    RangeLineIterator( TextDocument* doc, int start, int end );

    bool hasNext() const;
    int next();

private:
    int curLine_;              ///< The current line number
    int endLine_;              ///< The last line
};


} // edbee
