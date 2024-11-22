/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QMap>
#include <QList>
#include <QVector>

#include "edbee/models/textlexer.h"

namespace edbee {

class MultiLineScopedTextRange;
class RegExp;
class ScopedTextRange;
class ScopedTextRangeList;
class TextDocumentScopes;
class TextGrammar;
class TextGrammarRule;

/// A simple lexer matches texts with simple regular expressions
class EDBEE_EXPORT GrammarTextLexer : public TextLexer
{
public:
    GrammarTextLexer( TextDocumentScopes* scopes );
    virtual ~GrammarTextLexer();

    virtual void textChanged( const TextBufferChange& change );

private:
    virtual bool lexLine(int line, int& currentDocOffset );

public:
    virtual void lexLines( int line, int lineCount );
    virtual void lexRange( int beginOffset, int endOffset );

private:

    RegExp* createEndRegExp( RegExp* startRegExp, const QString &endRegExpStringIn);

    void findNextGrammarRule(const QString &line, int offsetInLine, TextGrammarRule *activeRule, TextGrammarRule *&foundRule, RegExp*& foundRegExp, int& foundPosition );
    void processCaptures( RegExp *foundRegExp, const QMap<int,QString>* foundCaptures );

    TextGrammarRule* findAndApplyNextGrammarRule(int currentDocOffset, const QString& line, int& offsetInLine  );

    MultiLineScopedTextRange* activeMultiLineRange();
    ScopedTextRange* activeScopedTextRange();

    void popActiveRange();
    void pushActiveRange( ScopedTextRange* range, MultiLineScopedTextRange* multiRange );

    TextGrammarRule* findIncludeGrammarRule( TextGrammarRule* base );

private:

    QVector<MultiLineScopedTextRange*> activeMultiLineRangesRefList_;        ///< The current active scoped text ranges, DOC  (this is only valid during parsing)
    QVector<MultiLineScopedTextRange*> currentMultiLineRangeList_;           ///< The doc ranges currently created            (only valid during parsing
    QVector<MultiLineScopedTextRange*> closedMultiRangesRangesRefList_;      ///< A list of all ranges (from other lines) that have been closed. (only valid during parsing)

    QVector<ScopedTextRange*> activeScopedRangesRefList_;                    ///< The current active scoped text ranges, LINE (this is only valid during parsing)

//    QVector<MultiLineScopedTextRange*> currentLineRangesList_;      ///< The current scope ranges (only valid during parsing)

    ScopedTextRangeList* lineRangeList_;                            ///< The scopes at current line (only valid during parsing)

};

} // edbee
