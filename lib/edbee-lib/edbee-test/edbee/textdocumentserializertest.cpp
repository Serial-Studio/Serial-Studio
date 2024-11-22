/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textdocumentserializertest.h"

#include <QBuffer>

#include "edbee/models/chardocument/chartextdocument.h"
#include "edbee/models/textdocument.h"
#include "edbee/io/textdocumentserializer.h"

#include "edbee/debug.h"

namespace edbee {

void TextDocumentSerializerTest::testLoad()
{
    CharTextDocument doc;
    TextDocumentSerializer serializer( &doc );
    testEqual( doc.text(), QStringLiteral("") );

    // load the data
    QByteArray data("Test,\r\nWerkt het?\r\nRick!!");
    QBuffer buffer(&data);
    testTrue( serializer.load(&buffer) );
    testEqual( doc.text(), QStringLiteral("Test,\nWerkt het?\nRick!!"));    // windows line endings should be removed


    // clear the buffer
    doc.buffer()->setText("");
    testEqual( doc.text(), QStringLiteral("") );

    data = "Test,\nWerkt het?\nRick!!";
    buffer.setData(data);
    testTrue( serializer.load(&buffer) );
    testEqual( doc.text(), QStringLiteral("Test,\nWerkt het?\nRick!!"));    // windows line endings should be ke



}


} // edbee
