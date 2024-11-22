/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/util/test.h"

namespace edbee {

class TextBuffer;
class TextEditorController;
class TextEditorWidget;
class TextEditorController;

class ReplaceSelectionCommandTest : public edbee::test::TestCase
{
    Q_OBJECT
public:
    ReplaceSelectionCommandTest();
    virtual ~ReplaceSelectionCommandTest();

private slots:

    void init();
    void clean();


    void testUndo();

#if 0

    void testCoalesce_normalEntry();
    void testCoalesce_replaceRange();
    void testCoalesce_removeRange();
    void testCoalesce_backspace();
    void testCoalesce_delete();

    void testCoalesce_normalEntry_multiRange();
    void testCoalesce_replaceRange_multiRange();
    void testCoalesce_removeRange_multiRange();
    void testCoalesce_backspace_multiRange();
    void testCoalesce_delete_multiRange();
#endif
private:

    TextBuffer* buf() { return bufRef_; }
    TextEditorController* ctrl() { return ctrlRef_; }


    TextEditorWidget* widget_;
    TextEditorController* ctrlRef_;
    TextBuffer* bufRef_;

};


}

DECLARE_TEST(edbee::ReplaceSelectionCommandTest);
