/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "cascadingqvariantmaptest.h"

#include "edbee/util/cascadingqvariantmap.h"

#include "edbee/debug.h"

namespace edbee {


/// tests the value options of the map
void CascadingQVariantMapTest::testValue()
{
    CascadingQVariantMap* map = createFixture();
    testEqual( map->stringValue("a"), "root-a" );
    testEqual( map->stringValue("b"), "child-b" );
    testEqual( map->stringValue("c"), "child2-c" );
    testEqual( map->stringValue("d"), "child2-d" );
    testEqual( map->stringValue("e","x"), "x" );
    testEqual( map->stringValue("e"), QString() );

    destroyFixture(map);
}

/// test the retrieval of the root value
void CascadingQVariantMapTest::testRoot()
{
    CascadingQVariantMap* map = createFixture();
    CascadingQVariantMap* root = map->parent()->parent();
    testTrue( root->parent() == 0 );
    testTrue( root->root() == root );
    testTrue( map->root() == root );
    destroyFixture(map);
}

/// Creates a general fixture for testing this class
CascadingQVariantMap* CascadingQVariantMapTest::createFixture()
{
    CascadingQVariantMap* root = new CascadingQVariantMap();
    root->insert("a","root-a");
    root->insert("b","root-b");
    root->insert("c","root-c");
    root->insert("d","root-d");
    CascadingQVariantMap* child = new CascadingQVariantMap( root );
    child->insert("b","child-b");
    child->insert("c","child-c");
    CascadingQVariantMap* child2 = new CascadingQVariantMap( child );
    child2->insert("c","child2-c");
    child2->insert("d","child2-d");
    return child2;
}

/// Deletes the fixture and all it's parents
void CascadingQVariantMapTest::destroyFixture(CascadingQVariantMap *item)
{
    while( item ) {
        CascadingQVariantMap* parent = item->parent();
        delete item;
        item = parent;
    }

}

} // edbee
