/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "replaceselectioncommandtest.h"

#include <QDebug>

#include "edbee/models/textbuffer.h"
#include "edbee/models/textdocument.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/texteditorwidget.h"
#include "edbee/models/textrange.h"
#include "edbee/models/textundostack.h"

#include "edbee/debug.h"

//#include "texteditor/commands/undoablereplaceselectionbytextcommand.h"

namespace edbee {

/*
/// Creates a selection object
/// @param doc the document to use
/// @param definition the definition. In the format  anchor>caret,anchor>caret
static TextRangeSet* createTextSelection( TextEditorController* ctrl, const QString& definition )
{
    TextRangeSet* sel = new TextRangeSet( ctrl->textDocument());
    QStringList ranges = definition.split(",", QString::SkipEmptyParts);
    foreach( QString range, ranges ) {
        QStringList values = range.split(">");
        Q_ASSERT(values.length() == 2);
        int anchor = values.at(0).trimmed().toInt();
        int caret  = values.at(1).trimmed().toInt();
        sel->addRange(anchor,caret);
    }
    return sel;
}
*/

ReplaceSelectionCommandTest::ReplaceSelectionCommandTest()
    : widget_(0)
    , ctrlRef_(0)
    , bufRef_(0)
{
}


ReplaceSelectionCommandTest::~ReplaceSelectionCommandTest()
{
    delete widget_;
    widget_  = 0;
    ctrlRef_ = 0;
    bufRef_  = 0;
}

/// test case initialisation
void ReplaceSelectionCommandTest::init()
{
    widget_  = new TextEditorWidget();
    ctrlRef_ = widget_->controller();
    bufRef_  = widget_->textDocument()->buffer();
}

/// test case destruction
void ReplaceSelectionCommandTest::clean()
{
    delete widget_;
    widget_  = 0;
    ctrlRef_ = 0;
    bufRef_  = 0;
}


/// This method test the undo functionality of the text
void ReplaceSelectionCommandTest::testUndo()
{
    TextEditorWidget* widget = new TextEditorWidget();
    TextEditorController* ctrl = widget->controller();
    TextDocument *doc = widget->textDocument();
    TextBuffer* buf = doc->buffer();

    // check if we can set the txt  
    ctrl->replaceSelection("test");
    testEqual( buf->text(), "test" );

    // test basic, undo/redo
        ctrl->undo();
        testEqual( buf->text(), "" );

        // test coalesce
        ctrl->replaceSelection("emma");
        ctrl->replaceSelection(" sarah",true);
        ctrl->replaceSelection(" david",true);
        testEqual( buf->text(), "emma sarah david" );
        ctrl->undo();
        testEqual( buf->text(), "emma" );

        ctrl->redo();
        testEqual( buf->text(), "emma sarah david");

    // test ranged replacement
        doc->textUndoStack()->clear();
        buf->setText("Blommers");

        testEqual( buf->text(), "Blommers" );

        // now replace 'mm' by nothing
        TextRangeSet sel( doc );
        sel.addRange(3,5);
        ctrl->replaceRangeSet( sel, "" );
        testEqual( buf->text(), "Bloers" );

        ctrl->undo();
        testEqual( buf->text(), "Blommers" );
        ctrl->redo();
        testEqual( buf->text(), "Bloers" );



        delete widget;
}


/// BIG FIXME, we need to get back these unit test. The tests below are for the older implementation

#if 0

/// Asserts the documents.
/// The marker list is a list of markers, with the end marker closed with a -1
#define testState( ctrl, cm, state, txt ) \
do { \
    testEqual( ctrl->textDocument()->text(), txt ); \
    testEqual( cm->testString(), state ); \
} while(false)

//testEqual( cm->ranges()->rangesAsString(), range );
//testEqual( cm->textState()->toString(), state );


#define testReplaceCommandBegin( change1, range1, text1, testState1, testDoc1, executeIt ) \
    MultiTextChange* change1 = new MultiTextChange( ctrl() ); \
    { \
        TextRangeSet* rangeSet = createTextSelection( ctrl(), range1 ); \
        change1->setReplaceTextRange( *rangeSet, QString(text1) ); \
        delete rangeSet; \
    }\
    change1->storeSelection(); \
    if( executeIt ) { \
        change1->execute( ctrl()->textDocument() ); \
        testState( ctrl(), change1, testState1, testDoc1 ); \
    }

#define testReplaceCommandEnd( change1 ) \
    delete change1

#define testCoalesce( cmd, cmd2, testState3, testDoc3 ) \
    testEqual( cmd->merge( ctrl()->textDocument(), cmd2 ), true ); \
    testState( ctrl(), cmd, testState3, testDoc3 )



#define testReplace2Coalesc( range1, text1, testState1, testDoc1, \
                             range2, text2, testState2, testDoc2, \
                             testState3, testDoc3 ) \
do { \
    MultiTextChange* change1 = new MultiTextChange( ctrl() ); \
    TextRangeSet* rangeSet = createTextSelection( ctrl(), range1 ); \
    change1->setReplaceTextRange( *rangeSet, QString(text1) ); \
    delete rangeSet; \
    change1->storeSelection(); \
    change1->execute( ctrl()->textDocument() ); \
    testState( ctrl(), change1, testState1, testDoc1 ); \
    \
    MultiTextChange* change2 = new MultiTextChange( ctrl() ); \
    rangeSet = createTextSelection( ctrl(), range2 ); \
    change2->setReplaceTextRange( *rangeSet, QString(text2) ); \
    delete rangeSet; \
    change2->storeSelection(); \
    \
    testEqual( change1->merge( ctrl()->textDocument(), change2 ), true  ); \
    testState( ctrl(), change1, testState3, testDoc3 ); \
    \
    delete change2; \
    delete change1; \
} while(false)


/// BASIC ENTRY: ==============  ""
/// -  "0>0" ,  "0:" ,  "rick"         => "rick|"
/// -  "4>4" ,  "4:" ,  "bit"          => "rickbit|
/// =>  0>0,    "0:" ,  "rickbit"
void ReplaceSelectionCommandTest::testCoalesce_normalEntry()
{
    buf()->setText("");

    testReplace2Coalesc( "0>0", "rick", "0:4:", "rick",        // "|"  =>  "rick|"
                         "4>4", "bit", "4:3:", "rickbit",      // "|"
                         "0:7:", "rickbit" );


}

/// CHANGING: ============== "abcd"
///-  "0>2" ,  "0:ab" ,  "rick"        => "rick|cd"
///-  "4>6" ,  "3:cd" ,  "bit"         => "rickbit|"
///=>  0>4,    "0:abcd" ,  "rickbit"
void ReplaceSelectionCommandTest::testCoalesce_replaceRange()
{
    buf()->setText("abcd");

    testReplaceCommandBegin( cmd1, "0>2", "rick", "0:4:ab", "rickcd",true );     // "[ab>cd"  =>  "rick|cd"
    testReplaceCommandBegin( cmd2, "4>6", "bit", "4:3:cd", "rickbit",false );          // "rick[cd>" => "rickbit"

    testCoalesce( cmd1, cmd2, "0:7:abcd", "rickbit" );


    testReplaceCommandEnd(cmd1);
    testReplaceCommandEnd(cmd2);

//    testReplace2Coalesc( "0>2", "rick", "0>2", "0:ab", "rickcd",      // "[ab>cd"  =>  "rick|cd"
//                         "4>6", "bit", "4>6", "4:cd", "rickbit",      // "rick[cd>" => "rickbit"
//                         "0>4", "0:abcd", "rickbit" );
}

///-RANGE REMOVE
///========================== "abcdef"
///- "2>4"  ,  "2:cd" , ""             => "ab|ef"
///- "0>2"  ,  "0:ab" , ""             => "|ef"
///=> 0>4   ,  "0:abcd",  ""
void ReplaceSelectionCommandTest::testCoalesce_removeRange()
{
     buf()->setText("abcdef");

     testReplace2Coalesc( "2>4", "", "2:0:cd", "abef",    // "ab[cd>ef"  =>  "ab|cd"
                          "0>2", "", "0:0:ab", "ef",      // "rick[cd>" => "rickbit"
                          "0:0:abcd", "ef" );

}

/// BACKSPACE
///========================== "abcdef"
///- "3>2"  ,  "2:c" , ""              => "ab|def"
///- "2>1"  ,  "1:b" , ""              => "a|def"
///=> 3>1   ,  "1:bc",  ""
void ReplaceSelectionCommandTest::testCoalesce_backspace()
{
    buf()->setText("abcdef");

    testReplace2Coalesc( "3>2", "", "2:0:c", "abdef",    // "ab<c]def"  =>  "ab|def"
                         "2>1", "", "1:0:b", "adef",     // "a<b]def" => "adef"
                         "1:0:bc", "adef" );
}

/// DELETE
///========================== "abcdef"
///- "2>3"  ,  "2:c" , ""              => "ab|def"
///- "2>3"  ,  "2:d" , ""              => "ab|ef"
///=> 2>4   ,  "2:cd",  ""
void ReplaceSelectionCommandTest::testCoalesce_delete()
{
    buf()->setText("abcdef");

    testReplace2Coalesc( "2>3", "", "2:0:c", "abdef",    // "ab[c>def"  =>  "ab|def"
                         "2>3", "", "2:0:d", "abef",     // "ab[d>ef" => "abdef"
                         "2:0:cd", "abef" );

}

/// MULTIPLE CHANGE: ============== "abcdefABCDEF"
///-  "1>1,6>6" ,    "1:,6:" ,  "www"         => "awww|bcdefwww|ABCDEF"
///-  "4>4,11>11" ,  "4:,11:" ,  "bit"        => "awwwbit|bcdefwwwbit|ABCDEF"
///=> "1>1,6>6       "1:,11:,"  "wwwbit"

void ReplaceSelectionCommandTest::testCoalesce_normalEntry_multiRange()
{
    buf()->setText("abcdefABCDEF");

    testReplace2Coalesc( "1>1,6>6", "www", "1:3:|9:3:", "awwwbcdefwwwABCDEF",
                         "4>4,12>12", "bit", "4:3:|15:3:", "awwwbitbcdefwwwbitABCDEF",
                         "1:6:|12:6:", "awwwbitbcdefwwwbitABCDEF" );
}

/// MULTIPLE REPLACE: ============== "abcdefABCDEF"
///-  "1>2,6>8" ,    "1:b,6:AB" ,  "www"         => "awww|cdefwww|CDEF"
///-  "4>6,11>14" ,  "4:cd,11:CDE" ,  "x"        => "awwwx|efwwwx|F"
///=> "1>4,6>11      "1:bcd,6:ABCDE" ,  "wwwx"
void ReplaceSelectionCommandTest::testCoalesce_replaceRange_multiRange()
{
    buf()->setText("abcdefABCDEF");

    testReplace2Coalesc( "1>2,6>8", "www", "1:3:b|8:3:AB", "awwwcdefwwwCDEF",      // "a[b>cdef[AB>CDEF"    => "awww|cdefwww|CDEF"
                         "4>6,11>14", "x", "4:1:cd|12:1:CDE", "awwwxefwwwxF",     // "awww[cd>efwww[CDE>F" => "awwwx|efwwwx|F
                         "1:4:bcd|7:4:ABCDE", "awwwxefwwwxF" );
}

///-MUTIPLE RANGE REMOVE
///========================== "abcdefABCDEF"
///- "1>2,7>10"  ,  "1:b,7:BCD" , ""       => "a|cdefA|EF"
///- "1>3,6>8"   ,  "1:cd,6:DEF" , ""      => "a|efA"
///=> 1>4,7>12   ,  "1:bcd,7:BCDEF",  ""
void ReplaceSelectionCommandTest::testCoalesce_removeRange_multiRange()
{
    buf()->setText("abcdefABCDEF");

    testReplace2Coalesc( "1>2,7>10", "", "1:0:b|6:0:BCD", "acdefAEF",     // "a[b>cdefA[BCD>EF"    => "a|cdefA|EF"
                         "1>3,6>8", "",  "1:0:cd|5:0:EF", "aefA",     // "a[cd>efA[EF>" => "a|efA|
                         "1:0:bcd|4:0:BCDEF", "aefA" );
}

/// MULTIPLE BACKSPACE
///========================== "abcdefABCDEF"
///- "4>3,9>8"  ,  "3:d,8:C"  ,  ""         => "abc|efAB|DEF"
///- "3>2,7>6"  ,  "2:c,6:B"  ,  ""         => "ab|efA|DEF"
///=> 4>2,9>7   ,  "2:cd,7:BC",  ""
void ReplaceSelectionCommandTest::testCoalesce_backspace_multiRange()
{
    buf()->setText("abcdefABCDEF");

    testReplace2Coalesc( "4>3,9>8", "", "3:0:d|7:0:C", "abcefABDEF",   // "abc<d]efAB<C]DEF"  => "abc|efAB|DEF"
                         "3>2,7>6", "", "2:0:c|5:0:B", "abefADEF",     // "ab<c]efA<B]DEF"    => "ad|efA|DEF"
                         "2:0:cd|5:0:BC", "abefADEF" );

}

/// MULTIPLE DELETE
///========================== "abcdefABCDEF"
///- "1>2,8>9"  ,  "1:b,8:C" , ""              => "a|cdefAB|DEF"
///- "1>2,7>8"  ,  "1:c,7:D" , ""              => "a|defAB|EF"
///=> 1>3,9>10  ,  "1:bc,8:CD",  ""
void ReplaceSelectionCommandTest::testCoalesce_delete_multiRange()
{
    buf()->setText("abcdefABCDEF");
    testReplace2Coalesc( "1>2,8>9", "", "1:0:b|7:0:C", "acdefABDEF",    // "a[b>cdefAB[C>DEF"  =>  "a|cdefAB|DEF"
                         "1>2,7>8", "", "1:0:c|6:0:D", "adefABEF",    // "a[c>defAB[D>EF" => "a|defAB|EF"
                         "1:0:bc|6:0:CD", "adefABEF" );
}
#endif

} // edbee
