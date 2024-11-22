#include "movelinecommand.h"

#include "edbee/models/change.h"
#include "edbee/models/textdocument.h"
#include "edbee/models/textrange.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/views/textselection.h"

#include "edbee/debug.h"


namespace edbee {

MoveLineCommand::MoveLineCommand(int direction)
    : direction_(direction)
{

}

MoveLineCommand::~MoveLineCommand()
{
}


/// Calculates the new ranges after moving
/*
static void calculateNewSelectionRanges( TextDocument* doc, TextRangeSet& newCaretSelection, int direction )
{
    for( int i=0,cnt=newCaretSelection.rangeCount(); i<cnt; ++i) {
        TextRange& range = newCaretSelection.range(i);
        int& min = range.minVar();
        int& max = range.maxVar();

        if( direction < 0 ) {
            int line = doc->lineFromOffset(min);
            int len = doc->lineLength(line-1);
            min -= len;
            max -= len;
        } else {
            int line = doc->lineFromOffset(max);
            int len = doc->lineLength(line+1);
            min += len;
            max += len;
        }
    }
}
*/


/// all carets in the newCaretSelection are changed in the movedRange
void updateNewCaretSelectionForMove( TextDocument* doc, TextRangeSet& newCaretSelection, TextRange& movedRange, int direction )
{
    for( int i=0, cnt=newCaretSelection.rangeCount(); i<cnt; ++i ) {
        TextRange& newRange = newCaretSelection.range(i);
        if( movedRange.min() <= newRange.min() && newRange.max() <= movedRange.max() ) {
            int min = movedRange.min();
            int max = movedRange.max();

            int delta=0;
            if( direction < 0 ) {
                int line = doc->lineFromOffset(min);
                delta = - doc->lineLength(line-1);
            } else {
                int line = doc->lineFromOffset(max-1); // -1 exclude the last newline
                delta = doc->lineLength(line+1);
            }

            newRange.setCaretBounded( doc, newRange.caret()+delta);
            newRange.setAnchorBounded( doc, newRange.anchor()+delta);
        }
    }
}



void MoveLineCommand::execute(TextEditorController *controller)
{
    TextRangeSet* sel = controller->textSelection();
    TextDocument* doc = controller->textDocument();

    // expand the selection in full-lines
    TextRangeSet moveRangeSet( *sel );
    moveRangeSet.expandToFullLines(1);
    moveRangeSet.mergeOverlappingRanges(true);

    // because of the full-line expansion, we can assume that 0 means at the first line
    // moving up on the first line is impossible
    int firstOffset = moveRangeSet.firstRange().minVar();
    if( firstOffset + direction_ <= 0 ) return;

    int lastOffset = moveRangeSet.lastRange().maxVar();
    if( lastOffset + direction_ > doc->length() ) return;

    // Calculate the new caret positions
    TextRangeSet newCaretSelection( *sel );
    for( int i=0,cnt=moveRangeSet.rangeCount(); i<cnt; ++i ) {
        TextRange& range = moveRangeSet.range(i);
        updateNewCaretSelectionForMove( doc, newCaretSelection, range, direction_ );
    }

    // move the text in the correct direciton
    controller->beginUndoGroup();
    for( int i=0,cnt=moveRangeSet.rangeCount(); i<cnt; ++i ) {
        TextRange& range = moveRangeSet.range(i);

        int line = doc->lineFromOffset(range.min());
        QString text = doc->textPart(range.min(), range.length());

        controller->replace( range.min(), range.length(), QStringLiteral(), 0);               // remove the line
        controller->replace( doc->offsetFromLine(line+direction_), 0, text, 0 ); // add the line back
    }

    *sel = newCaretSelection; // change the selection

    int coalesceID = CoalesceId_MoveLine + direction_ + 10;
    controller->endUndoGroup(coalesceID,false);
}


QString MoveLineCommand::toString()
{
    return "MoveLineCommand";
}


}
