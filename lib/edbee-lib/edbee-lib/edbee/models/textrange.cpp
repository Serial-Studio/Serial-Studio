/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include <QStringList>

#include "textdocument.h"
#include "textrange.h"
#include "texteditorconfig.h"
#include "edbee/debug.h"

namespace edbee {

/// This method compares selection ranges
bool TextRange::lessThan(TextRange &r1, TextRange &r2)
{
    return r1.min() < r2.min();
}


/// Makes sure the caret isn't in-between a unicode boundary
/// refs #19 - Dirty hack to improve caret-movement by skipping non-BMP characters
void TextRange::fixCaretForUnicode(TextDocument *doc, int direction)
{
    if( caret_ >= doc->length() ) return;

    // unicode-emoji hack (Really really dirty!!)
    int code = doc->charAt(caret_).unicode();
//qlog_info() << "fixCaretForUnicode: >> " << caret_ << " : " << code <<  "(" << direction << ")";
    if( 0xDC00 <= code && code <= 0xDFFF ) {
        if( direction > 0 ) {
            setCaretBounded( doc, caret_ + 1 );
        } else {
            setCaretBounded( doc, caret_ - 1 );
        }
    }
}

/// Sets the anchor to the given location, and forces the anchor to say between the document bounds
/// @param doc document to set the anchor for
/// @param anchor the anchor location to set
void TextRange::setAnchorBounded(TextDocument* doc, int anchor)
{
    setAnchor( qBound( 0,  anchor, doc->length() ) );
}


/// Sets the caret to the given location, and forces the caret to say between the document bounds
/// @param doc the document (used for checking the document bounds)
/// @param caret the caret position to set
void TextRange::setCaretBounded(TextDocument* doc, int caret)
{
    setCaret( qBound( 0,  caret, doc->length() ) );
}


/// Changes the length by modifying the max-variable
void TextRange::setLength(int newLength)
{
    int& vMin = minVar();
    int& vMax = maxVar();
    vMax = vMin + newLength;
}


/// This method converts a text-selection range to a string helpfull with debuggin
QString TextRange::toString() const
{
    return QStringLiteral("%1>%2").arg(anchor_).arg(caret_);
}


/// Moves the caret the given amount
/// @param doc the document to move the caret for
/// @param amount the amount to move
void TextRange::moveCaret(TextDocument* doc, int amount )
{
    setCaretBounded( doc, caret_ + amount );
    fixCaretForUnicode(doc,amount);
}


/// move the caret or deselect the given amount
/// @param doc the document to move the caret in
/// @param amount the amount to move
void TextRange::moveCaretOrDeselect(TextDocument* doc, int amount)
{
    // when there's a selection clear it (move the caret to the right side
    if( hasSelection() ) {
        if( amount < 0 ) {
            setCaret(min());
        } else {
            setCaret(max());
        }
        setAnchor( caret() );

    // just moves the caret
    } else {
        setCaretBounded( doc, caret_ + amount );
    }
    fixCaretForUnicode(doc,amount);
}


/// This method charactes after the given char group
/// When moving to the left the cursor is placed AFTER the last character
/// @param doc the text document
/// @param var the initial position
/// @param amount the amount to move
int TextRange::moveWhileChar(TextDocument* doc, int pos, int amount, const QString& chars)
{
    int docLength = doc->length();
    if( amount < 0 ) {
        --pos;   // first move left
        while( pos >= 0 && chars.indexOf( doc->charAt(pos) )>=0 ) { --pos; }
        ++pos;
    } else {
        while( pos <  docLength && chars.indexOf( doc->charAt(pos) )>=0 ) { ++pos; }
    }
    return pos;
}


/// This method charactes until the given chargroup is found
/// When moving to the LEFT the cursor is placed AFTER the found character
int TextRange::moveUntilChar(TextDocument* doc, int pos, int amount, const QString& chars)
{
    int docLength = doc->length();
    if( amount < 0 ) {
        --pos;
        while( pos >= 0 && chars.indexOf( doc->charAt(pos) )<0 ) { --pos; }
        ++pos;
    } else {
        while( pos <  docLength && chars.indexOf( doc->charAt(pos) )<0 ) { ++pos; }
    }
    return pos;
}


/// moves the caret while a character is moving
void TextRange::moveCaretWhileChar(TextDocument* doc, int amount, const QString& chars)
{
    caret_ = moveWhileChar(doc, caret_, amount, chars );
}


void TextRange::moveCaretUntilChar(TextDocument *doc, int amount, const QString& chars)
{
    caret_ = moveUntilChar(doc, caret_, amount, chars );
}


void TextRange::moveAnchortWhileChar(TextDocument *doc, int amount, const QString &chars)
{
    anchor_ = moveWhileChar(doc, anchor_, amount, chars );
}


void TextRange::moveAnchorUntilChar(TextDocument *doc, int amount, const QString &chars)
{
    anchor_ = moveWhileChar(doc, anchor_, amount, chars );
}


/// This method moves the caret to/from the given seperator
void TextRange::moveCaretByCharGroup(TextDocument *doc, int amount, const QString& whitespace, const QStringList& characterGroups )
{
    int count = qAbs(amount);

    for( int i=0; i<count; ++i ) {

        // first 'skip' the whitespaces
        int oldCaret = caret_;
        moveCaretWhileChar( doc, amount, whitespace );

        // find the character group
        QChar chr;
        if( amount < 0 ) {
            if( caret_ == 0 ) return;
            chr = doc->charAt( caret_-1 );
        } else {
            if( caret_ == doc->length() ) return;
            chr = doc->charAt( caret_ );
        }

        // newline is a special operation :(
        if( chr == '\n') {
            if( caret_ == oldCaret ) {
                caret_ += amount < 0 ? -1 : 1;
            }
        } else {

            // while the character is found
            bool found = false;
            for( int i=0,cnt= characterGroups.length(); i<cnt; ++i ) {
                const QString& group = characterGroups.at(i);
                if( group.indexOf(chr) >= 0 ) {
                    moveCaretWhileChar(doc,amount,group);
                    found = true;
                    break;
                }
            }

            // all other characters are valid
            if( !found ) {
                QString str = characterGroups.join("");
                str.append(whitespace);
                str.append('\n');
                moveCaretUntilChar(doc,amount,str);
            }
        }
    }
}


/// This moves the caret to a line boundary
/// @param doc the document this range operates on
/// @param amount the direction to move to
/// @par    am whitespace the characters that need to be interpreted as whitespace
void TextRange::moveCaretToLineBoundary(TextDocument* doc, int amount, const QString& whitespace )
{
    TextBuffer* buf     = doc->buffer();
    int caret           = caret_;
    int line            = doc->lineFromOffset( caret );
    int offsetNextLine  = doc->offsetFromLine( line + 1 );
    if( amount < 0 ) {
        int lineStart = doc->offsetFromLine( line );

        // find the first word
        int wordStart = buf->findCharPosWithinRangeOrClamp( lineStart, 1, whitespace, false, lineStart, offsetNextLine );
        if( caret > wordStart || lineStart ==  caret ) {
            caret = wordStart;
        } else {
            caret = lineStart;
        }
    } else {

        caret = offsetNextLine;
        if( line != doc->lineCount()-1 ) {
            --caret;
        }

    }
    setCaretBounded( doc, caret );
}

/// Moves the caret to a word boundary  (used for word dragging selections)
void TextRange::moveCaretToWordBoundaryAtOffset(TextDocument *doc, int newOffset)
{
    TextEditorConfig* config = doc->config();

    // left direction
    if( newOffset < anchor()) {
        setAnchor(max());
        setCaret(newOffset);
        moveCaretByCharGroup(doc, -1, config->whitespaces(), config->charGroups());
    // right direction
    } else {
        setCaret(newOffset);
        moveCaretByCharGroup(doc, 1, config->whitespaces(), config->charGroups());
    }
}

/// Moves the caret to a word boundary  (used for word dragging selections)
void TextRange::moveCaretToLineBoundaryAtOffset(TextDocument *doc, int newOffset)
{
    int firstLine = doc->lineFromOffset(min());
    int lastLine = doc->lineFromOffset(max());

    // changed offset
    setCaret(newOffset);

    int newLine = doc->lineFromOffset(newOffset);

    // left direction
    if( newLine < lastLine ) {
        this->caret_ = doc->offsetFromLine(newLine);
        this->anchor_ = doc->offsetFromLine(lastLine);
    // right direction
    } else if( newLine > firstLine) {
        this->anchor_ = doc->offsetFromLine(firstLine);
        this->caret_ = doc->offsetFromLine(newLine+1);
    }
}

/// Expands the selection range so it only consists of full lines
/// amount specifies the amount (and the direction) of the expansions
/// -1 means expand lines to top (and add extra lines)
/// 1 means expand lines at the bottom (and add extras lines)
/// 0 is a special case, it moves the caret to the start of the current line and expands to the end of the line. It does not add lines
void TextRange::expandToFullLine(TextDocument* doc, int amount)
{
    int minOffset = min();
    int maxOffset = max();

    // select the current line (everse caret
    if( amount == 0 ) {
        minOffset = doc->offsetFromLine( doc->lineFromOffset(minOffset) );
        maxOffset = doc->offsetFromLine( doc->lineFromOffset(maxOffset)+1);
        caret_ = minOffset;
        anchor_ = maxOffset;

    } else if( amount > 0 ) {
        minOffset = doc->offsetFromLine( doc->lineFromOffset(minOffset) );
        maxOffset = doc->offsetFromLine( doc->lineFromOffset(maxOffset)+amount );

        caret_ = maxOffset;
        anchor_ = minOffset;

    } else {

        int minLine = doc->lineFromOffset( minOffset );
        int minLineStartOffset = doc->offsetFromLine( minLine );

        // only select line above if the full line isn't selected yet
        if( minOffset == minLineStartOffset ) {
            if( minOffset > 0 ) { --minOffset; }
        }
        ++amount;
        minOffset = doc->offsetFromLine( doc->lineFromOffset(minOffset) + amount );

        // select to eol if required
        int maxLine = doc->lineFromOffset( maxOffset );
        int maxLineStartOffset = doc->offsetFromLine( maxLine );
        if( maxLineStartOffset != maxOffset ) {
            maxOffset = doc->offsetFromLine( doc->lineFromOffset(maxOffset)+1 );
        }

        caret_ = minOffset;
        anchor_ = maxOffset;

    }
}


/// This method deselects the last character if it's a newline.
void TextRange::deselectTrailingNewLine(TextDocument* doc)
{
    if( caret_ != anchor_ && doc->charAtOrNull(max()-1)=='\n' ) {
        --maxVar();
    }
}


/// Internal method, for finding a character group with the given character
static QString findCharGroup( QChar c, const QString& whitespace, const QStringList& characterGroups )
{
    QString foundGroup;
    if( whitespace.indexOf(c) >= 0) {
        foundGroup = whitespace;
    } else {
        for( int i=0,cnt=characterGroups.length(); i<cnt; ++i ) {
            if( characterGroups.at(i).indexOf(c) >=0 ) {
                foundGroup = characterGroups.at(i);
                break;
            }
        }
    }
    return foundGroup;
}

/// Expands the selection to a words
void TextRange::expandToWord(TextDocument *doc, const QString& whitespace, const QStringList& characterGroups )
{
    int& min = minVar();
    int& max = maxVar();

    // first check which character is under the caret to find the character grouop
    if( min > 0 ) {
        QString group = findCharGroup( doc->charAt(min-1), whitespace, characterGroups );
        if( group.isEmpty() ) {
            group = characterGroups.join("").append(whitespace);     // the 'else' group
            min = moveUntilChar(doc,min,-1,group);
        } else {
            min = moveWhileChar(doc,min,-1,group);
        }
    }

    if( max < doc->length() ) {
        QString group = findCharGroup( doc->charAt(max), whitespace, characterGroups );
        if( group.isEmpty() ) {
            group = characterGroups.join("").append(whitespace);     // the 'else' group
            max = moveUntilChar(doc,max,1,group);
        } else {
            max = moveWhileChar(doc,max,1,group);
        }
    }
}

void TextRange::expandToIncludeRange(TextRange& range)
{
    int& min = minVar();
    int& max = maxVar();
    min = qMin( min, range.min() );
    max = qMax( max, range.max() );
}

/// Sets the bounds
void TextRange::forceBounds(TextDocument *doc)
{
    setAnchorBounded( doc, anchor_ );
    setCaretBounded( doc, caret_ );
}

/// This method checks if the two ranges are equal
/// @param range the range to compare it to
bool TextRange::equals( const TextRange& range)
{
    return range.caret_ == caret_ && range.anchor_ == anchor_;
}

/// Checks if two ranges touch eachother. Touching means that the start and end of two ranges are onto eachother
/// Possible situations ( [ = anchor, > = caret, ( = don't care )
///  (A)(B) or (B)(A)
bool TextRange::touches(TextRange& range)
{
    int min1 = min();
    int max1 = max();
    int min2 = range.min();
    int max2 = range.max();
    return( max1 == min2 || max2 == min1 );
}

/// checks if the given position is in this textrange
bool TextRange::contains(int pos)
{
    return min() <= pos && pos < max();
}



//=========================================================================


TextRangeSetBase::TextRangeSetBase(TextDocument* doc )
    : textDocumentRef_( doc )
    , changing_(0)
{
}

/// this method returns the last range
TextRange &TextRangeSetBase::lastRange()
{
    return range( rangeCount()-1);
}

/// the first range
TextRange &TextRangeSetBase::firstRange()
{
    return range(0);
}


/// This method returns the range index at the given offset
/// @param offset the offset to check
/// @return the offset index or -1 if not found
int TextRangeSetBase::rangeIndexAtOffset(int offset)
{
    // find the range of this offset
    for( int i=0, cnt = rangeCount(); i<cnt; ++i ) {
        TextRange& found = this->range(i);
        int minOffset = found.min();
        int maxOffset = found.max();
        if( minOffset <= offset && offset <= maxOffset ) {
            return i;
        }
    }
    return -1;
}


/// returns the range indices that are being overlapped by the given offsetBegin and offsetEnd
/// @param offsetBegin the offset to search
/// @param offsetEnd the end-offset to search
/// @param firstIndex(out) The first index found (-1 if not found)
/// @param lastIndex(out) The last index found (-1 if not found)
/// @return true if the range is found
bool TextRangeSetBase::rangesBetweenOffsets( int offsetBegin, int offsetEnd, int& firstIndex, int& lastIndex )
{
    firstIndex = -1;
    lastIndex  = -1;
    /// Todo optimize with a binairy search
    for( int i=0, cnt = rangeCount(); i<cnt; ++i ) {
        TextRange& range = this->range(i);
        int minOffset = range.min();
        int maxOffset = range.max();

        if( (offsetBegin <= minOffset && minOffset <= offsetEnd) || (minOffset <= offsetBegin && offsetBegin <= maxOffset) ) {
            if( firstIndex < 0 ) firstIndex = i;
            lastIndex = i;
        }
    }
    return firstIndex>=0;
}

/// returns the range indices that are being overlapped by the given offsetBegin and offsetEnd
/// @param offsetBegin the offset to search
/// @param offsetEnd the end-offset to search
/// @param firstIndex(out) The first index found (-1 if not found)
/// @param lastIndex(out) The last index found (-1 if not found)
/// @return true if the range is found
bool TextRangeSetBase::rangesBetweenOffsetsExlusiveEnd(int offsetBegin, int offsetEnd, int &firstIndex, int &lastIndex)
{
    firstIndex = -1;
    lastIndex  = -1;
    /// Todo optimize with a binairy search
    for( int i=0, cnt = rangeCount(); i<cnt; ++i ) {
        TextRange& range = this->range(i);
        int minOffset = range.min();
        int maxOffset = range.max();

        if( (offsetBegin <= minOffset && minOffset < offsetEnd) || (minOffset <= offsetBegin && offsetBegin < maxOffset) ) {
            if( firstIndex < 0 ) firstIndex = i;
            lastIndex = i;
        }
    }
    return firstIndex>=0;
}


/// Returns the range indices that are being used on the given line
bool TextRangeSetBase::rangesAtLine(int line, int& firstIndex, int& lastIndex)
{
    TextDocument* doc = textDocument();
    int offsetBegin = doc->offsetFromLine(line);
    int offsetEnd   = doc->offsetFromLine(line+1)-1;
    return rangesBetweenOffsets( offsetBegin, offsetEnd, firstIndex, lastIndex );
}

/// Returns the range indices that are being used on the given line (Excluding the last offset)
bool TextRangeSetBase::rangesAtLineExclusiveEnd(int line, int &firstIndex, int &lastIndex)
{
    TextDocument* doc = textDocument();
    int offsetBegin = doc->offsetFromLine(line);
    int offsetEnd   = doc->offsetFromLine(line+1)-1;
    return rangesBetweenOffsetsExlusiveEnd( offsetBegin, offsetEnd, firstIndex, lastIndex );
}


/// This method checks if there's a selection available
/// A selection is an range with a different anchor then it's caret
bool TextRangeSetBase::hasSelection()
{
    for( int i=0, cnt = rangeCount(); i<cnt; ++i ) {
        if( range(i).hasSelection() ) return true;
    }
    return false;
}


/// This method checks if two selections are equal
bool TextRangeSetBase::equals( TextRangeSetBase& sel)
{
    if( sel.rangeCount() != rangeCount() ) return false;
    for( int i=rangeCount()-1; i>=0; --i ) {
        if( !range(i).equals( sel.range(i) ) ) return false;
    }
    return true;
}


/// Replaces all ranges with the supplied ranges
void TextRangeSetBase::replaceAll(const TextRangeSetBase& base)
{
    clear();
    for( int i=0,cnt=base.rangeCount(); i<cnt; ++i ) {
        addRange( base.constRange(i) );
    }

}


/// This method returns all selected text
/// For every filled selection range a line is returned
QString TextRangeSetBase::getSelectedText()
{
    TextDocument* doc = textDocument();
    QString buffer;
    for( int i=0, cnt=rangeCount(); i<cnt; ++i ) {
        TextRange& range = this->range(i);
        if( range.hasSelection() ) {
            buffer.append( doc->textPart( range.min(), range.length() ) );
            buffer.append("\n");
        }
    }
    if( !buffer.isEmpty() ) {
        buffer.remove( buffer.length()-1, 1);  // remove the last(newline character)
    }
    return buffer;
}


/// Returns ALL lines that are touched by the selection. This means
/// Full lines are always returned
QString TextRangeSetBase::getSelectedTextExpandedToFullLines()
{
    TextDocument* doc = textDocument();
    QString buffer;
    int lastLine = -1;
    for( int i=0, cnt=rangeCount(); i<cnt; ++i ) {
        TextRange& range = this->range(i);
        int min = range.min();
        int max = range.max();
        int line = doc->lineFromOffset(min);
        int maxLine = doc->lineFromOffset(max);

        // skip the current line if it's the same as last one
        if( line == lastLine ) { ++line; }
        while( line <= maxLine ) {
            buffer.append( doc->lineWithoutNewline(line) );
            buffer.append("\n");
            ++line;
        }
        lastLine = line;
    }
    return buffer;
}


/// This method converts the selection ranges as a string, in the format:   anchor>caret,anchor>caret
QString TextRangeSetBase::rangesAsString() const
{
    QString str;
    for( int i=0, cnt=rangeCount(); i<cnt; ++i ) {
        const TextRange& range = constRange(i);
        if( !str.isEmpty() ) str.append(",");
        str.append( range.toString() );

    }
    return str;
}



/// This method resets all anchors to the positions of the carets
void TextRangeSetBase::resetAnchors()
{
    for( int i=0, cnt=rangeCount(); i<cnt; ++i ) {
        TextRange& range = this->range(i);
        range.reset();
    }
}

/// This method moves all carets to the anchor positions
void TextRangeSetBase::clearSelection()
{
    for( int i=0, cnt=rangeCount(); i<cnt; ++i ) {
        TextRange& range = this->range(i);
        range.clearSelection();
    }
}


/// This method starts the changes
void TextRangeSetBase::beginChanges()
{
    Q_ASSERT(changing_ < 10 );  // 10 times nesting is a LOT!!
    ++changing_;
}


void TextRangeSetBase::endChanges()
{
    Q_ASSERT(changing_ > 0);
    --changing_;
    processChangesIfRequired();
}


/// Ends the changes without processing.
/// WARNING, you should ONLY call this method if the operation you performed kept the rangeset
/// in a valid state. This (at least) means the ranges need to be sorted
void TextRangeSetBase::endChangesWithoutProcessing()
{
    Q_ASSERT(changing_ > 0);
    --changing_;
}


/// Checks if the current rangeset is in a changing state
bool TextRangeSetBase::changing() const
{
    return changing_ != 0;
}


/// An union operation
/// This method adds all text selection-items.
/// Merges all ranges
void TextRangeSetBase::addTextRanges(const TextRangeSetBase& sel)
{
    for( int i=0,cnt=sel.rangeCount(); i<cnt; ++i ) {
        addRange( sel.constRange(i) );
    }
    processChangesIfRequired(true );
}


/// The difference operation
/// This method substracts the text-selection from the current selection
void TextRangeSetBase::substractTextRanges(const TextRangeSetBase& sel)
{
    ++changing_;
    for( int i=0,cnt=sel.rangeCount(); i<cnt; ++i ) {
        const TextRange& r = sel.constRange(i);
        substractRange(r.min(), r.max());
    }
    --changing_;
    processChangesIfRequired();
}


/// This method substracts a single range from the ranges list
void TextRangeSetBase::substractRange(int minB, int maxB)
{
    beginChanges();
    for( int i=rangeCount()-1; i >=0; --i ) {
        TextRange& rangeItem = range(i);
        int& minA = rangeItem.minVar();
        int& maxA = rangeItem.maxVar();

        // A: [             ]
        // B:     [XXXXX]
        // =  [   ]     [   ]
        if( ( minA < minB && minB < maxA ) && (minA<maxB && maxB < maxA ) )
        {
            /// TODO Add support for copyrange
            addRange( maxB, maxA );
            maxA = minB;
        }
        // A:     [    ]
        // B: [XXXXXXXXXXXXX]
        // =:    (leeg)
        else if( ( minB <= minA && minA <= maxB ) && ( minB <= maxA && maxA <= maxB ) )
        {
            removeRange(i);
        }

        // A: [     ]
        // B:    [XXXXXX]
        // =: [  ]
        else if( minA < minB && minB < maxA )
        {
            maxA = minB;
        }
        // A:     [     ]
        // B: [XXXXXX]
        // =:        [  ]
        else if( minA < maxB && maxB < maxA )
        {
            minA = maxB;
        }
    }
    endChanges();
}


/// Expands the selection so it selects full lines
void TextRangeSetBase::expandToFullLines( int amount )
{
    for( int i=0, cnt=rangeCount(); i<cnt; ++i ) {
        range(i).expandToFullLine( textDocument(), amount );
    }
    processChangesIfRequired();

}


/// Expands the selection to full words
void TextRangeSetBase::expandToWords(const QString& whitespace, const QStringList& characterGroups)
{
    for( int i=0, cnt=rangeCount(); i<cnt; ++i ) {
        range(i).expandToWord( textDocument(), whitespace, characterGroups );
    }
    processChangesIfRequired();
}


/// Selects the word at the given offset
/// @param offset the offset of the word to select
void TextRangeSetBase::selectWordAt(int offset, const QString& whitespace, const QStringList& characterGroups )
{
    TextRange newRange(offset,offset);
    newRange.expandToWord( textDocument(), whitespace, characterGroups );
    addRange(newRange);
    processChangesIfRequired();
}


/// Toggles a word selection at the given location
/// The idea is the following, double-click an empty place to select the word at the given location
/// Double click an existing selection to remove the selection (and caret)
void TextRangeSetBase::toggleWordSelectionAt(int offset, const QString& whitespace, const QStringList& characterGroups)
{
    int idx = rangeIndexAtOffset( offset );

    // range found
    if( idx >= 0) {
        TextRange& found = this->range(idx);

        // found a range with selection
        if( found.hasSelection() ) {

            // when multiple range, just remove the range
            if( rangeCount() > 1 ) {
                removeRange(idx);
                return;

            // when this is the last range, just place the caret
            } else {
                found.set(offset,offset);
                return;
            }
        }
    }
    // default operation is select word at
    selectWordAt( offset, whitespace, characterGroups );
}

/// This method moves the carets by character
void TextRangeSetBase::moveCarets( int amount )
{
    for( int i=0, cnt=rangeCount(); i<cnt; ++i ) {
        range(i).moveCaret( textDocument(), amount);
    }
    processChangesIfRequired();
}


/// This method moves the carets or deslects the given character
void TextRangeSetBase::moveCaretsOrDeselect(int amount)
{
    for( int i=0, cnt=rangeCount(); i<cnt; ++i ) {
        range(i).moveCaretOrDeselect( textDocument(), amount);
    }
    processChangesIfRequired();
}


/// This method moves the carets
void TextRangeSetBase::moveCaretsByCharGroup(int amount, const QString& whitespace, const QStringList& characterGroups )
{
    for( int rangeIdx=rangeCount()-1; rangeIdx >= 0; --rangeIdx ) {
        range(rangeIdx).moveCaretByCharGroup( textDocument(), amount, whitespace, characterGroups );
    }
    processChangesIfRequired();
}


/// Moves all carets to the given line boundary (line-boundary automatically switches between column 0 and first non-whitespace character)
/// @param direction the direction < 0 to the start of the line (or first char)  > 0 to the end of the line
/// @param whitespace the characters to see as whitespace
void TextRangeSetBase::moveCaretsToLineBoundary(int direction , const QString& whitespace)
{
    // process all carets
    for( int rangeIdx=rangeCount()-1; rangeIdx >= 0; --rangeIdx ) {
        range(rangeIdx).moveCaretToLineBoundary(textDocument(), direction, whitespace );
    }
    processChangesIfRequired();
}


/// This method merges overlapping ranges
/// @param joinBorders if joinborders is set, borders next to eachother are also interpretted as overlap. (inclusive/exclusive switch)
void TextRangeSetBase::mergeOverlappingRanges( bool joinBorders )
{
    // check the ranges
    beginChanges();
    for( int i=rangeCount()-1; i>=0; --i ) {
        TextRange& range1 = range(i);
        int min1 = range1.min();
        int max1 = range1.max();

        // check overlap with all other ranges
        for( int j=i-1; j>=0; --j ) {
            TextRange& range2 = range(j);
            int min2 = range2.min();
            int max2 = range2.max();

            // Overlappping possibilities:
            // 1: [        ]
            // 2    [XXX]
            // =  (delete 1)
            if( min1 <= min2 && max2 <= max1 ) {
                removeRange(j);
                --i;
                continue;
            }

            // Overlappping possibilities:
            // 1:     [   ]
            // 2    [XXXXXXX]
            // =  (delete 1)
            if( min2 <= min1 && max1 <= max2 ) {
                removeRange(i);
                //selectionRanges_.remove(i);
                break;
            }

            // Overlappping possibilities:
            // 1:       [        ]
            // 2    [XXXXXX]
            // =  [ min2, max1]     (update range 2, delete range 1 )
            if(
               ( min2 < min1 && min1 < max2 && max2 < max1 )
               || ( joinBorders && min2 <= min1 && min1 <= max2 && max2 <= max1 )
            )
            {
                range2.maxVar() = max1;
                //selectionRanges_.remove(i);
                removeRange(i);
                break;
            }

            // Overlappping possibilities:
            // 1:   [        ]
            // 2         [XXXXXXX]
            // =  [ min1, max2]     (update range 2, delete range 1 )
            if(
               ( min1 < min2 && min2 < max1 && max1 < max2 )
               || ( joinBorders && min1 <= min2 && min2 <= max1 && max1 <= max2 )
            )
            {
                range2.minVar() = min1;
                //selectionRanges_.remove(i);
                removeRange(i);
                break;
            }
        }
    }
    endChanges();
}





/// This method sets the first range item
/// @param anchor the anchor of the selection
/// @param caret the caret position
/// @param index the default range index (default 0)
void TextRangeSetBase::setRange(int anchor, int caret, int index)
{
    range(index).set( anchor, caret );
}


/// Sets the range at the given index.
/// Make sure the given index exists!!
/// @param the range to use
/// @param index the index of the range to change (when not given index 0 is assumed)
void TextRangeSetBase::setRange(const TextRange& range, int index )
{
    setRange( range.anchor(), range.caret(), index );
}



/// This method process the changes if required
void TextRangeSetBase::processChangesIfRequired(bool joinBorders )
{
    if( !changing_ ) {
        ++changing_; // prevent changing by functions below:
        mergeOverlappingRanges(joinBorders);
        sortRanges();
        --changing_;
    }
}


/// Returns the associated textdocument
/// @return the associated text document
TextDocument*TextRangeSetBase::textDocument() const
{
    return textDocumentRef_;
}


/// This method adds (or removes) the given spatial length at the given location.
///
/// It adjusts all locations, anchors with the given locations. It automatically moves carets,
/// 'removes' selection etc.
///
/// @param pos the position to add the spatial length to
/// @param length the length of the text that's changed
/// @param newLength the new length of the text
/// @param sticky, when sticky the caret/anchor is sticky and isn't moved if the change happens at the same location
void TextRangeSetBase::changeSpatial(int pos, int length, int newLength, bool sticky , bool performDelete)
{
    int stickyDelta = sticky ? 0 : -1;

    // change the ranges
    int endPos = pos + length;
    int delta = newLength - length;
    int newEndPos = endPos + delta;
    beginChanges();
    for( int i=rangeCount()-1; i>=0; --i ) {

        TextRange& range = this->range(i);

        int& min = range.minVar();
        int& max = range.maxVar();

        // cut 'off' the endpos
        if( pos <= min && min < endPos ) {
            min = endPos;
            // when the range becomes invalid simply remove it
            if( min > max ) {
                if( performDelete ) {
                    removeRange(i);
                    continue;
                } else {
                    range.set(pos,pos);
                    continue;
                }
            }
        }

        // anchor
        if( pos < min && min < endPos ) {
            min = newEndPos;
        } else if( min > (pos+stickyDelta) ) {
            min  += delta;
        }

        // caret
        if( pos < max && max < endPos ) {
            max  = newEndPos;
        } else if( max > (pos+stickyDelta) ) {
            max  += delta;
        }

        range.set( min, max );
    }
    endChanges();
}


// =====================================


/// Constructs a textrange set
/// @param doc the document for this rangeset
TextRangeSet::TextRangeSet(TextDocument* doc)
    : TextRangeSetBase( doc )
{
}


/// the copy constructor for copying a selection
/// @param sel the ranges to copy
TextRangeSet::TextRangeSet( const TextRangeSet& sel )
    : TextRangeSetBase( sel.textDocument() )
{
    selectionRanges_ = sel.selectionRanges_;
}


// An operation to copy the selection with a pointer
TextRangeSet::TextRangeSet( const TextRangeSet* sel )
    : TextRangeSetBase( sel->textDocument() )
{
    selectionRanges_ = sel->selectionRanges_;
}


/// copy the value from the rangeset
TextRangeSet& TextRangeSet::operator=(const TextRangeSet& sel)
{
    textDocumentRef_ = sel.textDocumentRef_;
    selectionRanges_ = sel.selectionRanges_;
    return *this;
}


///// This method clones the text selection
TextRangeSet *TextRangeSet::clone() const
{
    return new TextRangeSet( *this );
}


/// returns the selection range
TextRange& TextRangeSet::range(int idx)
{
    Q_ASSERT( idx >= 0 );
    Q_ASSERT( idx < selectionRanges_.size() );
    return selectionRanges_[idx];
}


/// Returns a const reference from the
const TextRange &TextRangeSet::constRange(int idx) const
{
    Q_ASSERT( idx >= 0 );
    Q_ASSERT( idx < selectionRanges_.size() );
    return selectionRanges_.at(idx);
}


/// Adds a text range
void TextRangeSet::addRange(int anchor, int caret)
{
    selectionRanges_.append( TextRange( anchor, caret ) );
    processChangesIfRequired();
}



/// adds a range
void TextRangeSet::addRange(const TextRange& range)
{
    addRange( range.anchor(), range.caret() );
}



/// this method removes the range
void TextRangeSet::removeRange(int idx)
{
    selectionRanges_.remove(idx);
    processChangesIfRequired();
}


/// This method removes ALL carets except the 'global' selection
void TextRangeSet::clear()
{
//    selectionRanges_.remove(1,selectionRanges_.size()-1);
//    selectionRanges_[0].reset();
    selectionRanges_.clear();
}


/// Converts the range to a single range selection
void TextRangeSet::toSingleRange()
{
    selectionRanges_.remove(1,selectionRanges_.size()-1);
}


/// Sorts the ranges
void TextRangeSet::sortRanges()
{
    std::sort(selectionRanges_.begin(), selectionRanges_.end(), TextRange::lessThan);
}


// =====================================



/// Constructs the dynamic textrange set with a document
/// @param doc the document to use
/// @param stickyMode is the stickymode enabled
/// @param parent the parent owner
DynamicTextRangeSet::DynamicTextRangeSet(TextDocument* doc, bool stickyMode, bool deleteMode, QObject* parent)
    : QObject(parent)
    , TextRangeSet(doc)
    , stickyMode_( stickyMode )
    , deleteMode_( deleteMode )
{
    connect( textDocument(), &TextDocument::textChanged, this, &DynamicTextRangeSet::textChanged );
}


/// Constructs the dynamic textrange set with an existing rangeset
/// @param sel the text range to copy
/// @param stickyMode is the stickymode enabled
/// @param parent the parent owner
DynamicTextRangeSet::DynamicTextRangeSet(const TextRangeSet& sel, bool stickyMode, bool deleteMode, QObject* parent)
    : QObject(parent)
    , TextRangeSet(sel)
    , stickyMode_( stickyMode )
    , deleteMode_( deleteMode )
{
    connect( textDocument(), &TextDocument::textChanged, this, &DynamicTextRangeSet::textChanged );
}


/// constructs the dynamic rangeset
/// @param sel the text range to copy
/// @param stickyMode is the stickymode enabled
/// @param parent the parent owner
DynamicTextRangeSet::DynamicTextRangeSet(const TextRangeSet* sel, bool stickyMode, bool deleteMode, QObject* parent)
    : QObject(parent)
    , TextRangeSet(sel)
    , stickyMode_( stickyMode )
    , deleteMode_( deleteMode )
{
    connect( textDocument(), &TextDocument::textChanged, this, &DynamicTextRangeSet::textChanged );
}


/// destructs the reviever
DynamicTextRangeSet::~DynamicTextRangeSet()
{
    disconnect();
}


/// Is the stickymode enabled
/// @param mode the mode that is sticky
void DynamicTextRangeSet::setStickyMode(bool mode)
{
    stickyMode_ = mode;
}


/// returns the current stickymode
bool DynamicTextRangeSet::stickyMode() const
{
    return stickyMode_;
}


/// Sets the delete mode
void DynamicTextRangeSet::setDeleteMode(bool mode)
{
    deleteMode_ = mode;
}


/// Returns the current delete mode
bool DynamicTextRangeSet::deleteMode() const
{
    return deleteMode_;
}


/// This method is notified if a change happens to the textbuffer
void DynamicTextRangeSet::textChanged(edbee::TextBufferChange change, QString oldText)
{
    Q_UNUSED(oldText)
    changeSpatial( change.offset(), change.length(), change.newTextLength(), stickyMode_, deleteMode_ );
}



} // edbee
