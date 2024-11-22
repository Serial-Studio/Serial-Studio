/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "commentcommand.h"

#include "edbee/edbee.h"
#include "edbee/models/dynamicvariables.h"
#include "edbee/models/textlexer.h"
#include "edbee/models/texteditorconfig.h"
#include "edbee/models/textbuffer.h"
#include "edbee/models/textdocument.h"
#include "edbee/models/textdocumentscopes.h"
#include "edbee/views/textselection.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/util/rangelineiterator.h"
#include "edbee/util/regexp.h"

#include "edbee/debug.h"

namespace edbee {

// TODO, we need to rewrite this for improved testability
// TODO, this code can be much clearer, if we let go of the TM_ structure!!

/// A helper structures for storing comment start and end definitions
class CommentDefinitionItem
{
public:
    /// constructs the definition
    CommentDefinitionItem( const QString& startIn, const QString& endIn )
        : start_(startIn)
        , end_(endIn)
        , commentStartRegExp_(0)
        , removeCommentStartRegeExp_(0)
    {
    }

    /// the destructor
    virtual ~CommentDefinitionItem()
    {
        delete removeCommentStartRegeExp_;
        delete commentStartRegExp_;
    }


    /// returns the start regular expression
    const QString& start() { return start_;  }

    /// returns the end regular expression
    const QString& end() { return end_; }

    /// returns true if the definition is a block comment
    bool isBlockComment() { return !end_.isEmpty(); }

    /// creates the regexp for the start regular expression
    RegExp* commentStartRegExp()
    {
        if( !commentStartRegExp_ ) {
            commentStartRegExp_ = new RegExp( QStringLiteral("^\\s*%1").arg( RegExp::escape( start().trimmed() ) ) );
        }
        return commentStartRegExp_;
    }

    /// creates the regexp for removing a comment
    RegExp* removeCommentStartRegeExp()
    {
        if( !removeCommentStartRegeExp_ ) {
            removeCommentStartRegeExp_ = new RegExp( QStringLiteral("^[^\\S\n]*(%1[^\\S\n]?)").arg( RegExp::escape( start().trimmed() ) ) );
        }
        return removeCommentStartRegeExp_;
    }


private:
    QString start_;
    QString end_;
    RegExp* commentStartRegExp_;
    RegExp* removeCommentStartRegeExp_;
};


/// A class to temporary store all comment definitions for the current scope
/// placed in a class so we can make use of
class CommentDefinition
{
public:
    /// the destructor deletes all mail comment definitions
    ~CommentDefinition() { qDeleteAll(commentDefinitionList_); }

    /// returns a reference to the list for direct access
    QList<CommentDefinitionItem*>& list() { return commentDefinitionList_; }

    /// this method returns the first comment definition with the given criteria
    /// It return 0 if no block definition has been found
    /// @param block should we find a block definition
    CommentDefinitionItem* findDefinition( bool block )
    {
        foreach( CommentDefinitionItem* item, commentDefinitionList_ ) {
            if( block == item->isBlockComment() ) { return item; }
        }
        return 0;
    }

private:
    QList<CommentDefinitionItem*> commentDefinitionList_;
};


//--------------------------------


/// This method checks if all lines are comment
/// @param doc the document to check
/// @param range the range for checking all commented lines
/// @param commentStart the text used as the line comment prefix
/// @return true if all lines are commented
static bool areAllLinesCommented( TextDocument* doc, TextRange& range, const QList<CommentDefinitionItem*>& definitions )
{

    // we directly check in the buffer for fast regexp without QString creation
    TextBuffer* buf = doc->buffer();

    // iterate over all lines
    RangeLineIterator itr( doc, range );
    while( itr.hasNext() ) {

        int line = itr.next();
        int lineLength = doc->lineLengthWithoutNewline(line);

        // when it's the last line and its blank, we must skip it
        if( !itr.hasNext() && lineLength == 0 ) {
            continue;
        }

        // directly search in the raw data pointer buffer to prevent QString creation
        int offset = doc->offsetFromLine(line);

        // iterate over all line definitions
        bool found = false;
        foreach( CommentDefinitionItem* def, definitions ) {

            // skip block comments
            if( def->isBlockComment() ) { continue; }

            RegExp* commentStartRegExp = def->commentStartRegExp();

            /// toggle the found flag if a comment is found and break
            if( commentStartRegExp->indexIn( buf->rawDataPointer(), offset, offset+doc->lineLength(line)-1 ) >= 0 ) {
                found = true;
                break;
            }
        }

        if( !found ) {
            return false;
        }
    }
    return true;
}


/// removes all line comments from the iven line
/// @param doc the document to change
/// @param range the range to remove the comments
/// @param commentStart the text used as the line comment prefix
static void removeLineComment( TextDocument* doc, TextRange& range, const QList<CommentDefinitionItem*>& definitions  )
{
//    RegExp regExp( QStringLiteral("^[^\\S\n]*(%1[^\\S\n]?)").arg( RegExp::escape(commentStart.trimmed() ) ) );
    TextBuffer* buf = doc->buffer();

    // iterate over all lines and build all ranges
    RangeLineIterator itr( doc, range );
    while( itr.hasNext() ) {

        // directly search in the raw data pointer buffer to prevent QString creation
        int line = itr.next();
        int offset = doc->offsetFromLine(line);

        // iterate over alll definitions
        foreach( CommentDefinitionItem* def, definitions ) {

            // skip block comments
            if( def->isBlockComment() ) { continue; }

            RegExp* regExp = def->removeCommentStartRegeExp();

            // perform a regexp to extract the comment that needs to be removed
            if( regExp->indexIn( buf->rawDataPointer(), offset, offset + doc->lineLength(line) ) >= 0 ) {

                // remove the found regexp and goto the next line
                doc->replace( regExp->pos(1), regExp->len(1), "" );
                break;
            }
        }


    }

}


/// Adds comments on all line starts
/// @param doc the document to insert the comment
/// @param range the textrange to insert comments
/// @param str the comment prefix
static void insertLineComments( TextDocument* doc, TextRange& range, const QString& str )
{
    TextBuffer* buf = doc->buffer();

    // iterate over all lines and build all ranges
    RangeLineIterator itr( doc, range );
    while( itr.hasNext() ) {

        // directly search in the raw data pointer buffer to prevent QString creation
        int line = itr.next();
        int offset = doc->offsetFromLine(line);
        int lineLength = doc->lineLengthWithoutNewline(line);

        // when it's the last line and its blank, we must skip it
        if( !itr.hasNext() && lineLength == 0 ) {
            continue;
        }

        // when there's no comment found at this line return false
        int wordStart = buf->findCharPosWithinRangeOrClamp( offset, 1, doc->config()->whitespaces(), false, offset, offset + lineLength );
        doc->replace(wordStart,0,str);

    }
}


/// Removes a block comment if possible
/// @param controller the controller to perform the operation on
/// @param range the current textrange
/// @param commentStart the start of the commment
/// @param commentEnd the end of the comment
/// @return this method returns true if the block comment was remoed
static bool removeBlockComment( TextEditorController* controller, TextRange& range, const QList<CommentDefinitionItem*>& definitions )
{
    TextDocument* doc = controller->textDocument();
    TextDocumentScopes* scopes = doc->scopes();

    // it seems that every language in sublime/textmate use the comment.block convention
    TextScope* blockCommentScope = Edbee::instance()->scopeManager()->refTextScope("comment.block");

    // we only fetch multi-line scoped textranges
    int middleRange= range.min()+(range.max()-range.min())/2;

    // With this uberdirty for loop, we also check if there's a commentscope
    // 1 character to the left. (this is required to fix the uncommnt block issue when the caret is next to the comment :)
    for( int i=0; middleRange&&i<2; ++i) {

        QVector<ScopedTextRange*> scopedRanges = scopes->createScopedRangesAtOffsetList(middleRange);

        foreach( ScopedTextRange* scopedRange, scopedRanges ) {
            TextScope* scope = scopedRange->scope();

            // did we found a block comment
            if( scope->startsWith( blockCommentScope ) ){
                int min = scopedRange->min();
                int max = scopedRange->max();

                // check all 'block' comments
                foreach( CommentDefinitionItem* def, definitions)  {

                    // skip if not a blockcomment
                    if( !def->isBlockComment() ) { continue; }

                    const QString& commentStart = def->start();
                    const QString& commentEnd = def->end();

                    // when the scope starts and ends with the comment start and end remove it
                    if( doc->textPart(min, commentStart.length() ) == commentStart  &&
                        doc->textPart(max - commentEnd.length(), commentEnd.length() ) == commentEnd ) {

                        // * warning! * we cannot use the scoped rangeset directly
                        // when we replace a text, the scoped rangeset could be invalidated and destroyed!

                        // next remove the range
                        doc->replace( min, commentStart.length(), "" );
                        doc->replace( max - commentEnd.length() - commentStart.length(), commentEnd.length(), "" );

                        qDeleteAll(scopedRanges);
                        return true;
                    }
                }
            }
        }
        qDeleteAll(scopedRanges);

        // 1 char to the left
        --middleRange;
    }
    return false;
}


/// Insert a block comment
/// Adds a comment start and end around the current text range.
/// If the textrange is empty, it first is expanded to a full line
/// @param controller the controller to insert a comment for
/// @param range the range to insert a block comment for
/// @param commentStart the start of the comment
/// @param commentEnd the end of the comment
/// @return the last affected range
static int insertBlockComment( TextEditorController* controller, TextRange& range, const QString& commentStart, const QString& commentEnd )
{
    TextDocument* doc = controller->textDocument();

    // for safety I don't use the range anymore. It's adjusted automatically, but could in theory vanish
    int min = range.min();
    int max = range.max();

    // when the range is 'blank' expand it to a full line
    if( range.isEmpty() ) {
        int line = doc->lineFromOffset(min);
        min = doc->offsetFromLine(line);
        max = min + doc->lineLengthWithoutNewline(line);
//        range.expandToFullLine(doc,0);
//        range.deselectTrailingNewLine(doc);
    }


    doc->replace(min,0, commentStart);
    doc->replace(max+commentStart.length(),0, commentEnd);

    return max + commentStart.length() + commentEnd.length();
}



/// Collect all active comment structures
static bool getCommentDefinitions( TextEditorController* controller, TextRange& range, QList<CommentDefinitionItem*>& definitions )
{
    TextDocument* doc = controller->textDocument();
    TextDocumentScopes* scopes = doc->scopes();

    // retrieve the starting scope
    TextScopeList startScopeList = scopes->scopesAtOffset( range.min(), true );

    // next find all comment definitions
    QString padding;
    int index=1;
    do
    {
        // retrieve the comment start
        // when there's no comment start then we're done
        QString commentStart = controller->dynamicVariables()->value( QStringLiteral("TM_COMMENT_START%1").arg(padding), &startScopeList ).toString();
        if( commentStart.isEmpty() ) {  break; }

        // when there's a start there could be an ending (for block comments)
        QString commentEnd = controller->dynamicVariables()->value( QStringLiteral("TM_COMMENT_END%1").arg(padding), &startScopeList ).toString();
        definitions.push_back( new CommentDefinitionItem( commentStart, commentEnd ) );

        // increase the index and change the padding
        ++index;
        padding = QStringLiteral("_%1").arg(index);
    }
    while( true );


    // return true if the comment isn't empty
    return !definitions.isEmpty();
}


/// Comments the given range
/// @param controller the controller that conatins the controller
/// @param range the range to comment
/// @return the last affected offset
static int commentRange( TextEditorController* controller, TextRange& range, bool block )
{
    CommentDefinition defs;
    // retrieve all comment definitions
    if( !getCommentDefinitions( controller, range, defs.list() ) ) {
         return range.max();
    }

    // here we need to remove an active block comment, if there is one
    if( removeBlockComment( controller, range, defs.list()) ) {
        return range.max();
    }

    // find the first block definition
    CommentDefinitionItem* blockDef = defs.findDefinition(true);
    CommentDefinitionItem* lineDef = defs.findDefinition(false);

    // check if we need to do a block comment
    if( blockDef && ( block || !lineDef ) ) {
        return insertBlockComment( controller, range, blockDef->start(), blockDef->end() );

    // check if we need to perform a line comment
    } else if( lineDef ) {
        TextDocument* doc = controller->textDocument();

        // we need to remove all comments
        if( areAllLinesCommented( doc, range, defs.list() ) ) {
            removeLineComment( doc, range, defs.list() );

        // add comments
        } else {
            insertLineComments( doc, range, lineDef->start() );
        }
    }
    return range.max();
}


//---------------------------


/// constructs the comment comment
/// @param block should we try a block comment
CommentCommand::CommentCommand(bool block)
    : block_(block)
{
}


/// Executes the given comment
/// @param controller the controller this command is executed for
void CommentCommand::execute(TextEditorController* controller)
{
    // iterate over all ranges
    TextRangeSet* selection = new DynamicTextRangeSet( controller->textSelection() );
    TextDocument* doc = controller->textDocument();

    // start changing the document
    // int lastOffset = -1;

    doc->beginChanges(controller);
    for( int i=0,cnt=selection->rangeCount(); i<cnt; ++i ) {

        TextRange range = selection->range(i);

        // assert the ranges are all lexed!
        controller->textDocument()->textLexer()->lexRange(range.min(), range.max());

        // comment the range and get the last offset
        // lastOffset = commentRange( controller, range, block_ );
        commentRange( controller, range, block_ );
    }

    doc->giveSelection( controller, selection );
    doc->endChanges(0);

    // the following call shouldn't be necessary
    controller->update();
}


/// converts this command to a string
QString CommentCommand::toString()
{
    return "CommentCommand";
}



} // edbee
