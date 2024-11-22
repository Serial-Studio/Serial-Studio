/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QObject>
#include <QMap>
#include <QString>


namespace edbee {

class TextEditorCommand;

/// This is a texteditor map. This is used to map Command-Names to commands
/// This class is the owner of the the given commands
class EDBEE_EXPORT TextEditorCommandMap : public QObject
{
    Q_OBJECT
public:
    explicit TextEditorCommandMap(QObject *parent = 0);
    virtual ~TextEditorCommandMap();
    
    void loadFactoryCommandMap();

    void give( const QString& key, TextEditorCommand* command );
    TextEditorCommand* get( const QString& key );

private:
    QMap<QString,TextEditorCommand*> commandMap_;       ///< The command map
};

} // edbee
