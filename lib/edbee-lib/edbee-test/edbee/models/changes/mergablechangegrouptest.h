/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/util/test.h"

namespace edbee {

class MergableChangeGroup;
class TextChange;
class Change;
class TextDocument;

/// for testing the complex textchanges
class MergableChangeGroupTest : public edbee::test::TestCase
{
    Q_OBJECT

private slots:
    void init();
    void clean();

private slots:
    void testMoveChangesFromGroup_grow();
    void testMoveChangesFromGroup_backspace();
    void testMoveChangesFromGroup_delete();
    void testMoveChangesFromGroup_multiRangeOverlap();
    void testMoveChangesFromGroup_trippleDuplicateIssue();
    void testMoveChangesMergeTest1();
private slots:
    void testMoveChangesMergeTest2();

public slots:
    void testGiveSingleTextChange_addMerge();
    void testGiveSingleTextChange_overlap();

private:
    void runSingleTextChange( int offset, int length, const QString& replacement );
    void runSingleTextChange2( int   offset, int length, const QString& replacement );

    QString fillGroup( const QString& changes, MergableChangeGroup* group=0);
    QString mergeResult(const QString& groupDef1, const QString& groupDef2 );

//    TextLineDataManager* manager();
//    LineDataListTextChange* createChange( int line, int length, int newLength );
//    LineDataListTextChange* takeChange(LineDataListTextChange* change);

    TextDocument* doc_;
    MergableChangeGroup* group_;
    MergableChangeGroup* group2_;
    QList<Change*> changeList_;
};


} // edbee

DECLARE_TEST(edbee::MergableChangeGroupTest );
