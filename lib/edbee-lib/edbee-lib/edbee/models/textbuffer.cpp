/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textbuffer.h"

#include "edbee/models/textrange.h"
#include "edbee/util/lineoffsetvector.h"

#include "edbee/debug.h"

namespace edbee {


/// Initializes the textbuffer change
/// @param buffer when buffer is 0 NO line calculation is done
TextBufferChangeData::TextBufferChangeData(TextBuffer* buffer, int off, int len, const QChar *text, int textlen)
    : offset_( off )
    , length_(len)
    , newText_(text)
    , newTextLength_(textlen)
    , newLineOffsets_()
{
    Q_ASSERT(buffer);

    // decide which lines
    line_         = buffer->lineFromOffset( offset_ );
    int endLine   = buffer->lineFromOffset( offset_ + length_ );
    lineCount_    = endLine - line_;
    Q_ASSERT(lineCount_>=0);

    // find the newlines in the text
    for( int i=0; i< newTextLength_; ++i ) {
        if( newText_[i] == '\n' ) {
            newLineOffsets_.append( offset_ + i + 1 );    // +1 because it points to the start of the next line
        }
    }
}

/// Initializes the textbuffer change
/// @param buffer when buffer is 0 NO line calculation is done
TextBufferChangeData::TextBufferChangeData(LineOffsetVector* lineOffsets, int off, int len, const QChar *text, int textlen)
    : offset_( off )
    , length_(len)
    , newText_(text)
    , newTextLength_(textlen)
    , newLineOffsets_()
{
    Q_ASSERT(lineOffsets );

    // decide which lines
    line_         = lineOffsets->findLineFromOffset( offset_ );
    int endLine   = lineOffsets->findLineFromOffset( offset_ + length_ );
    lineCount_    = endLine - line_;
    Q_ASSERT(lineCount_>=0);

    // find the newlines in the text
    for( int i=0; i< newTextLength_; ++i ) {
        if( newText_[i] == '\n' ) {
            newLineOffsets_.append( offset_ + i + 1 );    // +1 because it points to the start of the next line
        }
    }
}


TextBufferChange::TextBufferChange()
{
    d_ = new TextBufferChangeData( (TextBuffer*)0, 0, 0, 0, 0 );
}

TextBufferChange::TextBufferChange(TextBuffer* buffer, int off, int len, const QChar* text, int textlen)
{
    d_ = new TextBufferChangeData( buffer, off, len, text, textlen );
}

TextBufferChange::TextBufferChange(LineOffsetVector* lineOffsets, int off, int len, const QChar *text, int textlen)
{
    d_ = new TextBufferChangeData( lineOffsets, off, len, text, textlen );
}

TextBufferChange::TextBufferChange(const TextBufferChange& other) : d_(other.d_)
{
}



//=====================================================


/// The textbuffer constructor
TextBuffer::TextBuffer(QObject *parent)
    : QObject(parent)
{
}


/// Replaces the given text
/// @param offset the offset to replace
/// @param length the of the text to replace
/// @param text the new text to insert
void TextBuffer::replaceText(int offset, int length, const QString& text)
{
    replaceText( offset, length, text.data(), text.length() );
}


/// Returns the full text as a QString
QString TextBuffer::text()
{
     return textPart(0, length());
}


/// A convenient method for directly filling the textbuffer with the given content
void TextBuffer::setText(const QString& text)
{
    replaceText( 0, length(), text.data(), text.length() );
}


/// this method translates the given position to a column number.
/// @param offset the character offset
/// @param line the line index this position is on. (Use this argument for optimization if you already know this)
int TextBuffer::columnFromOffsetAndLine( int offset, int line  )
{
    if( line < 0 ) line = lineFromOffset( offset );
    // const QList<int>& lofs = lineOffsets();
    if( line < lineCount() ) {
        int col = offset - offsetFromLine(line);
        if( col < 0 ) return 0;
        return qMin( lineLength(line), col );
    } else {
        return 0;
    }
}


/// Appends the given text to the textbuffer
/// @param text the text to appendf
void TextBuffer::appendText(const QString& text)
{
    replaceText(length(),0,text.data(), text.length() );
}


/// This method returns the offset from the give line and column
/// If the column exceed the number of column the caret is placed just before the newline
int TextBuffer::offsetFromLineAndColumn(int line, int col)
{
    int offsetLine = offsetFromLine(line);
    int offsetNextLine = offsetFromLine(line+1);
    int offset = offsetLine + col;
    if( offset >= offsetNextLine && offset < length() ) { --offset; }
    return offset;
}


/// Returns the line at the given line position. This line INCLUDES the newline character (if it's there)
/// @param line the line to return
QString TextBuffer::line(int line)
{
    int off = offsetFromLine(line);
    int endOff = offsetFromLine(line+1);
    return textPart( off, endOff - off  ); // skip the return
}


/// Returns the line without the newline character
QString TextBuffer::lineWithoutNewline(int line)
{
    int off = offsetFromLine(line);
    int removeNewlineCount = 1;
    if( line == lineCount()-1 ) { removeNewlineCount = 0; }
    return textPart( off , offsetFromLine(line+1) - off - removeNewlineCount  ); // skip the return
}


/// Returns the length of the given line. Also counting the trailing newline character if present
/// @param line the line to retrieve the length for
/// @return the length of the given line
int TextBuffer::lineLength(int line)
{
    return offsetFromLine(line+1) - offsetFromLine(line);
}


/// Returns the length of the given line. Without counting a trailing newline character
/// @param line the line to retrieve the length for
/// @return the length of the given line
int TextBuffer::lineLengthWithoutNewline(int line)
{
    int removeNewlineCount = 1;
    if( line == lineCount()-1 ) { removeNewlineCount = 0; }
    int lastOffset = offsetFromLine(line+1) - removeNewlineCount;
    return  lastOffset - offsetFromLine(line);
}


/// replace the texts
/// @param range the range to replace
/// @param text the text to insert at the given location
void TextBuffer::replaceText( const TextRange& range, const QString& text )
{
    return replaceText( range.min(), range.length(), text.data(), text.length() );
}


/// See documentation at findCharPosWithinRange
/// @param offset the offset to start searching
/// @parm direction the direction (left < 0, or right > 0 )
/// @param chars the chars to search
/// @param equals when setting to true if will search for the first given char. When false it will stop when another char is found
int TextBuffer::findCharPos(int offset, int direction, const QString& chars, bool equals)
{
    return findCharPosWithinRange(offset, direction, chars, equals, 0, length() );

}


/// This method finds the find the first character position that equals the given char
///
/// @param offset the offset to search from. A negative offset means the CURRENT character isn't used
/// @param direction the direction to search. If the direction is multiple. the nth item is returned
/// @param chars the character direction
/// @param equals when setting to true if will search for the first given char. When false it will stop when another char is found
/// @param beginRange the start of the range to search in
/// @param endRange the end of the range to search in (exclusive)
/// @return the offset of the first character
int TextBuffer::findCharPosWithinRange(int offset, int direction, const QString& chars, bool equals, int beginRange, int endRange)
{
    int charStep      = direction < 0 ? -1 : 1;
    int charNumber    = qAbs(direction);

    while( beginRange <= offset && offset < endRange ) {
        if( chars.contains( charAt(offset) ) == equals ) {
            if( --charNumber <= 0 ) { return offset; }
        }
        offset += charStep;
    }
    return -1;
}


/// See documentation at findCharPosWithinRange.
/// This method searches a char position within the given rang (from the given ofset)
int TextBuffer::findCharPosOrClamp(int offset, int direction, const QString& chars, bool equals)
{
    return findCharPosWithinRangeOrClamp( offset, direction, chars, equals, 0, length() );
}


/// See documentation at findCharPosWithinRange.
/// This method searches a char position within the given rang (from the given ofset)
int TextBuffer::findCharPosWithinRangeOrClamp(int offset, int direction, const QString& chars, bool equals, int beginRange, int endRange)
{
    int pos = findCharPosWithinRange(offset, direction, chars, equals, beginRange, endRange);
    if( pos < 0 ) {
        if( direction < 0 ) return beginRange;
        if( direction > 0 ) return endRange;
    }
    return pos;
}



// This method converts the line offsets as a comma-seperated string (easy for debugging)
QString TextBuffer::lineOffsetsAsString()
{
    QString str;
    for( int idx=0,cnt=lineCount(); idx<cnt; ++idx  ) {
        int offset = offsetFromLine(idx);
        if( !str.isEmpty() ) str.append(',');
        str.append( QStringLiteral("%1").arg(offset) );
    }
    return str;
}


} // edbee
