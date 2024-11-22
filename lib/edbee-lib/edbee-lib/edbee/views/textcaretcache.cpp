/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textcaretcache.h"

#include "edbee/models/textbuffer.h"
#include "edbee/models/textdocument.h"
#include "edbee/views/textrenderer.h"
#include "edbee/models/textrange.h"

#include "edbee/debug.h"

//#define STRICT_CHECK_ON_CACHE

namespace edbee {

/// The text document cache
TextCaretCache::TextCaretCache(TextDocument* doc, TextRenderer* renderer )
    : textDocumentRef_(doc)
    , textRendererRef_(renderer)
{
}


void TextCaretCache::clear()
{
//    qlog_info() << "[CACHE:clear]";
    xPosCache_.clear();
}

void TextCaretCache::fill( TextRangeSet& selection)
{
    clear();
//    qlog_info() << "[CACHE:fill]";
    for( int i=0,cnt=selection.rangeCount(); i<cnt; ++i ) {
        TextRange& range = selection.range(i);
        add( range.caret() );
        if( range.hasSelection() ) {
            add( range.anchor() );
        }
    }
//    dump();
}

/// This method replaces all cached content with the one given
void TextCaretCache::replaceAll(TextCaretCache& cache)
{
//    qlog_info() << "[CACHE:replaceAll]";
    xPosCache_ = cache.xPosCache_;
//    dump();
}


int TextCaretCache::xpos(int offset)
{
//    qlog_info() << "xpos-cache: " << this->xPosCache_.size();
//    qlog_info() << "[CACHE:xpos]" << offset << " >> " << xPosCache_.contains(offset);
#ifdef STRICT_CHECK_ON_CACHE
    Q_ASSERT( xPosCache_.contains(offset) );
#endif

    // when not in debug-mode fallback to calculating the position!
    if( !xPosCache_.contains(offset) ) {
        add(offset);
    }
    return xPosCache_.value(offset);
}

/// Adds an xposition for the given offset
void TextCaretCache::add(int offset, int xpos)
{
//    qlog_info() << "[CACHE:add]" << offset<< " => " << xpos;
    xPosCache_.insert(offset,xpos);
}

/// Adds the given offset by calculating the position
void TextCaretCache::add(int offset)
{
    int line = textDocumentRef_->lineFromOffset(offset);
    int col  = textDocumentRef_->columnFromOffsetAndLine(offset,line);
    int xpos = textRendererRef_->xPosForColumn(line,col);
    add( offset, xpos );
}

/// This method should be called if the caret moves
void TextCaretCache::caretMovedFromOldOffsetToNewOffset(int oldOffset, int newOffset)
{
//    qlog_info() << "[CACHE:move]" << oldOffset<< " => " << newOffset << " << " << xPosCache_.contains(oldOffset);
    Q_ASSERT( xPosCache_.contains(oldOffset) );
    int xpos = xPosCache_.take(oldOffset);
    xPosCache_.insert( newOffset, xpos );

//    qlog_info() << " CACHE: move " << oldOffset << " to " << newOffset << " => " << xpos;
}


/// This method checks if the cache is filled
bool TextCaretCache::isFilled()
{
    return !xPosCache_.isEmpty();
}

void TextCaretCache::dump()
{
    qlog_info() << "DUMP CARET CACHE:" << xPosCache_.size();
    foreach( int key, xPosCache_.keys() ) {
        qlog_info() << " - " << key << ": " << xPosCache_.value(key);
    }
}


} // edbee
