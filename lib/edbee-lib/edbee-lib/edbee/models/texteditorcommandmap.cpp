/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "texteditorcommandmap.h"

#include "edbee/data/factorycommandmap.h"
#include "edbee/texteditorcommand.h"

#include "edbee/debug.h"

namespace edbee {

/// Constructs the editor commandmap
/// This constructors also initializes the map with the default editor actions
/// @param parent a reference to the parent object
TextEditorCommandMap::TextEditorCommandMap(QObject* parent)
    : QObject(parent)
{
}


/// Destructs the command map
TextEditorCommandMap::~TextEditorCommandMap()
{
    qDeleteAll( commandMap_ );
    commandMap_.clear();
}


/// This method fills this command map with the factory commands
void TextEditorCommandMap::loadFactoryCommandMap()
{
    FactoryCommandMap().fill( this );
}


/// gives the command to this command map
/// @param key the unique identifier for this command
/// @param command the command to give
void TextEditorCommandMap::give(const QString& key, TextEditorCommand* command)
{
    delete commandMap_.value(key);
    commandMap_.insert(key,command);
}


/// Returns the command identified with the given key
/// @return the command assigned to this identifier or 0 if the command doesn't exeist
TextEditorCommand* TextEditorCommandMap::get(const QString& key)
{
    return commandMap_.value(key);
}


} // edbee
