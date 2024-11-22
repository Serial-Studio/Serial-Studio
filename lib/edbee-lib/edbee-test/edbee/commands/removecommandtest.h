/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/util/test.h"

namespace edbee {

class RemoveCommand;

/// The unit tests for deleting characters/words and lines
class RemoveCommandTest : public edbee::test::TestCase
{
    Q_OBJECT
public:
    RemoveCommandTest();

private slots:

    void init();
    void clean();
    void testSmartBackspace();

private:
    RemoveCommand* command_;
};


} // edbee

DECLARE_TEST(edbee::RemoveCommandTest );

