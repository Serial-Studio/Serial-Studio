/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "mergablechangegrouptest.h"

#include <QStringList>

#include "edbee/models/changes/mergablechangegroup.h"
#include "edbee/models/changes/textchange.h"
#include "edbee/models/chardocument/chartextdocument.h"

#include "edbee/debug.h"

namespace edbee {


/// intitialze the tests
void MergableChangeGroupTest::init()
{
    // create the document
    doc_ = new CharTextDocument();
    doc_->setText("abcdefgh");

    // create the group
    group_ = new MergableChangeGroup(0);
    group2_ = new MergableChangeGroup(0);
}


/// the clean operation
void MergableChangeGroupTest::clean()
{
    delete group2_;
    delete group_;
    group_ = 0;
    delete doc_;
    doc_= 0 ;
}


/// Test the moving of changes between groups
/// - ab|cd|efgh  =>  abX|cdY|efg  =>  abXP|cdYQ|efg => abXQR|cdYPSefg
///
void MergableChangeGroupTest::testMoveChangesFromGroup_grow()
{
    testEqual( doc_->text(), "abcdefgh" );
    testEqual( group_->toSingleTextChangeTestString(), "" );

    // Build first change
    {
        runSingleTextChange(2,0,"X");
        testEqual( doc_->text(), "abXcdefgh" );
        testEqual( group_->toSingleTextChangeTestString(), "2:1:" );

        runSingleTextChange(5,0,"Y");
        testEqual( doc_->text(), "abXcdYefgh" );
        testEqual( group_->toSingleTextChangeTestString(), "2:1:,5:1:" );
    }

    // build second change
    {
        runSingleTextChange2(3,0,"P");
        testEqual( doc_->text(), "abXPcdYefgh" );
        testEqual( group2_->toSingleTextChangeTestString(), "3:1:" );

        runSingleTextChange2(7,0,"Q");
        testEqual( doc_->text(), "abXPcdYQefgh" );
        testEqual( group2_->toSingleTextChangeTestString(), "3:1:,7:1:" );
    }

    // next test the merge of these groups
    group_->moveChangesFromGroup( doc_, group2_ );
    testEqual( doc_->text(), "abXPcdYQefgh" );
    testEqual( group_->toSingleTextChangeTestString(), "2:2:,6:2:" );

    // perform a third operation to test if this works
    {
        runSingleTextChange2(4,0,"R");
        testEqual( doc_->text(), "abXPRcdYQefgh" );
        testEqual( group2_->toSingleTextChangeTestString(), "4:1:" );

        runSingleTextChange2(9,0,"S");
        testEqual( doc_->text(), "abXPRcdYQSefgh" );
        testEqual( group2_->toSingleTextChangeTestString(), "4:1:,9:1:" );
    }

    // next test the merge of these groups
    group_->moveChangesFromGroup( doc_, group2_ );
    testEqual( doc_->text(), "abXPRcdYQSefgh" );
    testEqual( group_->toSingleTextChangeTestString(), "2:3:,7:3:" );
}


/// Tests the (backspace) removal of characters
/// - abc|def|gh  =>  ab|de|gh  =>  a|d|gh => |gh
void MergableChangeGroupTest::testMoveChangesFromGroup_backspace()
{
    testEqual( doc_->text(), "abcdefgh" );
    testEqual( group_->toSingleTextChangeTestString(), "" );

    // build the first change
    {
        runSingleTextChange(2,1,"");
        testEqual( doc_->text(), "abdefgh" );
        testEqual( group_->toSingleTextChangeTestString(), "2:0:c" );

        runSingleTextChange(4,1,"");
        testEqual( doc_->text(), "abdegh" );
        testEqual( group_->toSingleTextChangeTestString(), "2:0:c,4:0:f" );

    }

    // build the second change
    {
        runSingleTextChange2(1,1,"");
        testEqual( doc_->text(), "adegh" );
        testEqual( group2_->toSingleTextChangeTestString(), "1:0:b" );

        runSingleTextChange2(2,1,"");
        testEqual( doc_->text(), "adgh" );
        testEqual( group2_->toSingleTextChangeTestString(), "1:0:b,2:0:e" );
    }


    // next test the merge of these groups
    group_->moveChangesFromGroup( doc_, group2_ );
    testEqual( doc_->text(), "adgh" );
    testEqual( group_->toSingleTextChangeTestString(), "1:0:bc,2:0:ef" );

    // build the third change
    {
        runSingleTextChange2(0,1,"");
        testEqual( doc_->text(), "dgh" );
        testEqual( group2_->toSingleTextChangeTestString(), "0:0:a" );

        // this is a special case. These two change are merged together, because they touch eachother
        runSingleTextChange2(0,1,"");
        testEqual( doc_->text(), "gh" );
        testEqual( group2_->toSingleTextChangeTestString(), "0:0:ad" );
    }

    // next test the merge of these groups
    group_->moveChangesFromGroup( doc_, group2_ );
    testEqual( doc_->text(), "gh" );
    testEqual( group_->toSingleTextChangeTestString(), "0:0:abcdef" );
}


/// Test the delete operation
/// - a|bcd|efgh  =>  a|cd|fgh  =>  a|d|gh
void MergableChangeGroupTest::testMoveChangesFromGroup_delete()
{
    testEqual( doc_->text(), "abcdefgh" );
    testEqual( group_->toSingleTextChangeTestString(), "" );

    // build the first change
    {
        runSingleTextChange(1,1,"");
        testEqual( doc_->text(), "acdefgh" );
        testEqual( group_->toSingleTextChangeTestString(), "1:0:b" );

        runSingleTextChange(3,1,"");
        testEqual( doc_->text(), "acdfgh" );
        testEqual( group_->toSingleTextChangeTestString(), "1:0:b,3:0:e" );

    }

    // build the second change
    {
        runSingleTextChange2(1,1,"");
        testEqual( doc_->text(), "adfgh" );
        testEqual( group2_->toSingleTextChangeTestString(), "1:0:c" );

        runSingleTextChange2(2,1,"");
        testEqual( doc_->text(), "adgh" );
        testEqual( group2_->toSingleTextChangeTestString(), "1:0:c,2:0:f" );
    }

    // next test the merge of these groups
    group_->moveChangesFromGroup( doc_, group2_ );
    testEqual( doc_->text(), "adgh" );
    testEqual( group_->toSingleTextChangeTestString(), "1:0:bc,2:0:ef" );
}



/// Relates with the test for TextUndoStackTest::testMultiCaretUndoIssue196
/// Aa[B]b[C]cDd  =>  Aa[b][c]Dd  =>  Aa|Dd
void MergableChangeGroupTest::testMoveChangesFromGroup_multiRangeOverlap()
{
    doc_->setText("AaBbCcDd");
    testEqual( doc_->text(), "AaBbCcDd" );
    testEqual( group_->toSingleTextChangeTestString(), "" );

    // build the first change
    {
        runSingleTextChange(2,1,"");
        testEqual( doc_->text(), "AabCcDd" );
        testEqual( group_->toSingleTextChangeTestString(), "2:0:B" );

        runSingleTextChange(3,1,"");
        testEqual( doc_->text(), "AabcDd" );
        testEqual( group_->toSingleTextChangeTestString(), "2:0:B,3:0:C" );
    }

    // build the second change
    {
        runSingleTextChange2(2,1,"");
        testEqual( doc_->text(), "AacDd" );
        testEqual( group2_->toSingleTextChangeTestString(), "2:0:b" );

        runSingleTextChange2(2,1,"");
        testEqual( doc_->text(), "AaDd" );
        testEqual( group2_->toSingleTextChangeTestString(), "2:0:bc" );

    }

    group_->moveChangesFromGroup( doc_, group2_ );
    testEqual( doc_->text(), "AaDd" );
    testEqual( group_->toSingleTextChangeTestString(), "2:0:BbCc" );
}



/// When having three lines |a$|b$|c$ (|=caret, $=eol). Pressing duplicate 3 times results in an incorrect merge :
void MergableChangeGroupTest::testMoveChangesFromGroup_trippleDuplicateIssue()
{
    doc_->setText("a0b0c0");

    // build the first change
    {
        runSingleTextChange(0,0,"a1");
        testEqual( doc_->text(), "a1a0b0c0" );
        testEqual( group_->toSingleTextChangeTestString(), "0:2:" );

        runSingleTextChange(4,0,"b1");
        testEqual( doc_->text(), "a1a0b1b0c0" );
        testEqual( group_->toSingleTextChangeTestString(), "0:2:,4:2:" );

        runSingleTextChange(8,0,"c1");
        testEqual( doc_->text(), "a1a0b1b0c1c0" );
        testEqual( group_->toSingleTextChangeTestString(), "0:2:,4:2:,8:2:" );
    }

    // build the seconds change
    {
        runSingleTextChange2(2,0,"a2");
        testEqual( doc_->text(), "a1a2a0b1b0c1c0" );
        testEqual( group2_->toSingleTextChangeTestString(), "2:2:" );

        runSingleTextChange2(8,0,"b2");
        testEqual( doc_->text(), "a1a2a0b1b2b0c1c0" );
        testEqual( group2_->toSingleTextChangeTestString(), "2:2:,8:2:" );

        runSingleTextChange2(14,0,"c2");
        testEqual( doc_->text(), "a1a2a0b1b2b0c1c2c0" );
        testEqual( group2_->toSingleTextChangeTestString(), "2:2:,8:2:,14:2:" );
    }

    group_->moveChangesFromGroup( doc_, group2_ );
    testEqual( doc_->text(), "a1a2a0b1b2b0c1c2c0" );
    testEqual( group_->toSingleTextChangeTestString(), "0:4:,6:4:,12:4:" );
}



/// This method tests the merge changes
void MergableChangeGroupTest::testMoveChangesMergeTest1()
{
    testEqual( mergeResult("0:2:,4:2:,8:2:", "2:2:,8:2:,14:2:"), "0:4:,6:4:,12:4:");
    testEqual( mergeResult("2:0:c,4:0:f", "1:0:b,2:0:e"), "1:0:bc,2:0:ef");

    // single group:
    // a[bc]defgh     [x]=>  axdefgh          (1:1:bc)
    // ax[de]fgh      [y]=>  axyfgh           (2:1:de)
    // ax[yfg]h    [QWER]=>  axQWERh          (2:4:yfg)
    // ====
    // This should compress in:               (1:5:bcdefg)
    testEqual( fillGroup("1:1:bc,2:1:de,2:4:yfg"), "1:5:bcdefg");


    // simple merge
    // a[bcdefg]h    [xQWER]=>  axQWERh          (1:5:bcdefg)
    // -----
    // [axQW]ERh      [?]=>  ?ERh                (0:1:axQW)
    testEqual( mergeResult("1:5:bcdefg", "0:1:axQW"), "0:3:abcdefg");

    // complex example:
    // a[bc]defgh     [x]=>  axdefgh          (1:1:bc)
    // ax[de]fgh      [y]=>  axyfgh           (2:1:de)
    // ax[yfg]h    [QWER]=>  axQWERh          (2:4:yfg)
    // -----
    // [axQW]ERh      [?]=>  ?ERh             (0:1:axQW)
    // ====
    // This should compress in:               (0:3:abcdefg)
    testEqual( mergeResult("1:1:bc,2:1:de,2:4:yfg", "0:1:axQW"), "0:3:abcdefg");
}


/// A large (and complex) merge test, which spans multiple changes
void MergableChangeGroupTest::testMoveChangesMergeTest2()
{
    // GROUP1:
    ///
    // a|bcdefgh     [x]=>  axbcdefgh         (1:1:)                (insert)
    // axb[cd]efgh   [y]=>  axbyefgh          (3:1:cd)              (replace-shrink)
    // axbye[fg]h    []=>   axbyeh            (5:0:fg)              (delete)
    testEqual( fillGroup("1:1:,3:1:cd,5:0:fg"), "1:1:,3:1:cd,5:0:fg");          // this shouldn't change

    //
    // GROUP2:
    // [axbye]h      [RT]=> RTh               (0:2:axbye)           (replace)
    testEqual( mergeResult("1:1:,3:1:cd,5:0:fg", "0:2:axbye"), "0:2:abcdefg");
}



/// test the single text change
void MergableChangeGroupTest::testGiveSingleTextChange_addMerge()
{
    testEqual( doc_->text(), "abcdefgh" );
    testEqual( group_->toSingleTextChangeTestString(), "" );

    // test add
    runSingleTextChange(1,0,"x");
    testEqual( doc_->text(), "axbcdefgh" );
    testEqual( group_->toSingleTextChangeTestString(), "1:1:" );

    // test add merging
    runSingleTextChange(2,0,"y");
    testEqual( doc_->text(), "axybcdefgh" );
    testEqual( group_->toSingleTextChangeTestString(), "1:2:" );
}


/// A test for checking overlapping
/// - a[bc]defgh  =(x)=> axdefgh   (0) 1:1:bc
/// - a[xd]efgh   =(y)=> ayefgh    (1) 1:1:xd
///
/// This should be merge into:  (m) 1:1:bd
void MergableChangeGroupTest::testGiveSingleTextChange_overlap()
{
    testEqual( doc_->text(), "abcdefgh" );
    testEqual( group_->toSingleTextChangeTestString(), "" );

    // convert
    runSingleTextChange(1,2,"x");
    testEqual( doc_->text(), "axdefgh" );
    testEqual( group_->toSingleTextChangeTestString(), "1:1:bc" );

    runSingleTextChange(1,2,"y");
    testEqual( doc_->text(), "ayefgh" );
    //testEqual( group_->toSingleTextChangeTestString(), "1:1:bc,1:1:xd" );  // Without coalescing
    testEqual( group_->toSingleTextChangeTestString(), "1:1:bcd" );

}


/// adds a new textchange
void MergableChangeGroupTest::runSingleTextChange(int offset, int length, const QString& replacement)
{
    TextChange* change = new TextChange(offset,length,replacement);
    change->execute( doc_ );
    group_->giveSingleTextChange(doc_,change);
}


/// adds a new textchange to group 2
void MergableChangeGroupTest::runSingleTextChange2(int offset, int length, const QString& replacement)
{
    TextChange* change = new TextChange(offset,length,replacement);
    change->execute( doc_ );
    group2_->giveSingleTextChange(doc_,change);
}


/// Fills the given group with the given changes
QString MergableChangeGroupTest::fillGroup(const QString& changes, MergableChangeGroup* group)
{
    if( !group ) { group = group_; }
    group->clear(true);
    QStringList defs = changes.split(",");
    foreach( QString def, defs) {
        QStringList fields = def.split(":");
        Q_ASSERT( fields.size() == 3 );
        int offset = fields.at(0).toInt();
        int length = fields.at(1).toInt();
        group->giveSingleTextChange(doc_, new TextChange(offset, length, fields[2] ) );
    }
    return group->toSingleTextChangeTestString();
}


/// This method returns the merge result of several textchange
/// @param groupDef1 the group definition. Example:   0:2:AB:,4:2:AX
/// @param groupDef2 the second group definition.
QString MergableChangeGroupTest::mergeResult(const QString& groupDef1, const QString& groupDef2)
{
    fillGroup( groupDef1, group_ );
    fillGroup( groupDef2, group2_ );
    group_->moveChangesFromGroup(doc_,  group2_ );
    return group_->toSingleTextChangeTestString();
}


} // edbee
