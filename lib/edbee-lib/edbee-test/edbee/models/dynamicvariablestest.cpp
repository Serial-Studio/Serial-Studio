/**
 * Copyright 2011-2014 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "dynamicvariablestest.h"
#include "edbee/models/chardocument/chartextdocument.h"
#include "edbee/models/textdocumentscopes.h"
#include "edbee/models/dynamicvariables.h"

#include "edbee/debug.h"

namespace edbee {


/// cleans the scope list
void DynamicVariablesTest::clean()
{
    qDeleteAll(scopeList_);
    scopeList_.clear();
}


/// Tests the size of the DynamicVariable class
void DynamicVariablesTest::testSize()
{
    // make sure it all starts empty
    DynamicVariables vars;
    testEqual( vars.size(), 0 );
    testEqual( vars.valueCount("test"), 0 );

    // add some variables
    vars.set("test","1");
    vars.setAndGiveScopedSelector( "test", "2", "source.ruby" );

    testEqual( vars.size(), 1 );
    testEqual( vars.valueCount("test"), 2 );
}


/// Tests the simple value retrieval
void DynamicVariablesTest::testValue()
{

    // initialize the variables
    TextScopeManager scopeManager;
    DynamicVariables vars;
    vars.set("a","as");
    vars.set("b","bs");
    vars.setAndGiveScopedSelector("b","bd1", "source.ruby" );
    vars.setAndGiveScopedSelector("b","bd2", "text.html.basic source.php.embedded.html" );
    vars.setAndGiveScopedSelector("c","cd1", "source.c" );

    // test the retrieval of a non-existing value
    testTrue( vars.value("xx", 0).isNull() );

    // first test retrieval of a simple value
    testEqual( vars.value("a").toString(), "as" );

    // test the retrieval of a scoped value with static value (no match on scope)
    testEqual( vars.value("b", createTextScopeList(scopeManager,"text.python")).toString(), "bs" );

    // test the retrieval of a scoped value without a backup static value (no match on scope)
    testTrue( vars.value("c", createTextScopeList(scopeManager,"dummy.*")).isNull() );

    // test a matched scope
    testEqual( vars.value("b", createTextScopeList(scopeManager,"source.ruby")).toString(), "bd1" );
    testEqual( vars.value("b", createTextScopeList(scopeManager,"source.*")).toString(), "bd1" );      // equal scores should return the first match
    testEqual( vars.value("b", createTextScopeList(scopeManager,"source.php")).toString(), "bs" );     // this shouldn't match and falback to the static variant
    testEqual( vars.value("b", createTextScopeList(scopeManager,"text.html.* source.php.*.*")).toString(), "bd2" );

    // test the retrieval of a scoped value with match
    testEqual( vars.value("c", createTextScopeList(scopeManager,"source.c")).toString(), "cd1" );
}


/// adds a definition
TextScopeList* DynamicVariablesTest::createTextScopeList( TextScopeManager& tm, const QString& name )
{
    TextScopeList* result = tm.createTextScopeList(name);
    scopeList_.push_back(result);
    return result;
}


} // edbee
