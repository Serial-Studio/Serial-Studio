/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "texteditorcontroller.h"

#include <QApplication>
#include <QAction>
#include <QAccessibleTextUpdateEvent>
#include <QThread>

#include "edbee/commands/selectioncommand.h"
#include "edbee/models/changes/selectionchange.h"
#include "edbee/models/chardocument/chartextdocument.h"
#include "edbee/models/textbuffer.h"
#include "edbee/models/change.h"
#include "edbee/models/textdocument.h"
#include "edbee/models/textdocumentscopes.h"
#include "edbee/models/texteditorcommandmap.h"
#include "edbee/models/texteditorconfig.h"
#include "edbee/models/texteditorkeymap.h"
#include "edbee/models/textrange.h"
#include "edbee/models/textsearcher.h"
#include "edbee/models/textundostack.h"
#include "edbee/texteditorcommand.h"
#include "edbee/edbee.h"
#include "edbee/texteditorwidget.h"
#include "edbee/views/accessibletexteditorwidget.h"
#include "edbee/views/components/texteditorcomponent.h"
#include "edbee/views/components/textmargincomponent.h"
#include "edbee/views/textrenderer.h"
#include "edbee/views/textcaretcache.h"
#include "edbee/views/textselection.h"

#include "edbee/debug.h"



namespace edbee {


/// The constructor
/// @param widget the widget this controller is associated with
/// @paarm parent the QObject parent of the controller
TextEditorController::TextEditorController( TextEditorWidget* widget, QObject *parent)
    : QObject(parent)
    , widgetRef_(widget)
    , textDocument_(nullptr)
    , textDocumentRef_(nullptr)
    , textSelection_(nullptr)
    , keyMap_(nullptr)
    , keyMapRef_(nullptr)
    , commandMap_(nullptr)
    , commandMapRef_(nullptr)
    , textRenderer_(nullptr)
    , textCaretCache_(nullptr)
    , textSearcher_(nullptr)
    , autoScrollToCaret_(AutoScrollAlways)
    , borderedTextRanges_(nullptr)
{

    // create the keymap
    keyMapRef_ = Edbee::instance()->defaultKeyMap();
    commandMapRef_ = Edbee::instance()->defaultCommandMap();

    // create the text renderer
    textRenderer_ = new TextRenderer( this );

    // create a text document (this should happen AFTER the creation of the renderer)
    giveTextDocument( new CharTextDocument() );

    // Now all objects have been created we can init them
    textRenderer_->init();
    textRenderer_->setThemeByName( textDocument()->config()->themeName() );
}


/// Destroys the controller and associated objects
TextEditorController::~TextEditorController()
{
    delete borderedTextRanges_;
    delete textSearcher_;
    delete textRenderer_;
    delete textCaretCache_;
    delete textSelection_;
    delete textDocument_;
    delete commandMap_;
    delete keyMap_;
}


/// This method is called to reset the caret timer and update the ui
void TextEditorController::notifyStateChange()
{
    if( widgetRef_ ) {
        widgetRef_->resetCaretTime();

        // scrolling is only required when focused (When scrolling without focus the sync-editor goes wacko :P )
        if( autoScrollToCaret_ == AutoScrollAlways || (autoScrollToCaret_ == AutoScrollWhenFocus && hasFocus())   ) {
            scrollOffsetVisible( textSelection()->range(0).caret() );
        }

        widgetRef_->updateComponents();
    }
}


/// sets the document and transfers the ownership of the textdocument to this class
/// @param doc the new document for this controller
void TextEditorController::giveTextDocument(TextDocument* doc)
{
    if( doc != textDocument_ ) {
        setTextDocument( doc );
        textDocument_ = doc;        // this is all that's required for getting the ownership
    }
}


/// Set the text document
/// @param doc the document for this controller
void TextEditorController::setTextDocument(TextDocument* doc)
{
    Q_ASSERT_GUI_THREAD;

    if( doc != textDocumentRef_ ) {
        // disconnect the old document
        TextDocument* oldDocumentRef = textDocument();
        if( oldDocumentRef ) {
            oldDocumentRef->textUndoStack()->unregisterController(this);
            disconnect( oldDocumentRef, SIGNAL(textChanged(edbee::TextBufferChange, QString)), this, SLOT(onTextChanged(edbee::TextBufferChange, QString)) );
            disconnect( textDocumentRef_->lineDataManager(), SIGNAL(lineDataChanged(int,int,int)), this, SLOT(onLineDataChanged(int,int,int)));
        }

        // delete some old and dependent objects
        delete textCaretCache_;
        delete textSelection_;

        // create the document
        textDocumentRef_ = doc;
        textCaretCache_ = new TextCaretCache( textDocumentRef_, textRenderer_ ); // warning the cache needs to be created BEFORE the selection!!!
        textSelection_ = new TextSelection( this );
        textSelection_->addRange(0,0);  // add at least one cursor :-)

        delete borderedTextRanges_;
        borderedTextRanges_ = new DynamicTextRangeSet(textDocument());


        textDocumentRef_->textUndoStack()->registerContoller(this);

        connect( textDocumentRef_, SIGNAL(textChanged(edbee::TextBufferChange, QString)), this, SLOT(onTextChanged(edbee::TextBufferChange, QString)));
        connect( textDocumentRef_->lineDataManager(), SIGNAL(lineDataChanged(int,int,int)), this, SLOT(onLineDataChanged(int,int,int)) );

        // force an repaint when the grammar is changed
        connect( textDocumentRef_, &TextDocument::languageGrammarChanged, this, &TextEditorController::update );

        // connect the configuration changes to the controller
        connect( textDocumentRef_->config(), &TextEditorConfig::configChanged, this, &TextEditorController::updateAfterConfigChange );

        // create the new document
        emit textDocumentChanged( oldDocumentRef, textDocumentRef_ );

        // delete the old stuff
        delete textDocument_;
        textDocument_ = nullptr;
    }
}


/// Changes the autoScrollToCaret setting
/// @param autoScroll the new autoscroll to caret setting. This can be one of the following values:
///  - AutoScrollAlways => Always scroll the view so the caret is visible
//   - AutoScrollWhenFocus => Only scroll the view when the editor has got the focus
//   - AutoScrollNever => Never perform automatic scrolling
void TextEditorController::setAutoScrollToCaret(TextEditorController::AutoScrollToCaret autoScroll)
{
     autoScrollToCaret_ = autoScroll;
}


/// Returns the autoScrollToCaret setting
TextEditorController::AutoScrollToCaret TextEditorController::autoScrollToCaret() const
{
     return autoScrollToCaret_;
}


/// This method return true if the text-editor has focus
bool TextEditorController::hasFocus()
{
    return widget()->hasFocus();
}


/// This method creates an editor action that is
/// @param command the command that needs to be executed.
/// @param text description of the command
/// @param icon the optional icon of the command
/// @param owner the QObject owner of this action
/// @return the newly created QAction
QAction* TextEditorController::createUnconnectedAction(const QString& command, const QString& text, const QIcon& icon, QObject* owner )
{
    // create the action
    QAction* action = new QAction( icon, text, owner );

    // set the key if there's an assigned keyboard command
    TextEditorKey* key = keyMap()->get(command);
    if( keyMap()->get( command ) ) {
        action->setShortcut( key->sequence() );
    }

    // set the data
    action->setData( command );
    return action;
}


/// Creates a QAction object that performs the given editor action
/// The shortcut of the given editor-command is retrieved from the keymap
/// @param command the command that needs to be executed.
/// @param text description of the command
/// @param icon the optional icon of the command
/// @param owner the QObject owner of this action
/// @return the newly created QAction
QAction* TextEditorController::createAction(const QString& command, const QString& text, const QIcon& icon , QObject* owner)
{
    // create the action
    QAction* action = createUnconnectedAction( command, text, icon, owner );
    /// connect the signal to executeCommand
    connect( action, SIGNAL(triggered()), SLOT(executeCommand()) );
    return action;
}


/// Returns a reference to the textdocument
TextDocument*TextEditorController::textDocument() const
{
    return textDocumentRef_;
}


/// Returns the textselection
TextSelection* TextEditorController::textSelection() const
{
    return textSelection_;
}


/// Returns the current text renderer
TextRenderer*TextEditorController::textRenderer() const
{
    return textRenderer_;
}


/// returns the bordered textranges
/// These are textranges that are rendered with a border, but aren't truly selected
TextRangeSet *TextEditorController::borderedTextRanges() const
{
    return borderedTextRanges_;
}


/// sets the keymap. It will replace (and if owned, delete) the previous keymap
/// @param keyMap the new keyMap to use
void TextEditorController::setKeyMap(TextEditorKeyMap* keyMap)
{
    delete keyMap_;
    keyMap_ = nullptr;
    keyMapRef_= keyMap;
}


/// gives a keymap to the editor. The ownership is transferred to this controller
/// @param keyMap the new keymap to give to the controller
void TextEditorController::giveKeyMap(TextEditorKeyMap* keyMap)
{
    delete keyMap_;
    keyMap_ = keyMap;
    keyMapRef_= keyMap;
}


/// Returns the current keymap
TextEditorKeyMap*TextEditorController::keyMap() const
{
    return keyMapRef_;
}


/// set a commandmap
/// the ownership is NOT transferred to this object. The old owned command-map is deleted
/// @parm commandMap the new commandMap of this object
void TextEditorController::setCommandMap(TextEditorCommandMap* commandMap)
{
    delete commandMap_;
    commandMap_ = nullptr;
    commandMapRef_ = commandMap;
}


/// gives a commandmap to the editor
/// The old associated command map is deleted
/// @param commandMap the new commandMap
void TextEditorController::giveCommandMap(TextEditorCommandMap* commandMap)
{
    delete commandMap_;
    commandMap_ = commandMap;
    commandMapRef_ = commandMap;
}


/// Returns the current commandmap
TextEditorCommandMap*TextEditorController::commandMap() const
{
    return commandMapRef_;
}


/// Returns the active widget
TextEditorWidget*TextEditorController::widget() const
{
    return widgetRef_;
}


/// Returns the textCaretCache
TextCaretCache*TextEditorController::textCaretCache() const
{
     return textCaretCache_;
}


/// Gives the text-searcher to this document
/// @param searcher the new textsearcher for this document (The old one is deleted)
void TextEditorController::giveTextSearcher(TextSearcher* searcher)
{
    delete textSearcher_;
    textSearcher_ = searcher;
}


/// Returns the associated text searcher object
/// @return the textsearcher object
TextSearcher *TextEditorController::textSearcher()
{
    if( !textSearcher_ ) { textSearcher_ = new TextSearcher(); }
    return textSearcher_;
}


/// this method returns the dynamic variables object for this controller
/// The current implementation simply returns the application-wide environment variables
DynamicVariables* TextEditorController::dynamicVariables() const
{
    return Edbee::instance()->environmentVariables();
}


//==========================================================================================
// Slots
//==========================================================================================


/// This slot is placed if a piece of text is replaced
void TextEditorController::onTextChanged( edbee::TextBufferChange change, QString oldText )
{
    /// update the selection
//    textSelection()->changeSpatial( change.offset(), change.length(), change.newTextLength() );

    /// TODO: improve this:
    if( widgetRef_) {
        widget()->updateGeometryComponents();
        notifyStateChange();

        AccessibleTextEditorWidget::notifyTextChangeEvent(widget(), &change, oldText);
    }
}


/// the old selection has been changed
/// @param oldRangeSet the old range set of the change
void TextEditorController::onSelectionChanged(TextRangeSet* oldRangeSet)
{
    Q_UNUSED(oldRangeSet)

    /// TODO: improve this:
    if( widgetRef_) {
        notifyStateChange();
    }
}


/// The line-data is changed, we need to repaint the selected lines
/// @param line the line number that had a data change
/// @param length the number of lines changed
/// @param newLength the new number of lines
void TextEditorController::onLineDataChanged(int line, int length, int newLength)
{
    if( this->widgetRef_ ) {
        widgetRef_->updateLine( line, qMax(length,newLength));
    }
}


/// This method is used to update the component when the configuration has been changed.
/// This is a temporary solution, perhaps we should make TextConfig signal changes
/// A lot of changes don't require an updates, but some do
void TextEditorController::updateAfterConfigChange()
{
    textRenderer()->setThemeByName( textDocument()->config()->themeName() );

    // we need to figure out a better way to do this
    QFont font = textDocument()->config()->font();
    widget()->setFont( font );
    widget()->textEditorComponent()->setFont( font );
    widget()->textMarginComponent()->setFont( font );
    widget()->fullUpdate();
}


/// This method updates the status text. This is the text as displayed in the lower status bar
/// @param extraText the extra text to show in the status bar
void TextEditorController::updateStatusText( const QString& extraText )
{
    QString text;
    TextDocument* doc = textDocument();

    if( !doc->textUndoStack()->isPersisted() ) {
        text.append("[*] ");
    }

    // add the ranges
    if( textSelection_->rangeCount() > 1 ) {
        text.append( QObject::tr("%1 ranges").arg(textSelection_->rangeCount() ) );
    } else {
        TextRange& range = textSelection_->range(0);
        int caret = range.caret();
        int line = doc->lineFromOffset( caret ) ;
        int col  = doc->columnFromOffsetAndLine( caret, line ) + 1;
        text.append( QObject::tr("Line %1, Column %2").arg(line+1).arg(col) );

        if( textDocument()->config()->showCaretOffset() ) {
            text.append( QObject::tr(", Offset %1").arg(caret) );
        }

        if( range.length() > 0 ) {
            text.append( QObject::tr(" | %1 chars selected").arg(range.length()) );

        // add the current scopes
        } else {
            text.append( QObject::tr(" | scope: ") );

            QString str;
            QVector<TextScope*> scopes = textDocument()->scopes()->scopesAtOffset( caret ) ;
            for( int i=0,cnt=scopes.size(); i<cnt; ++i ) {
                TextScope* scope = scopes[i];
                str.append( scope->name() );
                str.append(" ");
            }
            text.append( str );
            text.append( QObject::tr(" (%1)").arg( textDocument()->scopes()->lastScopedOffset() ) );
        }
    }

    // add the extra text
    if( !extraText.isEmpty() ) {
        text.append(" | " );
        text.append(extraText);
    }
    emit updateStatusTextSignal( text );
}


/// updates the main widget
void TextEditorController::update()
{
    if( widgetRef_ ) {
        widgetRef_->fullUpdate();
    }
}


/// Asserts the view shows the given position
void TextEditorController::scrollPositionVisible(int xPos, int yPos)
{
    emit widgetRef_->scrollPositionVisible( xPos, yPos );
}


/// Assets the view shows the given offset
void TextEditorController::scrollOffsetVisible(int offset)
{
    TextRenderer* renderer = textRenderer();
    int xPos = renderer->xPosForOffset(offset);
    int yPos = renderer->yPosForOffset(offset);
    scrollPositionVisible( xPos, yPos );
}


/// This method makes sure caret 1 is visible
void TextEditorController::scrollCaretVisible()
{
    scrollOffsetVisible( textSelection()->range(0).caret() );
}


/// This method adds a text change on the stack that simply stores the current text-selection
/// @param coalsceId the coalescing identifier for merging/coalescing undo operations
void TextEditorController::storeSelection(int coalesceId)
{
    SelectionChange* change = new SelectionChange(this);
    change->giveTextRangeSet( new TextRangeSet( *textSelection() ) );
    textDocument()->executeAndGiveChange( change, coalesceId );
}


/// This method executes the command
void TextEditorController::executeCommand( edbee::TextEditorCommand* textCommand )
{
    // Only readonly commands can be executed in readonly mode
    if( readonly() && !textCommand->readonly() ) {
        updateStatusText( "Cannot edit a readonly document!" );
        return;
    }

    emit commandToBeExecuted( textCommand  );
    textCommand->execute( this );
    emit commandExecuted( textCommand );

    // TODO: move this to a nicer place!!
    updateStatusText( textCommand->toString() );
}


/// Executes a command with the given name
///
/// When the name hasn't been supplied. This function assumes the command is triggered by a QAction
/// and it will retrieve the command-name from the QAction data method
///
/// @param name of the command to execute
void TextEditorController::executeCommand(const QString& name)
{
    // check if an empty command name has been supplied
    QString commandName = name;
    if( commandName.isEmpty() ) {

        // when the command name is blank  try to get the data of the QAction command
        QAction* action= qobject_cast<QAction*>(sender());
        if( !action ) {
            qlog_warn() << "executeCommand was triggered without argument and without QAction data attribute!";
            return;
        }
        commandName = action->data().toString();
    }

    // try to retrieve the command
    TextEditorCommand* command = commandMap()->get(commandName);
    if( command ) { executeCommand( command ); }
}

/// Return the readonly state.
bool TextEditorController::readonly() const
{
    return widget()->readonly();
}

/// Sets the readonly state
void TextEditorController::setReadonly(bool value)
{
    widget()->setReadonly(value);
}


/// replaces the given text (single ranges)
/// @param offset the character offset in the document
/// @param length the number of characters to replace
/// @param text the text to replace
/// @param coalesceId the identifier for grouping undo operations
void TextEditorController::replace(int offset, int length, const QString& text, int coalesceId, bool stickySelection)
{
//    SelectionTextChange* change = new SelectionTextChange(this);
    TextRangeSet ranges(textDocument());
    ranges.addRange(offset, offset+length);
    replaceRangeSet(ranges,text,coalesceId, stickySelection);
}


/// This method replaces the selection with the given text
/// @param text the text to replace the selection with
/// @param coalesceId the identifier for grouping undo operations
void TextEditorController::replaceSelection(const QString& text, int coalesceId, bool stickySelection)
{
    replaceRangeSet( *dynamic_cast<TextRangeSet*>( textSelection() ), text, coalesceId, stickySelection);
}


/// This method replaces the given selection with the given texts
/// @param texts the list of texts that need to be replaced
/// @param coalesceID the identifier for grouping undo operation
void TextEditorController::replaceSelection(const QStringList& texts, int coalesceId, bool stickySelection)
{
    replaceRangeSet(*dynamic_cast<TextRangeSet*>( textSelection() ), texts, coalesceId, stickySelection);
}


///  Replaces the given rangeset with the given text
/// @param rangeset the ranges to replace
/// @param text the text to replace the selection with
/// @param coalesceId the identifier for grouping undo operations
void TextEditorController::replaceRangeSet(edbee::TextRangeSet& rangeSet, const QString& text, int coalesceId, bool stickySelection)
{
    if(readonly()) return;

    textDocument()->beginChanges( this );
    textDocument()->replaceRangeSet(rangeSet, text, stickySelection);
    textDocument()->endChanges(coalesceId);
    notifyStateChange();
}


/// Replaces the given ranges with the given texts. Different text per range is possible
/// @param rangeSet the rangeset to fill
/// @param text the texts to fill the given ranges with.
/// @param coalesceId the identifier for grouping undo operations
void TextEditorController::replaceRangeSet(edbee::TextRangeSet& rangeSet, const QStringList& texts, int coalesceId, bool stickySelection)
{
    if(readonly()) return;

    textDocument()->beginChanges( this );
    textDocument()->replaceRangeSet( rangeSet, texts, stickySelection);
    textDocument()->endChanges( coalesceId );
    notifyStateChange();
}


/// This method creates a command that moves the caret to the given line/column position
/// A negative number means that we're counting from the end
/// This method assumes line 0 is the first line!
///
/// For example:
///     moveCaretTo( 2, 1 ) => Moves the caret to the 3rd line and 2nd column
///     moveCaretTo( -1, -2 ) => Moves the caret to the character before the last character
///
/// The rangeIndex is used to specify which range to move.. (Defaults to -1 which changes to a single range)
void TextEditorController::moveCaretTo(int line, int col, bool keepAnchors, int rangeIndex )
{
    if( line < 0) {
        line = textDocument()->lineCount() + line;
        if( line < 0 ) { line = 0; }
    }
    int offset = textDocument()->offsetFromLine(line);
    int lineLength = textDocument()->lineLength(line);
    if( col < 0 ){
        col = lineLength + col;
    }

    int minusNewLineChar = textDocument()->lineCount()-1 == line ? 0 : 1;
    offset += qBound(0, col, qMax(lineLength-minusNewLineChar, 0));

//textDocument()->offsetFromLineAndColumn(line,col)

    return moveCaretToOffset( offset , keepAnchors, rangeIndex );
}


/// Moves the caret to the given offset
/// @param offset the offset to move the caret to
/// @param keepAnchors should the anchors stay at the current position (extending the selection range)
/// The rangeIndex is used to specify which range to move.. (Defaults to -1 which changes to a single range)
void TextEditorController::moveCaretToOffset(int offset, bool keepAnchors, int rangeIndex)
{
//    SelectionCommand* command = new SelectionCommand( SelectionCommand::MoveCaretToExactOffset, offset, keepAnchors );
    SelectionCommand command( SelectionCommand::MoveCaretToExactOffset, offset, keepAnchors, rangeIndex );
    return executeCommand( &command );
}

/// Move the caret and the anchor to the given offset
/// @param caret the caret location
/// @param anchor the anchor location
/// The rangeIndex is used to specify which range to move.. (Defaults to -1 which changes to a single range)
void TextEditorController::moveCaretAndAnchorToOffset(int caret, int anchor, int rangeIndex)
{
    SelectionCommand command( SelectionCommand::MoveCaretToExactOffset, caret, true, rangeIndex);
    command.setAnchor(anchor);
    return executeCommand( &command );
}


/// Adds a new caret to the selection
/// @param line the line number
/// @param col the column
void TextEditorController::addCaretAt(int line, int col)
{
    return addCaretAtOffset( textDocument()->offsetFromLineAndColumn(line,col) );
}


/// Adds a carert at the given offset
/// @param offset the character offset to add the caret
void TextEditorController::addCaretAtOffset(int offset)
{
    SelectionCommand command( SelectionCommand::AddCaretAtOffset, offset );
    return executeCommand( &command );
}


/// This method changes the text selection
void TextEditorController::changeAndGiveTextSelection(edbee::TextRangeSet* rangeSet, int coalesceId )
{
    SelectionChange* change = new SelectionChange(this);
    change->giveTextRangeSet( rangeSet );
    textDocument()->executeAndGiveChange( change, coalesceId );
}


/// This method performs an undo operation. By supplying soft only
/// controller based operations are undone. When supplying false a Document operation is being undone
void TextEditorController::undo(bool soft)
{
    textDocument()->textUndoStack()->undo( soft ? this : nullptr, soft );
}


/// This method performs an redo operation. By supplying soft only controller based operations are redone.
/// When supplying false a Document operation is being redone
/// @param soft perform a soft undo?
void TextEditorController::redo(bool soft)
{
    textDocument()->textUndoStack()->redo( soft ? this : nullptr, soft );
}



/// Starts an undo group
/// @param group the undogroup to use (defaults to a MergableChangeGroup)
void TextEditorController::beginUndoGroup( edbee::ChangeGroup* group )
{
    if( !group ) {
        group = new ChangeGroup( this );
    }
    textDocument()->beginUndoGroup(group);
}


/// Ends the undo-group.
/// @param coalesceId is used to decide if merging of groups is required.
///                  a value of 0 means NO merging (default)
///                  and id > 0 means if the previous command had the same id, the command is merged
/// @param flatten when an undogroup is ended and flatten is set to true ALL sub-undo-groups are merged to this group (default=false)
void TextEditorController::endUndoGroup(int coalesceId, bool flatten )
{
    textDocument()->endUndoGroup(coalesceId,flatten);
}


} // edbee
