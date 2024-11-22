/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QVector>

#include "gapvector.h"


namespace edbee {

class TextBufferChange;



/// This class implements the vector for storing the line numbers at certain offsets/
/// The class allows the 'gap' position to contain a delta offset. Which means that
///
/// all offsets after the gap are increased with the offsetDelta when searched. Inserting/deleting
/// text this way usually only results in the changine of the offset delta. Which means speeeed
///
/// The line offset pointed at by each index is the first character in the given line.
class EDBEE_EXPORT LineOffsetVector {
public:
    /// a structure to describe the line change that happend
//    struct LineChange {
//        int line;                   ///< the first line that's going to be replaced
//        int lineCount;              ///< the number of lines that are replaced
//        int newLineCount;           ///< the number of lines added/removed
//        int offsetDelta;            ///< the change in offset
//        QVector<int> newOffsets;    ///< the new offsets
//    };


    LineOffsetVector();

    void applyChange( TextBufferChange change );

    int at( int idx ) const;
    int length() const;

    int findLineFromOffset( int offset );

    int offsetDelta() { return offsetDelta_; }
    int offsetDeltaIndex() { return offsetDeltaIndex_; }

    void appendOffset( int offset );

    /// TODO: temporary method (remove)
    GapVector<int> offsetList() { return offsetList_; }

protected:
    int searchOffsetIgnoringOffsetDelta(int offset, int org_start, int org_end );

    void moveDeltaToIndex( int index );
    void changeOffsetDelta( int index, int delta );

public:
    QString toUnitTestString();
    void initForUnitTesting( int offsetDelta, int offsetDeltaIndex, ... );


private:

    GapVector<int> offsetList_;    ///< All offsets
    int offsetDelta_;              ///< The offset delta at the given offset index
    int offsetDeltaIndex_;         ///< The index that contains the offset delta


friend class LineOffsetVectorTest;


};

} // edbee
