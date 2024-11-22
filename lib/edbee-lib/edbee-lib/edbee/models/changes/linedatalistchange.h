/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QVector>

#include "edbee/models/changes/abstractrangedchange.h"

namespace edbee {

class TextLineDataManager;
class TextLineDataList;

/// A full line data text change. This means the growing or shrinking of the line data buffer
/// It stores the old-data list that needs to be remebered for undoing
class EDBEE_EXPORT LineDataListChange : public AbstractRangedChange
{
public:
    LineDataListChange( TextLineDataManager* manager, int offset , int lenght, int newLength );
    virtual ~LineDataListChange();

    virtual void execute(TextDocument* document);
    virtual void revert(TextDocument* doc);

    virtual void mergeStoredData(AbstractRangedChange* change);
    virtual bool giveAndMerge(TextDocument* document, Change* textChange );

    virtual QString toString();

    int offset() const;
    void setOffset( int value );

    virtual int docLength() const;
    void setDocLength( int value );

    virtual int storedLength() const;

    TextLineDataList** oldListList();
    int oldListListLength();

private:

    TextLineDataManager* managerRef_;         ///< A reference to the manager
    int offset_;                              ///< The line number start
    int docLength_;                           ///< The number of new items (they all will be 0)

    TextLineDataList** oldListList_;          /// The lists of items (old items)
    int contentLength_;                       ///< The number of elements in the oldListList
};

} // edbee
