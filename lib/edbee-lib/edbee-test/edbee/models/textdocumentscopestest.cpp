/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textdocumentscopestest.h"

#include "edbee/models/textdocumentscopes.h"
#include "edbee/edbee.h"

#include "edbee/debug.h"

namespace edbee {


void TextDocumentScopesTest::testStartsWith()
{
    TextScopeManager* sm = Edbee::instance()->scopeManager();
    TextScope* source = sm->refTextScope("aa.bb.cc.dd.ee");

    testTrue( source->startsWith( sm->refTextScope("aa.bb") ) );
    testTrue( source->startsWith( sm->refTextScope("aa.*") ) );
    testTrue( source->startsWith( sm->refTextScope("*.bb") ) );
    testTrue( source->startsWith( sm->refTextScope("*") ) );
    testFalse( source->startsWith( sm->refTextScope("bb") ) );
    testFalse( source->startsWith( sm->refTextScope("bb") ) );
}


void TextDocumentScopesTest::testRindexOf()
{
    TextScopeManager* sm = Edbee::instance()->scopeManager();
    TextScope* source = sm->refTextScope("aa.bb.cc.dd.ee");

    testEqual( source->rindexOf( sm->refTextScope("aa.bb") ), 0 );
    testEqual( source->rindexOf( sm->refTextScope("cc.dd") ), 2 );
    testEqual( source->rindexOf( sm->refTextScope("dd.ee") ), 3 );
    testEqual( source->rindexOf( sm->refTextScope("bb.aa") ), -1 );

    testEqual( source->rindexOf( sm->refTextScope("*.cc.*.ee") ), 1 );
    testEqual( source->rindexOf( sm->refTextScope("aa.bb.cc.dd.ee") ), 0 );

    testEqual( source->rindexOf( sm->refTextScope("*") ), 4 );
}



/* Orgingal textmate tests:
class ScopeSelectorTests : public CxxTest::TestSuite
{
public:
    void test_child_selector ()
    {
        TS_ASSERT_EQUALS(scope::selector_t("foo fud").does_match("foo bar fud"),   true);
        TS_ASSERT_EQUALS(scope::selector_t("foo > fud").does_match("foo bar fud"), false);
        TS_ASSERT_EQUALS(scope::selector_t("foo > foo > fud").does_match("foo foo fud"), true);
        TS_ASSERT_EQUALS(scope::selector_t("foo > foo > fud").does_match("foo foo fud fud"), true);
        TS_ASSERT_EQUALS(scope::selector_t("foo > foo > fud").does_match("foo foo fud baz"), true);

        TS_ASSERT_EQUALS(scope::selector_t("foo > foo fud > fud").does_match("foo foo bar fud fud"), true);
    }

    void test_mixed ()
    {
        TS_ASSERT_EQUALS(scope::selector_t("^ foo > bar").does_match("foo bar foo"), true);
        TS_ASSERT_EQUALS(scope::selector_t("foo > bar $").does_match("foo bar foo"), false);
        TS_ASSERT_EQUALS(scope::selector_t("bar > foo $").does_match("foo bar foo"), true);
        TS_ASSERT_EQUALS(scope::selector_t("foo > bar > foo $").does_match("foo bar foo"), true);
        TS_ASSERT_EQUALS(scope::selector_t("^ foo > bar > foo $").does_match("foo bar foo"), true);
        TS_ASSERT_EQUALS(scope::selector_t("bar > foo $").does_match("foo bar foo"), true);
        TS_ASSERT_EQUALS(scope::selector_t("^ foo > bar > baz").does_match("foo bar baz foo bar baz"), true);
        TS_ASSERT_EQUALS(scope::selector_t("^ foo > bar > baz").does_match("foo foo bar baz foo bar baz"), false);

    }

    void test_anchor ()
    {
        TS_ASSERT_EQUALS(scope::selector_t("^ foo").does_match("foo bar"), true);
        TS_ASSERT_EQUALS(scope::selector_t("^ bar").does_match("foo bar"), false);
        TS_ASSERT_EQUALS(scope::selector_t("^ foo").does_match("foo bar foo"), true);
        TS_ASSERT_EQUALS(scope::selector_t("foo $").does_match("foo bar"), false);
        TS_ASSERT_EQUALS(scope::selector_t("bar $").does_match("foo bar"), true);
    }

    void test_scope_selector ()
    {
        static scope::scope_t const textScope = "text.html.markdown meta.paragraph.markdown markup.bold.markdown";
        static scope::selector_t const matchingSelectors[] =
        {
            "text.* markup.bold",
            "text markup.bold",
            "markup.bold",
            "text.html meta.*.markdown markup",
            "text.html meta.* markup",
            "text.html * markup",
            "text.html markup",
            "text markup",
            "markup",
            "text.html",
            "text"
        };

        double lastRank = 1;
        for(size_t i = 0; i < sizeofA(matchingSelectors); ++i)
        {
            double rank;
            TS_ASSERT(matchingSelectors[i].does_match(textScope, &rank));
            TS_ASSERT_LESS_THAN(rank, lastRank);
            lastRank = rank;
        }
    }
};
*/


/// This method tests the score selector
void TextDocumentScopesTest::testScopeSelectorRanking()
{
    TextScopeManager* sm = Edbee::instance()->scopeManager();
    sm->reset();

    TextScopeList* multiScope = sm->createTextScopeList("text.html.markdown meta.paragraph.markdown markup.bold.markdown");

    QList<TextScopeSelector*> selectors;
    selectors.append( new TextScopeSelector("text.* markup.bold") );
    selectors.append( new TextScopeSelector("text markup.bold") );
    selectors.append( new TextScopeSelector("markup.bold") );
    selectors.append( new TextScopeSelector("text.html meta.*.markdown markup") );
    selectors.append( new TextScopeSelector("text.html meta.* markup") );
    selectors.append( new TextScopeSelector("text.html * markup") );
    selectors.append( new TextScopeSelector("text.html markup") );
    selectors.append( new TextScopeSelector("text markup") );
    selectors.append( new TextScopeSelector("markup") );
    selectors.append( new TextScopeSelector("text.html") );
    selectors.append( new TextScopeSelector("text") );

    double lastRank = 1.0;
    for(int i = 0; i < selectors.size(); ++i)
    {
        TextScopeSelector* sel = selectors.at(i);
        double rank = sel->calculateMatchScore( multiScope );
        if( !(rank < lastRank ) ) {
            qlog_info() << "SCOPES: " << multiScope->toString();
            qlog_info() << "  PREV: " << selectors.at(i-1)->toString();
            qlog_info() << "  rank: " << lastRank;
            qlog_info() << "";
            qlog_info() << "   NEW: " << sel->toString();
            qlog_info() << "  rank: " << rank;
        }

        testTrue( rank < lastRank );
        lastRank = rank;
    }
    qDeleteAll(selectors);
    delete multiScope;
}




} // edbee
