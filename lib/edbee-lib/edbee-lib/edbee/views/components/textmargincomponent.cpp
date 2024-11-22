/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textmargincomponent.h"

#include <QApplication>
#include <QLinearGradient>
#include <QPainter>
#include <QScrollBar>
#include <QWheelEvent>

#include "edbee/models/texteditorconfig.h"
#include "edbee/models/textbuffer.h"
#include "edbee/models/textdocument.h"
#include "edbee/views/components/texteditorcomponent.h"
#include "edbee/views/texteditorscrollarea.h"
#include "edbee/views/textrenderer.h"
#include "edbee/views/texttheme.h"
#include "edbee/views/textselection.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/texteditorwidget.h"

#include "edbee/debug.h"

namespace edbee {

static const int LineNumberRightPadding = 10;
static const int MarginPaddingRight = 5;

/// The default constructor
TextMarginComponentDelegate::TextMarginComponentDelegate()
    : marginComponentRef_(nullptr)
    , startLine_(0)
{
}

/// The default text (of course) is the line-number
QString TextMarginComponentDelegate::lineText(int line)
{
    return QStringLiteral("%1").arg(line+1);
}

/// The extra space required before the line-number
/// The default implemenation returns 0
int TextMarginComponentDelegate::widthBeforeLineNumber()
{
    return 5;
}

/// Custom rendering before the line-numbers etc are drawn
void TextMarginComponentDelegate::renderBefore(QPainter *painter, int startLine, int endLine, int width)
{
    Q_UNUSED(painter)
    Q_UNUSED(startLine)
    Q_UNUSED(endLine)
    Q_UNUSED(width)
}

/// The delegate can berform custom rendering on a given line
void TextMarginComponentDelegate::renderAfter(QPainter* painter, int startLine, int endLine, int width)
{
    Q_UNUSED(painter)
    Q_UNUSED(startLine)
    Q_UNUSED(endLine)
    Q_UNUSED(width)
}

/// Make this method return true to enable mouse tracking
bool TextMarginComponentDelegate::requiresMouseTracking()
{
    return true;
}

/// This method is called when the mouse is moved over a certain line
/// To use it you MUST manually call setMouseTracking() on the TextMarginComponent
void TextMarginComponentDelegate::mouseMoveEvent(int line, QMouseEvent* event)
{
    Q_UNUSED(line)
    Q_UNUSED(event)
    if( event->buttons() & Qt::LeftButton ) {
        if( line >= 0 ) {
            TextEditorController* controller = marginComponent()->editorWidget()->controller();
            if( line < startLine_ ) {
                controller->moveCaretTo(startLine_+1,0,false);
                controller->moveCaretTo(line,0,true);
            } else {
                controller->moveCaretTo(line+1,0,true);
            }
        }
    }
}

void TextMarginComponentDelegate::mousePressEvent(int line, QMouseEvent* event)
{
    Q_UNUSED(event)
    if( line >= 0 ) {
        TextEditorController* controller = marginComponent()->editorWidget()->controller();
        controller->moveCaretTo(line,0,false);
        controller->moveCaretTo(line+1,0,true);
        startLine_ = line;
    }
}

void TextMarginComponentDelegate::leaveEvent(QEvent* event)
{
    Q_UNUSED(event)
}



//=============================


/// Constructs the textmarginc compnonent.
/// @param editor the editor this component is connceted to
/// @param parent the parent widget
TextMarginComponent::TextMarginComponent(TextEditorWidget* editor, QWidget* parent)
    : QWidget( parent )
    , top_(0)
    , width_(-1)
    , lastLineCount_(-1)
    , marginFont_(nullptr)
    , editorRef_( editor )
    , delegate_(nullptr)
    , delegateRef_(nullptr)
{
    this->setFocusPolicy( Qt::NoFocus );
    this->setAutoFillBackground(false);
    this->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Expanding );
}


/// The margin component destructor
TextMarginComponent::~TextMarginComponent()
{
    delete delegate_;
    delete marginFont_;
}


/// initalizes this component
void TextMarginComponent::init()
{
    giveDelegate( new TextMarginComponentDelegate() );
    updateMarginFont();
    connect( editorRef_->textRenderer(), SIGNAL(themeChanged(TextTheme*)), SLOT(updateFont()));
    connect( editorRef_->config(), SIGNAL(configChanged()), SLOT(updateFont()));
    connectScrollBar();
}


/// Updates the margin font
void TextMarginComponent::updateMarginFont()
{
    delete marginFont_;
    if( renderer()->textWidget() ) {
        const QFont& font = editorWidget()->font();
        marginFont_ = new QFont( editorWidget()->font().family());
        if( font.pointSizeF() > 0 ) marginFont_->setPointSizeF( font.pointSizeF() );
        if( font.pixelSize() > 0 ) marginFont_->setPixelSize( font.pixelSize() );
    } else {
        marginFont_ = new QFont( QFont().family(), 10 ); // fallback for test-suite
    }
}


/// Returns the required width for this control
int TextMarginComponent::widthHint() const
{
    TextDocument* doc = renderer()->textDocument();
    int lineCount = doc->lineCount();
    if( lastLineCount_ != lineCount ) {
        lastLineCount_ = lineCount;

        // count the number of characters required
        int decimals = 0;
        int decimalValue = 1;
        do {
            ++decimals;
            decimalValue *= 10;
        }
        while( decimalValue <= lineCount );

        // we at least reserve space for 2 decimals
        decimals = qMax(decimals,2);

        width_ = qMax( renderer()->nrWidth(), 10 ) * ( decimals ) + LineNumberRightPadding + MarginPaddingRight;
        width_ += delegate()->widthBeforeLineNumber();
    }
    return width_;
}


/// This method returns the size hint
QSize TextMarginComponent::sizeHint() const
{
//    QSize size = QWidget::sizeHint();
    return QSize( widthHint(), 78 );
    //return QSize(widthHint(), renderer()->totalHeight() );
}


/// This emthod is called when the text is changed
bool TextMarginComponent::isGeometryChangeRequired()
{
    return this->widthHint() != this->width();
}


/// A slow and full update of the control
void TextMarginComponent::fullUpdate()
{
    updateGeometry();
    update();
}


/// Returns the renderer
TextRenderer* TextMarginComponent::renderer() const
{
    return editorWidget()->textRenderer();
}


/// This method sets the delegate
void TextMarginComponent::setDelegate(TextMarginComponentDelegate* delegate)
{
    delete delegate_;
    delegate_ = nullptr;
    if( delegate == nullptr ) { delegate = new TextMarginComponentDelegate(); }
    delegateRef_ = delegate;

    // perform some updates
    delegateRef_->setMarginCompenent( this );
    setMouseTracking( delegateRef_->requiresMouseTracking() );
}

void TextMarginComponent::giveDelegate(TextMarginComponentDelegate* delegate)
{
    setDelegate( delegate );
    delegate_ = delegate;
}

void TextMarginComponent::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);

    painter.translate(0, -top_);

    QRect paintRect = event->rect();
//    QRect translatedRect( clipRect.x()+offsetX, clipRect.y()+offsetY, clipRect.width(), clipRect.height() );
    paintRect.adjust(0,top_,0,top_);
    renderer()->renderBegin( paintRect );


    int startLine     = renderer()->startLine();
    int endLine       = renderer()->endLine();
    const QSize& size = this->size();

    QRect totalRect( 0, 0, size.width(), size.height()+top_ );
    QRect rect( totalRect.intersected( *renderer()->clipRect() ) );

//qlog_info()<< "** top:"<<top_<<", total:"<<totalRect<<", paintRect: "<<paintRect<<" => rect: " << rect;

    // fill the backgound
    painter.fillRect( rect,  renderer()->theme()->backgroundColor() );

    delegate()->renderBefore( &painter, startLine, endLine, size.width() );
    renderCaretMarkers( &painter, startLine, endLine, size.width() );
    renderLineNumber( &painter, startLine, endLine, size.width() );
    delegate()->renderAfter( &painter, startLine, endLine, size.width() );

    renderer()->renderEnd( paintRect );
    painter.translate(0, top_);

}


/// Renders the caret markers in the sidebar
/// @param painter the Qt Painting context
/// @param startLine the first line to render
/// @param endLine the last line to render
/// @param width the width for rendering
void TextMarginComponent::renderCaretMarkers(QPainter* painter, int startLine, int endLine , int width)
{
    TextDocument* doc = renderer()->textDocument();
    TextSelection* sel = renderer()->textSelection();
    QColor lineColor = renderer()->theme()->lineHighlightColor();
    int lineHeight = renderer()->lineHeight();

    QRect marginRect(0,0,width-MarginPaddingRight,lineHeight);
    for( int i=0,cnt=sel->rangeCount(); i<cnt; ++i ) {

        TextRange& range = sel->range(i);
        int line = doc->lineFromOffset(range.caret());
        if( startLine <= line ) {
            if( line > endLine ) { break; }
            int y = renderer()->yPosForLine(line);
            marginRect.moveTop(y);
            painter->fillRect( marginRect, lineColor );
        }
    }
}


/// Renders the line number in the sidebar
/// @param painter the Qt Painting context
/// @param startLine the first line to render
/// @param endLine the last line to render
/// @param width the width for rendering
void TextMarginComponent::renderLineNumber(QPainter* painter, int startLine, int endLine , int width)
{
    painter->setFont(*marginFont_);
    QColor selectedPenColor = renderer()->theme()->foregroundColor();
    QColor penColor( selectedPenColor);
    penColor.setAlphaF(0.5);

    int lineHeight = renderer()->lineHeight();
    int textWidth =  width-LineNumberRightPadding-MarginPaddingRight - delegate()->widthBeforeLineNumber();

    for( int line=startLine; line<=endLine; ++line ) {
        int y = renderer()->yPosForLine(line);

        // highlight the selected lines
        int firstIndex=0, lastIndex=0;
        if( renderer()->textSelection()->rangesAtLineExclusiveEnd(line, firstIndex, lastIndex) ) {
            painter->setPen(selectedPenColor);
        } else {
            painter->setPen(penColor);
        }

        painter->drawText( delegate()->widthBeforeLineNumber(), y+2, textWidth, lineHeight, Qt::AlignRight, delegate()->lineText( line) );
    }
}

/// Can be used to track mouse events
/// This method sends the the mouse movement events to the delegate
void TextMarginComponent::mouseMoveEvent(QMouseEvent* event)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    int y = event->pos().y() + top_;
#else
    int y = event->position().toPoint().y() + top_;
#endif
    int line = renderer()->lineIndexForYpos( y );
    delegate()->mouseMoveEvent(line, event);
    QWidget::mouseMoveEvent(event);
}


/// Can be used to track mouse events
/// This method sends the the mouse movement events to the delegate
void TextMarginComponent::mousePressEvent(QMouseEvent* event)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    int y = event->pos().y() + top_;
#else
    int y = event->position().toPoint().y() + top_;
#endif
    int line = renderer()->lineIndexForYpos( y );
    delegate()->mousePressEvent( line, event );
    QWidget::mousePressEvent(event);

    if(!editorWidget()->textEditorComponent()->hasFocus()) {
        editorWidget()->textEditorComponent()->setFocus();
    }
}


/// A double click
void TextMarginComponent::mouseDoubleClickEvent(QMouseEvent *)
{
}


/// The mouse leave
void TextMarginComponent::leaveEvent(QEvent* event)
{
    delegate()->leaveEvent(event);
}


/// forward the mouse wheel event to the scrollbar so you can use the mouse wheel on this component
void TextMarginComponent::wheelEvent(QWheelEvent* event)
{
//    if( event->orientation() == Qt::Vertical) {
    if( event->angleDelta().y() != 0 ) {
        QApplication::sendEvent( editorRef_->textScrollArea()->verticalScrollBar(), event );
    }
}


/// updates the given line so it will be repainted
void TextMarginComponent::updateLineAtOffset(int offset)
{
    int yPos = renderer()->yPosForOffset( offset ) - top_;

    update( 0,  yPos, width(), renderer()->lineHeight()  );
}


/// This method repaints the given lines
void TextMarginComponent::updateLine(int line, int length)
{
    TextRenderer* ren = renderer();
    int startY = ren->yPosForLine( line ) - top_;
    int endY   = ren->yPosForLine( line + length ) - top_;

    update( 0, startY, width(), endY-startY );
}


void TextMarginComponent::topChanged(int value)
{
    top_ = value;
    update();
}


/// You must reconnect the scrollbars when scrollbars are changed
void TextMarginComponent::connectScrollBar()
{
    connect( editorRef_->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(topChanged(int)) );
}

void TextMarginComponent::updateFont()
{
    updateMarginFont();
}



} // edbee
