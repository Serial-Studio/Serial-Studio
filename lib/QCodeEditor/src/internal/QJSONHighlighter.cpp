// QCodeEditor
#include <QJSONHighlighter>
#include <QSyntaxStyle>

QJSONHighlighter::QJSONHighlighter(QTextDocument *document)
  : QStyleSyntaxHighlighter(document)
  , m_highlightRules()
  , m_keyRegex(R"(("[^\r\n:]+?")\s*:)")
{
  auto keywords = QStringList() << "null" << "true" << "false";

  for (auto &&keyword : keywords)
  {
    m_highlightRules.append(
        {QRegularExpression(QString(R"(\b%1\b)").arg(keyword)), "Keyword"});
  }

  // Numbers
  m_highlightRules.append(
      {QRegularExpression(R"(\b(0b|0x){0,1}[\d.']+\b)"), "Number"});

  // Strings
  m_highlightRules.append({QRegularExpression(R"("[^\n"]*")"), "String"});
}

void QJSONHighlighter::highlightBlock(const QString &text)
{
  for (auto &&rule : m_highlightRules)
  {
    auto matchIterator = rule.pattern.globalMatch(text);

    while (matchIterator.hasNext())
    {
      auto match = matchIterator.next();

      setFormat(match.capturedStart(), match.capturedLength(),
                syntaxStyle()->getFormat(rule.formatName));
    }
  }

  // Special treatment for key regex
  auto matchIterator = m_keyRegex.globalMatch(text);

  while (matchIterator.hasNext())
  {
    auto match = matchIterator.next();

    setFormat(match.capturedStart(1), match.capturedLength(1),
              syntaxStyle()->getFormat("Keyword"));
  }
}
