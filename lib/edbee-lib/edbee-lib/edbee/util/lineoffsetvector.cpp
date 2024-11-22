/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "lineoffsetvector.h"

#include "edbee/models/textbuffer.h"

#include "edbee/debug.h"

namespace edbee {


LineOffsetVector::LineOffsetVector()
    : offsetList_(16)
    , offsetDelta_(0)
    , offsetDeltaIndex_(0)
{
    int v=0;
    offsetList_.replace(0,0,&v,1);
}


void LineOffsetVector::applyChange(TextBufferChange change)
{
//qlog_info() << "== [ applyChange ] =================";

    int line = change.line() + 1;    // line 0 may NEVER be replaced!

    // repace the lines
    moveDeltaToIndex( line );


//QString offsets;
//for( int i=0; i<change.newLineOffsets().size(); ++i ) {
//    if( i ) { offsets.append(","); }
//    offsets.append( QStringLiteral("%1").arg(change.newLineOffsets().at(i)));
//    //qlog_info() << "-- " << i << ":" << change.newLineOffsets().at(i);
//}


//qlog_info() << "- before: " << toUnitTestString() ;
//qlog_info() << "- offsetList_.replace(" << change.line() << "," << change.lineCount() << ": " << offsets << ")";
    offsetList_.replace( line, change.lineCount(), change.newLineOffsets().constData(), change.newLineCount() );
//qlog_info() << "- after: " << toUnitTestString();


    // Add the delta to all the lines
    int endLine = line + change.newLineCount();


    if( offsetDeltaIndex_ < endLine ) {
        offsetDeltaIndex_ = endLine;
    }

    Q_ASSERT(offsetDeltaIndex_ >= 0);
    Q_ASSERT(offsetDeltaIndex_ <= this->length() );

//qlog_info() <<"- offsetDeltaindex:" << offsetDeltaIndex_;

    changeOffsetDelta( endLine, change.newTextLength() - change.length() );
//qlog_info() << "end: " << toUnitTestString();
//Q_ASSERT(this->length() !=5 );
}



/*
/// This method is should be called when text is replaced. This method
/// updates all elements of the split vector and calculates all offsets
/// @param offset the offset to change
/// @param length the number of chars that are replaced
/// @param text the new text
void LineOffsetVector::textReplaced(int offset, int length, const QChar *newData , int newDataLength)
{
    LineChange change;
    prepareChange(offset,length, newData, newDataLength, &change);
    applyChange( &change );

}
*/

/// this method returns the line offset at the given line offset
int LineOffsetVector::at(int idx) const
{
    Q_ASSERT( idx < length() );
    if( idx >= offsetDeltaIndex_ ) {
        return offsetList_.at(idx) + offsetDelta_;
    }
    return offsetList_.at(idx);
}

int LineOffsetVector::length() const
{
    return offsetList_.length();
}


/// this method searches the line from the given offset
int LineOffsetVector::findLineFromOffset(int offset)
{

    if( offset == 0 ) return 0;
    int offsetListLength = offsetList_.length();
    int offsetAtDelta = 0;
    if( offsetDeltaIndex_ < offsetListLength ) {
        offsetAtDelta = offsetList_.at(offsetDeltaIndex_) + offsetDelta_;
    } else {
        offsetAtDelta = offsetList_.at( offsetListLength-1);
    }

    // only binary search the left part
    int line = 0;
    if( offset < offsetAtDelta ) {
        line = searchOffsetIgnoringOffsetDelta( offset, 0, offsetDeltaIndex_ );

    // binary search the right part
    } else {
        line = searchOffsetIgnoringOffsetDelta( offset - offsetDelta_, offsetDeltaIndex_, offsetListLength );
    }   
    return line;

}

/// This method appends an offset to the end of the list
/// It simply applies the current offsetDelta because it will always be AFTER the current offsetDelta
void LineOffsetVector::appendOffset(int offset)
{
    this->offsetList_.append( offset - offsetDelta_ );
}

/// This method returns the offset to string
QString LineOffsetVector::toUnitTestString()
{
    QString s;
    for( int i=0,cnt=offsetDeltaIndex_; i<cnt; ++i ) {
        if( i != 0 ) { s.append(","); }
        s.append( QStringLiteral("%1").arg( offsetList_[i] ) );
    }
    s.append( QStringLiteral("[%1>").arg( offsetDelta_ ) );
    for( int i=offsetDeltaIndex_,cnt=offsetList_.length(); i<cnt; ++i ) {
        if( i != offsetDeltaIndex_ ) { s.append(","); }
        s.append( QStringLiteral("%1").arg( offsetList_[i] ) );
    }
    return s;
}

/// initializes the construct for unit testing
/// @param offsetDelta the offsetDelta to use
/// @param offsetDeltaIndex the offsetDeltaIndex to use
/// @param a list of integer offsets. Close the list with -1 !!!
void LineOffsetVector::initForUnitTesting(int offsetDelta, int offsetDeltaIndex, ... )
{
    offsetDelta_ = offsetDelta;
    offsetDeltaIndex_ = offsetDeltaIndex;
    offsetList_.clear();

    va_list offsets;
    va_start( offsets, offsetDeltaIndex );
    int val = va_arg ( offsets, int );
    while( val >= 0 ) {
        offsetList_.append( val);
        val = va_arg ( offsets, int );
    }
    va_end(offsets);
}



/// This method returns the line from the start position. This method uses
/// a binary search. Warning this method uses RAW values from the offsetList it does NOT
/// take in account the offsetDelta
int LineOffsetVector::searchOffsetIgnoringOffsetDelta(int offset, int org_start, int org_end )
{
//    const QList<int>& starts = lineOffsets_;
    // This search happens in binary below
    //
    //    for( int i=0; i < starts.length(); ++i ) {
    //        if( starts[i] > offset ) return i-1;
    //    }
    //    return starts.length() - 1;

//    int org_start = 0;
//    int org_end   = starts.length();

    int begin = org_start;
    int end   = org_end-1;

    if( begin == end ) return begin;


    int half = end;
    int r = 0;
    while( begin <= end  ) {

        // BINAIRY SEARCH:
        half = begin + ((end-begin)>>1);

        // compre the value
        r = offsetList_.at(half) - offset;
        if( r == 0 ) { return half; }
        if( r < 0 ) {
            begin = half + 1;
        } else {
            end = half -1;
        }
    }

    if( r > 0 ) {
        return half - 1;
    } else {
        if( half == org_end ) return org_end -1 ;
        return half;
    }

}

/// This method moves the delta to the given index
void LineOffsetVector::moveDeltaToIndex(int index)
{
    if( offsetDelta_ ) {

        // move to the right
        if( offsetDeltaIndex_ < index ) {
            for( int i=offsetDeltaIndex_; i<index; ++i ) {
                offsetList_[i] += offsetDelta_; // apply the delta
            }
        // move to the left
        } else if( index < offsetDeltaIndex_ ) {

            // In this situation there are 2 solution.
            // (1) Make the delta 0 or (2) apply the 'negative index' to the items on the left
            int length            = offsetList_.length();
            int delta0cost        = length - offsetDeltaIndex_;
            int negativeDeltaCost = offsetDeltaIndex_ - index;

            // prefer delta 0
            if( delta0cost <= negativeDeltaCost ) {
                for( int i=offsetDeltaIndex_; i < length; ++i ) {
                    offsetList_[i] += offsetDelta_;
                }
                offsetDelta_ = 0;
            // else apply the negative delta
            } else {
                for( int i=index; i < offsetDeltaIndex_; ++i ) {
                    offsetList_[i] -= offsetDelta_;
                }
            }
        }
    }
    offsetDeltaIndex_ = index;

}

/// This method moves the offset delta to the given location
void LineOffsetVector::changeOffsetDelta(int index, int delta)
{
    // there are 3 situations.
    // (1) The delta is at the exact location of the previous delta
    if( index == offsetDeltaIndex_ ) {
        offsetDeltaIndex_ = index;
        offsetDelta_ += delta;

    // (2) The new index is BEFORE the old index.
    } else if( index < offsetDeltaIndex_ ) {

        // apply the 'negative' offset to the given location
        for( int i=index; i < offsetDeltaIndex_; ++i ) {
            offsetList_[i] -= ( offsetDelta_ + delta );
        }
        offsetDeltaIndex_ = index;
        offsetDelta_ += delta;

    // (3) the index is after the old index.
    } else if( offsetDeltaIndex_ < index ) {

        // apply the old offsetsDelta to the indices before
        for( int i = offsetDeltaIndex_; i<index; ++i ) {
            offsetList_[i] += offsetDelta_;
        }
        offsetDeltaIndex_ = index;
        offsetDelta_ += delta;
    }

    // set the delta to 0 when at the end of the line
    if( offsetDeltaIndex_ == length() ) {
        offsetDelta_ = 0;
    }

}


} // edbee
