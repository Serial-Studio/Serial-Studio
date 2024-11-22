/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include "edbee/models/change.h"

namespace edbee {

/// This is an abstract class for ranged changes
/// This are changes (text changes and line changes) that span a range in an array.
/// These ranges share a common alogrithm for performing merges, detecting overlaps etc.
class EDBEE_EXPORT AbstractRangedChange : public Change
{
public:
    virtual ~AbstractRangedChange();

    /// this method should return the offset of the change
    virtual int offset() const = 0;

    /// this method should set the offset
    virtual void setOffset( int value ) = 0;
    void addOffset( int amount );

    /// this method should set the old length
    virtual void setDocLength( int value ) = 0;

    /// this method should return the length in the document
    virtual int docLength() const = 0;

    /// this method should return the length of this item in memory
    virtual int storedLength() const = 0;


protected:

    /// implement this method to merge to old data. Sample implementation
    ///
    /// SingleTextChange* singleTextChange = dynamic_cast<SingleTextChange*>(change);
    /// QString newText;
    /// newText.resize( calculateMergeDataSize( change) );
    /// mergeData( newText.data(), text_.data(), singleTextChange->text_.data(), change, sizeof(QChar) );
    virtual void mergeStoredData( AbstractRangedChange* change ) = 0;


    int getMergedDocLength(AbstractRangedChange* change);
    int getMergedStoredLength(AbstractRangedChange* change);
    void mergeStoredDataViaMemcopy(void* targetData, void* data, void* changeData, AbstractRangedChange* change, int itemSize );
    bool merge( AbstractRangedChange* change );

public:
    bool isOverlappedBy( AbstractRangedChange* secondChange );
    bool isTouchedBy( AbstractRangedChange* secondChange );

};


} // edbee
