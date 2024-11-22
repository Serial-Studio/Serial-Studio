/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QList>
#include <QMap>
#include <QStack>
#include <QVariant>

class QFile;
class QIODevice;
class QXmlStreamReader;

namespace edbee {

class TextGrammar;
class TextGrammarManager;
class TextGrammarRule;

/// For parsing a Textmate Language
class EDBEE_EXPORT TmLanguageParser
{
public:
    TmLanguageParser();
    TextGrammar* parsePlist(QIODevice* device);
    TextGrammar* parseJson(QIODevice* device);

    TextGrammar* parse(QIODevice* device, bool json=false);
    TextGrammar* parse(QFile& file);
    TextGrammar* parse(const QString& fileName);

    QString lastErrorMessage() const;

protected:
    void setLastErrorMessage( const QString& str );

    void addCapturesToGrammarRule( TextGrammarRule* rule, QHash<QString,QVariant> captures, bool endCapture=false );
    void addPatternsToGrammarRule( TextGrammarRule* rule, QList<QVariant> patterns );

    TextGrammarRule* createGrammarRule(TextGrammar *grammar, const QVariant &data );
    TextGrammar* createLanguage(QVariant& data );

private:
    QString lastErrorMessage_;               ///< The last error message
};

} // edbee
