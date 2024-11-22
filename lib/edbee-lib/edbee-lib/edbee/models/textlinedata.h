/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QObject>

#include "edbee/util/gapvector.h"

namespace edbee {


enum TextLineDataPredefinedFields {
    LineTextScopesField=0,
//    LineDataMarkers,      /// Bookmarks etc
    PredefinedFieldCount=1
};


class Change;
class TextLineDataManager;



/// A text line item reference
class EDBEE_EXPORT TextLineData {
public:
    TextLineData() {}
    virtual ~TextLineData() {}
//    bool undoable() = 0;
};

/// a simple class to store a QString in a line
template<typename T>
class EDBEE_EXPORT BasicTextLineData : public TextLineData
{
public:
    BasicTextLineData( const T& val ) : value_(val) {}
    T value() { return value_; }
    void setValue( const T& value ) { value_ = value; }

private:
    T value_;
};

typedef BasicTextLineData<QString> QStringTextLineData;


//-------

/// the line data items
class EDBEE_EXPORT TextLineDataList {
public:
    TextLineDataList( );
    virtual ~TextLineDataList();
    virtual void destroy( TextLineDataManager* manager );

    void give( TextLineDataManager* manager, int field, TextLineData *dataItem );
    TextLineData* take( TextLineDataManager* manager, int field );
    TextLineData* at( TextLineDataManager* manager, int field );

    void realloc( TextLineDataManager* manager, int oldFieldPerLine, int newFieldsPerLine );

private:
    TextLineData** lineDataList_;   ///< The text line data items
};

//-------

/// This manager manages all line definitions
class EDBEE_EXPORT TextLineDataManager : public QObject
{
Q_OBJECT

public:
    TextLineDataManager( int fieldsPerLine = PredefinedFieldCount);
    virtual ~TextLineDataManager();

    void clear();


    void give(int line, int field, TextLineData *dataItem );
    TextLineData* take(int line, int field );
    TextLineData* get( int line, int field );

    /// returns the number of items per line
    int fieldsPerLine() { return fieldsPerLine_; }
    void setFieldsPerLine( int count );

    /// returns the number of items
    int length() const { return textLineDataList_.length(); }
    /// returns the textline data list item
    TextLineDataList* at(int idx) const { return textLineDataList_.at(idx); }

    // internal functions
    Change* createLinesReplacedChange( int lineStart, int lineCount, int newLineCount );
    TextLineDataList* takeList( int line );
    void giveList( int line, TextLineDataList* list );

    void fillWithEmpty( int line, int length, int newLength );
    void replace( int line, int length, TextLineDataList** items, int newLength );

    /// internal method for direct accesss
    GapVector<TextLineDataList*>* textLineDataList() { return &textLineDataList_; }

protected:

    void destroyRange( int line, int length );


public slots:

//    void linesReplaced( int lineStart, int lineCount, int newLineCount );
//    void dumpGapvector();
signals:

    void lineDataChanged( int line, int length, int newLength );   ///< This signal is emitted if line-data is changed

private:

    int fieldsPerLine_;                                  ///< The number of items per line
    GapVector<TextLineDataList*>  textLineDataList_;    ///< The textline data list
//    NoGapVector<TextLineDataList*>  textLineDataList_;
};


} // edbee
