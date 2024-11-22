#pragma once

// QCodeEditor
#include <QStyleSyntaxHighlighter> // Required for inheritance
#include <QHighlightRule>

// Qt
#include <QRegularExpression>
#include <QVector>

class QSyntaxStyle;

/**
 * @brief Class, that describes Glsl code
 * highlighter.
 */
class QGLSLHighlighter : public QStyleSyntaxHighlighter
{
  Q_OBJECT
public:
  /**
   * @brief Constructor.
   * @param document Pointer to document.
   */
  explicit QGLSLHighlighter(QTextDocument *document = nullptr);

protected:
  void highlightBlock(const QString &text) override;

private:
  QVector<QHighlightRule> m_highlightRules;

  QRegularExpression m_includePattern;
  QRegularExpression m_functionPattern;
  QRegularExpression m_defTypePattern;

  QRegularExpression m_commentStartPattern;
  QRegularExpression m_commentEndPattern;
};
