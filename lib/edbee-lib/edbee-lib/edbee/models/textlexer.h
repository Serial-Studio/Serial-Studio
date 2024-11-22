/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include "edbee/models/textbuffer.h"

namespace edbee {

class TextGrammar;
class TextDocument;
class TextDocumentScopes;

/// This is a single lexer
class EDBEE_EXPORT TextLexer {
public:
    TextLexer( TextDocumentScopes* scopes );
    virtual ~TextLexer() {}

    /// This method is called to notify the lexer some data has been changed
//    virtual void textReplaced( int offset, int length, int newLength ) = 0;
    virtual void textChanged( const edbee::TextBufferChange& change ) = 0;


    /// This method is called to notify the lexer the grammar has been changed
    void setGrammar( TextGrammar* grammar );
    inline TextGrammar* grammar() { return grammarRef_; }

    /// This method is called when the given range needs to be lexed
    /// WARNING, this method must be VERY optimized and should 'remember' the lexing
    /// states between calls. To invalidate the scopes/lexing state the textReplaced method can be used
    ///
    /// @param beginOffset the first offset
    /// @param endOffset the last offset to
    virtual void lexRange( int beginOffset, int endOffset ) = 0;


    TextDocumentScopes* textScopes() { return textDocumentScopesRef_; }
    TextDocument* textDocument();

private:
    TextDocumentScopes* textDocumentScopesRef_;     ///< A Text document refs
    TextGrammar* grammarRef_;                   ///< The reference to the active grammar
};

} // edbee
