/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "rangesetlineiterator.h"

#include "edbee/models/textdocument.h"
#include "edbee/models/textrange.h"

#include "edbee/debug.h"

namespace edbee {

/// Constructs the rangeset line iterator
RangeSetLineIterator::RangeSetLineIterator( TextRangeSet* rangeSet )
    : rangeSetRef_(rangeSet)
    , rangeIndex_(0)
    , rangeEndLine_(-1)
    , curLine_(-1)
{
    findNextLine();
}


/// Checks if there's a next line number available
bool RangeSetLineIterator::hasNext() const
{
    return curLine_ >= 0;
}


/// returns the next line number
int RangeSetLineIterator::next()
{
    int result = curLine_;
    findNextLine();
    return result;
}


/// finds the next line number
void RangeSetLineIterator::findNextLine()
{
    // current line isn't at the end of the current range
    if( curLine_ < rangeEndLine_ ) {
        ++curLine_;

    // the curLine_ is finished with the current range
    } else {

        // when the rangeset is finishedd
        while( rangeIndex_ < rangeSetRef_->rangeCount() ) {
            TextRange& range = rangeSetRef_->range(rangeIndex_);
            ++rangeIndex_;

            // get the start/end lines
            int startLine = rangeSetRef_->textDocument()->lineFromOffset( range.min() );
            int endLine = rangeSetRef_->textDocument()->lineFromOffset( range.max() );
            if( startLine == curLine_ ) { ++startLine; }

            // still in range?
            if( startLine <= endLine ) {
                curLine_ = startLine;
                rangeEndLine_ = endLine;
                return;
            }
        }
        curLine_ = -1;  // done
    }
}


} // edbee
