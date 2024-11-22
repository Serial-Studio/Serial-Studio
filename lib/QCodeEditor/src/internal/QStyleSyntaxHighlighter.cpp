// QCodeEditor
#include <QStyleSyntaxHighlighter>

QStyleSyntaxHighlighter::QStyleSyntaxHighlighter(QTextDocument *document)
  : QSyntaxHighlighter(document)
  , m_syntaxStyle(nullptr)
{
}

void QStyleSyntaxHighlighter::setSyntaxStyle(QSyntaxStyle *style)
{
  m_syntaxStyle = style;
}

QSyntaxStyle *QStyleSyntaxHighlighter::syntaxStyle() const
{
  return m_syntaxStyle;
}
