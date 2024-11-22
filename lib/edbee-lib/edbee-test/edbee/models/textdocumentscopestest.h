/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/util/test.h"

namespace edbee {

/// A special scope-selector test. Based on the official scopeselector tests of textmate
class TextDocumentScopesTest : public edbee::test::TestCase
{
Q_OBJECT

private slots:

    void testStartsWith();
    void testRindexOf();

    void testScopeSelectorRanking();

};


} // edbee

DECLARE_TEST(edbee::TextDocumentScopesTest);
