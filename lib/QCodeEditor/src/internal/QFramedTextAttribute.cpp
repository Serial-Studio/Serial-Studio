// QCodeEditor
#include <QFramedTextAttribute>
#include <QSyntaxStyle>

// Qt
#include <QFontMetrics>
#include <QPainter>
#include <QDebug>
#include <QTextBlock>

int QFramedTextAttribute::type()
{
  return QTextFormat::UserFormat + 1;
}

QFramedTextAttribute::QFramedTextAttribute(QObject *parent)
  : QObject(parent)
  , m_style(nullptr)
{
}

void QFramedTextAttribute::setSyntaxStyle(QSyntaxStyle *style)
{
  m_style = style;
}

QSyntaxStyle *QFramedTextAttribute::syntaxStyle() const
{
  return m_style;
}

QSizeF QFramedTextAttribute::intrinsicSize(QTextDocument *, int,
                                           const QTextFormat &)
{
  return {0, 0};
}

void QFramedTextAttribute::drawObject(QPainter *painter, const QRectF &rect,
                                      QTextDocument *, int,
                                      const QTextFormat &format)
{
  // Casting
  auto textCharFormat = reinterpret_cast<const QTextCharFormat &>(format);

  // Getting font data
  auto font = textCharFormat.font();
  QFontMetrics metrics(font);

  // Getting required size
  auto string = format.property(FramedString).toString();
  auto stringSize = metrics.boundingRect(string).size();

  // Creating frame rect
  QRectF drawRect(rect.topLeft(), stringSize);
  drawRect.moveTop(rect.top() - stringSize.height());
  drawRect.adjust(0, 4, 0, 4);

  // Drawing
  painter->setPen(m_style->getFormat("Occurrences").background().color());
  painter->setRenderHint(QPainter::Antialiasing);
  painter->drawRoundedRect(drawRect, 4, 4);
}

void QFramedTextAttribute::frame(QTextCursor cursor)
{
  auto text = cursor.document()->findBlockByNumber(cursor.blockNumber()).text();

  QTextCharFormat format;
  format.setObjectType(type());
  format.setProperty(FramedString, cursor.selectedText());

  if (cursor.selectionEnd() > cursor.selectionStart())
  {
    cursor.setPosition(cursor.selectionStart());
  }
  else
  {
    cursor.setPosition(cursor.selectionEnd());
  }

  cursor.insertText(QString(QChar::ObjectReplacementCharacter), format);
}

void QFramedTextAttribute::clear(QTextCursor cursor)
{
  auto doc = cursor.document();

  for (auto blockIndex = 0; blockIndex < doc->blockCount(); ++blockIndex)
  {
    auto block = doc->findBlockByNumber(blockIndex);

    auto formats = block.textFormats();
    int offset = 0;

    for (auto &format : formats)
    {
      if (format.format.objectType() == type())
      {
        cursor.setPosition(block.position() + format.start - offset);
        cursor.deleteChar();
        ++offset;
      }
    }
  }
}
