/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textrenderer.h"

#include <QBrush>
#include <QDateTime>
#include <QRect>
#include <QPainter>
#include <QStringList>
#include <QTextLayout>

#include "edbee/util/simpleprofiler.h"

#include "edbee/models/chardocument/chartextdocument.h"
#include "edbee/models/textdocument.h"
#include "edbee/models/texteditorconfig.h"
#include "edbee/models/textlexer.h"
#include "edbee/views/textlayout.h"
#include "edbee/views/textselection.h"
#include "edbee/views/texttheme.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/texteditorwidget.h"

#include "edbee/debug.h"


/// Using control picutres makes it possible to show the control characters (rquires a special font)
//#define USE_CONTROL_PICTURES

namespace edbee {


/// The default textrenderer constructor
TextRenderer::TextRenderer(TextEditorController* controller)
    : QObject(nullptr)
    , controllerRef_(controller)
    , caretTime_(0)
    , caretBlinkRate_(0)
    , totalWidthCache_(0)
    , textThemeStyler_(nullptr)
    , clipRectRef_(nullptr)
    , startOffset_(0)
    , endOffset_(0)
    , startLine_(0)
    , endLine_(0)
    , placeHolderDocument_(0)
{
    connect( controller, SIGNAL(textDocumentChanged(edbee::TextDocument*,edbee::TextDocument*)), this, SLOT(textDocumentChanged(edbee::TextDocument*,edbee::TextDocument*)));
    textThemeStyler_ = new TextThemeStyler(controller);
    placeHolderDocument_ = new CharTextDocument();
}


/// the destructor
TextRenderer::~TextRenderer()
{
    delete placeHolderDocument_;
    delete textThemeStyler_;
    cachedTextLayoutList_.clear();
//    qDeleteAll(cachedTextLayoutList_);
}


/// The init method is called if all objects required for editing have been created!
void TextRenderer::init()
{
    caretBlinkRate_ = config()->caretBlinkingRate();
    resetCaretTime();
}


/// This method resets all caching information
void TextRenderer::reset()
{
    totalWidthCache_ = 0;
    cachedTextLayoutList_.clear();
}


/// This method returns the (maximum) line-height in pixels
int TextRenderer::lineHeight()
{

/// TODO: cache the height :-)
    QFontMetrics fm = textWidget()->fontMetrics();
    return fm.ascent() + fm.descent() + fm.leading() + config()->extraLineSpacing(); // (the 1 is for the base line).
}


/// This method converts the give y position to a line index
/// Warning this returns a RAW line index, which means it can be an invalid line
/// @param y the y position
int TextRenderer::rawLineIndexForYpos(int y)
{
    return y / lineHeight();
}


/// This method returns a valid line index for the given y-pos
/// If the y-position isn't on a line it returns a value < 0
int TextRenderer::lineIndexForYpos(int y)
{
    int line = rawLineIndexForYpos(y);
    if( line >= textDocument()->lineCount() ) return -1;
    return line;
}


/// Returns the total width of the editor. This method is NOT the real with
/// This method takes the maximum line length and multiplies it with the widest character.
int TextRenderer::totalWidth()
{
    if( !totalWidthCache_ ) {
        for( int line=0,cnt=textDocument()->lineCount(); line<cnt; ++line ) {
            TextLayout* layout = textLayoutForLine( line );
            totalWidthCache_ = qMax( qRound(layout->boundingRect().right()+0.5), totalWidthCache_ );
        }

    }
    return totalWidthCache_;

    /* maxWidth returns 0 on Mac
    if( !totalWidthCache_ ) {
        TextDocument* doc =textDocument();
        int maxChars = 0;
        for( int line=0,cnt=doc->lineCount(); line<cnt; ++line ) {
            maxChars = qMax( doc->lineLength(line), maxChars );
        }

        QFontMetrics fm = QtextWidget()->fontMetrics();
        qlog_info() << "FontMetrics: " << fm.averageCharWidth();
        qlog_info() << "Maximum width:" << maxChars << " * " << fm.maxWidth() << " :: " << (fm.maxWidth() * maxChars);
        ttoalWidthCache_ = fm.maxWidth() * maxChars;
    }
    return totalWidthCache_;
    */
}


/// This method returns the total height
int TextRenderer::totalHeight()
{
    return textDocument()->lineCount() * lineHeight() + lineHeight();
}


/// This method returns width of the M cahracter
int TextRenderer::emWidth()
{
    return textWidget()->fontMetrics().horizontalAdvance('M');
}


/// The M-width isn't good enough for calculating the width of numbers.
/// Often the M is to wide. That why we have a nr width which takes the 8 for the width
int TextRenderer::nrWidth()
{
    return textWidget()->fontMetrics().horizontalAdvance('8');
}


/// This method returns the number of lines
int TextRenderer::viewHeightInLines()
{
    int result = viewportHeight() / lineHeight() -1;
    return result;
}


/// This method returns the first visible line
int TextRenderer::firstVisibleLine()
{
    return viewportY() / lineHeight();
}


/// This method returns the (closet) valid column for the given x-position
int TextRenderer::columnIndexForXpos(int line, int x )
{
    TextLayout* layout = textLayoutForLine( line );
    if(!layout) return 0;

    //x -= sideBarLeftWidth();

    return layout->xToCursor( x );
}


/// This method returns the x position for the given column
int TextRenderer::xPosForColumn(int line, int column)
{
    TextLayout* layout = textLayoutForLine( line );
    qreal x = 0;// sideBarLeftWidth();
    if(layout) {
        x += layout->cursorToX(column);
    }
    return qRound(x);
}


/// This method returns the x-coordinate for the given offset
int TextRenderer::xPosForOffset(int offset)
{
    int line = textDocument()->lineFromOffset(offset);
    int x = 0;
    if( line >= 0 )
    {
        int lineStartOffset = textDocument()->offsetFromLine(line);
        int col = offset - lineStartOffset;
        x += xPosForColumn( line, col );
    }
    return x;
}


/// This method returns the y position for the given line
int TextRenderer::yPosForLine(int line)
{
    return line * lineHeight();
}


/// This method returns the offset position for the given line
int TextRenderer::yPosForOffset(int offset)
{
    int line = textDocument()->lineFromOffset(offset);
    return yPosForLine(line);
}

/// This method returns the textlayout for the given line
TextLayout *TextRenderer::textLayoutForLine(int line)
{
    Q_ASSERT( line >= 0 );
/// FIXME:  Invalide TextLayout cache when required!!!
    if( controller()->textDocument()->length() == 0){
        return textLayoutForLineForPlaceholder(line);
    } else {
        return textLayoutForLineNormal(line);
    }
}


// Sample from: https://github.com/microsoft/vscode/pull/136347/commits/d2c24cc1d1161193c46ac44364a053c082657d69
static bool isControlCharacter(QChar charCode)
{
    if (charCode < QChar(32)) {
        // TAB
        return (charCode != '\t');
    }
    if (charCode == QChar(127)) {
        // DEL
        return true;
    }

    if (
        (charCode >= QChar(0x202A) && charCode <= QChar(0x202E))
        || (charCode >= QChar(0x2066) && charCode <= QChar(0x2069))
        || (charCode >= QChar(0x200E) && charCode <= QChar(0x200F))
        || charCode == QChar(0x061C)
    ) {
        // Unicode Directional Formatting Characters
        // LRE	U+202A	LEFT-TO-RIGHT EMBEDDING
        // RLE	U+202B	RIGHT-TO-LEFT EMBEDDING
        // PDF	U+202C	POP DIRECTIONAL FORMATTING
        // LRO	U+202D	LEFT-TO-RIGHT OVERRIDE
        // RLO	U+202E	RIGHT-TO-LEFT OVERRIDE
        // LRI	U+2066	LEFT‑TO‑RIGHT ISOLATE
        // RLI	U+2067	RIGHT‑TO‑LEFT ISOLATE
        // FSI	U+2068	FIRST STRONG ISOLATE
        // PDI	U+2069	POP DIRECTIONAL ISOLATE
        // LRM	U+200E	LEFT-TO-RIGHT MARK
        // RLM	U+200F	RIGHT-TO-LEFT MARK
        // ALM	U+061C	ARABIC LETTER MARK
        return true;
    }
    return false;
 }


TextLayout *TextRenderer::textLayoutForLineForPlaceholder(int line)
{
    Q_ASSERT( line >= 0 );
/// FIXME:  Invalide TextLayout cache when required!!!

    TextDocument* doc = textDocument();
    if( line >= doc->lineCount() ) return nullptr;

    TextLayout* textLayout = cachedTextLayoutList_.object(line);
    if( !textLayout ) {
        textLayout = new TextLayout(textDocument());
        textLayout->setCacheEnabled(true);
        int tabWidth = controllerRef_->widget()->fontMetrics().horizontalAdvance('M');

        QTextOption option;
        option.setTabStopDistance(config()->indentSize() * tabWidth);
        if( config()->showWhitespaceMode() == TextEditorConfig::ShowWhitespaces ) {
            option.setFlags( QTextOption::ShowTabsAndSpaces );        /// TODO: Make an option to show spaces and tabs
        }

        textLayout->qTextLayout()->setFont( textWidget()->font() );
        textLayout->qTextLayout()->setTextOption( option );

        // add extra format (no format)
        QTextCharFormat format;

        QColor color = themeStyler()->theme()->foregroundColor();
        color.setAlpha(180);
        format.setForeground(color);


        QString text = doc->lineWithoutNewline(line);

        QVector<QTextLayout::FormatRange> formatRanges;
        QTextLayout::FormatRange formatRange;
        formatRange.start = 0;
        formatRange.length = text.length();
        formatRange.format = format;
        formatRanges.append(formatRange);
        textLayout->setFormats(formatRanges);

        textLayout->setText(text);
        textLayout->buildLayout();

        // update the width cache
        totalWidthCache_ = qMax( totalWidthCache_, qRound(textLayout->boundingRect().width()+0.5));

        // add to the cache
        cachedTextLayoutList_.insert( line, textLayout );
//qlog_info() << "Cache Line: " << line;

    }
    return textLayout;
}


TextLayout *TextRenderer::textLayoutForLineNormal(int line)
{
    Q_ASSERT( line >= 0 );
/// FIXME:  Invalide TextLayout cache when required!!!

    TextDocument* doc = textDocument();
    if( line >= doc->lineCount() ) return nullptr;

    TextLayout* textLayout = cachedTextLayoutList_.object(line);
    if( !textLayout ) {
        textLayout = new TextLayout(textDocument());
        textLayout->setCacheEnabled(true);
        int tabWidth = controllerRef_->widget()->fontMetrics().horizontalAdvance('M');

        QTextOption option;
        option.setTabStopDistance(config()->indentSize() * tabWidth);

        if( config()->showWhitespaceMode() == TextEditorConfig::ShowWhitespaces ) {
            option.setFlags( QTextOption::ShowTabsAndSpaces );        /// TODO: Make an option to show spaces and tabs
        }

        textLayout->qTextLayout()->setFont( textWidget()->font() );
        //qlog_info() << "font: " <<   textWidget()->font().pointSizeF();
        textLayout->qTextLayout()->setTextOption( option );

        // add extra format
        QString text = doc->lineWithoutNewline(line);
        QVector<QTextLayout::FormatRange> formatRanges = themeStyler()->getLineFormatRanges(line);

        TextLayoutBuilder textLayoutBuilder(textLayout, text, formatRanges);

        if( config()->renderBidiContolCharacters() ) {

            QTextCharFormat textFormat;
            textFormat.setBackground(Qt::red); //QBrush(QColor::red()));
            textFormat.setForeground(Qt::white); //QBrush(QColor::red()));

            for( int i=0; i<text.size(); ++i ) {
                QChar c = text.at(i);
                if( isControlCharacter(c) ) {

                  QString str = QString("[U+%1]").arg(QString::number(c.unicode(),16));
                  //QString newString = "⚠️";
                  // text.replace(i, 1, str);

                  /* ORIGINAL first solution
                  // Better replacement for control character: http://unicode.org/charts/PDF/U2400.pdf
                  // This fixes the strange caret behaviour
                  //QString newString(0x2426); // Arabiq question mark
                  QString newString(0x2425); // YMBOL FOR DELETE FORM TWO
                  text.replace(i, 1, newString);

                  QTextLayout::FormatRange formatRange;
                  formatRange.format = textFormat;
                  formatRange.start = i;
                  formatRange.length = newString.length();
                  formatRange.format.setToolTip(str);
                  formatRanges.append(formatRange);
                  */
                  textLayoutBuilder.replace(i, 1, str, textFormat);

                }
            }
        }
        textLayout->setFormats(formatRanges);

#ifdef USE_CONTROL_PICTURES
        for( int i=0; i<text.size(); ++i ) {
            QChar c = text.at(i);
            if( c == 32 ) {
                c = 0x2423;
                text.replace(i,1,c);
            } else if( c == '\t') {
                c = 0x21E5;
                text.replace(i,1,c);
            } else if( c == '\n') {
                c = 0x23CE;
                text.replace(i,1,c);
            }
            if( c < 32 && c != 8 ) {
                c = QChar( 0x2400+c.unicode() );  // add     the Control Pictures range (see: http://unicode.org/charts/PDF/U2400.pdf )
                text.replace(i,1,c);
            }
        }
#endif



        textLayout->setText( text );
        textLayout->buildLayout();

        // update the width cache
        totalWidthCache_ = qMax( totalWidthCache_, qRound(textLayout->boundingRect().width()+0.5));

        // add to the cache
        cachedTextLayoutList_.insert( line, textLayout );

//qlog_info() << "Cache Line: " << line;

    }
    return textLayout;
}


/// This method starts rendering
void TextRenderer::renderBegin( const QRect& rect )
{

//PROF_BEGIN
    TextDocument* doc = textDocument();

    clipRectRef_ = &rect;

    int y = rect.y();

    // the first line to render
    int calculatedEndLine = rawLineIndexForYpos( y + rect.height() ) + 1;   // add 1 line extra (for half visible lines)

    // assign the 'work' variables
    int lineCount = doc->lineCount();
    startLine_   = qBound( 0, rawLineIndexForYpos( y ), lineCount-1 );
    endLine_     = qBound( 0, calculatedEndLine, lineCount-1 );

    Q_ASSERT( startLine_ <= endLine_ );

//    qlog_info() << ">> render startLine" << startLine_ << " t/m endLine=" << endLine_  << "  ==> " << ( endLine_ - startLine_ ) << " y="<<y<<",height="<<rect.height()<<"-------------------" ;
    startOffset_ = doc->offsetFromLine(startLine_);
    endOffset_   = doc->offsetFromLine(endLine_+1);


    // Make sure  the cache-data is filled
//PROF_BEGIN_NAMED("layouts")
    for( int line = startLine_; line <= endLine_; ++line  ) {
        textLayoutForLine( line );  // make sure the cache is filled
    }
//PROF_END

/// TODO: move this lexing stuff to the controller
    // prepare the style
    if( textDocument()->textLexer() ) {
//PROF_BEGIN_NAMED("lexer")
        textDocument()->textLexer()->lexRange( startOffset_, endOffset_ );
//PROF_END
    }

}

/// This method starts rendering
void TextRenderer::renderEnd( const QRect& rect )
{
    Q_UNUSED(rect)
}


/// This method returns the document
TextDocument* TextRenderer::textDocument()
{
    if( controllerRef_->textDocument()->length() == 0 ){
        // return placeholder textdocument
        return placeHolderDocument_;
    }
    return controllerRef_->textDocument();
}

TextDocument *TextRenderer::placeholderTextDocument()
{
    return placeHolderDocument_;
}


/// This method returns the textselection
TextSelection* TextRenderer::textSelection()
{
    return controllerRef_->textSelection();
}


/// Returns the editor configuration
TextEditorConfig* TextRenderer::config()
{
    return textDocument()->config();
}


/// returns a reference to the given controller
TextEditorController*TextRenderer::controller()
{
    return controllerRef_;
}


/// This method returns the widget
TextEditorWidget *TextRenderer::textWidget()
{
    return controllerRef_->widget();
}


/// Sets the current viewport of the renderer
void TextRenderer::setViewport(const QRect& viewport)
{
    viewport_ = viewport;
}


/// sets the caret time on 0
void TextRenderer::resetCaretTime()
{
    if( caretTime_ >= 0 ) {
        caretTime_ = QDateTime::currentMSecsSinceEpoch();
    }
}


/// this method returns true if the caret is visible
bool TextRenderer::shouldRenderCaret()
{
    if( caretTime_ < 0 ) { return false; }
    if( !caretBlinkRate_ ) { return true; }
    qint64 time = QDateTime::currentMSecsSinceEpoch() - caretTime_;
    return (time % caretBlinkRate_) < (caretBlinkRate_>> 1);
}


/// Returns true if the caret is visible
bool TextRenderer::isCaretVisible()
{
    return caretTime_ >= 0;
}


/// sets the carets to visible or invisible
void TextRenderer::setCaretVisible(bool visible)
{
    caretTime_ = visible ? QDateTime::currentMSecsSinceEpoch() : -1;
}


/// returns the current theme name
QString TextRenderer::themeName() const
{
    return textThemeStyler_->themeName();
}


/// Returns the active theme
TextTheme* TextRenderer::theme()
{
    return textThemeStyler_->theme();
}


/// Selects the active theme name
void TextRenderer::setThemeByName(const QString& name)
{
    textThemeStyler_->setThemeByName(name);
    invalidateCaches();
    emit themeChanged(textThemeStyler_->theme());
}


/// sets the theme
/// @param theme the them to set
void TextRenderer::setTheme(TextTheme* theme)
{
    textThemeStyler_->setTheme( theme );
    invalidateCaches();
    emit themeChanged(theme);
}


/// The text-document has been changed
void TextRenderer::textDocumentChanged(edbee::TextDocument *oldDocument, edbee::TextDocument *newDocument)
{
    // disconnect an old document (if required)
    if( oldDocument ) {
        disconnect(oldDocument, nullptr, this, nullptr);
    }
    reset();

    // connect with the new dpcument
    connect( newDocument, SIGNAL(textChanged(edbee::TextBufferChange, QString)), this, SLOT(textChanged(edbee::TextBufferChange, QString)));
    connect( newDocument, SIGNAL(lastScopedOffsetChanged(int,int)), this, SLOT(lastScopedOffsetChanged(int,int)) );
}


/// The text is replaced
void TextRenderer::textChanged(edbee::TextBufferChange change, QString oldText)
{
    Q_UNUSED(oldText)
    int lineCount = qMax( change.lineCount(), change.newLineCount()) + 1;

    // Unfortunately when the line-count is changed we need to invalidate all
    // because we cannot move the cached layouts easy in the layout list
    if( change.lineCount() - change.newLineCount() ) {
        cachedTextLayoutList_.clear();
    } else {

        // invalidate all if there are to many lines
        if( lineCount > cachedTextLayoutList_.size()/2 ) {
            cachedTextLayoutList_.clear();
        } else {
            for( int i=0,cnt=lineCount; i<cnt; ++i ) {
                cachedTextLayoutList_.remove( change.line() + i );
            }
        }
    }
}


/// The scoped to offset has been changed
void TextRenderer::lastScopedOffsetChanged(int previousOffset, int newOffset)
{
//qlog_info() << "** lastScopedOffsetChanged("<<previousOffset<<","<<newOffset<<") **";
    Q_UNUSED(newOffset)
    int lastValidLine = textDocument()->lineFromOffset(previousOffset);
    invalidateTextLayoutCaches( lastValidLine );
        //    textWidget()->fullUpdate();
}


/// Invalidates the QTextLayout caches
void TextRenderer::invalidateTextLayoutCaches(int fromLine)
{
//qlog_info() << "** invalidateTextLayoutCache("<<fromLine<<") **";
    if( fromLine ) {
        QList<int> keys = cachedTextLayoutList_.keys();
        foreach( int key, keys ) {
            if( key >= fromLine) {
                cachedTextLayoutList_.remove(key);
            }
        }
    } else {
        cachedTextLayoutList_.clear();
    }
}


/// call this method to invalidate all caches!
void TextRenderer::invalidateCaches()
{
//qlog_info() << "** invalidateCaches() **";
    totalWidthCache_ = 0;
    cachedTextLayoutList_.clear();
}


} // edbee
