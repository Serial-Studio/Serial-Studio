/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textlinedata.h"

#include "edbee/models/changes/linedatalistchange.h"
#include "edbee/models/textlinedata.h"

#include "edbee/debug.h"

namespace edbee {

//-------

/// construct the text line data item
TextLineDataList::TextLineDataList()
    : lineDataList_(0)
{
}

TextLineDataList::~TextLineDataList()
{
    if( lineDataList_ ) {
//        Q_ASSERT(false);    /// YOU MUST call destroy before the destructor!
        qlog_warn() << "** Warning TextLineDataList requires manual destruction! ** ";
    }
}

/// destroys the data items
void TextLineDataList::destroy( TextLineDataManager* manager )
{
    if( lineDataList_ ) {

        for( int i=0, cnt=manager->fieldsPerLine(); i<cnt; ++i ) {
            delete lineDataList_[i];
            lineDataList_[i] = 0;
        }
    }
    delete[] lineDataList_;
    lineDataList_ = 0;
}



/// this method gives a given dat item to the given field position
void TextLineDataList::give(TextLineDataManager* manager, int field, TextLineData* dataItem)
{
    Q_ASSERT( field < manager->fieldsPerLine() );
    if( !lineDataList_ ) { lineDataList_ = new TextLineData*[ manager->fieldsPerLine() ](); }
    delete lineDataList_[field];    // delete the old item
    lineDataList_[field] = dataItem;
}


/// This method returns the given data item and transfers the ownership. The item in the list is set to 0
/// @param manager the manager to use
/// @param field the field index to retrieve
/// @return the TextLineData item in the given field or 0
TextLineData *TextLineDataList::take(TextLineDataManager* manager, int field)
{
    Q_ASSERT( field < manager->fieldsPerLine() );
    Q_UNUSED(manager);
    if( !lineDataList_ ) { return 0; }
    TextLineData* dataItem = lineDataList_[field];
    lineDataList_[field] = 0;
    return dataItem;
}

/// This method returns the given data item /
/// @param manager the manager to use
/// @param field the field index to retrieve
/// @return the TextLineData item in the given field or 0
TextLineData *TextLineDataList::at(TextLineDataManager* manager, int field)
{
    Q_ASSERT( field < manager->fieldsPerLine() );
    Q_UNUSED(manager);
    if( !lineDataList_ ) { return 0; }
    return lineDataList_[field];
}


/// This method reallocates the number of fields
void TextLineDataList::realloc(TextLineDataManager* manager , int oldFieldsPerLine, int newFieldsPerLine)
{
//qlog_info() << "realloc:" << oldFieldsPerLine << " >> " << newFieldsPerLine;
    TextLineData** oldData = lineDataList_;

    // copy the data
    if( oldData ) {
        lineDataList_ = new TextLineData*[ manager->fieldsPerLine() ]();
        int copyCount = qMin( oldFieldsPerLine, newFieldsPerLine );
        for( int i=0; i < copyCount; ++i ) {
            lineDataList_[i] = oldData[i];
            oldData[i] = 0;
        }

        for( int i=copyCount; i<oldFieldsPerLine; ++i ) {
            delete oldData[i];
            oldData[i] = 0;
        }
        delete[] oldData;
    }
}


//-------


TextLineDataManager::TextLineDataManager(int fieldsPerLine)
    : fieldsPerLine_( fieldsPerLine )
    , textLineDataList_(2)
{
    textLineDataList_.setGrowSize(2);
    // add a 0 element
    textLineDataList_.fill(0,0,0,1);
}

TextLineDataManager::~TextLineDataManager()
{
    clear();
}

/// this method clears all field items
void TextLineDataManager::clear()
{
    for( int i=0,cnt=textLineDataList_.length(); i < cnt; ++i  ) {
        TextLineDataList* list = textLineDataList_.at(i);
        if( list ) {
            list->destroy(this);
            delete list;
//            textLineDataList_.set(i,0);
        }
    }
    textLineDataList_.clear();
    textLineDataList_.fill(0,0,0,1);
}


/// this method gives the given data at the given line and field
void TextLineDataManager::give(int line, int field, TextLineData* dataItem)
{
    Q_ASSERT(field<fieldsPerLine_);
    TextLineDataList* list = textLineDataList_.at(line);
    if( !list ) { textLineDataList_.set(line, list = new TextLineDataList() ); }
    list->give( this, field, dataItem );
    emit lineDataChanged( line, 1, 1 );
}

TextLineData *TextLineDataManager::take(int line, int field)
{
    Q_ASSERT(field<fieldsPerLine_);
    TextLineDataList* list = textLineDataList_.at(line);
    if( list ) { return list->take( this, field ); }
    return 0;
}

TextLineData *TextLineDataManager::get(int line, int field)
{
    Q_ASSERT(field<fieldsPerLine_);
    TextLineDataList* list = textLineDataList_.at(line);
    if( list ) { return list->at( this, field ); }
    return 0;
}


/// This method is called to notify that some lines have been replaced
//void TextLineDataManager::linesReplaced(int lineStart, int lineCount, int newLineCount)
//{
//    Q_ASSERT(false);
///// TODO: use linedatalist-textchange for storing undo/redo operations
//qlog_info() << "TODO: We need to support destroy line data operation!";

/* WE MUST MAKE THIS  A TEXT CHANGE !!
    // remove all items
    for( int i=lineStart,end=lineStart+lineCount; i<end; ++i ){
        TextLineDataList* list = textLineDataList_.at(i);

        LineDataTextChange* linesRemoves

        if( list ) { list->destroy(this); }
        delete list;
    }

//    // remove all items
//    for( int i=lineStart,end=lineStart+lineCount; i<end; ++i ){
//        TextLineDataList* list = textLineDataList_.at(i);
//        if( list ) { list->destroy(this); }
//        delete list;
//    }
//
//  // replace all old items with 0 values
//  textLineDataList_.fill( lineStart, lineCount, 0, newLineCount );
*/
//}

//void TextLineDataManager::dumpGapvector()
//{
//    qlog_info() << "GAPVECTOR: ["<<textLineDataList_.gapBegin() << ","<<textLineDataList_.gapEnd()<<">";
//    for( int i=0; i < textLineDataList_.capacity(); ++i ) {
//        TextLineDataList* item = textLineDataList_.rawAt(i);
//        qlog_info() << "-" << i <<  ":" << QStringLiteral("%1").arg((quintptr)item);
//    }
//    qlog_info()<<"- done";

//}

/// This method can be used to change the number of reserved fields by the document
/// Increasing the amount will result in a realoc
/// Decreasting the fieldcount reults in the lost of the 'old' fields
/// At least the 'PredefinedFieldCount' amont of fields are required
void TextLineDataManager::setFieldsPerLine(int count)
{
    Q_ASSERT( count >= PredefinedFieldCount );
    for( int i=0,cnt=textLineDataList_.length(); i < cnt; ++i  ) {
        TextLineDataList* list = textLineDataList_.at(i);
        if( list ) {
            list->realloc(this, fieldsPerLine_, count );
        }
    }
    fieldsPerLine_ = count;
}

/// This method creates a new lines replace change
Change* TextLineDataManager::createLinesReplacedChange(int lineStart, int lineCount, int newLineCount )
{
    // no changes
    if( lineCount == 0 && lineCount == newLineCount ) { return 0; }

    LineDataListChange* change = new LineDataListChange(this,lineStart, lineCount, newLineCount );
    return change;
}


/// This method takes the given list (and repalces it with a 0 value)
TextLineDataList* TextLineDataManager::takeList(int line)
{
    TextLineDataList* list = textLineDataList_[line];
    textLineDataList_[line] = 0;
    return list;
}

/// This method gives a list to the given line
/// emits a lineDataChanged signal
void TextLineDataManager::giveList(int line, TextLineDataList *newList)
{
    // delete the old list if required
    TextLineDataList* list = textLineDataList_[line];
    if( list ) {
        list->destroy(this);
        delete list;
    }
    textLineDataList_[line] = newList;
    emit lineDataChanged( line, 1, 1 );
}

/// replace the given area with no-data
/// emits a lineDataChanged signal
void TextLineDataManager::fillWithEmpty(int line, int length, int newLength)
{

    destroyRange(line, length);
    textLineDataList_.fill( line, length, 0, newLength );
//qlog_info() << "- TextLineDataManager::fillWithEmpty( line: " << line << ", length " << length << " newLength: " << newLength << ") len:" << this->textLineDataList_.length();
    emit lineDataChanged( line, length, newLength );
}

/// Replace the given items with
/// emits a lineDataChanged signal
void TextLineDataManager::replace(int line, int length, TextLineDataList** items, int newLength )
{
    destroyRange(line, length);
    textLineDataList_.replace( line, length, items, newLength );
    emit lineDataChanged( line, length, newLength );
}

/// destroys all items in the given range
void TextLineDataManager::destroyRange(int line, int length)
{
    for( int i=0; i<length; i++ ) {
        TextLineDataList* list = takeList(line+i);
        if( list ) {
            list->destroy(this);
            delete list;
        }
    }
}


} // edbee
