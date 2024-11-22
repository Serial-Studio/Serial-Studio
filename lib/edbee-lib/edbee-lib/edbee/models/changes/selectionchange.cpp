/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "selectionchange.h"

#include "edbee/models/textbuffer.h"
#include "edbee/models/texteditorconfig.h"
#include "edbee/models/change.h"
#include "edbee/models/textdocument.h"
#include "edbee/models/textrange.h"
#include "edbee/models/textundostack.h"
#include "edbee/views/accessibletexteditorwidget.h"
#include "edbee/views/textrenderer.h"
#include "edbee/views/textselection.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/texteditorwidget.h"

#include "edbee/debug.h"



namespace edbee {


/// The selection selection change constructor
/// @param controller the crontroler this selection change is for
SelectionChange::SelectionChange(TextEditorController* controller )
    : ControllerChange( controller )
    , rangeSet_(0)
{
}


/// destructs the textrange
SelectionChange::~SelectionChange()
{
    delete rangeSet_;
}


/// Gives the textrange to the textchange
void SelectionChange::giveTextRangeSet(TextRangeSet* rangeSet)
{
    delete rangeSet_;
    rangeSet_ = rangeSet;
}


/// Takes ownership of the rangeset and clears the clearset
/// @return the rangeset
TextRangeSet* SelectionChange::takeRangeSet()
{
    TextRangeSet* result = rangeSet_;
    rangeSet_ = 0;
    return result;
}


/// Executes the textchange
/// @param document the textdocument to execute this change for
void SelectionChange::execute( TextDocument* document )
{
    if( !rangeSet_ ) { return; }
    Q_UNUSED(document);
    TextRangeSet* currentSelection = dynamic_cast<TextRangeSet*>( controllerContext()->textSelection() );
    TextRangeSet oldSelection( *currentSelection );

    *currentSelection = *rangeSet_;
    *rangeSet_ = oldSelection;

    notifyChange();
}


/// Reverts the selection change
/// @param document the textdocument to revert this change for
void SelectionChange::revert(TextDocument* document)
{
    if( !rangeSet_ ) { return; }

    Q_UNUSED(document);
    TextRangeSet* currentSelection = dynamic_cast<TextRangeSet*>( controllerContext()->textSelection() );
    TextRangeSet newSelection( *currentSelection );

    *currentSelection = *rangeSet_;
    *rangeSet_ = newSelection;
    notifyChange();
}


/// This method tries to merge the given change with the other change
/// The textChange supplied with this method. Should NOT have been executed yet.
/// It's the choice of this merge operation if the execution is required
bool SelectionChange::giveAndMerge( TextDocument* document, Change* textChange)
{
    Q_UNUSED( document );
    SelectionChange* selectionChange = dynamic_cast<SelectionChange*>( textChange );
    if( selectionChange ) {
//        *rangeSet_ = *selectionChange->rangeSet_;
        delete textChange;
        return true;
    }
    return false;
}


/// Convert this change to a string
QString SelectionChange::toString()
{
    return QStringLiteral("SelectionTextChange(%1)").arg(rangeSet_->rangesAsString());
}


/// This method is called internally for notifying the control the selection has been changed
/// Perhaps we should make e proper emit-signal for this purpose
void SelectionChange::notifyChange()
{
    /// TODO: make the controllerContext only repaint the affected areas via the TextRangeSets
    controllerContext()->onSelectionChanged( rangeSet_ );

    AccessibleTextEditorWidget::notifyTextSelectionEvent(controller()->widget(), controller()->textSelection());
}


} // edbee
