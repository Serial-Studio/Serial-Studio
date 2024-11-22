/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QObject>
#include <QVector>
#include <QSharedData>
#include <QExplicitlySharedDataPointer>

namespace edbee {

class TextBuffer;
class TextBufferChange;
class TextRange;
class TextLineData;
class LineOffsetVector;


class EDBEE_EXPORT TextBufferChangeData : public QSharedData
{
public:
    TextBufferChangeData( TextBuffer* buffer, int off, int len, const QChar* text, int textlen );
    TextBufferChangeData( LineOffsetVector* lineOffsets, int off, int len, const QChar* text, int textlen );

    // text information
    int offset_;             ///< The offset in the buffer
    int length_;             ///< The number of chars to replaced
    const QChar* newText_;   ///< The reference to a new text
    int newTextLength_;      ///< The length of this text

    // line informationm
    int line_;                    ///< The line number were the change occured
    int lineCount_;               ///< the number of lines that are involved.
    QVector<int> newLineOffsets_; ///< A list of new line offset

};


/// This clas represents a text buffer change and is used to pass around between events
/// This is a shareddata object so the data can be thrown between different threads (delayed emit-support)_
/// TODO: Still problematic maybe the QChar* text pointer. It is possible that this pointer is being freed.
class EDBEE_EXPORT TextBufferChange {
public:
    TextBufferChange();
    TextBufferChange( TextBuffer* buffer, int off, int len, const QChar* text, int textlen );
    TextBufferChange( LineOffsetVector* lineOffsets, int off, int len, const QChar* text, int textlen );
    TextBufferChange( const TextBufferChange& other );

    int offset() const { return d_->offset_; }
    int length() const { return d_->length_; }
    const QChar* newText() const  { return d_->newText_; }
    int newTextLength() const { return d_->newTextLength_; }
    int line() const { return d_->line_; }
    int lineCount() const { return d_->lineCount_; }
    inline int newLineCount() const { return d_->newLineOffsets_.size(); }
    const QVector<int>& newLineOffsets() const { return d_->newLineOffsets_; }


private:
    QExplicitlySharedDataPointer<TextBufferChangeData> d_;
};

/// This class represents the textbuffer of the editor
class EDBEE_EXPORT TextBuffer : public QObject
{
Q_OBJECT

public:
    TextBuffer( QObject* parent = 0);

// Minimal abstract interface to implement

    /// should return the number of 'characters'.
    virtual int length() const = 0;

    /// A method for returning a single char
    virtual QChar charAt( int offset ) const = 0;

    /// return the given text.
    virtual QString textPart( int offset, int length ) const = 0;

    /// this method should replace the given text
    /// And fire a 'text-replaced' signal    
    virtual void replaceText( int offset, int length, const QChar* buffer, int bufferLength ) = 0;

    /// this method should return an array with all line offsets. A line offset pointsto the START of a line
    /// So it does NOT point to a newline character, but it points to the first character AFTER the newline character
    virtual int lineCount() = 0; // { return lineOffsets().length(); }
    virtual int lineFromOffset(int offset ) = 0;
    virtual int offsetFromLine( int line ) = 0;

// raw loading methods

    /// This method starts raw appending
    virtual void rawAppendBegin() = 0;

    /// this method should append the given character to the buffer
    virtual void rawAppend( QChar c ) = 0;

    /// This method should raw append the given character string
    virtual void rawAppend( const QChar* data, int dataLength ) = 0;

    /// the end raw append method should bring the document in a consistent state and
    /// emit the correct "replaceText" signals
    ///
    /// WARNING the textAboutToBeReplaced signals are given but at that moment the text is already replaced
    /// And the newlines are already added to the newline list!
    virtual void rawAppendEnd() = 0;

    /// This method returns the raw data buffer.
    /// WARNING this method CAN be slow because when using a gapvector the gap is moved to the end to make a full buffer
    /// Modifying the content of the data will mess up the line-offset-vector and other dependent classes. For reading it's ok :-)
    virtual QChar* rawDataPointer() = 0;


// easy functions

    /// Replace the given text.
    virtual void replaceText( int offset, int length, const QString& text );

    QString text();
    void setText( const QString& text );
    virtual int columnFromOffsetAndLine( int offset, int line=-1 );
    virtual void appendText( const QString& text );
    virtual int offsetFromLineAndColumn( int line, int col );
    virtual QString line( int line);
    virtual QString lineWithoutNewline( int line );

    virtual int lineLength(int line);
    virtual int lineLengthWithoutNewline(int line);
    virtual void replaceText( const TextRange& range, const QString& text  );

    virtual int findCharPos( int offset, int direction, const QString& chars, bool equals );
    virtual int findCharPosWithinRange( int offset, int direction, const QString& chars, bool equals, int beginRange, int endRange );
    virtual int findCharPosOrClamp( int offset, int direction, const QString& chars, bool equals );
    virtual int findCharPosWithinRangeOrClamp( int offset, int direction, const QString& chars, bool equals, int beginRange, int endRange );

    virtual QString lineOffsetsAsString();

 signals:

    void textAboutToBeChanged( edbee::TextBufferChange change );
    void textChanged( edbee::TextBufferChange change, QString oldText = QString() );

};

} // edbee

// needs to be OUTSIDE the namespace!!
Q_DECLARE_METATYPE(edbee::TextBufferChange)


