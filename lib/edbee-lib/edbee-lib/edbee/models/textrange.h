/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QObject>
#include <QVector>

#include "edbee/models/textbuffer.h"

namespace edbee {

class TextDocument;

/// A single text region
/// A region constists of an anchor and a caret:
/// The anchor defines the 'start' of the range. Then caret the end.
///
/// Some definitions:
/// - caret == anchor  => length=0
///
/// samples  ( [ = anchor,  > = caret )
///   a[>bcdef    (anchor=1, caret=1)  => ""
///   a[b>cdef    (anchor=1, caret=2)  => "b"
///   a<b]cdef    (anchor=2, caret=1)  => "b"
///
class EDBEE_EXPORT TextRange {
public:
    TextRange( int anchor=0, int caret=0 ) : anchor_(anchor), caret_(caret) {}

    inline int anchor() const { return anchor_; }
    inline int caret() const { return caret_; }

    /// returns the minimal value
    inline int min() const { return qMin( caret_, anchor_ ); }
    inline int max() const { return qMax( caret_, anchor_ ); }

    /// returns the minimal variable reference
    inline int& minVar() { return caret_ < anchor_ ? caret_ : anchor_; }
    inline int& maxVar() { return caret_ < anchor_ ? anchor_: caret_; }

    inline int length() const { return qAbs(caret_ - anchor_ ); }

    void fixCaretForUnicode(TextDocument* doc, int direction );


    void setAnchor( int anchor ) { anchor_ = anchor; }
    void setAnchorBounded( TextDocument* doc, int anchor );
    void setCaret( int caret ) { caret_ = caret; }
    void setCaretBounded( TextDocument* doc, int caret );
    void setLength( int newLength );


    void set( int anchor, int caret ) { anchor_ = anchor; caret_ = caret; }

    void reset() { anchor_ = caret_; }
    bool hasSelection() const  { return anchor_ != caret_; }
    bool isEmpty() const { return anchor_==caret_; }
    void clearSelection() { caret_ = anchor_;  }

    QString toString() const;

    void moveCaret( TextDocument* doc, int amount );
    void moveCaretOrDeselect( TextDocument* doc, int amount );
    int moveWhileChar( TextDocument* doc, int pos, int amount, const QString& chars );
    int moveUntilChar( TextDocument* doc, int pos, int amount, const QString& chars );
    void moveCaretWhileChar( TextDocument* doc, int amount, const QString& chars );
    void moveCaretUntilChar( TextDocument* doc, int amount, const QString& chars );
    void moveAnchortWhileChar( TextDocument* doc, int amount, const QString& chars );
    void moveAnchorUntilChar( TextDocument* doc, int amount, const QString& chars );
    void moveCaretByCharGroup( TextDocument* doc, int amount, const QString& whitespace, const QStringList& characterGroups );
    void moveCaretToLineBoundary( TextDocument* doc, int amount, const QString& whitespace );
    void moveCaretToWordBoundaryAtOffset( TextDocument* doc, int offset );
    void moveCaretToLineBoundaryAtOffset( TextDocument* doc, int offset );


    void expandToFullLine( TextDocument* doc, int amount );
    void deselectTrailingNewLine( TextDocument* doc );
    void expandToWord( TextDocument* doc, const QString& whitespace, const QStringList& characterGroups );
    void expandToIncludeRange( TextRange& range );

    void forceBounds( TextDocument* doc );

    bool equals(const TextRange &range );
    bool touches( TextRange& range );
    bool contains( int pos );

    static bool lessThan( TextRange& r1, TextRange& r2 );

private:
    int anchor_;        ///< The position of the anchor
    int caret_;         ///< The position of the caret
};


//======================================================================


/// This abstract class represents a set of textranges
/// The ranges are kept ordered and will not contain overlapping regions.
///
/// Every method automatically orders and merges overlapping ranges.
/// Except when the changing_ flag is != 0. The sorting and merging only happens
/// when changing is 0. This way it possible to add/update muliple rages without the direct
/// performance hit of sorting and merging.
class EDBEE_EXPORT TextRangeSetBase {
public:
    TextRangeSetBase(TextDocument* doc);
    virtual ~TextRangeSetBase() {}
    //    TextRangeSet(const TextRangeSet& sel);

  // pure virtual methods
//    virtual TextRangeSetBase* clone() const = 0;
    //    TextRangeSet & operator=( const TextRangeSet& sel );

    virtual int rangeCount() const  = 0;
    virtual TextRange& range(int idx) = 0;
    virtual const TextRange& constRange(int idx) const = 0;
    virtual void addRange( int anchor, int caret ) = 0;
    virtual void addRange( const TextRange& range ) = 0;
    virtual void removeRange( int idx ) = 0;
    virtual void clear() = 0;
    virtual void toSingleRange() = 0;
    virtual void sortRanges() = 0;

    TextRange& lastRange();
    TextRange& firstRange();

    int rangeIndexAtOffset( int offset );
    bool rangesBetweenOffsets( int offsetBegin, int offsetEnd, int& firstIndex, int& lastIndex );
    bool rangesBetweenOffsetsExlusiveEnd( int offsetBegin, int offsetEnd, int& firstIndex, int& lastIndex );
    bool rangesAtLine( int line, int& firstIndex, int& lastIndex );
    bool rangesAtLineExclusiveEnd( int line, int& firstIndex, int& lastIndex );
    bool hasSelection();
    bool equals( TextRangeSetBase& sel );
    void replaceAll( const TextRangeSetBase& base );


    QString getSelectedText();
    QString getSelectedTextExpandedToFullLines();

    QString rangesAsString() const;

  // changing
    void beginChanges();
    void endChanges();
    void endChangesWithoutProcessing();
    bool changing() const;

    void resetAnchors();
    void clearSelection();

    void addTextRanges( const TextRangeSetBase& sel);
    void substractTextRanges( const TextRangeSetBase& sel );
    void substractRange( int min, int max );


  // selection
    void expandToFullLines(int amount);
    void expandToWords( const QString& whitespace, const QStringList& characterGroups);
    void selectWordAt(int offset , const QString& whitespace, const QStringList& characterGroups);
    void toggleWordSelectionAt( int offset, const QString& whitespace, const QStringList& characterGroups);

  // movement
    void moveCarets( int amount );
    void moveCaretsOrDeselect( int amount );
    void moveCaretsByCharGroup( int amount, const QString& whitespace, const QStringList& charGroups );
    void moveCaretsToLineBoundary(int direction, const QString& whitespace  );
    //    void moveCaretsByLine( int amount );  //< Impossible to do without view when having a flexible font, guess that's why it wasn't implemented


  // changing
    //    void growSelectionAtBegin( int amount );
    void changeSpatial(int pos, int length, int newLength, bool sticky=false, bool performDelete=false);

    void setRange( int anchor, int caret, int index = 0 );
    void setRange( const TextRange& range , int index = 0 );

    virtual void processChangesIfRequired(bool joinBorders=false);

  // getters
    TextDocument* textDocument() const;
    //TextBuffer* textBuffer() const { return textBufferRef_; }


    void mergeOverlappingRanges( bool joinBorders );

protected:

    TextDocument* textDocumentRef_;       ///< The reference to the textbuffer
    int changing_;                       ///< A (integer) boolean for handling changes between beginChagnes and endChanges
};


//======================================================================


/// The basic textrange class. A simple class of textrange with a simple vector implementation
class EDBEE_EXPORT TextRangeSet : public TextRangeSetBase
{
public:
    TextRangeSet( TextDocument* doc );
    TextRangeSet( const TextRangeSet& sel );
    TextRangeSet( const TextRangeSet* sel );
    virtual ~TextRangeSet() {}

    TextRangeSet& operator=(const TextRangeSet& sel);
    TextRangeSet* clone() const;

    virtual int rangeCount() const { return selectionRanges_.size(); }
    virtual TextRange& range(int idx);
    virtual const TextRange& constRange(int idx ) const;
    virtual void addRange(int anchor, int caret);
    virtual void addRange( const TextRange& range );
    virtual void removeRange(int idx);
    virtual void clear();
    virtual void toSingleRange();
    virtual void sortRanges();

private:

    QVector<TextRange> selectionRanges_;     ///< A list of selection ranges. After endChanges this array is sorted and non-overlapping!
};


//======================================================================


/// A smart QObject implemenation of a TextRangeSet which listens to changes
/// in the document. When a change happens it's changes the spatial of the ranges
///
/// The stickymode is used to change the behavior of the changes the the textChange event.
/// To put it simple, you should enable stickymode if this selection is the one you are
/// using to modify the document
///
/// The delete mode is used to tell the rangeset if 'deleted' ranges need to be deleted
/// or simply need to be moved
class EDBEE_EXPORT DynamicTextRangeSet : public QObject, public TextRangeSet
{
Q_OBJECT

public:
    DynamicTextRangeSet( TextDocument* doc, bool stickyMode=false, bool deleteMode=false, QObject* parent=0 );
    DynamicTextRangeSet( const TextRangeSet& sel, bool stickyMode=false, bool deleteMode=false, QObject* parent=0 );
    DynamicTextRangeSet( const TextRangeSet* sel, bool stickyMode=false, bool deleteMode=false, QObject* parent=0 );
    virtual ~DynamicTextRangeSet();

    void setStickyMode( bool mode );
    bool stickyMode() const;

    void setDeleteMode( bool mode );
    bool deleteMode() const;

public slots:
    void textChanged( edbee::TextBufferChange change, QString oldText = QString() );

private:
    bool stickyMode_;                       ///< Sticky mode means if this rangeset is the current selection (This requires a different approach)
    bool deleteMode_;                       ///< When delete mode is enabled ranges are deleted. If it's false ranges are moved to the left
};


} // edbee
