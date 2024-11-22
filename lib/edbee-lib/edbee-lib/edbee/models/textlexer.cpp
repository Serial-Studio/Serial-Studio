/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textlexer.h"

#include "edbee/models/textgrammar.h"
#include "edbee/models/textdocumentscopes.h"

#include "edbee/debug.h"

namespace edbee {

TextLexer::TextLexer( TextDocumentScopes* scopes)
    : textDocumentScopesRef_( scopes )
    , grammarRef_(0)
{
}

void TextLexer::setGrammar(TextGrammar* grammar)
{
    Q_ASSERT(grammar);
    grammarRef_ = grammar;
    textScopes()->setDefaultScope( grammarRef_->mainRule()->scopeName(), grammarRef_->mainRule() );
    textScopes()->removeScopesAfterOffset(0); // invalidate the complete scopes
}

/// This method returns the text document
TextDocument* TextLexer::textDocument()
{
    return textDocumentScopesRef_->textDocument();
}


} // edbee
