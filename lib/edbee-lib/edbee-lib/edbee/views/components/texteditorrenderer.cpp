/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "texteditorrenderer.h"

#include <QPainter>
#include <QPainterPath>
#include <QTextLayout>

#include "edbee/models/textdocument.h"
#include "edbee/models/texteditorconfig.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/views/texttheme.h"
#include "edbee/views/textselection.h"
#include "edbee/views/textrenderer.h"
#include "edbee/views/textlayout.h"
#include "edbee/util/simpleprofiler.h"

#include "edbee/debug.h"


namespace edbee {

static const int ShadowWidth=5;


TextEditorRenderer::TextEditorRenderer( TextRenderer *renderer )
    : rendererRef_(renderer)
    , themeRef_(0)
    , shadowGradient_(0)
{
    shadowGradient_ = new QLinearGradient( 0, 0, ShadowWidth, 0 );
    shadowGradient_ ->setColorAt(0, QColor( 0x00, 0x00, 0x00, 0x99 ));
    shadowGradient_ ->setColorAt(1, QColor( 0x00, 0x00, 0x00, 0x00 ));

}

TextEditorRenderer::~TextEditorRenderer()
{
    delete shadowGradient_;
}

int TextEditorRenderer::preferedWidth()
{
    return renderer()->totalWidth();
}

void TextEditorRenderer::render(QPainter *painter)
{
    int startLine = renderer()->startLine();
    int endLine   = renderer()->endLine();

//    TextDocument* doc = renderer()->textDocument();
    themeRef_ = renderer()->theme();
    painter->setBackground( themeRef_->backgroundColor() );
    painter->setPen( themeRef_->foregroundColor() );
    painter->fillRect( *renderer()->clipRect(), themeRef_->backgroundColor() );

    // process the items
    for( int line = startLine; line <= endLine; ++line  ) {
        renderLineBackground( painter, line );
        renderLineSelection( painter, line );
        renderLineSeparator( painter, line );
        renderLineText( painter, line );
        renderLineBorderedRanges( painter, line );
    }

    renderCarets(painter);
//    renderShade( painter, *renderer()->clipRect() );        /// TODO, deze renderShade moet misschien in de viewport render-code gebreuren

}


void TextEditorRenderer::renderLineBackground(QPainter *painter,int line)
{
    Q_UNUSED(line);
    Q_UNUSED(painter);
}


void TextEditorRenderer::renderLineSelection(QPainter *painter,int line)
{
//PROF_BEGIN_NAMED("render-selection")
    TextDocument* doc = renderer()->textDocument();
    TextSelection* sel = renderer()->textSelection();
    int lineHeight = renderer()->lineHeight();


    int firstRangeIdx=0;
    int lastRangeIdx=0;
/// TODO: iprove ranges at line by calling rangesForOffsets first for only the visible offsets!
    if( sel->rangesAtLine( line, firstRangeIdx, lastRangeIdx ) ) {

        TextLayout* textLayout = renderer()->textLayoutForLine(line);
        QRectF rect = textLayout->boundingRect();

        int lastLineColumn = doc->lineLength(line);

        // draw all 'ranges' on this line
        for( int rangeIdx = firstRangeIdx; rangeIdx <= lastRangeIdx; ++rangeIdx ) {
            TextRange& range = sel->range(rangeIdx);
            int startColumn = doc->columnFromOffsetAndLine( range.min(), line );
            int endColumn   = doc->columnFromOffsetAndLine( range.max(), line );

            int startX = textLayout->cursorToX(startColumn);
            int endX   = textLayout->cursorToX(endColumn);

            if( range.length() > 0 && endColumn+1 >= lastLineColumn) endX += 3;

            painter->fillRect(startX, line*lineHeight + rect.top(), endX - startX, rect.height(),  themeRef_->selectionColor() ); //QColor::fromRgb(0xDD, 0x88, 0xEE) );
        }
    }
//PROF_END
}


void TextEditorRenderer::renderLineBorderedRanges(QPainter *painter,int line)
{
//PROF_BEGIN_NAMED("render-selection")
    TextDocument* doc = renderer()->textDocument();
    TextRangeSet* sel = renderer()->controller()->borderedTextRanges();
    int lineHeight = renderer()->lineHeight();

    QPen pen(themeRef_->foregroundColor(), 0.5);
//    QPen pen(themeRef_->findHighlightForegroundColor(), 0.5);
//    QBrush brush(themeRef_->findHighlightBackgroundColor());
    painter->setRenderHint(QPainter::Antialiasing);

    int firstRangeIdx=0;
    int lastRangeIdx=0;
/// TODO: improve ranges at line by calling rangesForOffsets first for only the visible offsets!
    if( sel->rangesAtLine( line, firstRangeIdx, lastRangeIdx ) ) {

        TextLayout* textLayout = renderer()->textLayoutForLine(line);
        QRectF rect = textLayout->boundingRect();

        int lastLineColumn = doc->lineLength(line);

        // draw all 'ranges' on this line
        for( int rangeIdx = firstRangeIdx; rangeIdx <= lastRangeIdx; ++rangeIdx ) {
            TextRange& range = sel->range(rangeIdx);
            int startColumn = doc->columnFromOffsetAndLine( range.min(), line );
            int endColumn   = doc->columnFromOffsetAndLine( range.max(), line );

            qreal startX = textLayout->cursorToX( startColumn );
            qreal endX   = textLayout->cursorToX( endColumn );

            if( range.length() > 0 && endColumn+1 >= lastLineColumn) endX += 3;

            QPainterPath path;
            path.addRoundedRect(startX, line*lineHeight + rect.top(), endX - startX, rect.height(),5,5);
//            painter->fillPath(path, brush);
            painter->strokePath(path, pen);
        }
    }
//PROF_END
}



void TextEditorRenderer::renderLineSeparator(QPainter *painter,int line)
{
//PROF_BEGIN_NAMED("render-selection")
    TextEditorConfig* config = renderer()->config();
    int lineHeight = renderer()->lineHeight();
//    int viewportX = renderer()->viewportX();
//    int viewportWidth = renderer()->vie\wportWidth();
    QRect visibleRect( renderer()->viewport() );

    if( config->useLineSeparator() ) {
        const QPen& pen = config->lineSeparatorPen();
        painter->setPen( pen );
        int y = (line+1)*lineHeight; // - pen.width();
//        p.drawLine( viewportX(), y, viewportWidth(), y );
        painter->drawLine( visibleRect.left(), y, visibleRect.right(), y ); // draw from 0 to allow correct rendering of dotted lines
    }
//PROF_END
}


void TextEditorRenderer::renderLineText(QPainter *painter, int line)
{
//PROF_BEGIN_NAMED("render-line-text")
    TextLayout* textLayout = renderer()->textLayoutForLine(line);

    // draw the text layout
    QPoint lineStartPos(0, line*renderer()->lineHeight() );
//PROF_BEGIN_NAMED("fetch-formats")
//    QVector<QTextLayout::FormatRange>& formats = renderer()->themeStyler()->getLineFormatRanges( line );
    QVector<QTextLayout::FormatRange> formats;
//PROF_END

//painter->setClipping(false);
//painter->setPen( QPen( QColor( 0,0,200), 2 ));
//QRect clipDrawRect( renderer()->translatedClipRect() );
//clipDrawRect.adjust(2,2,-2,-2);
//painter->drawRect( clipDrawRect );
//painter->setClipping(true);

//PROF_BEGIN_NAMED("draw-texts")
    painter->setPen( themeRef_->foregroundColor() );
    textLayout->draw( painter, lineStartPos, formats, *renderer()->clipRect() );
//PROF_END

//PROF_END
}

// This method renders all carets
void TextEditorRenderer::renderCarets(QPainter *painter)
{
//PROF_BEGIN_NAMED("render-carets")

    if( renderer()->shouldRenderCaret() ) {
        TextDocument* doc= renderer()->textDocument();
        TextRangeSet* sel = renderer()->textSelection();
        painter->setPen( themeRef_->caretColor() );

        int startLine = renderer()->startLine();
        int endLine = renderer()->endLine();
        int caretWidth = renderer()->config()->caretWidth();

        for( int line = startLine; line <= endLine; ++line  ) {

            QPoint lineStartPos(0, renderer()->yPosForLine(line));

            TextLayout* textLayout = renderer()->textLayoutForLine(line);
            for( int caret = 0, rangeCount=sel->rangeCount(); caret < rangeCount; ++caret ) {

                TextRange& range = sel->range(caret);
                int caretLine = doc->lineFromOffset( range.caret() );
                if( caretLine == line ) {
                    int caretCol = doc->columnFromOffsetAndLine( range.caret(), caretLine );
                    textLayout->drawCursor( painter, lineStartPos, caretCol, caretWidth );
                }
            }
        }
    }
//PROF_END
}


void TextEditorRenderer::renderShade(QPainter *painter, const QRect& rect )
{
    // render a shade
    if( renderer()->viewportX()) {
        int shadowStart = renderer()->viewportX();
        painter->setPen( Qt::NoPen );
        QBrush oldBrush = painter->brush();
        painter->setBrushOrigin(shadowStart, rect.y() );
        painter->setBrush( *shadowGradient_ );

        QRect shadowRect(shadowStart, rect.y(), ShadowWidth, rect.height());
        painter->drawRect( shadowRect.intersected( *renderer()->clipRect() ) );

        painter->setBrush(oldBrush);
    }


}

/// should return the extra pixels to update when updating a line
/// This way it is possible to render margins around the lines
int TextEditorRenderer::extraPixelsToUpdateAroundLines()
{
    return 0;
}


} // edbee

