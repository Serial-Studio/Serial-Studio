/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include <QLinearGradient>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QWidget>

#include "texteditorscrollarea.h"

#include "edbee/debug.h"

namespace edbee {

static const int ShadowSize=8;


/// A special class to draw a shadow over the widget
class PrivateShadowWidget : public QWidget
{
    TextEditorScrollArea* scrollAreaRef_;
    QLinearGradient* leftShadow_;   ///< The shadow gradients to draw
    QLinearGradient* topShadow_;    ///< The shadow gradients to draw
    QLinearGradient* rightShadow_;  ///< The shadow gradients to draw
    QLinearGradient* bottomShadow_; ///< The shadow gradients to draw

public:
    PrivateShadowWidget( TextEditorScrollArea* parent )
        : QWidget(parent)
        , scrollAreaRef_(parent)
        , leftShadow_(0)
        , topShadow_(0)
        , rightShadow_(0)
        , bottomShadow_(0)
    {
        QColor black = QColor( 0x00, 0x00, 0x00, 0x99 );
        QColor trans = QColor( 0x00, 0x00, 0x00, 0x00 );

        leftShadow_ = new QLinearGradient( 0, 0, ShadowSize, 0 );
        leftShadow_ ->setColorAt(0, black );
        leftShadow_ ->setColorAt(1, trans );

        topShadow_ = new QLinearGradient( 0, 0, 0, ShadowSize );
        topShadow_ ->setColorAt(0, black );
        topShadow_ ->setColorAt(1, trans );

        rightShadow_ = new QLinearGradient( 0, 0, ShadowSize, 0 );
        rightShadow_ ->setColorAt(0, trans );
        rightShadow_ ->setColorAt(1, black );

        bottomShadow_ = new QLinearGradient( 0, 0, 0, ShadowSize );
        bottomShadow_ ->setColorAt(0, trans);
        bottomShadow_ ->setColorAt(1, black );


        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_TranslucentBackground );
    }
    ~PrivateShadowWidget()
    {
        delete leftShadow_;
        delete topShadow_;
        delete rightShadow_;
        delete bottomShadow_;
    }

protected:

    void paintEvent( QPaintEvent* event )
    {
        QPainter painter(this);
        QBrush oldBrush = painter.brush();
        renderShade( event,&painter );
        painter.setBrush(oldBrush);
    }



    void renderShade(QPaintEvent* event, QPainter* painter )
    {
        painter->setPen( Qt::NoPen );

        int scrollX = scrollAreaRef_->horizontalScrollBar()->value();
        int minX    = scrollAreaRef_->horizontalScrollBar()->minimum();
        int maxX    = scrollAreaRef_->horizontalScrollBar()->maximum();

        int scrollY = scrollAreaRef_->verticalScrollBar()->value();
        int minY    = scrollAreaRef_->verticalScrollBar()->minimum();
        int maxY    = scrollAreaRef_->verticalScrollBar()->maximum();

        // render shadow at the left side
        if( scrollX != minX ) {
            painter->setBrush( *leftShadow_ );
            QRect shadowRect(0, 0, ShadowSize, height());
            painter->setBrushOrigin( shadowRect.topLeft() );
            painter->drawRect( shadowRect.intersected( event->rect() ) );
        }

        // render shadow top
        if( scrollX != maxX ) {
            painter->setBrush( *rightShadow_);
            QRect shadowRect(width()-ShadowSize, 0, ShadowSize, height());
            painter->setBrushOrigin( shadowRect.topLeft() );
            painter->drawRect( shadowRect.intersected( event->rect() ) );
        }
        // render shadow at the left side
        if( scrollY != minY ) {
            painter->setBrush( *topShadow_);
            QRect shadowRect(0, 0, width(), ShadowSize);
            painter->setBrushOrigin( shadowRect.topLeft() );
            painter->drawRect( shadowRect.intersected( event->rect() ) );
        }

        // render shadow bottom
        if( scrollY != maxY ) {
            painter->setBrush( *bottomShadow_);
            QRect shadowRect(0, height()-ShadowSize, width(), ShadowSize );
            painter->setBrushOrigin( shadowRect.topLeft() );
            painter->drawRect( shadowRect.intersected( event->rect() ) );
        }

    }


};



TextEditorScrollArea::TextEditorScrollArea(QWidget* parent)
    : QScrollArea(parent)
    , leftWidgetRef_(0)
    , topWidgetRef_(0)
    , rightWidgetRef_(0)
    , bottomWidgetRef_(0)
    , shadowWidgetRef_(0)
{
    shadowWidgetRef_ = new PrivateShadowWidget(this);
    setFrameShape(QFrame::NoFrame);
    setFocusPolicy(Qt::NoFocus);
}

TextEditorScrollArea::~TextEditorScrollArea()
{
}

void TextEditorScrollArea::setLeftWidget(QWidget* widget)
{
    leftWidgetRef_ = widget;
}

void TextEditorScrollArea::setTopWidget(QWidget* widget)
{
    topWidgetRef_ = widget;
}

void TextEditorScrollArea::setRightWidget(QWidget* widget)
{
    rightWidgetRef_ = widget;
}

void TextEditorScrollArea::setBottomWidget(QWidget* widget)
{
    bottomWidgetRef_ = widget;
}

/// this method layouts the margin-widgets
void TextEditorScrollArea::layoutMarginWidgets()
{
    int left   = leftWidgetRef_ ? leftWidgetRef_->sizeHint().width() : 0;
    int top    = topWidgetRef_ ? topWidgetRef_->sizeHint().height() : 0;
    int right  = rightWidgetRef_ ? rightWidgetRef_->sizeHint().width() : 0;
    int bottom = bottomWidgetRef_ ? bottomWidgetRef_->sizeHint().height() : 0;

    // set the margins
//    int pixelFix = -1;  // removes the dirty little gray line between the components
    setViewportMargins( left, top, right, bottom );

    // overlay the shadow over the viewport
    if( shadowWidgetRef_ ) {
        this->shadowWidgetRef_->setGeometry( viewport()->geometry() );
    }

    // set the widget
    if( leftWidgetRef_) {
        QRect rect( 0, top, left, qMin( leftWidgetRef_->maximumHeight(), viewport()->height()+1 ) );
        leftWidgetRef_->setGeometry(rect);
    }

    if( topWidgetRef_) {
        QRect rect( 0, 0, qMin( topWidgetRef_->maximumWidth(), width() )+1, top+1  );
        topWidgetRef_->setGeometry(rect);
    }

    if( rightWidgetRef_) {
        QRect rect( viewport()->width()+left, top, right+1, qMin( rightWidgetRef_->maximumHeight(), viewport()->height() )+1  );
        rightWidgetRef_->setGeometry(rect);
    }

    if( bottomWidgetRef_) {
        QRect rect( 0, viewport()->height()+top, qMin( bottomWidgetRef_->maximumWidth(), viewport()->width() )+1, bottom+1  );
        bottomWidgetRef_->setGeometry(rect);
    }

}

void TextEditorScrollArea::enableShadowWidget(bool enabled)
{
    shadowWidgetRef_->setVisible(enabled);
}


void TextEditorScrollArea::resizeEvent(QResizeEvent* event)
{
    QScrollArea::resizeEvent(event);
    layoutMarginWidgets();
}


} // edbee
