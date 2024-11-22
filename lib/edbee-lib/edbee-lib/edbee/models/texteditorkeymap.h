/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QKeySequence>
#include <QHash>
#include <QMultiHash>
#include <QStringList>

namespace edbee {

class TextEditorCommand;
class TextEditorController;


/// A contexted keysequence
/// This contains a keysequence and optionally a given context
class EDBEE_EXPORT TextEditorKey {
public:
    TextEditorKey( const QKeySequence& seq );
    TextEditorKey* clone() const;

    const QKeySequence& sequence();

private:
    QKeySequence sequence_;             ///< The key sequence

};


//------------------------------------------------------------


/// A text editor key map
/// This key map, maps key-sequences to action-names.
class EDBEE_EXPORT TextEditorKeyMap {
public:

    TextEditorKeyMap( TextEditorKeyMap* parentKeyMap=0 );
    virtual ~TextEditorKeyMap();

    void copyKeysTo( TextEditorKeyMap* keyMap );

    TextEditorKey* get( const QString& name ) const;
    QKeySequence getSequence( const QString& name ) const;
    QList<TextEditorKey*> getAll( const QString& name  ) const;
    bool has( const QString& name ) const;

    void add( const QString& command, TextEditorKey *sequence );
    void add( const QString& command, const QKeySequence& seq );
    bool add( const QString& command, const QString& seq );
//    void set( const QString& name, const QKeySequence::StandardKey key );
    void replace(const QString& name, TextEditorKey *sequence );

    QString findBySequence( QKeySequence sequence, QKeySequence::SequenceMatch& match );

    static QKeySequence joinKeySequences( const QKeySequence seq1, const QKeySequence seq2 );
    static QKeySequence::StandardKey standardKeyFromString( const QString& str );

    QString toString() const;

    TextEditorKeyMap* parentMap() { return parentRef_; }

private:

    TextEditorKeyMap* parentRef_;                       ///< a reference to a parent keymap
    QMultiHash<QString,TextEditorKey*> keyMap_;      ///< a map to convert a name to a
};


//-------------------------------------------------------------------


/// There can be different keymaps for different file-types/
/// The keymap manager manages all the available keymaps
/// There's ALWAYS a keymap with the name ""
class EDBEE_EXPORT TextKeyMapManager {
public:
    TextKeyMapManager();
    virtual ~TextKeyMapManager();

    void loadAllKeyMaps( const QString& path );
    void loadKeyMap( const QString& file );
    void loadFactoryKeyMap();

    TextEditorKeyMap* get( const QString& name=QString() );
    TextEditorKeyMap* findOrCreate( const QString& name );

private:

    QHash<QString,TextEditorKeyMap*> keyMapHash_;   ///< All loaded keymaps identified by name

};



} // edbee
