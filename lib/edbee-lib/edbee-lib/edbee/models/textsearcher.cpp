/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textsearcher.h"

#include "edbee/models/texteditorconfig.h"
#include "edbee/models/textrange.h"
#include "edbee/models/textbuffer.h"
#include "edbee/models/textdocument.h"
#include "edbee/texteditorwidget.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/util/regexp.h"
#include "edbee/views/textselection.h"

#include "edbee/debug.h"

namespace edbee {

TextSearcher::TextSearcher( QObject* parent )
    : QObject(parent)
    , searchTerm_()
    , syntax_(SyntaxPlainString)
    , caseSensitive_(false)
    , wrapAround_(true)
    , reverse_(false)
    , regExp_(nullptr)
{
}

/// destroys the textsearcher
TextSearcher::~TextSearcher()
{
    delete regExp_;
}

/// Returns the current active searchterm
QString TextSearcher::searchTerm() const
{
    return searchTerm_;
}


/// Sets the current search term
void TextSearcher::setSearchTerm(const QString &term)
{
    searchTerm_ = term;
    setDirty();
}

/// Returns the syntax type of the current search operation
TextSearcher::SyntaxType TextSearcher::syntax() const
{
    return syntax_;
}


/// Sets the syntax mode of the searcher
/// @parm syntax the SyntaxType to use (SyntaxPlainString, SyntaxRegExp)
void TextSearcher::setSyntax(TextSearcher::SyntaxType syntax)
{
    syntax_ = syntax;
    setDirty();
}


/// Returns the case sensitivity of the textsearcher
bool TextSearcher::isCaseSensitive() const
{
    return caseSensitive_;
}


/// sets the case sensitivity of the search operation
void TextSearcher::setCaseSensitive(bool sensitive)
{
    caseSensitive_ = sensitive;
    setDirty();
}


/// Should the search wrap around the contents of the document
bool TextSearcher::isWrapAroundEnabled() const
{
     return wrapAround_;
}


/// For changing the wrap around mode of the TextSearcher
void TextSearcher::setWrapAround(bool on)
{
    wrapAround_ = on;
    setDirty();
}


/// Checks the reverse mode
bool TextSearcher::isReverse() const
{
    return reverse_;
}


/// Sets the search reverse mode.
void TextSearcher::setReverse(bool on)
{
    reverse_ = on;
}


/// Finds the next matching textrange
/// This method does not alter the textrange selection. It only returns the range of the next match
/// @param selection the text-selection to use
/// @return the textRange with the found text.  TextRange::isEmpty() can be called to check if nothing has been found
TextRange TextSearcher::findNextRange(TextRangeSet* selection)
{
    TextDocument* document = selection->textDocument();
    QChar* buffer = document->buffer()->rawDataPointer();

    if( !regExp_ ) { regExp_ = createRegExp(); }

    int caretPos= 0;
    if( selection->rangeCount() > 0 ) {
        if( isReverse() ) {
            caretPos = selection->firstRange().min();
        } else {
            caretPos = selection->lastRange().max();
        }
    }

    int idx = 0;
    if( isReverse() ) {
        idx = regExp_->lastIndexIn(buffer,0,caretPos);
    } else {
        idx = regExp_->indexIn(buffer,caretPos,document->length());
    }

    // wrapped around? Let's try it from the beginning
    if( idx < 0 && isWrapAroundEnabled() ) {
        if( isReverse() ) {
            idx = regExp_->lastIndexIn(buffer,0,document->length());
        } else {
            idx = regExp_->indexIn(buffer,0,document->length());
        }
    }
    if( idx >= 0 ) {
        int len = regExp_->len(0);
        return TextRange(idx,idx+len);
    }
    return TextRange();
}


/// Finds the next item based on the given caret position in the selection
/// @param selection the selection to use for searching
/// @return true if the next selection has been foudn
bool TextSearcher::findNext( TextRangeSet* selection )
{
    TextRange range = findNextRange(selection);
    if( !range.isEmpty()) {
        selection->clear();
        selection->addRange(range.anchor(),range.caret());
        return true;
    }
    return false;
}


/// Finds the previous item based on the given caret position in the selection
/// @param selection the selection to use and alter
/// @return true if the previous item has been found false if not found
bool TextSearcher::findPrev( TextRangeSet* selection )
{
    reverse_ = !reverse_;
    bool result = findNext( selection );
    reverse_ = !reverse_;
    return result;
}


/// Selects the next item that matches the given critaria (Adds an extra selection range )
/// @param widget the widget to search in
/// @param selection the selection to use
/// @return true if the next item has been found false if it hasn't been.
bool TextSearcher::selectNext( TextRangeSet* selection )
{
    TextRange range = findNextRange(selection);
    if( !range.isEmpty()) {

        // edge case, when currently ony 1 caret is available and doesn't have a selection
        // we need to remove this range
        if( selection->rangeCount() == 1 && selection->range(0).isEmpty() ) {
            selection->removeRange(0);
        }
        selection->addRange( range.anchor(), range.caret() );
        return true;
    }
    return false;
}


/// Selects the previous item based on the given caret position in the selection
/// @param selection the selection to use
/// @return true if the selection has been found false if it hasn't been found
bool TextSearcher::selectPrev( TextRangeSet* selection )
{
    reverse_ = !reverse_;
    bool result = selectNext( selection );
    reverse_ = !reverse_;
    return result;
}


/// Select all matching items in the document
/// @param selection the selection to search for
/// @return true if one ore more items are selected false if not found
bool TextSearcher::selectAll( TextRangeSet* selection )
{
    TextRange oldRange = selection->range(0);

    markAll(selection);

    // no selection, we MUST place a caret
    if( selection->rangeCount() == 0 ) {
        selection->addRange(oldRange.caret(), oldRange.caret());
        return false;
    }
    return true;
}

/// Put all found items in the ranges
/// @param rangeset the rangeset to search for
void TextSearcher::markAll(TextRangeSet *rangeset)
{
    // clear the selection and add all matches
    bool oldReverse = reverse_;     // we must NOT reverse find
    bool oldWrapAround = wrapAround_;
    reverse_ = false;
    wrapAround_ = false;
    rangeset->clear();
    rangeset->beginChanges();
    TextRange range = findNextRange(rangeset);
    while( !range.isEmpty() )
    {
        rangeset->addRange(range.anchor(), range.caret());
        range = findNextRange( rangeset );
    }
    rangeset->endChanges();
    wrapAround_ = oldWrapAround;
    reverse_ = oldReverse;
}


/// Finds the next item based on the given caret position in the selection
/// @param widget the widget to find the next item
bool TextSearcher::findNext(TextEditorWidget* widget )
{
    std::unique_ptr<TextRangeSet> newRangeSet( new TextRangeSet( widget->textSelection() ) );
    if( findNext( newRangeSet.get() ) ) {
        widget->controller()->changeAndGiveTextSelection( newRangeSet.release() );
        return true;
    }
    return false;
}


/// Finds the previous item based on the given caret position in the selection
/// @param widget the widget to search in
bool TextSearcher::findPrev( TextEditorWidget* widget)
{
    std::unique_ptr<TextRangeSet> newRangeSet( new TextRangeSet( widget->textSelection() ) );
    if( findPrev( newRangeSet.get() ) ) {
        widget->controller()->changeAndGiveTextSelection( newRangeSet.release() );
        return true;
    }
    return false;
}


/// Selects the next item that matches the given critaria (Adds an extra selection range )
/// @param widget the widget to search in
bool TextSearcher::selectNext( TextEditorWidget* widget )
{
    std::unique_ptr<TextRangeSet> newRangeSet( new TextRangeSet( widget->textSelection() ) );
    if( selectNext( newRangeSet.get() ) ) {
        widget->controller()->changeAndGiveTextSelection( newRangeSet.release() );
        return true;
    }
    return false;
}


/// Selects the previous item based on the given caret position in the selection
/// @param widget the widget to to search in
bool TextSearcher::selectPrev( TextEditorWidget* widget )
{
    std::unique_ptr<TextRangeSet> newRangeSet( new TextRangeSet( widget->textSelection() ) );
    if( selectPrev( newRangeSet.get() ) ) {
        widget->controller()->changeAndGiveTextSelection( newRangeSet.release() );
        return true;
    }
    return false;
}


/// Selects all matches of the current item
/// @param widget the widget to search in
/// @param selection the selection to change
bool TextSearcher::selectAll( TextEditorWidget* widget )
{
    std::unique_ptr<TextRangeSet> newRangeSet( new TextRangeSet( widget->textSelection() ) );
    if( selectAll( newRangeSet.get() ) ) {
        widget->controller()->changeAndGiveTextSelection( newRangeSet.release() );
        return true;
    }
    return false;
}

/// A smart selection: when there's no selection the current word is used and selected.
/// The next word will be selected, if all is specified all words are selected
/// @param widget the widget to select
/// @param selectAllTexts if true then all occurences are selected
void TextSearcher::selectUnderExpand( TextEditorWidget* widget, bool selectAllTexts )
{
    std::unique_ptr<TextRangeSet> newRangeSet( new TextRangeSet( widget->textSelection() ) );
    bool runSelect = true;  // should the select be called

    // when no selection is used select it
    if( !newRangeSet->hasSelection() || newRangeSet->rangeCount() == 1 ) {
        TextRange& range = newRangeSet->lastRange();
        TextDocument* doc = widget->textDocument();

        // make sure there's a selection
        if( !newRangeSet->hasSelection() ) {
            if( range.isEmpty() ) {
                range.expandToWord( doc, doc->config()->whitespaces(), doc->config()->charGroups());
                runSelect = selectAllTexts;  // only run select when we need to select all
            }
        }
        // next set the search term to the given word
        setSearchTerm( doc->textPart( range.min(), range.length() ) );
    }

    // next select the next word
    if( runSelect) {
        if( selectAllTexts ) {
            selectAll( newRangeSet.get() );
        } else {
            selectNext( newRangeSet.get() );
        }
    }

    // set the new rangeset
    widget->controller()->changeAndGiveTextSelection( newRangeSet.release() );

}


/// Marks the regexp as dirty (and deletes it)
void TextSearcher::setDirty()
{
    delete regExp_;
    regExp_ = nullptr;
}


/// Creates a regular expression for the current selected options
/// @return the new regular expression
RegExp* TextSearcher::createRegExp()
{
    RegExp::Syntax regExpSyntax = RegExp::SyntaxFixedString;
    if( syntax() == SyntaxRegExp ) regExpSyntax = RegExp::SyntaxDefault;

    RegExp* regExp = new RegExp( searchTerm(), caseSensitive_, regExpSyntax, RegExp::EngineOniguruma );
    return regExp;
}


} // edbee
