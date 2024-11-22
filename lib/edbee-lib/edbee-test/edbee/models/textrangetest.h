/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/util/test.h"

namespace edbee {

class TextBuffer;
class TextDocument;
class TextEditorController;
class TextRangeSet;


/// The base TextRange test classs
class TextRangeTest : public edbee::test::TestCase
{
    Q_OBJECT
public:
    TextRangeTest();


private slots:
    void init();
    void clean();

    void testMoveCaret();
    void testMoveCaretOrDeselect();
    void testMoveCaretWhileChar();
    void testMoveCaretUntilChar();
    void testMoveCaretByCharGroup();
    void testMoveCaretToLineBoundary();

    void testMoveNonBmpCharacters();

private:
    TextDocument* doc_;
    TextBuffer* bufRef_;
};


/// Tests a set of ranges
class TextRangeSetTest : public edbee::test::TestCase
{
    Q_OBJECT

private slots:

    void init();
    void clean();

    void testConstructor();
    void testAddRange();
    void testRangesBetweenOffsets();
    void testMoveCarets();
    void testChangeSpatial();
    void testGetSelectedTextExpandedToFullLines();

    void testSubstractRange();
    void testMergeOverlappingRanges();


private:
    TextEditorController* controller_;
    TextDocument* docRef_;
    TextBuffer* bufRef_;
    TextRangeSet* selRef_;
};


/// Tests the dynamic rangeset
class DynamicTextRangeSetTest : public edbee::test::TestCase
{
    Q_OBJECT

private slots:
    void testDynamicChanges();
    void testDeleteMode();

};


} // edbee

DECLARE_NAMED_TEST(range, edbee::TextRangeTest);
DECLARE_TEST(edbee::TextRangeSetTest);
DECLARE_NAMED_TEST(dynamic,edbee::DynamicTextRangeSetTest);
