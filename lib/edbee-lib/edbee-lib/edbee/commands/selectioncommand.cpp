/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "selectioncommand.h"

#include "edbee/models/change.h"
#include "edbee/models/textdocument.h"
#include "edbee/models/texteditorconfig.h"
#include "edbee/models/textrange.h"
#include "edbee/views/textselection.h"
#include "edbee/views/textrenderer.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/texteditorwidget.h"

#include "edbee/debug.h"

namespace edbee {


/// The constructor of the selection command
/// @param unit the unit of this command
/// @param amount the number of steps
/// @param keepSelection when true the anchor stays put (and the selection is expanded)
SelectionCommand::SelectionCommand(SelectionType unit, int amount, bool keepSelection , int rangeIndex)
    : unit_(unit)
    , amount_(amount)
    , anchor_(-1)
    , keepSelection_(keepSelection)
    , rangeIndex_(rangeIndex)
{
}


/// The descructor of the command
SelectionCommand::~SelectionCommand()
{
}


/// Return the command id
/// This is th coalesceId + the given selection item
int SelectionCommand::commandId()
{
    int coalesceId = CoalesceId_Selection + unit_* 10;
    if( amount_ > 0 ) { coalesceId +=1 ; }
    return coalesceId;
}


/// execute the given selection command
/// @param controller the controller to execute the selection for
void SelectionCommand::execute( TextEditorController* controller )
{
    // save the selection state
    TextDocument* document = controller->textDocument();
    TextRangeSet* currentSelection = dynamic_cast<TextRangeSet*>( controller->textSelection() );
    TextRangeSet* sel = new TextRangeSet(*currentSelection);  // start with the current selection

    bool resetAnchors = !keepSelection_;

    // handle the select operation
    switch( unit_ ) {
        // character movement
        case MoveCaretByCharacter:
            sel->moveCarets(amount_);
            break;

        // This results in clearing the selection if a selection is present or it results in a movement of the caret.
        // When clearing a selection the caret is placed next to the selection (which side depends on the direction)
        case MoveCaretsOrDeselect:
            if( keepSelection_ ) {
                sel->moveCarets(amount_);
            } else {
                sel->moveCaretsOrDeselect(amount_);
            }
            break;

        case MoveCaretByWord:
            sel->moveCaretsByCharGroup(amount_, document->config()->whitespaceWithoutNewline(), document->config()->charGroups() );
            break;

        case MoveCaretByLine:
            TextSelection::moveCaretsByLine( controller, sel, amount_ );
            break;

        case MoveCaretToWordBoundary:
            sel->moveCaretsByCharGroup(amount_,  document->config()->whitespaceWithoutNewline(), document->config()->charGroups() );
            break;


        case MoveCaretToLineBoundary:
            sel->moveCaretsToLineBoundary( amount_, document->config()->whitespaceWithoutNewline() );
            break;

        case MoveCaretToDocumentBegin:
            sel->toSingleRange();
            sel->range(0).setCaret(0);
            break;

        case MoveCaretToDocumentEnd:
            sel->toSingleRange();
            sel->range(0).setCaret( controller->textDocument()->length() );
            break;

        case MoveCaretByPage:
        {
            // make sure the first line of the window is scrolled
            TextRenderer* renderer   = controller->textRenderer();
            TextEditorWidget* widget = controller->widget();
            int firstVisibleLine = renderer->firstVisibleLine();
            int linesPerPage     = renderer->viewHeightInLines();

            sel->beginChanges();
            TextSelection::moveCaretsByPage( controller, sel, amount_ );
            if( !keepSelection_ ) {
                sel->resetAnchors();    // we must reset anchors here because de process-changes will merge carets
            }
            sel->endChanges();

            firstVisibleLine += linesPerPage * amount_;
            widget->scrollTopToLine( firstVisibleLine );

            break;
        }
        case MoveCaretToExactOffset:
        {
            if( rangeIndex_ < 0 ) {
                sel->toSingleRange();

            }
            TextRange & range = sel->range( rangeIndex_ >= 0 ? rangeIndex_ : 0);
            range.setCaret(amount_);
            if( anchor_ >= 0 ) range.setAnchor(anchor_);
            break;
        }
        case SelectAll:
            sel->toSingleRange();
            sel->setRange(0, document->buffer()->length() );
            resetAnchors = false;   // do not reset the anchors
            break;

        case SelectWord:
            sel->expandToWords(document->config()->whitespaces(), document->config()->charGroups());
            resetAnchors = false;   // do not reset the anchors
            break;

        case SelectWordAt:
            sel->selectWordAt( amount_, document->config()->whitespaces(), document->config()->charGroups() );
            resetAnchors = false;
            break;

        case ToggleWordSelectionAt:
            sel->toggleWordSelectionAt( amount_, document->config()->whitespaces(), document->config()->charGroups() );
            resetAnchors = false;
            break;

        case SelectFullLine:
            sel->expandToFullLines( amount_);
            resetAnchors = false;
            break;

        case AddCaretAtOffset:
            sel->addRange( amount_, amount_ );
            resetAnchors = false;   // do not reset the anchors
            break;

        case AddCaretByLine:
            TextSelection::addRangesByLine( controller, sel, amount_ );
            break;

        case ResetSelection:

            // when there's a selection ESCAPE clears the selection
            if( sel->hasSelection() ) {
                sel->clearSelection();
            } else if( sel->rangeCount() > 1 ) {
                sel->toSingleRange();
            }
            resetAnchors = false;   // keep selection
            break;
    }

    if( resetAnchors ) {
        sel->resetAnchors();
    }

    // no change?
    if( currentSelection->equals( *sel ) ) {
        delete sel;
        return; // 0
    }

    controller->changeAndGiveTextSelection( sel, commandId() );
}


/// Converts the unit enumeration to a string
/// @param unit the unit enumeration value
/// @return the string representation of this unit
static QString unitToString( int unit ) {
    switch( unit ) {
      case SelectionCommand::MoveCaretByCharacter: return "MoveCaretByCharacter";
      case SelectionCommand::MoveCaretByWord: return "MoveCaretByWord";
      case SelectionCommand::MoveCaretByLine: return "MoveCaretByLine";
      case SelectionCommand::MoveCaretToLineBoundary: return "MoveCaretToLineBoundary";
      case SelectionCommand::MoveCaretToDocumentBegin: return "MoveCaretToDocumentBegin";
      case SelectionCommand::MoveCaretToDocumentEnd: return "MoveCaretToDocumentEnd";
      case SelectionCommand::MoveCaretToExactOffset: return "MoveCaretToExactOffset";
      case SelectionCommand::SelectAll: return "SelectAll";
      case SelectionCommand::SelectWord: return "SelectWord";
      case SelectionCommand::SelectWordAt: return "SelectWordAt";
      case SelectionCommand::ToggleWordSelectionAt: return "ToggleWordSelectionAt";
      case SelectionCommand::SelectFullLine: return "SelectFullLine";
      case SelectionCommand::AddCaretAtOffset: return "AddCaretAtOffset";
      case SelectionCommand::AddCaretByLine: return "AddCaretByLine";
      case SelectionCommand::ResetSelection: return "ResetToSingleCaret";
      default: return "Unkown";
    }
}


/// Converts this command to a strings
QString SelectionCommand::toString()
{
    return QStringLiteral("SelectionCommand(%1)").arg( unitToString(unit_) );
}

bool SelectionCommand::readonly()
{
    return true;
}

} // edbee
