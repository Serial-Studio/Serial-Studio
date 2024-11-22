/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QString>

class QIODevice;

namespace edbee {

class TextDocument;
class TextDocumentSerializer;

class EDBEE_EXPORT TextDocumentSerializerFilter {
public:
    /// A special filter class to filter lines while saving
    /// @param serializer the text serialzer
    /// @param lineIdx the line index that needs to be saved
    /// @param line the line that's save
    /// @return true if the line needs to be selected. return false to skip the line
    virtual bool saveLineSelector( TextDocumentSerializer* serializer, int lineIdx, QString& line ) = 0;
};


/// A class used to load/save a text-file from and to an IODevice
class EDBEE_EXPORT TextDocumentSerializer {
public:
    TextDocumentSerializer( TextDocument* textDocument );

    bool loadWithoutOpening( QIODevice* ioDevice );
    bool load( QIODevice* ioDevice );

    bool saveWithoutOpening( QIODevice* ioDevice );
    bool save( QIODevice* ioDevice );


    QString errorString() { return errorString_; }
    void setFilter( TextDocumentSerializerFilter* filter ) { filterRef_ = filter; }
    TextDocumentSerializerFilter* filter() { return filterRef_; }

private:
    QString appendBufferToDocument(const QString& strIn);

private:
    TextDocument* textDocumentRef_;             ///< The reference to the textdocument
    int blockSize_;                             ///< The block-size to read/write. you must NOT makes this to small.. The first block is used to detected the encoding!!
    QString errorString_;                       ///< The last error (This is reset when calling load/save)
    TextDocumentSerializerFilter* filterRef_;   ///< The line filter
};

} // edbee
