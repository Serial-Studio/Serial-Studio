/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QObject>
#include <QMap>
#include <QStack>

namespace edbee {

class Change;
class TextDocument;
class TextEditorController;
class ChangeGroup;


/// This is the undo stack for the texteditor. This stack is SHARED by all views of the document
/// The stack also stores view-specific commands of all views
///
/// Every view has got it's own pointer on the stack. This pointer points at the index AFTER the last item on the stack
/// that's used for this view.
///
/// When a view-performs a view-specific undo (soft-undo) it's own pointer is stepped back. Only view-specific commands are undone
/// When a view performs a document undo. ALL view-pointers a are undone to the point of the document undo.
///
/// ~~~~
///
/// 7 |       |  <= controllerIndexMap_(v1)
///   +-------+
/// 6 |  v1   |  <= controllerIndexMap_(v2)
///   +-------+
/// 5 |  v2   |
///   +-------+
/// 4 |  v1   |  <= changeIndex_
///   +-------+
/// 3 |  DOC  |
///   +-------+
/// 2 |  v1   |
///   +-------+
/// 1 |  DOC  |
///   +-------+
/// 0 |  v2   |
///   +-------+
///
/// ~~~~

class EDBEE_EXPORT TextUndoStack : public QObject
{
    Q_OBJECT

public:

    /// This enumeration is signaled to the listeners to notify what happend when ending an undo group
    enum EndUndoGroupAction {
        ActionUngrouped,        ///< The group has been ungrouped, the single change has been added to the stack
        ActionFullDiscard,      ///< The group contained nothing usefull, nothing is added to the stack
        ActionEnd               ///< A normal group end

    };

public:
    explicit TextUndoStack( TextDocument* doc, QObject* parent = 0);
    virtual ~TextUndoStack();
    void clear();

    void registerContoller( TextEditorController* controller );
    void unregisterController( TextEditorController* controller );
    bool isControllerRegistered( TextEditorController* controller );

    void beginUndoGroup( ChangeGroup* group );
    ChangeGroup* currentGroup();
    void endUndoGroup(int coalesceId , bool flatten);
    void endUndoGroupAndDiscard();
    int undoGroupLevel();

    int lastCoalesceIdAtCurrentLevel();
    void setLastCoalesceIdAtCurrentLevel(int id);
    void resetAllLastCoalesceIds();

    Change* giveChange( Change* change, int coalesceId );

    bool canUndo( TextEditorController* controller=0 );
    bool canRedo( TextEditorController* controller=0 );

    void undo( TextEditorController* controller=0, bool controllerOnly=false );
    void redo( TextEditorController* controller=0, bool controllerOnly=false );

    bool isCollectionEnabled() { return collectionEnabled_; }
    void setCollectionEnabled( bool enabled ) { collectionEnabled_ = enabled; }

    bool isUndoRunning() { return undoRunning_; }
    bool isRedoRunning() { return redoRunning_; }

    int size();
    Change* at(int idx);
    int currentIndex( TextEditorController* controller=0 );
    int lastIndex( TextEditorController* controller=0 );
    Change* last(TextEditorController* controller=0 );

    int sizeInDocChanges();
    int currentIndexInDocChanges();

    Change* findRedoChange( TextEditorController* controller=0);
    Change* findUndoChange( TextEditorController* controller=0 );

    void setPersisted(bool enabled);
    bool isPersisted();
    int persistedIndex();

    TextDocument* document() { return documentRef_; }

    QString dumpStack();
    void dumpStackInternal();

protected:
    int findRedoIndex( int index, TextEditorController* controller=0 );
    int findUndoIndex( int index, TextEditorController* controller=0 );
    void clearRedo( TextEditorController* controller );
    bool undoControllerChange( TextEditorController* controller );
    void undoDocumentChange();
    bool redoControllerChange( TextEditorController* controller );
    void redoDocumentChange();

    void setPersistedIndex( int index );
    void setChangeIndex( int index );

signals:

    void undoGroupStarted( edbee::ChangeGroup* group );

    /// This signal is fired when the group is ended. Warning, when the group is merged
    /// the group pointer will be 0!!
    void undoGroupEnded( int coalesceId, bool merged, int action );
    void changeAdded( edbee::Change* change );
//    void changeMerged( edbee::TextChange* oldChange, edbee::TextChange* change );
    void undoExecuted( edbee::Change* change );
    void redoExecuted( edbee::Change* change );

    /// This signal is emitted if the persisted state is changed
    void persistedChanged(bool persisted);


private:
    void clearHistoryLists();


    TextDocument* documentRef_;                           ///< A reference to the textdocument

    QList<Change*> changeList_;                       ///< The list of stack commands
    int changeIndex_;                                     ///< The current command index (TextDocument/Global)
    QMap<TextEditorController*,int> controllerIndexMap_;  ///< The current controller pointers (View specific)
    int persistedIndex_;                                  ///< The current persisted index. A persisted-index of -1, means it never is persisted

    QStack<ChangeGroup*> undoGroupStack_;             ///< A stack of all undo items
    QStack<int> lastCoalesceIdStack_;                     ///< The last coalesceId

//    int undoGroupLevel_;                                 ///< The undo group level
//    TextChangeGroup* undoGroup_;                         ///< The current undo group
    bool collectionEnabled_;                               ///< Is the undo-stack enabled?

    bool undoRunning_;                                    ///< This flag is set if undo is running
    bool redoRunning_;                                    ///< This flag is set if a redo is running
};

} // edbee
