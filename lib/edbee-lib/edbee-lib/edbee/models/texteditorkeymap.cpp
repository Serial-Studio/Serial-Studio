/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "texteditorkeymap.h"

#include <QDir>
#include <QKeySequence>

#include "edbee/data/factorykeymap.h"
#include "edbee/io/keymapparser.h"
#include "edbee/models/change.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/models/textdocument.h"


#include "edbee/debug.h"

namespace edbee {

/// Constructs the texteditor key sequence
TextEditorKey::TextEditorKey(const QKeySequence& seq)
    : sequence_(seq)
{
}


/// Clones the text editor key
TextEditorKey* TextEditorKey::clone() const
{
    return new TextEditorKey( sequence_ );
}


/// returns the keysequence
const QKeySequence& TextEditorKey::sequence()
{
    return sequence_;
}



//----------------------------------------------------


TextEditorKeyMap::TextEditorKeyMap(TextEditorKeyMap *parentKeyMap)
    : parentRef_(parentKeyMap)
{

}

/// the copy constructor
//TextEditorKeyMap::TextEditorKeyMap(const TextEditorKeyMap& keyMap)
//{
//    QMap<QKeySequence,QString>::const_iterator i = keyMap.actionMap_.constBegin();
//    while (i != keyMap.actionMap_.constEnd()) {
//        set( i.key(), i.value() );
//    }
//    maxSequenceLength_ = keyMap.maxSequenceLength_;
//}


/// empty the keymap
TextEditorKeyMap::~TextEditorKeyMap()
{
    qDeleteAll( keyMap_ );
}


/// copies the given keys to the other keymap
/// @param keyMap the keymap to copy the keys from
void TextEditorKeyMap::copyKeysTo(TextEditorKeyMap* keyMap)
{
    for( QMultiHash<QString,TextEditorKey*>::const_iterator itr = keyMap_.constBegin(); itr != keyMap_.constEnd(); ++itr ) {
        keyMap->keyMap_.insert( itr.key(), itr.value()->clone() );
    }
}


/// Returns the last inserted keysequence for the given command
/// It also tries to search the parent keymap if it's available
/// @param name the name of the given command
/// @return the keysequence or 0 if not found
TextEditorKey* TextEditorKeyMap::get(const QString& name) const
{
    QMultiHash<QString,TextEditorKey*>::const_iterator itr = keyMap_.find(name);
    if( itr != keyMap_.end() ) { return itr.value(); }
    if( parentRef_ ) { return parentRef_->get(name); }
    return nullptr;
}


/// Returns the key sequence for the given name
/// @param name the name of the key sequence
/// @return a QKeySequence
QKeySequence TextEditorKeyMap::getSequence(const QString& name) const
{
    TextEditorKey* key = get(name);
    if( !key ){ return QKeySequence(); }
    return key->sequence();
}


/// Returns a list of all keysequences for the given command
/// This method will also search the parents for the given keysequences
QList<TextEditorKey*> TextEditorKeyMap::getAll(const QString& name) const
{
    QList<TextEditorKey*> result = keyMap_.values(name);
    // also add the references of the parent
    if( parentRef_ ) {
        QList<TextEditorKey*> parentResult = parentRef_->getAll(name);
        foreach( TextEditorKey* seq, parentResult ) {
            if( !result.contains(seq) ) {
                result.append(seq);
            }
        }
    }
    return result;
}


/// Tests if the keymap has the given action
/// It will also search the parent key map
bool TextEditorKeyMap::has(const QString& name) const
{
    bool result = !(keyMap_.find(name) == keyMap_.end());
    if( !result && parentRef_ ) {
        result = parentRef_->has(name);
    }
    return result;
}


/// Adds the given key sequence to the keymap
/// @param command the name of the editor command
/// @param sequence the keysequence
void TextEditorKeyMap::add(const QString& command, TextEditorKey* sequence)
{
    keyMap_.insert(command,sequence);
}


/// Adds the given key sequence to the keymap
/// @param name the name of the editor command
/// @param sequence the keysequence
void TextEditorKeyMap::add(const QString& command, const QKeySequence& seq)
{
    keyMap_.insert(command, new TextEditorKey(seq) );
}


/// Adds a command - key sequence with a string
/// If first tries to match the string with the standardkey-texts
/// If this doesn't result in a sequence it assumes it's a keydefinition used by QKeySequence
/// @param command the command name
/// @param seq the key sequence definition
bool TextEditorKeyMap::add(const QString& command, const QString& seq)
{
    // when the given keys-string is a 'standard-key' name we use the standard key
    QKeySequence::StandardKey standardKey = TextEditorKeyMap::standardKeyFromString(seq);
    if( standardKey != QKeySequence::UnknownKey) {
        foreach(QKeySequence sequence,QKeySequence::keyBindings(standardKey)) {
            add( command, new TextEditorKey( QKeySequence(sequence) ) );
        }
        return true;
    }

    // When it's not a standard key we try a textual key sequence
    QKeySequence normalKey = QKeySequence(seq);
    if( normalKey != QKeySequence::UnknownKey ) {
        add( command, new TextEditorKey( normalKey ) );
        return true;
    }

    // well this didn't go well
    return false;

}


//void TextEditorKeyMap::set(const QString& name, const QKeySequence::StandardKey key)
//{
//    keyMap_.insertMulti(name, QKeySequence(key));
//}

/// replace the given command with the given key sequence
/// all other keysequences (on this level) are ignored
void TextEditorKeyMap::replace(const QString& name, TextEditorKey* sequence)
{
    qDeleteAll( keyMap_.values(name) );
    keyMap_.insert(name,sequence);
}


/// Searches for the given command with a keysequence
/// Warning when one keysequence is the prefix of the other keysequence it is possible
/// only the partial or full match is return (depending on the order)
///
/// @param sequence the pressed key-sequence
/// @param match the sequence match:   (NoMatch, PartialMatch, ExactMatch)
/// @return the (partial) matched command or a empty string if not found
QString TextEditorKeyMap::findBySequence(QKeySequence sequence, QKeySequence::SequenceMatch& match)
{
    // find the current map
    QMultiHash<QString,TextEditorKey*>::iterator itr = keyMap_.begin();
    while (itr != keyMap_.end()) {
        match =  sequence.matches( itr.value()->sequence() );
        if( match != QKeySequence::NoMatch ) {
            return itr.key();
        }
        ++itr;
    }
    if( parentRef_ ) return parentRef_->findBySequence(sequence,match);
    match = QKeySequence::NoMatch;
    return "";
}



/// This method joins 2 key-sequences to 1 sequence (if possible)
/// if seq1 + se2 length > 4 seq2 is returned
QKeySequence TextEditorKeyMap::joinKeySequences(const QKeySequence seq1, const QKeySequence seq2)
{
    if( seq1.count() + seq2.count() > 4 ) { return seq2; }
    int idx=0;
    int keys[] = {0,0,0,0};
    for( int i=0; i < seq1.count(); ++i ) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        keys[idx++] = seq1[i];
#else
        keys[idx++] = seq1[i].toCombined();
#endif
    }
    for( int i=0; i < seq2.count(); ++i ) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        keys[idx++] = seq2[i];
#else
        keys[idx++] = seq2[i].toCombined();
#endif
    }
    return QKeySequence(keys[0],keys[1],keys[2],keys[3]);
}


/// a map of all standard keys
static QHash<QString,QKeySequence::StandardKey> stringToStandardKey;

/// converts a string to a standard key
/// if the standard key isn't found it returns 'QKeySequence::UnknownKey '
QKeySequence::StandardKey TextEditorKeyMap::standardKeyFromString( const QString& str )
{
    QHash<QString,QKeySequence::StandardKey>& h = stringToStandardKey;
    if( h.isEmpty() ) {
        h.insert( "add_tab", QKeySequence::AddTab );
        h.insert( "back", QKeySequence::Back );
        h.insert( "bold", QKeySequence::Bold );
        h.insert( "close", QKeySequence::Close );
        h.insert( "copy", QKeySequence::Copy );
        h.insert( "cut", QKeySequence::Cut );
        h.insert( "delete", QKeySequence::Delete );
        h.insert( "delete_end_of_line", QKeySequence::DeleteEndOfLine );
        h.insert( "delete_end_of_word", QKeySequence::DeleteEndOfWord );
        h.insert( "delete_start_of_word", QKeySequence::DeleteStartOfWord );
        h.insert( "find", QKeySequence::Find );
        h.insert( "find_next", QKeySequence::FindNext );
        h.insert( "find_previous", QKeySequence::FindPrevious );
        h.insert( "forward", QKeySequence::Forward );
        h.insert( "help_contents", QKeySequence::HelpContents );
        h.insert( "insert_line_separator", QKeySequence::InsertLineSeparator );
        h.insert( "insert_paragraph_separator", QKeySequence::InsertParagraphSeparator );
        h.insert( "italic", QKeySequence::Italic );
        h.insert( "move_to_end_of_block", QKeySequence::MoveToEndOfBlock );
        h.insert( "move_to_end_of_document", QKeySequence::MoveToEndOfDocument );
        h.insert( "move_to_end_of_line", QKeySequence::MoveToEndOfLine );
        h.insert( "move_to_next_char", QKeySequence::MoveToNextChar );
        h.insert( "move_to_next_line", QKeySequence::MoveToNextLine );
        h.insert( "move_to_next_page", QKeySequence::MoveToNextPage );
        h.insert( "move_to_next_word", QKeySequence::MoveToNextWord );
        h.insert( "move_to_previous_char", QKeySequence::MoveToPreviousChar );
        h.insert( "move_to_previous_line", QKeySequence::MoveToPreviousLine );
        h.insert( "move_to_previous_page", QKeySequence::MoveToPreviousPage );
        h.insert( "move_to_previous_word", QKeySequence::MoveToPreviousWord );
        h.insert( "move_to_start_of_block", QKeySequence::MoveToStartOfBlock );
        h.insert( "move_to_start_of_document", QKeySequence::MoveToStartOfDocument );
        h.insert( "move_to_start_of_line", QKeySequence::MoveToStartOfLine );
        h.insert( "new", QKeySequence::New );
        h.insert( "next_child", QKeySequence::NextChild );
        h.insert( "open", QKeySequence::Open );
        h.insert( "paste", QKeySequence::Paste );
        h.insert( "preferences", QKeySequence::Preferences );
        h.insert( "previous_child", QKeySequence::PreviousChild );
        h.insert( "print", QKeySequence::Print );
        h.insert( "quit", QKeySequence::Quit );
        h.insert( "redo", QKeySequence::Redo );
        h.insert( "refresh", QKeySequence::Refresh );
        h.insert( "replace", QKeySequence::Replace );
        h.insert( "save_as", QKeySequence::SaveAs );
        h.insert( "save", QKeySequence::Save );
        h.insert( "select_all", QKeySequence::SelectAll );
        h.insert( "select_end_of_block", QKeySequence::SelectEndOfBlock );
        h.insert( "select_end_of_document", QKeySequence::SelectEndOfDocument );
        h.insert( "select_end_of_line", QKeySequence::SelectEndOfLine );
        h.insert( "select_next_char", QKeySequence::SelectNextChar );
        h.insert( "select_next_line", QKeySequence::SelectNextLine );
        h.insert( "select_next_page", QKeySequence::SelectNextPage );
        h.insert( "select_next_word", QKeySequence::SelectNextWord );
        h.insert( "select_previous_char", QKeySequence::SelectPreviousChar );
        h.insert( "select_previous_line", QKeySequence::SelectPreviousLine );
        h.insert( "select_previous_page", QKeySequence::SelectPreviousPage );
        h.insert( "select_previous_word", QKeySequence::SelectPreviousWord );
        h.insert( "select_start_of_block", QKeySequence::SelectStartOfBlock );
        h.insert( "select_start_of_document", QKeySequence::SelectStartOfDocument );
        h.insert( "select_start_of_line", QKeySequence::SelectStartOfLine );
        h.insert( "underline", QKeySequence::Underline );
        h.insert( "undo", QKeySequence::Undo );
        h.insert( "unknown_key", QKeySequence::UnknownKey );
        h.insert( "whats_this", QKeySequence::WhatsThis );
        h.insert( "zoom_in", QKeySequence::ZoomIn );
        h.insert( "zoom_out", QKeySequence::ZoomOut );
        h.insert( "full_screen", QKeySequence::FullScreen );
    }
    return h.value(str, QKeySequence::UnknownKey );
}


/// returns all added actions
QString TextEditorKeyMap::toString() const
{
    QString str;
    for( QMultiHash<QString,TextEditorKey*>::const_iterator itr = keyMap_.constBegin(); itr != keyMap_.constEnd(); ++itr ) {
        if( !str.isEmpty()) str.append(",");
        str.append( QStringLiteral("%1:%2").arg(itr.key()).arg(itr.value()->sequence().toString()) );
    }
    return str;
}


//----------------------------------------------------


/// destructs all keymaps
TextKeyMapManager::TextKeyMapManager()
{
    keyMapHash_.insert("", new TextEditorKeyMap());
}


/// The textkeymap manager destructor
TextKeyMapManager::~TextKeyMapManager()
{
    qDeleteAll( keyMapHash_);
}


/// This method loads all keymaps.
///
void TextKeyMapManager::loadAllKeyMaps(const QString& path)
{    
    QDir dir(path);
    QStringList filters("*.json");
    foreach( QFileInfo fileInfo, dir.entryInfoList( filters, QDir::Files, QDir::Name ) ) {
        loadKeyMap( fileInfo.absoluteFilePath() );
    }
}


/// This method loads a single keymap
void TextKeyMapManager::loadKeyMap(const QString& file)
{
    // parse the given file
    QFileInfo fileInfo( file );
    QString name = fileInfo.baseName();
    if( name == "default" ) { name = ""; }

    // parse the given keymap file
    KeyMapParser parser;
    if( !parser.parse( file, findOrCreate(name) ) ) {
        qlog_warn() << QObject::tr("Error parsing %1: %2 ").arg(file).arg(parser.errorMessage());

        // when there's an error parsing the defaultkeymap, fallback to the factory
        if( name == "" ) {
            qlog_warn() << "Error loading default keymap: fallback to the factory keymap!";
            loadFactoryKeyMap();
        }

    }
}


/// This method loads the internal factory keymap
void TextKeyMapManager::loadFactoryKeyMap()
{
    FactoryKeyMap().fill( findOrCreate("") );
}


/// Returns the given text-editor keymap
TextEditorKeyMap* TextKeyMapManager::get(const QString& name)
{
    return keyMapHash_.value(name);
}


/// finds or creates the given keymap
TextEditorKeyMap* TextKeyMapManager::findOrCreate(const QString& name)
{
    TextEditorKeyMap* keyMap = keyMapHash_.value(name);
    if( !keyMap ) {
        keyMap = new TextEditorKeyMap();
        keyMapHash_.insert( name, keyMap );
    }
    return keyMap;
}



} // edbee
