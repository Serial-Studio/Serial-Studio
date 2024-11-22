/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textdocument.h"

#include <QStringList>

#include "edbee/models/changes/mergablechangegroup.h"
#include "edbee/models/changes/linedatachange.h"
#include "edbee/models/changes/selectionchange.h"
#include "edbee/models/changes/textchange.h"
#include "edbee/models/changes/textchangewithcaret.h"

#include "edbee/models/textlinedata.h"
#include "edbee/models/textdocumentfilter.h"
#include "edbee/models/textrange.h"
#include "edbee/models/textundostack.h"

#include "edbee/debug.h"

namespace edbee {


/// Constructs the textdocument
TextDocument::TextDocument( QObject* obj )
    : QObject(obj)
    , documentFilter_(nullptr)
    , documentFilterRef_(nullptr)
    , textLineDataManager_(nullptr)
{
    textLineDataManager_ = new TextLineDataManager();
}


/// Destroys the textdocument
TextDocument::~TextDocument()
{
    delete textLineDataManager_;
    delete documentFilter_;
}


/// This method can be used to change the number of reserved fields by the document
/// Increasing the amount will result in a realoc
/// Decreasting the fieldcount  reults in the lost of the 'old' fields
/// At least the 'PredefinedFieldCount' amont of fields are required
/// This method EMPTIES the undo-stack. So after this call all undo history is gone!
void TextDocument::setLineDataFieldsPerLine( int count )
{
    Q_ASSERT( count >= PredefinedFieldCount );
    lineDataManager()->setFieldsPerLine( count );
    textUndoStack()->clear();
}

void TextDocument::giveLineDataManager(TextLineDataManager *manager)
{
    delete textLineDataManager_;
    textLineDataManager_ = manager;
}


/// This method gives a given data item to a text line
void TextDocument::giveLineData(int line, int field, TextLineData* dataItem)
{
    Q_ASSERT(line < lineCount() );
    LineDataChange* change = new LineDataChange( line, field );
    change->giveLineData( dataItem );
    executeAndGiveChange( change, true );
}


/// Returns the line specific data at the given line
/// @param line the line number to retrieve the line data for
/// @param field the field to retrieve the data for
/// @return TextLineData the associated line data
TextLineData* TextDocument::getLineData(int line, int field)
{
    int len = lineDataManager()->length();
    Q_ASSERT( len == lineCount() );
    Q_ASSERT( line < len );
    return lineDataManager()->get( line, field );
}


/// Starts an undo group
/// @param group the textchange group that groups the undo operations
void TextDocument::beginUndoGroup(ChangeGroup* group)
{
    if( !group ) {
        group = new ChangeGroup(nullptr);
    }
//    if( documentFilter() ) {
//        documentFilter()->filterBeginGroup( this, group );
//    }
    textUndoStack()->beginUndoGroup(group);
}


/// Ends the current undo group
/// @param coalesceId the coalesceId
/// @param flatten should the operation be flatten (flattens undo-group trees)
void TextDocument::endUndoGroup( int coalesceId, bool flatten)
{
//    if( documentFilter() ) {
//        TextChangeGroup* group = textUndoStack()->currentGroup();
//        documentFilter()->filterEndGroup( this, group, coalesceId, flatten );
//    }
    textUndoStack()->endUndoGroup(coalesceId,flatten);
}


/// Ends the undo group and discards all recorded information
/// Warning it does NOT undo all made changes!!!
void TextDocument::endUndoGroupAndDiscard()
{
    textUndoStack()->endUndoGroupAndDiscard();
}


/// this method return true if the undo stack is enabled
bool TextDocument::isUndoCollectionEnabled()
{
    return textUndoStack()->isCollectionEnabled();
}


/// Enables or disables the collection of undo commands
void TextDocument::setUndoCollectionEnabled(bool enabled)
{
    textUndoStack()->setCollectionEnabled(enabled);
}


/// This method should return true if the current change is the cause of an undo operation
bool TextDocument::isUndoRunning()
{
    return textUndoStack()->isUndoRunning();
}


/// Checks if currently an undo operation is running
bool TextDocument::isRedoRunning()
{
    return textUndoStack()->isRedoRunning();
}


/// Is it an undo or redo (which means all commands area already available)
bool TextDocument::isUndoOrRedoRunning()
{
    return isUndoRunning() || isRedoRunning();
}


/// Checks if the document is in a persited state
bool TextDocument::isPersisted()
{
    return textUndoStack()->isPersisted();
}


/// Calc this method to mark current state as persisted
void TextDocument::setPersisted(bool enabled)
{
    textUndoStack()->setPersisted(enabled);
}


/// Sets the document filter without tranfering the ownership
void TextDocument::setDocumentFilter(TextDocumentFilter* filter)
{
    delete documentFilter_;
    documentFilter_  = 0;
    documentFilterRef_ = filter;
}


/// this method sets the document filter
/// You can give a 0 pointer to delte the old filter!
void TextDocument::giveDocumentFilter(TextDocumentFilter* filter)
{
    delete documentFilter_;
    documentFilter_ = filter;
    documentFilterRef_ = filter;
}


/// This method returns the document filter
TextDocumentFilter* TextDocument::documentFilter()
{
    return documentFilterRef_;
}


/// Start the changes
void TextDocument::beginChanges(TextEditorController* controller)
{
    beginUndoGroup( new MergableChangeGroup( controller) );
}


/// Replaces the given rangeset
void TextDocument::replaceRangeSet(TextRangeSet& rangeSet, const QString& textIn, bool stickySelection)
{
    return replaceRangeSet(rangeSet, QStringList(textIn), stickySelection);
}


/// replaces the given rangeset
void TextDocument::replaceRangeSet(TextRangeSet& rangeSet, const QStringList& textsIn, bool stickySelection)
{

    QStringList texts = textsIn;
    if( documentFilter() ) {
        documentFilter()->filterReplaceRangeSet( this, rangeSet, texts );
    }

    rangeSet.beginChanges();
    int delta = 0;
    int idx = 0, oldRangeCount = 0;
    while( idx < (oldRangeCount = rangeSet.rangeCount())  ) {
        TextRange& range = rangeSet.range(idx);
        QString text = texts.at(idx%texts.size());  // rotating text-fetching

        //qlog_info() << idx << ">> " << text << " : delta="<<delta<<", " << range.toString();
        range.setCaret( range.caret() + delta );
        range.setAnchor( range.anchor() + delta );
        //qlog_info() << "  => " << range.toString();
        delta += (text.length() - range.length());


        TextChangeWithCaret* change = new TextChangeWithCaret(range.min(),range.length(),text,-1);
        Change* effectiveChange = executeAndGiveChange( change, false );
        TextChangeWithCaret* effectiveChangeWithCaret = dynamic_cast<TextChangeWithCaret*>(effectiveChange);

        // when a new caret position is supplied (can only happen via a TextDocumentFilter)
        // access it and change the caret to the given position
        int caret = 0;
        if( effectiveChangeWithCaret && effectiveChangeWithCaret->caret() >= 0 ) {
          caret = effectiveChangeWithCaret->caret();
        // Default caret location is change-independent: old location + length new text
        } else {
          caret = range.min() + text.length();
        }

        // sticky selection, keeps the selection around the text
        if(stickySelection) {
          range.set(range.min(), caret);
        } else {
          range.setCaret(caret);
          range.reset();
        }

        // next range
        if( rangeSet.rangeCount() < oldRangeCount ) {
qlog_info() <<  "TEST TO SEE IF THIS REALLY HAPPENS!! I think it cannot happen. (but I'm not sure)";
Q_ASSERT(false);

        // else we stay at the same location
        } else {
            ++idx;
        }

    }
    rangeSet.endChanges();
}


/// sets the selectioin for the current rangeset
/// The selection may never be empty
/// @param controller the controller to given the selection for
/// @param rangeSet the rangeset with the new selection
void TextDocument::giveSelection( TextEditorController* controller,  TextRangeSet* rangeSet)
{
    Q_ASSERT(rangeSet->rangeCount()>0);

    SelectionChange* selChange = new SelectionChange( controller );
    selChange->giveTextRangeSet( rangeSet );
    executeAndGiveChange( selChange, 0 );
}


//// end the changes
void TextDocument::endChanges(int coalesceId)
{
    endUndoGroup(coalesceId,true);
}


/// call this method to execute a change. The change is first passed to the filter
/// so the documentFilter can handle the processing of the change
/// When not filter is active the 'execute' method is called on the change
///
/// WARNING, you should never Access the change pointer given to this method
/// It's possible the change gets deleted
///
/// This method  should return the effective text-change (or 0 if no text-change has been stored)
Change *TextDocument::executeAndGiveChange(Change* change, int coalesceId )
{
    if( documentFilter() ) {
        return documentFilter()->filterChange( this, change, coalesceId );
    } else {

        beginUndoGroup();   // automatically group changes together (when changes happen on emission)
        change->execute( this );
        Change* result = giveChangeWithoutFilter( change, coalesceId );
        Q_UNUSED(result)
        endUndoGroup(coalesceId, true);
        return textUndoStack()->last();
//        return result;
    }
}


/// Appends the given text to the document
/// @param text the text to append
/// @param coalesceId (default 0) the coalesceId to use. Whe using the same number changes could be merged to one change. CoalesceId of 0 means no merging
void TextDocument::append(const QString& text, int coalesceId )
{
    replace( this->length(), 0, text, coalesceId );
}


/// Appends the given text
/// @param text the text to append
/// @param coalesceId (default 0) the coalesceId to use. Whe using the same number changes could be merged to one change. CoalesceId of 0 means no merging
void TextDocument::replace( int offset, int length, const QString& text, int coalesceId )
{
    executeAndGiveChange( new TextChange( offset, length, text ), coalesceId);
}


/// Changes the compelte document text
/// @param text the new document text
void TextDocument::setText(const QString& text)
{
    replace( 0, length(), text, 0 );
}


/// begins the raw append modes. In raw append mode data is directly streamed
/// to the textdocument-buffer. No undo-data is collected and no events are fired
void TextDocument::rawAppendBegin()
{
    setUndoCollectionEnabled(false); // no undo's
    buffer()->rawAppendBegin();
}


/// When then raw appending is done. The events are fired that the document has been changed
/// The undo-collection is enabled again
void TextDocument::rawAppendEnd()
{
    buffer()->rawAppendEnd();
    setUndoCollectionEnabled(true);
}


/// Appends a single char in raw append mode
void TextDocument::rawAppend(QChar c)
{
    buffer()->rawAppend(c);
}


/// Appends an array of characters
void TextDocument::rawAppend(const QChar* chars, int length)
{
    buffer()->rawAppend(chars,length);
}


/// Returns the length of the document in characters
/// default implementation is to forward this call to the textbuffer
int TextDocument::length()
{
    return buffer()->length();
}


/// Returns the number of lines
int TextDocument::lineCount()
{
    return buffer()->lineCount();
}


/// Returns the character at the given position
QChar TextDocument::charAt(int idx)
{
    return buffer()->charAt(idx);
}


/// returns the char at the given index if the index is valid
/// else the null character is returned
///
/// @param idx the index to retrieve
/// @return the character at the given index or the null-character
QChar TextDocument::charAtOrNull(int idx)
{
    if( 0 <= idx && idx < length() ) {
        return charAt(idx);
    }
    return QChar();
}


/// Retrieves the character-offset of the given line
/// @param line the line number (0-based) to retrieve the offset for
/// @return the character offset
int TextDocument::offsetFromLine(int line)
{
    return buffer()->offsetFromLine(line);
}


/// returns the line number which contains the given offset
/// @param offset the character offset
/// @return the line number (0 is the first line )
int TextDocument::lineFromOffset(int offset)
{
    return buffer()->lineFromOffset(offset);
}


/// return the column position for the given offset and line
/// @param offset the offset position
/// @param line the line number which contains this offset. (When -1 the line number is calculated)
/// @return the column position of the given offset
int TextDocument::columnFromOffsetAndLine(int offset, int line)
{
    return buffer()->columnFromOffsetAndLine(offset,line);
}


/// Returns the character offset of the given line and column
/// @param line the line number
/// @param column the column position
/// @return the character offset in the document
int TextDocument::offsetFromLineAndColumn(int line, int column)
{
    return buffer()->offsetFromLineAndColumn(line,column);
}


/// Returns the length of the given line
/// @param line the line number
/// @return the line length
int TextDocument::lineLength(int line)
{
    return buffer()->lineLength(line);
}


/// returns the length of the given lilne without the newline
/// @param line the line number
/// @return the length of the line excluding the newline character
int TextDocument::lineLengthWithoutNewline(int line)
{
    return buffer()->lineLengthWithoutNewline(line);
}


/// Returns the document text as a QString
/// @return the complete document context
QString TextDocument::text()
{
    return buffer()->text();
}


/// Returns the given part of the text
/// @param offset the character offset in the document
/// @param length the length of the part in characters
/// @return the text at the given positions
QString TextDocument::textPart(int offset, int length)
{
    return buffer()->textPart( offset, length );
}


/// Returns the given line without the trailing \n character
/// @param line the line number to retrieve the data for
/// @return the content at the given line
QString TextDocument::lineWithoutNewline(int line)
{
    return buffer()->lineWithoutNewline(line);
}


//// Returns the contents at the given line inclusive the trailing \n character
/// @pparam line the line number to retrieve
/// @return the line at the given position
QString TextDocument::line(int line)
{
    return buffer()->line(line);
}


} // edbee
