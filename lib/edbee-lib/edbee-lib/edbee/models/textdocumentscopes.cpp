/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textdocumentscopes.h"

#include <math.h>

#include "edbee/models/textbuffer.h"
#include "edbee/models/textdocument.h"
#include "edbee/edbee.h"
#include "edbee/util/regexp.h"

#include "edbee/debug.h"

namespace edbee {

/// A scoped text range
/// @param anchor the start of the range
/// @param caret the caret position of the range
/// @param scope the text scope
ScopedTextRange::ScopedTextRange(int anchor, int caret, TextScope* scope)
    : TextRange(anchor,caret)
    , scopeRef_(scope)
{
    Q_ASSERT(scopeRef_);
}


/// The default destructor
ScopedTextRange::~ScopedTextRange()
{
}


/// sets the scope of this textrange
void ScopedTextRange::setScope(TextScope* scope)
{
    Q_ASSERT(scope);
    scopeRef_ = scope;
}


/// returns the scope
TextScope* ScopedTextRange::scope() const
{
    return scopeRef_;
}


/// Converts the scoped textrange to a string
QString ScopedTextRange::toString() const
{
    return QStringLiteral("%1>%2:%3").arg(anchor()).arg(caret()).arg(scopeRef_->name() );
}


//===========================================



/// Creates a scoped text range based on a scope textrange
MultiLineScopedTextRangeReference::MultiLineScopedTextRangeReference(MultiLineScopedTextRange& range)
    : ScopedTextRange( range.anchor(), range.caret(), range.scope() )
    , multiScopeRef_( &range )
{
}


// the destructor
MultiLineScopedTextRangeReference::~MultiLineScopedTextRangeReference()
{
}


/// Returns the reference the multilinescoped textrange
MultiLineScopedTextRange* MultiLineScopedTextRangeReference::multiLineScopedTextRange()
{
    return multiScopeRef_;
}


//===========================================


/// A scoped textrange lsit
ScopedTextRangeList::ScopedTextRangeList()
    : ranges_()
    , independent_(false)
{
}


/// The default destructor
ScopedTextRangeList::~ScopedTextRangeList()
{
    qDeleteAll(ranges_);
    ranges_.clear();
}


/// Retursn the number of scoped textranges in the list
int ScopedTextRangeList::size() const
{
    return ranges_.size();
}


/// Returns the scoped textrange at the given list
ScopedTextRange* ScopedTextRangeList::at(int idx)
{
    Q_ASSERT(idx < ranges_.size() );
    return ranges_.at(idx);
}


/// give a rnage to the range set
/// @param range the scoped text range
void ScopedTextRangeList::giveRange(ScopedTextRange* range)
{
    ranges_.append(range);
}


/// Prepends a range
/// @param range the range to prepend
void ScopedTextRangeList::giveAndPrependRange(ScopedTextRange* range)
{
    ranges_.prepend(range);
}


/// Squeezes the ranges (reduces the memory usage)
void ScopedTextRangeList::squeeze()
{
    ranges_.squeeze();
}


/// set the independent flag. The independent flag means it's not dependent on other lines
void ScopedTextRangeList::setIndependent(bool enable)
{
    independent_ = enable;;
}


/// returns the independent flag
bool ScopedTextRangeList::isIndependent() const
{
    return independent_;
}


/// Converts the scoped textrange list to a strubg
QString ScopedTextRangeList::toString()
{
    QString result;
    result.append( independent_ ? "[-]" : "[M]");
    foreach( ScopedTextRange* scope, ranges_ ) {
        if( !result.isEmpty() ) { result.append("| "); }
        result.append( scope->toString() );
    }
    return result;
}


//===========================================


/// The multiline scoped textrange
/// @param anchor
MultiLineScopedTextRange::MultiLineScopedTextRange(int anchor, int caret, TextScope* scope )
    : ScopedTextRange(anchor,caret,scope)
    , ruleRef_(0)
    , endRegExp_(0)
{
}


/// The multi-line destructor
MultiLineScopedTextRange::~MultiLineScopedTextRange()
{
    delete endRegExp_;
}


/// Sets the rule (we need the rule to perform end-of-line matching)
void MultiLineScopedTextRange::setGrammarRule(TextGrammarRule* rule)
{
    ruleRef_ = rule;
}


/// Returns the active grammar rule
TextGrammarRule* MultiLineScopedTextRange::grammarRule() const
{
    return ruleRef_;
}


/// Gives the end regular expression
void MultiLineScopedTextRange::giveEndRegExp( RegExp* regExp)
{
    endRegExp_ = regExp;
}


/// returns the end-regular expression
RegExp*MultiLineScopedTextRange::endRegExp()
{
    return endRegExp_;
}


/// This method compares selection ranges
bool MultiLineScopedTextRange::lessThan(MultiLineScopedTextRange* r1, MultiLineScopedTextRange* r2)
{
    int diff = r1->min() - r2->min();
    if( diff == 0 ) {
        diff = r2->length() - r1->length();     // larger areas need to be placed before smaller areas
    }
    return ( diff < 0);
}


/// Returns a reference to the textdocument scopes
TextDocumentScopes* MultiLineScopedTextRangeSet::textDocumentScopes()
{
     return textDocumentScopesRef_;
}



//===========================================


/// A textcope is an object that represents a scope name
/// You usually don't need to supply the scopeManager parameter. It's only required on application startup
///
/// @param fullScope the name of the scope
/// @param scopeManager the scopemanager to use (when 0 this defaults to the global edbee scopemanager)
TextScope::TextScope(const QString& fullScope)
    : scopeAtomCount_(0)
    , scopeAtoms_(0)
{
    QStringList scopeElementNames = fullScope.split(".");
    scopeAtomCount_ = scopeElementNames.length();

    TextScopeManager* sm = Edbee::instance()->scopeManager();
    scopeAtoms_ = new TextScopeAtomId[scopeAtomCount_];
    for( int i=0; i < scopeAtomCount_; ++i ) {
        TextScopeAtomId id = sm->findOrRegisterScopeAtom(scopeElementNames.at(i));
        scopeAtoms_[i] = id;
    }
}

/// this method constructs a blank text scope.
TextScope::TextScope()
    : scopeAtomCount_(0)
    , scopeAtoms_(0)
{

}


// Destructs the textscope
TextScope::~TextScope()
{
    delete[] scopeAtoms_;
}


/// This method returns the name of this scope
const QString TextScope::name()
{
    QString str;
    TextScopeManager* sm = Edbee::instance()->scopeManager();
    for( int i=0; i < scopeAtomCount_; ++i ) {
        if( i ) { str.append("."); }
        str.append( sm->atomName( scopeAtoms_[i] )) ;
    }

    return str;
}


/// returns the number of atom items
int TextScope::atomCount()
{
    return scopeAtomCount_;
}


/// This method returns the atom at the given index
TextScopeAtomId TextScope::atomAt(int idx) const
{
    Q_ASSERT(idx < scopeAtomCount_ );
    return scopeAtoms_[idx];
}


/// Checks if the current scope starts with the given scope
/// wild-card atoms will always match
/// @param scope the scope to check
bool TextScope::startsWith(TextScope* scope)
{
    if( scope->atomCount() > atomCount() ) { return false; }    // match is never possible
    TextScopeAtomId wildCard = Edbee::instance()->scopeManager()->wildcardId();
    for( int i=0; i<scope->scopeAtomCount_; ++i ) {
        TextScopeAtomId atomId = scopeAtoms_[i];
        TextScopeAtomId scopeAtomId = scope->scopeAtoms_[i];
        bool atomEqual =  atomId == scopeAtomId || atomId == wildCard || scopeAtomId == wildCard ;
        if( !atomEqual ) { return false; }
    }
    return true;
}


/// This method returns the 'index' of the given full-scope. This is a kind of substring search
///
/// - wildcard atoms will always match
///
/// @param scope the scope to search
/// @return -1 if not found else the index where the scope is found
int TextScope::rindexOf(TextScope* scope)
{
    int searchSize = scope->atomCount();
    int end = atomCount() - searchSize;

    TextScopeAtomId wildCard = Edbee::instance()->scopeManager()->wildcardId();

    for( int idx=end; idx>=0; --idx ) {

        // try to match the scopes
        int i=0;
        for(; i<searchSize; ++i ) {
            TextScopeAtomId atomId = scopeAtoms_[idx+i];
            TextScopeAtomId scopeAtomId = scope->scopeAtoms_[i];
            bool atomEqual =  atomId == scopeAtomId || atomId == wildCard || scopeAtomId == wildCard ;
            if( !atomEqual ) { break; }
        }
        // when at the end we've found it
        if( i == searchSize ) { return idx; }
    }
    return -1;
}


//=============================================


/// A list of text scopes
TextScopeList::TextScopeList()
{
}


/// The text scopelist with an initial size
TextScopeList::TextScopeList(int initialSize)
{
    reserve(initialSize);
}


/// Fills the scopelist with a list of ranges
TextScopeList::TextScopeList(QVector<ScopedTextRange *>& ranges)
{
    reserve(ranges.size());
    foreach( ScopedTextRange* scopedRange, ranges ) {
        append( scopedRange->scope() );
    }
}


/// This method returns the total number of atoms
int TextScopeList::atomCount() const
{
    int result = 0;
    for( int i=0,cnt=size(); i<cnt; ++i ) {
        result += at(i)->atomCount();
    }
    return result;
}


/// Converts the scopelist to a string
QString TextScopeList::toString()
{
    QString result;
    for( int i=0; i<size(); ++i ) {
        if( i ) { result.append(" "); }
        result.append( at(i)->name() );
    }
    return result;
}

//=============================================


/// The textscope selector
/// @param selector the selector to select a scope
TextScopeSelector::TextScopeSelector(const QString& selector)
{
    QStringList selectorList = selector.split(",");
    selectorList_.reserve(selectorList.size());
    foreach( QString sel, selectorList ) {
        sel = sel.trimmed();
        selectorList_.append( Edbee::instance()->scopeManager()->createTextScopeList(sel) );
    }
}


/// The destructor
TextScopeSelector::~TextScopeSelector()
{
    qDeleteAll(selectorList_);
    selectorList_.clear();
}


/// This method calculates the matching score of the scope with this selector
/// Currently the scope-calculation is very 'basic'. Just enough to perform the most basic form of matching
///
/// The algorithm returns 1.0 on a full match...
///
///
/// @param scopeList the list of scopes
/// @return the matching score. -1 means no match found. >= 0 a match has been found
double TextScopeSelector::calculateMatchScore(const TextScopeList* scopeList)
{
    double result=-1.0;
    foreach( TextScopeList* selector, selectorList_ ) {
        result = qMax( result, calculateMatchScoreForSelector( selector, scopeList) );
    }
    return result;
}


/// Converts the scope to a string
QString TextScopeSelector::toString()
{
    QString result;
    for( int i=0,cnt=selectorList_.size(); i<cnt; ++i ) {
        if( i ) { result.append(", "); }
        result.append(selectorList_.at(i)->toString());
    }
    return result;
}


/// This method calculates the matching score of the scope with this selector
/// Currently the scope-calculation is very 'basic'. Just enough to perform the most basic form of matching
///
/// The algorithm returns 1.0 on a full match...
///
///
/// @param scopeList the list of scopes
/// @return the matching score. -1 means no match found. >= 0 a match has been found
double TextScopeSelector::calculateMatchScoreForSelector(TextScopeList* selector, const TextScopeList* scopeList)
{
    double result = 0.0;
    double power = 0.0;

    // this method checks if the scope matches
    bool foundMatch=false;
    int nextScopeIdx = scopeList->size()-1;
    for( int selectorIdx=selector->size()-1; selectorIdx>=0; --selectorIdx ) {
        TextScope* selectorPath = selector->at(selectorIdx);
        foundMatch=false;

        // find the given selector scope in the list
        for( int scopeIdx=nextScopeIdx; scopeIdx>= 0; --scopeIdx ) {
            TextScope* scope = scopeList->at(scopeIdx);
            power += scope->atomCount();

            // dit we found the scope
            if( scope->startsWith(selectorPath ) ) {
                nextScopeIdx = scopeIdx-1;
                foundMatch = true;
                for( int i=0,iCnt=selectorPath->atomCount(); i<iCnt; ++i ) {
                    result += 1.0 / pow( 2, power - i );
                }
                break;
            }
        }

        // no match found?
        if( !foundMatch ) { return -1; }
    }
    return result;
}



//=============================================


/// The scopemanager constructor
TextScopeManager::TextScopeManager()
{
    reset();
}


/// The destructor of the scope manager
TextScopeManager::~TextScopeManager()
{
    foreach( TextScope* textScope, textScopeList_ ) { delete textScope; }
    textScopeList_.clear();
    textScopeRefMap_.clear();
}


/// This method clears and delets all scopes. WARNING this destroys all registered text-scopes
/// This method also registers the wildcard scope atom id
void TextScopeManager::reset()
{
    // delete and clear the scopemaps
    if( !textScopeList_.isEmpty() ) {
        foreach( TextScope* textScope, textScopeList_ ) { delete textScope; }
        textScopeList_.clear();
        textScopeRefMap_.clear();
    }

    // clear the atomlists
    if( !atomNameList_.isEmpty() ) {
        atomNameList_.clear();
        atomNameMap_.clear();
    }

    // insert some defaults
    wildCardId_ = findOrRegisterScopeAtom("*");     // register the 'start' wildcard

    // create a blank textscope
    TextScope* scope = new TextScope();
    textScopeList_.append(scope);
    textScopeRefMap_.insert("",scope);
}


/// returns the wildcard reference id
TextScopeAtomId TextScopeManager::wildcardId()
{
    return wildCardId_;
}


/// This method registers the scope element
TextScopeAtomId TextScopeManager::findOrRegisterScopeAtom(const QString& atom)
{
//    element = element.toLower().trimmed();
    TextScopeAtomId id = atomNameMap_.value(atom,-1);
    if( id >= 0 ) { return id; }
    atomNameList_.append(atom);
    id = atomNameList_.size()-1;
    atomNameMap_.insert(atom,id);
    return id;
}


/// This method finds or creates a full-scope
TextScope* TextScopeManager::refTextScope(const QString& scopeString)
{
    TextScope* scope = textScopeRefMap_.value(scopeString,0);
    if( scope ) { return scope; }
    scope = new TextScope(scopeString);
    textScopeList_.append(scope);
    textScopeRefMap_.insert(scopeString,scope);
    return scope;
}


/// Returns the empty scope
/// @return the blank textscope
TextScope* TextScopeManager::refEmptyScope()
{
    return refTextScope("");
}


/// This method creates a text-scope list from the given scope string
TextScopeList* TextScopeManager::createTextScopeList(const QString& scopeListString)
{
    QStringList scopeParts = scopeListString.split(" ");
    TextScopeList* list = new TextScopeList(scopeParts.size());
    for( int i=0, cnt=scopeParts.size(); i<cnt; ++i ) {
        list->append(  refTextScope(scopeParts.at(i)) );
    }
    return list;
}


/// Returns the name of the given atom id
const QString& TextScopeManager::atomName(TextScopeAtomId id)
{
    Q_ASSERT(0 <= id && id < atomNameList_.length() );
    return atomNameList_.at(id);
}


//===========================================


/// A multilinescoped textrange set
MultiLineScopedTextRangeSet::MultiLineScopedTextRangeSet(TextDocument *textDocument , TextDocumentScopes *textDocumentScopes)
    : TextRangeSetBase( textDocument )
    , textDocumentScopesRef_( textDocumentScopes )
{
}

/// the destructor of the multiline scoped rangeset
MultiLineScopedTextRangeSet::~MultiLineScopedTextRangeSet()
{
    reset();
}


/// Completely empties the scope list
void MultiLineScopedTextRangeSet::reset()
{
    qDeleteAll( scopedRangeList_ );
    scopedRangeList_.clear();
}


/// Returns the number of ranges
int MultiLineScopedTextRangeSet::rangeCount() const
{
    return scopedRangeList_.size();
}


/// returns the given textrange
TextRange& MultiLineScopedTextRangeSet::range(int idx)
{
    Q_ASSERT( 0 <= idx && idx < rangeCount() );
    return *(scopedRangeList_[idx]);
}


/// returns the cont range
const TextRange& MultiLineScopedTextRangeSet::constRange(int idx) const
{
    Q_ASSERT( 0 <= idx && idx < rangeCount() );
    return *(scopedRangeList_[idx]);
}


/// This method adds a range with the default scope
void MultiLineScopedTextRangeSet::addRange(int anchor, int caret)
{
    scopedRangeList_.append( new MultiLineScopedTextRange(anchor, caret,Edbee::instance()->scopeManager()->refEmptyScope() ) );
}


///adds the given range
void MultiLineScopedTextRangeSet::addRange(const TextRange& range)
{
    addRange( range.anchor(), range.caret() );
}


///' removes the given scope
void MultiLineScopedTextRangeSet::removeRange(int idx)
{
    delete scopedRangeList_[idx];
    scopedRangeList_.removeAt(idx);
}


/// removes all scopes
void MultiLineScopedTextRangeSet::clear()
{
    qDeleteAll( scopedRangeList_ );
    scopedRangeList_.clear();
}


void MultiLineScopedTextRangeSet::toSingleRange()
{
    Q_ASSERT( false ); ///< NOT IMPLEMENTED
    //scopedRangeList_.remove(1, scopedRangeList_.size()-1);
}


/// This method sorts all ranges
void MultiLineScopedTextRangeSet::sortRanges()
{
    std::sort(scopedRangeList_.begin(), scopedRangeList_.end(), MultiLineScopedTextRange::lessThan);
}


/// this method returns the scoped range
MultiLineScopedTextRange& MultiLineScopedTextRangeSet::scopedRange(int idx)
{
    Q_ASSERT( 0 <= idx && idx < rangeCount() );
    return *(scopedRangeList_[idx]);
}


/// Adds a textrange with the given name
MultiLineScopedTextRange& MultiLineScopedTextRangeSet::addRange(int anchor, int caret, const QString& name, TextGrammarRule* rule )
{
    MultiLineScopedTextRange* tr = new MultiLineScopedTextRange(anchor, caret, Edbee::instance()->scopeManager()->refTextScope(name) );
    tr->setGrammarRule( rule );
    scopedRangeList_.append( tr  );
    return *tr;
}


/// This method removes all ranges after a given offset. This means it will remove all
/// complete ranges after the given offset. Ranges that start before the offset and
/// end after the offset are 'invalidated' which means the end offset is placed to the end of the document
void MultiLineScopedTextRangeSet::removeAndInvalidateRangesAfterOffset(int offset)
{
    int len = textDocument()->length();
    beginChanges();
    for( int idx=rangeCount()-1; idx >= 0; idx-- ) {
        TextRange& range = this->range(idx);
        if( range.caret() >= offset && range.anchor() >= offset ) {
            removeRange(idx);
        } else if( range.min() < offset && range.max() >= offset ) {
            range.maxVar() = len;   // move the marker to the end
        }

    }
    endChangesWithoutProcessing();  // we only deleted the last range. Do the result is still sorted
}


/// This method gives the scoped text range to this object
void MultiLineScopedTextRangeSet::giveScopedTextRange(MultiLineScopedTextRange* textScope)
{
    scopedRangeList_.append( textScope );
}


/// This method process the changes if required
void MultiLineScopedTextRangeSet::processChangesIfRequired(bool joinBorders )
{
    Q_UNUSED( joinBorders );
    if( !changing_ ) {
        ++changing_; // prevent changing by functions below:
        //mergeOverlappingRanges(joinBorders);
        sortRanges();
        --changing_;
    }
}


/// convert the found ranges to strings
QString MultiLineScopedTextRangeSet::toString()
{
    QString result;
    for( int i=0,cnt=rangeCount(); i<cnt; ++i ) {
        MultiLineScopedTextRange& range = scopedRange(i);
        result.append(range.toString());
        result.append("|");
    }
    return result;
}


//===========================================


/// The constructor for storing/managing the textdocument scopes
/// @param textDocument the textdocument this scoping is for
TextDocumentScopes::TextDocumentScopes(TextDocument* textDocument)
    : textDocumentRef_( textDocument )
    , defaultScopedRange_(0,0,Edbee::instance()->scopeManager()->refTextScope("text.plain"))
    , scopedRanges_( textDocument, this )
    , lastScopedOffset_(0)
{
    connect( textDocument, SIGNAL(languageGrammarChanged()), this, SLOT(grammarChanged()) );
}


/// The destructor
TextDocumentScopes::~TextDocumentScopes()
{
    for( int i=0,cnt=lineRangeList_.length(); i<cnt; ++i ) {
        delete lineRangeList_.at(i);
    }
    lineRangeList_.clear();
}


/// returns the last scoped offset
int TextDocumentScopes::lastScopedOffset()
{
    return lastScopedOffset_;
}


/// Sets the last scoped offset
/// @param offset the last scoped offset
void TextDocumentScopes::setLastScopedOffset(int offset)
{
    int previousOffset = lastScopedOffset_ ;
    if( previousOffset != offset ) {
        lastScopedOffset_ = offset;
        emit lastScopedOffsetChanged( previousOffset, lastScopedOffset_);
    }
}


/// This method sets the default scope
/// @param the name of the scope
/// @param rule the rule that matched
void TextDocumentScopes::setDefaultScope(const QString& name, TextGrammarRule* rule )
{
    defaultScopedRange_.setScope( Edbee::instance()->scopeManager()->refTextScope(name) );
    defaultScopedRange_.setGrammarRule(rule);
}


/// This method sets the scoped line list
/// @param line the line
/// @param list the list with all scopes on the given line
void TextDocumentScopes::giveLineScopedRangeList(int line, ScopedTextRangeList* list)
{
    int len = lineRangeList_.length();
    if( line >= len ) {
        lineRangeList_.fill(len,0,0,line-len+1);
    }
    delete lineRangeList_.at(line); // delete a possible old value
    lineRangeList_.set(line, list);
}



/// This method returns all scoped ranges on the given line
/// @param line the line to retrieve the scoped ranges for
/// @return the scoped textrange list
ScopedTextRangeList* TextDocumentScopes::scopedRangesAtLine(int line)
{
    if( line >= lineRangeList_.length() || line < 0 ) { return 0; }
    return lineRangeList_.at(line);
}


/// Returns the number of scopes lines in the lineRangeList_
int TextDocumentScopes::scopedLineCount()
{
    return lineRangeList_.length();
}


/// gives the multi-lined textrange to the scopedranges
void TextDocumentScopes::giveMultiLineScopedTextRange(MultiLineScopedTextRange *range)
{
    scopedRanges_.giveScopedTextRange(range);
}


/// This method invalidates all scopes after the given offset
/// @param offset the offset from which to remove the offset
void TextDocumentScopes::removeScopesAfterOffset(int offset)
{
    if( offset == 0 ) {
        scopedRanges_.clear();
    } else {
        scopedRanges_.removeAndInvalidateRangesAfterOffset(offset);
    }
    if( offset < lastScopedOffset_ ) {
        setLastScopedOffset(offset);
    }


    // delete/remove all line ranges (after this line)
    int line = this->textDocument()->lineFromOffset(offset) + 1;
    if( line < lineRangeList_.length() ) {
        for( int i=line,cnt=lineRangeList_.length(); i<cnt; ++i ) {
            delete lineRangeList_.at(i);
        }

        lineRangeList_.replace(line, lineRangeList_.length()-line, 0, 0 );
    }
}


/// This method retursn the default scoped textrange
/// Currently this is done very dirty, by retrieving the defaultscoped range the begin and end is set tot he complete document
/// a better solution would be a subclass that always returns 0 for an anchor and the documentlength for the caret
MultiLineScopedTextRange& TextDocumentScopes::defaultScopedRange()
{
    defaultScopedRange_.set(0,textDocument()->length() );   // make sure this is always set like this
    return defaultScopedRange_;
}


/// This method returns all scope-ranges at the given offset-ranges
QVector<MultiLineScopedTextRange*> TextDocumentScopes::multiLineScopedRangesBetweenOffsets(int offsetBegin, int offsetEnd)
{
    QVector<MultiLineScopedTextRange*> result;
    result.append( &defaultScopedRange_ );
    int minOffset=0;
    for( int i=0, cnt=scopedRanges_.rangeCount(); i<cnt && minOffset <= /** CHANGED 2013-01-22, was < */ offsetEnd; ++i ) {
        MultiLineScopedTextRange& range = scopedRanges_.scopedRange(i);
        minOffset = range.min();
        int maxOffset = range.max();
        if( (offsetBegin <= minOffset && minOffset < offsetEnd) || (minOffset <= offsetBegin && offsetBegin < maxOffset) ) {
            result.append(&range);
        }
    }
    return result;
}


/// returns all scopes between the given offsets
TextScopeList TextDocumentScopes::scopesAtOffset( int offset, bool includeEnd  )
{
    TextScopeList result;

//    qlog_info() << "";
//    qlog_info() <<  "------------[ scopesAtOffset("<<offset<<") ] --------";
    //QVector<MultiLineScopedTextRange*> ranges = scopedRangesBetweenOffsets(offsetBegin, offsetEnd);
    int line = textDocument()->lineFromOffset(offset);
    int offsetInLine = offset-textDocument()->offsetFromLine(line);
    ScopedTextRangeList* list = scopedRangesAtLine(line);
    if( list ) {
        //scopes.reserve( ranges.size() );
        for( int i=0,cnt=list->size(); i<cnt; ++i ) {
            ScopedTextRange* range = list->at(i);
//QString debug;
//debug.append( QStringLiteral("- %1.%5: %2<=%3<%4").arg(i).arg(range->min()).arg(offsetInLine).arg(range->max()).arg(range->scope()->name()) );
            if( range->min() <= offsetInLine ) {
                if( offsetInLine < range->max() || (includeEnd && offsetInLine <= range->max()) ) {
//debug.append(" Ok" );
                   result.append(range->scope());
                }
            }
//qlog_info() << debug;
        }
    }
//    qlog_info() << "";
    return result;
}


/// This method returns all scoped ranges at the given offset
/// Hmmm this is almost exactly the same implementation as the scopesAtOffset method !? (perhaps we should refactor this)
///
/// Warning you MUST destroy (qDeleteAll) the list with scoped textranges returned by this list
///
/// @param offset he offset to retrieve the scoped ranges
/// @return the vector with text scopes. These scopes are document wide
QVector<ScopedTextRange*> TextDocumentScopes::createScopedRangesAtOffsetList(int offset)
{
    QVector<ScopedTextRange*> result;

    // retrieve the line
    int line = textDocument()->lineFromOffset(offset);
    int lineOffset = textDocument()->offsetFromLine(line);
    int offsetInLine = offset-lineOffset;

    ScopedTextRangeList* list = scopedRangesAtLine(line);
    if( list ) {
        //scopes.reserve( ranges.size() );
        for( int i=0,cnt=list->size(); i<cnt; ++i ) {
            ScopedTextRange* range = list->at(i);
            if( range->min() <= offsetInLine && offsetInLine < range->max() ) {

                // it's a multi-line scope reference
                MultiLineScopedTextRange* ms = range->multiLineScopedTextRange();
                if( ms ) {
                    result.append( new ScopedTextRange( ms->min(), ms->max(), ms->scope() ) );

                // it's a line scope
                } else {
                    result.append( new ScopedTextRange( lineOffset + range->min(), lineOffset + range->max(), range->scope() ) );
                }
            }
        }
    }
    return result;
}


/// Converts the textdocument scoped to a string
QString TextDocumentScopes::toString()
{
    return scopedRanges_.toString();
}


/// Returns a string-list will all scopes. This list is mainly used for debugging and testing
QStringList TextDocumentScopes::scopesAsStringList()
{
    QStringList result;

    // first add all multi-line scopes
    for( int i=0,cnt=scopedRanges_.rangeCount(); i<cnt; ++i ) {
        MultiLineScopedTextRange& range = scopedRanges_.scopedRange(i);
        result.append( range.toString() );
    }

    result.append("**");

    // next add all line based scoped
    for( int i=0,lineCnt=lineRangeList_.length(); i<lineCnt; ++i ) {
        ScopedTextRangeList* list = lineRangeList_.at(i);
        if( list != 0 ) {
            result.append( list->toString() );
        } else {
            result.append( QStringLiteral(" << null value @ %1>>").arg(i));
        }
    }

    return result;
}


/// add all dumped line scopes
void TextDocumentScopes::dumpScopedLineAddresses(const QString& text)
{
    qlog_info()<< "dumpScopedLineAddresses("<< text << "): " << lineRangeList_.length();
    for( int i=0, cnt=lineRangeList_.length(); i<cnt; ++i ) {
        qlog_info() << "-" << i << ":" << QString::number((quintptr)lineRangeList_.at(i),16);
    }
    qlog_info() << ".";
}


/// returns the current textdocument scope
TextDocument*TextDocumentScopes::textDocument()
{
    return textDocumentRef_;
}


/// the grammar has been changed
void TextDocumentScopes::grammarChanged()
{
    removeScopesAfterOffset(0);
}



} // edbee
