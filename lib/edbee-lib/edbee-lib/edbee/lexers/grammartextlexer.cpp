/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "grammartextlexer.h"

#include <limits>
#include <QStack>

#include "edbee/models/textgrammar.h"
#include "edbee/models/textdocument.h"
#include "edbee/models/textdocumentscopes.h"
#include "edbee/util/regexp.h"
#include "edbee/edbee.h"

#include "edbee/debug.h"

namespace edbee {

/// Constructs the grammar textlexer
/// @param scopes a reference to the scopes model
GrammarTextLexer::GrammarTextLexer(TextDocumentScopes* scopes)
    : TextLexer( scopes )
    , lineRangeList_( 0 )
{
    setGrammar( Edbee::instance()->grammarManager()->defaultGrammar() );
}


/// The destructor
GrammarTextLexer::~GrammarTextLexer()
{
    delete lineRangeList_;  // just in case
}


/// This method builds the end-regexp for the given multi-line-regexp
/// @param startRegExp the start regexp
/// @param endRegExStringIn the end regexp string
RegExp* GrammarTextLexer::createEndRegExp( RegExp* startRegExp, const QString& endRegExpStringIn)
{
    // build the end-regexp string
    QString endRegExpString;
    RegExp matcher("\\\\(\\d+)");  // \(d+)
    int lastPos = 0;
    while( matcher.indexIn(endRegExpStringIn,lastPos) >= 0 ) {
        int len = matcher.len();
        int pos = matcher.pos();

        // append the previous string
        if( pos-lastPos > 0 ) {
            endRegExpString.append( endRegExpStringIn.mid(lastPos,pos-lastPos) );
        }

        // append the 'match'
        int nr = matcher.cap(1).toInt();
        if( nr >= 0 ) {
            endRegExpString.append( startRegExp->cap(nr) );
        }
        lastPos = pos + len + 1;
    }
    endRegExpString.append( endRegExpStringIn.mid(lastPos));
    return new RegExp(endRegExpString);
}


/// Search the next grammar rule
/// @param (out) foundRegExp the found regexp
/// @param (out) foundPosition the found position
/// @return the grammarRule found
void GrammarTextLexer::findNextGrammarRule( const QString& line, int offsetInLine, TextGrammarRule* activeRule, TextGrammarRule*& foundRule, RegExp*& foundRegExp, int& foundPosition )
{

    // next iterate over all rules and find the rule with the lowest offset
    QStack<TextGrammarRule::Iterator*> ruleIterators;
    ruleIterators.push( activeRule->createIterator() );
    while( !ruleIterators.isEmpty() ) {

        while( ruleIterators.top()->hasNext() ) {
            TextGrammarRule* rule = ruleIterators.top()->next(); //activeRule->rule(i);

            bool processNewRule = false;
            do {
                processNewRule = false;
//qlog_info() << "     -   " << ":" << rule->toString(false);
                switch( rule->instruction() ) {
                    case TextGrammarRule::SingleLineRegExp:
                    case TextGrammarRule::MultiLineRegExp:
                    {
                        // only use this match if the offset < foundPosition
                        int pos = rule->matchRegExp()->indexIn( line, offsetInLine );
                        if( pos >= 0 ) {
                            if( pos < foundPosition ) {
                                foundRule      = rule;
                                foundRegExp    = rule->matchRegExp();
                                foundPosition   = pos;
                            }
                        }
                        break;
                    }

                    // and include rule
                    case TextGrammarRule::IncludeCall:
                    {
                        TextGrammarRule* includedRule= findIncludeGrammarRule( rule);
                        if( includedRule ) {
                            // a rule list, append the iterator
                            if( includedRule->isRuleList() || includedRule->isMainRule() ) {
                                ruleIterators.push_back( includedRule->createIterator() );

                            // a normal rule? We need to process this one again
                            } else {
                                rule = includedRule;
                                processNewRule = true;
                            }
                        } else {
                            qlog_warn() << "ERROR, include rule" << rule->includeName() << "not found!" ;
                        }
                        break;
                    }
                    case TextGrammarRule::MainRule:
                        Q_ASSERT(false && "a mainrule as child is not allowed!");
                        break;
                    case TextGrammarRule::RuleList:
                        Q_ASSERT(false && "a rule list is not directly allowed!");
                        break;
                    default:
                        Q_ASSERT(false && "unkown rule");
                }
            }
            while( processNewRule );
        } // while hasNext

        delete ruleIterators.pop();

    }// while ruleIterators
}


/// This method processes the captures and adds them to the active line
/// @param foundRegExp the found regexp
/// @param foundCaptures the found captures
void GrammarTextLexer::processCaptures( RegExp* foundRegExp, const QMap<int, QString>* foundCaptures )
{
    if( !foundCaptures->isEmpty() ) {
//qlog_info() << "found: captures:";
        QMapIterator<int,QString> itr(*foundCaptures);
        while( itr.hasNext() ) {
            itr.next();
            int captureIndex = itr.key();
            const QString scope = itr.value();

            int capturePos = foundRegExp->pos(captureIndex);
            if( capturePos >=0 ) {
                int capLen = foundRegExp->len(captureIndex);
                int start  = capturePos;
                int end    = capturePos+capLen;

//                textScopes()->addScopedRange( currentDocOffset+capturePos, currentDocOffset+capturePos+capLen, scope, foundRule );
                lineRangeList_->giveRange( new ScopedTextRange( start, end, Edbee::instance()->scopeManager()->refTextScope(scope) ));

            }
        }
    }
}


/// This is the main algorithm for finding and matching the correct scopes with help of the regular expressions
/// @param currentDocOffset the current document offset
/// @param line the line that's being matches
/// @param offsetInLine (in/out) the current offset in the line
TextGrammarRule* GrammarTextLexer::findAndApplyNextGrammarRule( int currentDocOffset, const QString& line, int& offsetInLine  )
{
    Q_ASSERT(lineRangeList_);

//    qlog_info() << "--- matchNextGrammarRule: " << currentDocOffset << ", line:" << line << ":"<<offsetInLine << "--";
    MultiLineScopedTextRange* activeMultiRange = this->activeMultiLineRange();
    TextGrammarRule* activeRule      = activeMultiRange->grammarRule();
//qlog_info() << "--- matchNextGrammarRule ---------------------";
//qlog_info() << " - activeRule : " << activeRule->toString();
//qlog_info() << " - activeRange: " << activeRange->toString();

    TextGrammarRule* foundRule = 0;
    RegExp* foundRegExp    = 0;
    int foundPosition      = std::numeric_limits<int>::max();


    // first try to close the active rule
    if( activeMultiRange->endRegExp() ) {
        if( activeMultiRange->endRegExp()->indexIn( line, offsetInLine ) >= 0 ) {
            foundRule      = activeRule;
            foundRegExp    = activeMultiRange->endRegExp();
            foundPosition  = foundRegExp->pos();
        }
    }

    // find the grammar rule
    findNextGrammarRule( line, offsetInLine, activeRule, foundRule, foundRegExp, foundPosition );

    // next we have found the rule that matched a certain scope
    if( foundRule ) {
        int matchedLength = foundRegExp->matchedLength();
//        if( !matchedLength ) { matchedLength=1; }   // always match at least 1 character! (else we're in big trouble )

        int startPos = foundPosition;
        int endPos   = startPos + matchedLength;
//qlog_info() << " foundRule: " << foundRule->toString(false) << ", foundPosition:" << foundPosition <<" : " << startPos << "t/m" << endPos;
//qlog_info() << " foundRegExp: " << foundRegExp->pattern();

        // the found captures

        // Did we found the endrule? Then  we need to 'close' the current activeRule
        if( activeMultiRange->endRegExp() == foundRegExp ) {
            activeMultiRange->maxVar() = currentDocOffset + endPos;         // mark the end (DOC)
            activeScopedTextRange()->maxVar() = endPos;                     // mark the end (TextScope)

            processCaptures( foundRegExp, &activeRule->endCaptures() );

            popActiveRange();

        // a normal match or start of multi-line
        } else {
            TextScope* scopeRef = Edbee::instance()->scopeManager()->refTextScope(foundRule->scopeName());

            // did we find a multiline regexp. add the start of this scope
            if( foundRule->isMultiLineRegExp() ) {

                ScopedTextRange* range = new ScopedTextRange(startPos, line.length(), scopeRef );
                lineRangeList_->giveRange( range );

                MultiLineScopedTextRange* multiRange = new MultiLineScopedTextRange( currentDocOffset+startPos, textScopes()->textDocument()->length(), scopeRef );
                multiRange->setGrammarRule( foundRule );
                multiRange->giveEndRegExp( createEndRegExp( foundRegExp, foundRule->endRegExpString() ) );

                pushActiveRange( range, multiRange );

            // a single rule
            } else {
                // add the found regexp
                lineRangeList_->giveRange( new ScopedTextRange(startPos, endPos, scopeRef ));
                //textScopes()->addScopedRange( startPos, endPos, foundRule->scopeName(), foundRule );
            }           

            // next we need to add the 'captures'
            processCaptures( foundRegExp, &foundRule->matchCaptures() );
        }


        // increase the offset for the next search
//qlog_info() << " ++ " << matchedLength;
        offsetInLine = foundPosition + matchedLength;

        return foundRule;
    }
//qlog_info() << "-- not found ??";
    return 0;   // no rule found
}


/// internal method that returns the active grammar rule
MultiLineScopedTextRange* GrammarTextLexer::activeMultiLineRange()
{
    Q_ASSERT( !activeMultiLineRangesRefList_.isEmpty() );
    MultiLineScopedTextRange* result = activeMultiLineRangesRefList_.last();
    Q_ASSERT( result->grammarRule() );
    return result;
}


/// Returns the active scoped text range
ScopedTextRange* GrammarTextLexer::activeScopedTextRange()
{
    Q_ASSERT( !activeScopedRangesRefList_.isEmpty() );
    return activeScopedRangesRefList_.last();
}


/// removes the active range from the stack
/// If the ranges is added for the current line, it is removed from the currentLineRangeList.
/// If the range was started at a previous line, the item is added to the closedActiveRangeRefList
void GrammarTextLexer::popActiveRange()
{
    Q_ASSERT( activeMultiLineRangesRefList_.size() > 1 ); // there should be at least 2 items. The first one is the grammar rule!
    Q_ASSERT( activeScopedRangesRefList_.size() == activeMultiLineRangesRefList_.size() );
    //
    if( currentMultiLineRangeList_.isEmpty() ) {
        closedMultiRangesRangesRefList_.append( activeMultiLineRangesRefList_.last() );
//        closedActiveRangesRefList_.append(range);
    // pushed and popped on this line, no document-scope
    } else {
        delete currentMultiLineRangeList_.last();
        currentMultiLineRangeList_.pop_back();
    }

    activeMultiLineRangesRefList_.pop_back();
    activeScopedRangesRefList_.pop_back();
}


/// Adds the given range to the multiscoped textranges
/// And to the list of current line ranges
/// @param range the range top push
void GrammarTextLexer::pushActiveRange(ScopedTextRange* range, MultiLineScopedTextRange* multiRange )
{
    activeMultiLineRangesRefList_.push_back( multiRange );
    currentMultiLineRangeList_.push_back( multiRange );
    activeScopedRangesRefList_.push_back( range );
//qlog_info() << "[push]";
//    activeRangesRefList_.push_back( range );
//    currentLineRangesList_.push_back(range);
}


/// T his method finds the 'included' grammar rule
TextGrammarRule* GrammarTextLexer::findIncludeGrammarRule(TextGrammarRule* base)
{
    Q_ASSERT( base->isIncludeCall() );
    QString name = base->includeName();

    // repos call
    if( name.startsWith("#")) {
        return base->grammar()->findFromRepos( name.mid(1) );

    }
    // another language call
    // The difference between $base and $self is very subtle.. The exact difference is unkown to me..
    if( name=="$base" || name == "$self" ) {
        return grammar()->mainRule();
    }

    TextGrammar* grammar = Edbee::instance()->grammarManager()->get( name );
    if( grammar ) { return grammar->mainRule(); }
    return 0;
}



/// This method is called to notify the lexer some data has been changed
//void GrammarTextLexer::textReplaced( int offset, int length, int newLength )
void GrammarTextLexer::textChanged( const TextBufferChange& change )
{
    TextDocument* doc = textDocument();
    TextDocumentScopes* docScopes = textScopes();

    int offsetStart = doc->offsetFromLine(change.line());
    docScopes->removeScopesAfterOffset(offsetStart);

    /// TODO: rebuild an optimized scope-rebuilding algorithm
}


/// This method lexes a single line
/// @return the last indexed offset
/// WARNING lexline CANNOT be called indepdently of lexLines (beacuse lex-lines set the activeScopes!
bool GrammarTextLexer::lexLine( int lineIdx, int& currentDocOffset )
{
    TextDocument* doc = textDocument();
    TextDocumentScopes* docScopes = textScopes();

    QString line        = doc->line(lineIdx); //+ "\n";

    //    int lineStartOffset = doc->offsetFromLine(lineIdx);

    Q_ASSERT( currentMultiLineRangeList_.isEmpty() );
    Q_ASSERT( closedMultiRangesRangesRefList_.isEmpty() );
    Q_ASSERT( activeScopedRangesRefList_.isEmpty() );

    lineRangeList_ = new ScopedTextRangeList();

    // append the active ranges
    for( int i=0,cnt=activeMultiLineRangesRefList_.size(); i<cnt; ++i ) {
        MultiLineScopedTextRangeReference* range = new MultiLineScopedTextRangeReference( *activeMultiLineRangesRefList_.at(i) );
        range->setAnchor(0);
        range->setCaret(line.length());
        lineRangeList_->giveRange( range );
        activeScopedRangesRefList_.append(range);
    }

// qlog_info() << "";
// qlog_info() << "////////////////////////////////////////////////////////////////";
// qlog_info() << " * " << lineIdx << ":" << line;

    // find the first 'matching' rule
    int offsetInLine = 0;
    int lastOffsetInLine = 0;
    TextGrammarRule* lastFoundRule = 0;
    while( true ) {
//QString debug;
//debug.append( QStringLiteral((" =[%1,%2,%3]= ").arg(lineIdx).arg(offsetInLine).arg(currentDocOffset) );
        TextGrammarRule* foundRule = findAndApplyNextGrammarRule( currentDocOffset, line, offsetInLine  );
//debug.append( QStringLiteral((" %1  (%2)").arg(foundRule?foundRule->scopeName():"<<null>").arg(offsetInLine) );
//qlog_info() << debug;
        if( !foundRule ) break;

        /// check the next offset
        if( offsetInLine == lastOffsetInLine ) {

            if( lastFoundRule == foundRule ) {

                // I'm very much in doubt how to solve this. What should happend here?
                // when the same ruleis found, we need to stop else we will keep on
                // getting this rule forever.
                //
                // btw. A grammar that fires this situation is the Ruby Haml language
                // when using the %tagname construct
                //
                // There are several options to fix this:
                // 1. simply ending the parsing of this line with break (Risk is that the rest of the line isn't parsed)
                // 2. increase offsetInLine to the next character. (Risk is that prasing goes wrong, and start identifying at the wrong position)
                // 3. Keep a list with found rules and never allow the same rule again. (Compelexer to implement don't really know if this is the solution)


                // DEBUG info:
                //qlog_info() << "Found grammar-rule("<< lineIdx << "," << offsetInLine<<")";
                //qlog_info() << " - line: " << line;
                //qlog_info() << " - rule: " << foundRule->toString();
                //qlog_info() << " INFINITE LOOP PREVENTION";

                // we're going for option [2] for the moment
                ++offsetInLine; // never have an endless loop

                // option[1] also works.
                //break;
            }
        }
        lastFoundRule = foundRule;

        lastOffsetInLine = offsetInLine;
    }

    // when there are no-multi-line spanning rules, set the independent flag
    lineRangeList_->setIndependent( currentMultiLineRangeList_.isEmpty() && closedMultiRangesRangesRefList_.isEmpty());
    lineRangeList_->squeeze();  // free unused memory
    bool result = lineRangeList_->isIndependent();

    // give the line to the document scopes
    docScopes->giveLineScopedRangeList( lineIdx, lineRangeList_ );
    lineRangeList_ = 0;

    foreach( MultiLineScopedTextRange* scopedRange, currentMultiLineRangeList_ ) {
        docScopes->giveMultiLineScopedTextRange(scopedRange);
    }
    activeScopedRangesRefList_.clear();
    currentMultiLineRangeList_.clear();
    closedMultiRangesRangesRefList_.clear();

    // increase the current document offset
    currentDocOffset += line.size(); // + 1;    // +1 because we didn't retrieve the newline
    return result;
}


/// This method lexes a range of line
/// @return the last indexed offset
void GrammarTextLexer::lexLines(int lineStart,int lineCount)
{

    // (INIT) ALGORITHM BELOW:
    //
    // - first find the current active scopes.
    // - all multi-line-scopes that start on this line we need to 'remove'
    // - all multi-line-scopes that end on this line we need to set the end to 'unkown'
    // - next we must find the ruleset for the current scopes by going into the grammar rules

    // WHILE there are chars left to match
    // - run all rule reg-exps for this scope and
    // - also run all reg-exps for active multi-line scopes to find (a possibly new end-marker)
    // - use the  the rule regexp with the first offset. Add the range to the scopes
    // - if this is a begin-block regexp, activate the new ruleset.  (check if the end-regexp is here)
    //------------------------

    TextDocument* doc = textDocument();
    TextDocumentScopes* docScopes = textScopes();

//qlog_info() << "===== lexText(" << offset << "," << length << ") ["<<lineStart <<","<<lineEnd<<"] ======";

    // next find all 'active' scoped ranges
    int offsetStart = doc->offsetFromLine(lineStart);
    activeMultiLineRangesRefList_ = docScopes->multiLineScopedRangesBetweenOffsets( offsetStart, offsetStart);

//    GrammarRule* activeRule = grammarRef_->mainRule();
//    if( !activeScopedRanges.isEmpty() ) { activeRule = activeScopedRanges.last()->grammarRule(); }
//    Q_ASSERT( activeRule );

    // next find the rule
    int currentDocOffset = offsetStart;
    bool independent = true;
    for( int idx=0; idx<lineCount; ++idx) {
        independent = lexLine( lineStart+idx, currentDocOffset  ) && independent;
    }

    // only set the scoped offset if less and not indepdent
    if( currentDocOffset < docScopes->lastScopedOffset() ) {
        if( !independent ) {
            docScopes->setLastScopedOffset(currentDocOffset);
            docScopes->removeScopesAfterOffset(currentDocOffset);
        }
    // further down the document, update the scope
    } else {
        docScopes->setLastScopedOffset(currentDocOffset);
        docScopes->removeScopesAfterOffset(currentDocOffset);
    }
}


/// This method is called when the given range needs to be lexed
///
/// WARNING, this method must be VERY optimized and should 'remember' the lexing
/// states between calls. To invalidate the scopes/lexing state the textReplaced method can be used
///
/// @param beginOffset the first offset
/// @param endOffset the last offset to
void GrammarTextLexer::lexRange( int beginOffset, int endOffset )
{
    Q_UNUSED(beginOffset);

    // find the beginning of the given line
    TextDocument* doc = textDocument();
    TextDocumentScopes* docScopes = textScopes();

    // no lexing required
    if( endOffset <= docScopes->lastScopedOffset()) {
        return;
    }

    // first we need to find the correct location to start from
    int offset = docScopes->lastScopedOffset(); //qMin( docScopes->scopedToOffset(), offset );

    int lineStart   = doc->lineFromOffset(offset);
    int lineEnd     = doc->lineFromOffset(endOffset) + 1;

    lexLines(lineStart, lineEnd-lineStart);
}


} // edbee
