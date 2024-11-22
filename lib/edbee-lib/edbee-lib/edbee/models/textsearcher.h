/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QObject>

#include "edbee/models/textrange.h"

namespace edbee {


class RegExp;
class TextDocument;
class TextEditorWidget;

/// The text searcher is a class to remember the current search operation
/// It remembers the current searchTerm that is used for searching
/// The textsearcher component has got several option. Like case sensitivity
/// regular expressions etc.
class EDBEE_EXPORT TextSearcher : public QObject
{
    Q_OBJECT
public:

    enum SyntaxType {
        SyntaxPlainString,
        SyntaxRegExp
    };

    explicit TextSearcher(QObject *parent = 0);
    virtual ~TextSearcher();

    QString searchTerm() const;
    void setSearchTerm( const QString& term );

    SyntaxType syntax() const;
    void setSyntax( SyntaxType syntax);

    bool isCaseSensitive() const;
    void setCaseSensitive( bool sensitive );

    bool isWrapAroundEnabled() const;
    void setWrapAround( bool on );

    bool isReverse() const;
    void setReverse( bool on );

    TextRange findNextRange( TextRangeSet* selection );

    bool findNext( TextRangeSet* selection );
    bool findPrev( TextRangeSet* selection );
    bool selectNext( TextRangeSet* selection );
    bool selectPrev( TextRangeSet* selection );
    bool selectAll( TextRangeSet* selection );
    void markAll( TextRangeSet* rangeset );

public slots:

    bool findNext( TextEditorWidget* widget );
    bool findPrev( TextEditorWidget* widget );
    bool selectNext( TextEditorWidget* widget );
    bool selectPrev( TextEditorWidget* widget );
    bool selectAll( TextEditorWidget* widget );

    void selectUnderExpand(TextEditorWidget* widget, bool selectAllTexts );




protected:

    void setDirty();
    RegExp* createRegExp();

private:

    QString searchTerm_;        ///< The current search term
    SyntaxType syntax_;         ///< The syntax-type
    bool caseSensitive_;        ///< case sensitive?
    bool wrapAround_;           ///< should the search wrap around?
    bool reverse_;              ///< search in the reverse direction

    RegExp* regExp_;            ///< The current regexp

};

} // edbee
