/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QVariant>

class QIODevice;

namespace edbee {

/// A Json Parser that supports strings in json sources!
/// standard json doesn't support comments. This class strips the comments before passing it to qt
/// the comments are replaced by spaces, this way the line-numbers and column numbers will stay the same
class EDBEE_EXPORT JsonParser {
public:
    JsonParser();
    virtual ~JsonParser();

    bool parse( const QString& fileName);
    bool parse( QIODevice* device );
    bool parse( const QByteArray& bytesIn );

    QVariant result();

    void clearErrors();

    QString errorMessage() const;
    int errorLine() const;
    int errorColumn() const;

    QString fullErrorMessage() const;

protected:

    QByteArray stripCommentsFromJson( const QByteArray& bytesIn );

private:

    QString errorMessage_;      ///< The error message
    int errorOffset_;           ///< The character offset of the error
    int errorLine_;             ///< The line number of the error
    int errorColumn_;           ///< The column number of the error

    QVariant result_;           ///< variant is the easiest format to play around with in Qt. The result will be QVariantMap (no hash!) or QVariantArray

};

} //edbee
