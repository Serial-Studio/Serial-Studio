/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QObject>
#include <QIcon>

#include "edbee/models/textbuffer.h"

class QAction;

namespace edbee {

class ChangeGroup;
class DynamicVariables;
class TextBufferChange;
class TextCaretCache;
class TextCommand;
class TextDocument;
class TextEditorCommand;
class TextEditorKeyMap;
class TextEditorCommandMap;
class TextEditorComponent;
class TextEditorWidget;
class TextRenderer;
class TextRangeSet;
class TextSearcher;
class TextSelection;
class UndoableTextCommand;


/// The texteditor works via the controller. The controller is the central point/mediator
/// which maps/controls all messages between the different editor components
class EDBEE_EXPORT TextEditorController : public QObject
{
    Q_OBJECT
public:

    enum AutoScrollToCaret {
        AutoScrollAlways,
        AutoScrollWhenFocus,
        AutoScrollNever
    };


    explicit TextEditorController( TextEditorWidget* widget=0, QObject *parent = 0);
    virtual ~TextEditorController();

// public method
    void notifyStateChange();

    void giveTextDocument( TextDocument* doc );
    void setTextDocument( TextDocument* doc );

    void setAutoScrollToCaret( AutoScrollToCaret autoScroll );
    virtual AutoScrollToCaret autoScrollToCaret() const;

    bool hasFocus();
    QAction* createUnconnectedAction(const QString& command, const QString& text, const QIcon& icon=QIcon(), QObject* owner=0 );
    QAction* createAction(const QString& command, const QString& text , const QIcon& icon=QIcon(), QObject* owner=0 );


// getters
//    TextBuffer* textBuffer() const;
    TextDocument* textDocument() const;
    TextSelection* textSelection() const;
    TextRenderer* textRenderer() const;
    TextRangeSet* borderedTextRanges() const;

    void setKeyMap( TextEditorKeyMap* keyMap );
    void giveKeyMap( TextEditorKeyMap* keyMap );
    TextEditorKeyMap* keyMap() const;
    void setCommandMap( TextEditorCommandMap* commandMap );
    void giveCommandMap( TextEditorCommandMap* commandMap );
    TextEditorCommandMap* commandMap() const;
    TextEditorWidget* widget() const;
    TextCaretCache* textCaretCache() const;
    void giveTextSearcher( TextSearcher* searcher );
    TextSearcher* textSearcher();
    DynamicVariables* dynamicVariables() const;

signals:

    /// This signal is fired if the statusbar needs updating
    void updateStatusTextSignal( const QString& text );

    /// This signal is fired if the textdocument changes.
    void textDocumentChanged( edbee::TextDocument* oldDocument, edbee::TextDocument* newDocument );

    /// this method is executed when a command is going to be executed
    void commandToBeExecuted( edbee::TextEditorCommand* command );
    void commandExecuted( edbee::TextEditorCommand* command );

    void backspacePressed();

public slots:

    void onTextChanged( edbee::TextBufferChange change, QString oldText = QString() );
    void onSelectionChanged( edbee::TextRangeSet *oldRangeSet );
    void onLineDataChanged( int line, int length, int newLength );

    void updateAfterConfigChange();
    
public slots:

    // updates the status text
    virtual void updateStatusText( const QString& extraText=QString() );

    virtual void update();

    // scrolling
    virtual void scrollPositionVisible( int xPos, int yPos );
    virtual void scrollOffsetVisible( int offset );
    virtual void scrollCaretVisible();

    virtual void storeSelection( int coalesceId=0 );

    // replace the given selection
    virtual void replace( int offset, int length, const QString& text, int coalesceId, bool stickySelection=false);
    virtual void replaceSelection(const QString& text, int coalesceId=0, bool stickySelection=false);
    virtual void replaceSelection(const QStringList& texts, int coalesceId=0, bool stickySelection=false);
    virtual void replaceRangeSet(edbee::TextRangeSet& rangeSet, const QString& text, int coalesceId=0, bool stickySelection=false);
    virtual void replaceRangeSet(edbee::TextRangeSet& rangeSet, const QStringList& texts, int coalesceId=0, bool stickySelection=false);

    // caret movements
    virtual void moveCaretTo( int line, int col, bool keepAnchors, int rangeIndex=-1 );
    virtual void moveCaretToOffset( int offset, bool keepAnchors, int rangeIndex=-1 );
    virtual void moveCaretAndAnchorToOffset(int caret, int anchor, int rangeIndex= -1 );
    virtual void addCaretAt( int line, int col);
    virtual void addCaretAtOffset( int offset );
    virtual void changeAndGiveTextSelection(edbee::TextRangeSet* rangeSet , int coalesceId = 0);

    // perform an undo
    virtual void undo(bool soft=false);
    virtual void redo(bool soft=false);

    // command execution
    virtual void beginUndoGroup( edbee::ChangeGroup* group=0 );
    virtual void endUndoGroup(int coalesceId=0, bool flatten=false);

    // low level command execution
    virtual void executeCommand( edbee::TextEditorCommand* textCommand );
    virtual void executeCommand( const QString& name=QString() );

 public:

    // returns the readonly state
    virtual bool readonly() const;
    virtual void setReadonly(bool value);


private:

    TextEditorWidget* widgetRef_;             ///< A reference to the text editor widget
    TextDocument* textDocument_;              ///< The text document (only filled when owned)
    TextDocument* textDocumentRef_;           ///< The reference to the text-document

    TextSelection* textSelection_;            ///< The text selection

    TextEditorKeyMap* keyMap_;                ///< The ownership of the keymap
    TextEditorKeyMap* keyMapRef_;             ///< A reference to the keymap
    TextEditorCommandMap* commandMap_;        ///< the ownership
    TextEditorCommandMap* commandMapRef_;     ///< A reference to the command
    TextRenderer* textRenderer_;              ///< The text renderer
    TextCaretCache* textCaretCache_;          ///< The text-caret cache. (For remembering the x-position of the current carets)

    TextSearcher* textSearcher_;              ///< The text-searcher
    
    AutoScrollToCaret autoScrollToCaret_;     ///< This flags tells the editor to automatically scroll to the caret


    // extra highlight text
    TextRangeSet* borderedTextRanges_;       ///< Extra marked text ranges
};

} // edbee
