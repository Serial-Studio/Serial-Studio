/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QHash>
#include <QStack>
#include <QString>
#include <QVariant>

class QIODevice;
class QXmlStreamReader;

namespace edbee {

/// A general plist xml-file parser
class EDBEE_EXPORT BasePListParser {
public:
    BasePListParser();
    virtual ~BasePListParser();

    QString lastErrorMessage() const;

    void setLastErrorMessage( const QString& str );

    bool beginParsing( QIODevice* device );
    bool endParsing();

    void raiseError( const QString& str );

  // general plist parsing
    QList<QVariant> readList();
    QHash<QString, QVariant> readDict();
    QVariant readNextPlistType(int level=-1);

    bool readNextElement( const QString& name, int level=-1 );
    QString readElementText();

    int currentStackLevel();

private:

    QString lastErrorMessage_;               ///< The last error message
    QStack<QString> elementStack_;           ///< The elements currently on the stack
    QXmlStreamReader* xml_;                  ///< The reference of the xml reader

};

} // edbee
