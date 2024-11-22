#pragma once

// QCodeEditor
#include <QStyleSyntaxHighlighter> // Required for inheritance
#include <QHighlightRule>

// Qt
#include <QRegularExpression>
#include <QVector>

class QSyntaxStyle;

/**
 * @brief Class, that describes Javascript code highlighter.
 */
class QJavascriptHighlighter : public QStyleSyntaxHighlighter
{
  Q_OBJECT
public:
  /**
   * @brief Constructor.
   * @param document Pointer to document.
   */
  explicit QJavascriptHighlighter(QTextDocument *document = nullptr);

protected:
  void highlightBlock(const QString &text) override;

private:
  QVector<QHighlightRule> m_highlightRules;

  QRegularExpression m_includePattern;
  QRegularExpression m_functionPattern;
  QRegularExpression m_varPattern;

  QRegularExpression m_commentStartPattern;
  QRegularExpression m_commentEndPattern;
};
