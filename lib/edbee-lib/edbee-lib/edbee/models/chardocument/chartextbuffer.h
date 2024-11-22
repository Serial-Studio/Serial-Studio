/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include "edbee/models/textbuffer.h"
#include "edbee/util/gapvector.h"
#include "edbee/util/lineoffsetvector.h"

namespace edbee {


/// This textbuffer implementation uses QChars for storing the data.
class EDBEE_EXPORT CharTextBuffer : public TextBuffer
{
public:
    CharTextBuffer( QObject* parent=0);

    virtual int length() const;
    virtual QChar charAt(  int offset ) const;
    virtual QString textPart( int offset, int length ) const;

    virtual void replaceText( int offset, int length, const QChar* buffer, int bufferLength );

    virtual int lineCount() { return lineOffsetList_.length(); }

    virtual int lineFromOffset( int offset );
    virtual int offsetFromLine( int line );

    virtual void rawAppendBegin();
    virtual void rawAppend( QChar c );
    virtual void rawAppend( const QChar* data, int dataLength );
    virtual void rawAppendEnd();

    virtual QChar* rawDataPointer();

    /// TODO: Temporary debug method. REMOVE!!
    LineOffsetVector& lineOffsetList() { return lineOffsetList_; }

protected slots:

    void emitTextChanged( edbee::TextBufferChange* change, QString oldText = QString());

private:
    QCharGapVector buf_;                     ///< The textbuffer
    LineOffsetVector lineOffsetList_;        ///< The line offset vector

    int rawAppendStart_;                     ///< The start offset of raw appending. -1 means no appending is happening
    int rawAppendLineStart_;                 ///< The line start

};

} // edbee
