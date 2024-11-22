/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QObject>
#include <QList>

#include "edbee/models/textbuffer.h"

namespace edbee {


class TextGrammar;
class Change;
class ChangeGroup;
class TextAutoCompleteProviderList;
class TextCodec;
class TextDocumentFilter;
class TextDocumentScopes;
class TextEditorConfig;
class TextEditorController;
class LineEnding;
class TextLexer;
class TextLineData;
class TextLineDataManager;
class TextRangeSet;
class TextUndoStack;

/// This is the base and abstract class of a text document
/// A TextDocument is the model part of the editor.
///
/// It's the main owner of the following objects:
/// - A textbuffer, which holds the character data. Currently there's only a CharTextBuffer, a gap-vectored buffer.
/// - An undostack, a stack which holds the undo-operations of the editor
/// - the textdocument scopes, these are the language-dependent scopes found in the current document
/// - A textlexer, which is used for (re-)building the textdocument scopes.
///
class EDBEE_EXPORT TextDocument : public QObject
{

Q_OBJECT

public:

    TextDocument( QObject* parent=0);
    virtual ~TextDocument();

    /// This method should return the active textbuffer
    /// Warning you should NEVER directly modify the textbuffer unless you're absolutely sure what you're doing!
    virtual TextBuffer* buffer() const = 0 ;

    /// This method can be used to change the number of reserved fields by the document
    /// Increasing the amount will result in a realoc
    /// Decreasting the fieldcount reults in the lost of the 'old' fields
    /// At least the 'PredefinedFieldCount' amont of fields are required
    virtual void setLineDataFieldsPerLine( int count );

    /// this method can be used to give a 'custom' line data item to a given line
    virtual TextLineDataManager* lineDataManager() { return textLineDataManager_; }
    virtual void giveLineDataManager(TextLineDataManager* manager);
    virtual void giveLineData( int line, int field, TextLineData* dataItem );
    virtual TextLineData* getLineData( int line, int field );
//    virtual TextLineData* takeLineData( int line, int field ) = 0;

    /// Should return the document-scopes of this document
    virtual TextDocumentScopes* scopes() = 0;

    /// This method should return the current encoding
    virtual TextCodec* encoding() = 0;
    virtual void setEncoding( TextCodec* codec ) = 0;

    /// This method should return the current line ending
    virtual const LineEnding* lineEnding() = 0 ;
    virtual void setLineEnding( const LineEnding* lineENding ) = 0;

    ///  Should return the current document lexer
    virtual TextLexer* textLexer() = 0;

    /// This method should return the current language grammar
    virtual TextGrammar* languageGrammar() = 0;

    /// Changes the language grammar.
    /// This method should emit a grammarChanged signal (if the grammar is changed)
    virtual void setLanguageGrammar( TextGrammar* grammar ) = 0;


    /// This method should return the autcompletion provider list
    virtual TextAutoCompleteProviderList* autoCompleteProviderList() = 0;

    /// this method should return a reference to the undo stack
    virtual TextUndoStack* textUndoStack() = 0;
    virtual void beginUndoGroup(ChangeGroup* group=0);
    virtual void endUndoGroup(int coalesceId, bool flatten=false );
    virtual void endUndoGroupAndDiscard();
    virtual bool isUndoCollectionEnabled();
    virtual void setUndoCollectionEnabled( bool enabled );
    virtual bool isUndoRunning();
    virtual bool isRedoRunning();
    virtual bool isUndoOrRedoRunning();
    virtual bool isPersisted();
    virtual void setPersisted(bool enabled=true);

    /// this method should return the config
    virtual TextEditorConfig* config() const = 0;

    virtual void setDocumentFilter( TextDocumentFilter* filter );
    virtual void giveDocumentFilter( TextDocumentFilter* filter );
    virtual TextDocumentFilter* documentFilter();

    void beginChanges( TextEditorController* controller );
    void replaceRangeSet(TextRangeSet& rangeSet, const QString& text, bool stickySelection = false);
    void replaceRangeSet(TextRangeSet& rangeSet, const QStringList& texts, bool stickySelection = false);
    void giveSelection( TextEditorController* controller,  TextRangeSet* rangeSet);
    void endChanges( int coalesceId );



    Change* executeAndGiveChange(Change* change , int coalesceId );


//    void giveChange( TextChange* change, bool merge  );
    virtual Change* giveChangeWithoutFilter( Change* change, int coalesceId) = 0;
    void append(const QString& text, int coalesceId=0 );
    void replace( int offset, int length, const QString& text, int coalesceId=0);
    void setText( const QString& text );

    // raw access for filling the document
    void rawAppendBegin();
    void rawAppendEnd();
    void rawAppend( QChar c );
    void rawAppend(const QChar *chars, int length );

public:

 // Methods directly forwarded to the  textbuffer

    int length();
    int lineCount();
    QChar charAt(int idx);
    QChar charAtOrNull(int idx);
    int offsetFromLine( int line );
    int lineFromOffset( int offset );
    int columnFromOffsetAndLine( int offset, int line=-1 );
    int offsetFromLineAndColumn( int line, int column );
    int lineLength( int line );
    int lineLengthWithoutNewline( int line );
    QString text();
    QString textPart( int offset, int length );
    QString lineWithoutNewline( int line );
    QString line( int line );

signals:

    void textAboutToBeChanged( edbee::TextBufferChange change );
    void textChanged( edbee::TextBufferChange change, QString oldText = QString() );

    /// This signal is emitted if the persisted state is changed
    void persistedChanged(bool persisted);

    /// This signal is emitted if the grammar has been changed
    void languageGrammarChanged();

    /// this signal is emitted if the scoped range has been changed
    void lastScopedOffsetChanged( int previousOffset, int lastScopedOffset );


private:
    TextDocumentFilter* documentFilter_;             ///< The document filter if the filter is owned
    TextDocumentFilter* documentFilterRef_;          ///< The reference to the document filter.

    TextLineDataManager* textLineDataManager_;               ///< A class for managing text line data items

};

} // edbee
