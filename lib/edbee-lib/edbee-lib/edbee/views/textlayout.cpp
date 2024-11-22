#include "textlayout.h"

#include <QTextLayout>

#include "edbee/debug.h"

namespace edbee {


//=================================================

TextLayout::TextLayout(TextDocument* document)
    : qtextLayout_( new QTextLayout())
    , textDocumentRef_(document)
    , singleCharRanges_(nullptr)
{
}

TextLayout::~TextLayout()
{
    delete singleCharRanges_;
    delete qtextLayout_;
}

void TextLayout::setCacheEnabled(bool enable)
{
    qtextLayout_->setCacheEnabled(enable);
}

QTextLayout *TextLayout::qTextLayout() const
{
    return qtextLayout_;
}

QRectF TextLayout::boundingRect() const
{
    return qtextLayout_->boundingRect();
}

void TextLayout::buildLayout()
{
    qtextLayout_->beginLayout();
    qtextLine_ = qtextLayout_->createLine();
    qtextLayout_->endLayout();

}

/// Converts the document cursorPosition to a virtual cursorposition
int TextLayout::toVirtualCursorPosition(int cursorPos) const
{
    int delta = 0;
    // convert cursor to a valid location
    TextRangeSet* ranges = singleCharRanges();
    if(ranges) {
        for(int i=0, cnt = ranges->rangeCount(); i < cnt; i++ ) {
            TextRange& range = ranges->range(i);
            if( cursorPos + delta > range.min()) {
                delta += range.length() - 1;
            }
        }
    }
    return cursorPos + delta;
}

/// Converts the virtual cursorPosition to a docuemnt cursorposition
int TextLayout::fromVirtualCursorPosition(int cursor) const
{
    // when the cursor falls in a single-character range.
    // Set the cursor to the start of this range
    int delta = 0;

    TextRangeSet* ranges = singleCharRanges();
    if(ranges) {
        for(int i=0, cnt = ranges->rangeCount(); i < cnt; i++ ) {
            TextRange& range = ranges->range(i);
            if(range.min() <= cursor && cursor < range.max() ) {
               delta += cursor - range.min();
            } else if( range.max() <= cursor ) {
               delta += range.length() - 1;
            }
        }
    }
    return cursor - delta;
}

void TextLayout::draw(QPainter *p, const QPointF &pos, const QVector<QTextLayout::FormatRange> &selections, const QRectF &clip) const
{
    qtextLayout_->draw(p, pos, selections, clip);
}

void TextLayout::drawCursor(QPainter *painter, const QPointF &position, int cursorPosition, int width) const
{
    int virtualCursorPosition = toVirtualCursorPosition(cursorPosition);
    qtextLayout_->drawCursor(painter, position, virtualCursorPosition, width);
}

void TextLayout::setFormats(const QVector<QTextLayout::FormatRange> &formats)
{
    qtextLayout_->setFormats(formats);
}

void TextLayout::setText(const QString &string)
{
    qtextLayout_->setText(string);
}

void TextLayout::useSingleCharRanges()
{
    if(!singleCharRanges_) {
        singleCharRanges_ = new TextRangeSet(textDocumentRef_);
    }
}

TextRangeSet *TextLayout::singleCharRanges() const
{
    return singleCharRanges_;
}

void TextLayout::addSingleCharRange(int index, int length)
{
    useSingleCharRanges();
    singleCharRanges()->addRange(index, index+length);
}



qreal TextLayout::cursorToX(int cursorPos, QTextLine::Edge edge) const
{
    int virtualCursorPos = toVirtualCursorPosition(cursorPos);

    qreal x =  qtextLine_.cursorToX(virtualCursorPos, edge);
    return x;
}

int TextLayout::xToCursor(qreal x, QTextLine::CursorPosition cpos) const
{
    int virtualCursor = qtextLine_.xToCursor(x, cpos);
    return fromVirtualCursorPosition(virtualCursor);
}


//=================================================

TextLayoutBuilder::TextLayoutBuilder(TextLayout *textLayout, QString & baseString, QVector<QTextLayout::FormatRange> & baseFormatRanges)
    : textLayoutRef_(textLayout)
    , baseString_(baseString)
    , baseFormatRanges_(baseFormatRanges)
{

}

void TextLayoutBuilder::replace(int index, int length, const QString replacement, QTextCharFormat format)
{
    baseString_.replace(index, length, replacement);
    textLayoutRef_->addSingleCharRange(index, replacement.length());  /// TODO: Should we store the original length!?!?!?

    // change existing format ranges:
    int delta = replacement.length() - length;
    if( delta != 0 ) {
        for(int i=0, cnt = baseFormatRanges_.length(); i < cnt; i++ ) {
            QTextLayout::FormatRange& formatRange = baseFormatRanges_[i];
            if( formatRange.start >= index ) {
                formatRange.start += delta;
            }
        }
    }


    // append the text format
    QTextLayout::FormatRange formatRange;
    formatRange.format = format;
    formatRange.start = index;
    formatRange.length = replacement.length();
    baseFormatRanges_.append(formatRange);
}

} // edbee


