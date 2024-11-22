/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "grammartextlexertest.h"

#include "edbee/io/tmlanguageparser.h"
#include "edbee/lexers/grammartextlexer.h"
#include "edbee/models/chardocument/chartextdocument.h"
#include "edbee/models/textdocument.h"
#include "edbee/models/textdocumentscopes.h"
#include "edbee/models/textgrammar.h"
#include "edbee/models/textlexer.h"
#include "edbee/edbee.h"

#include "edbee/debug.h"

namespace edbee {


/// This method test the basic matching algorithm
GrammarTextLexerTest::GrammarTextLexerTest()
    : doc_(0)
{
}


/// initializes the grammer lexer
void GrammarTextLexerTest::init()
{
}


// cleanup auto cleanups the stuff
void GrammarTextLexerTest::clean()
{
    delete doc_;
    doc_ = 0;
}


/// A special test case to test (and solve the issues) that currently
/// exists with for example the haml lexer. We need more fine-graded unit-tests
/// for lexing. This test assumes the haml-language definition has been loaded
void GrammarTextLexerTest::testHamlLexer()
{
/* We need some good tests for the grammar lexer!
 *
 *
//    createFixtureDocument("%a=1\n%b");
    createFixtureDocument("%a a\n%b");

    // force the language to the given gramamr
    TextGrammar* grammar = Edbee::instance()->grammarManager()->get("text.haml");
    if( !grammar ) {
        testTrue( false && "Couldn't load the 'text.haml' Grammar!!" );
        return;
    }

    // set the grammar and force the lexing of the complete document
    doc_->setLanguageGrammar(grammar);
    doc_->scopes()->removeScopesAfterOffset(0);
    doc_->textLexer()->lexRange(0, doc_->length() );

    qlog_info() << "DUMP:====\n" << scopes()->scopesAsStringList().join("\n") << "\n============";
    //qlog_info() << " - " << scopes()->sc


    QStringList expected = QString(
        "0>7:text.haml |"
         // line 1
         "0>3:meta.tag.haml,"
            "0>1:punctuation.definition.tag.haml,"
            "1>2:entity.name.tag.haml,"
         "2>4:meta.line.ruby.haml,"
            "3>4:source.ruby.embedded.haml,"
                "3>4:constant.numeric.ruby,"
         // line 2
         "5>7:meta.tag.haml,"
            "5>6:punctuation.definition.tag.haml,"
            "6>7:entity.name.tag.haml").split(",");
*/
}


/// creates the main fixture document
void GrammarTextLexerTest::createFixtureDocument( const QString& data )
{
    doc_ = new CharTextDocument(0);
    doc_->setText( data );

}


/// Returns a references to the document scopes
TextDocumentScopes* GrammarTextLexerTest::scopes()
{
    return doc_->scopes();
}


/// This method returns the grammar text lexer
GrammarTextLexer* GrammarTextLexerTest::lexer()
{
    return dynamic_cast<GrammarTextLexer*>(doc_->textLexer());
}


} // edbee
