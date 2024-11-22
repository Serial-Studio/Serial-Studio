/**
 * Copyright 2011-2014 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "rangelineiterator.h"

#include "edbee/models/textdocument.h"
#include "edbee/models/textrange.h"

#include "edbee/debug.h"

namespace edbee {


/// Constructs the range line iterator
/// @param doc the textdocument this iterator is used for
/// @param range the range to iterate over the lines
RangeLineIterator::RangeLineIterator(TextDocument* doc, const TextRange& range )
{
    curLine_ = doc->lineFromOffset( range.min() );
    endLine_ = doc->lineFromOffset( range.max() );
}

 /// Constructs a line iterator by supplying two offsets
/// @param doc the textdocument for this iterator
/// @param startOffset the start offset for iterating
/// @param endOffset the endOffset for iterating
RangeLineIterator::RangeLineIterator(TextDocument* doc, int startOffset, int endOffset )
{
    curLine_ = doc->lineFromOffset( startOffset );
    endLine_ = doc->lineFromOffset( endOffset );
}


/// Checks if there's a next line number available
bool RangeLineIterator::hasNext() const
{
    return curLine_ <= endLine_;
}


/// returns the next line number
int RangeLineIterator::next()
{
    int result = curLine_;
    ++curLine_;
    return result;
}


} // edbee
