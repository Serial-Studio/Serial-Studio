// QCodeEditor
#include <QPythonHighlighter>
#include <QLanguage>
#include <QSyntaxStyle>

// Qt
#include <QFile>
#include <QDebug>

QPythonHighlighter::QPythonHighlighter(QTextDocument *document)
  : QStyleSyntaxHighlighter(document)
  , m_highlightRules()
  , m_highlightBlockRules()
  , m_includePattern(QRegularExpression(R"(import \w+)"))
  , m_functionPattern(
        QRegularExpression(R"(\b([A-Za-z0-9_]+(?:\.))*([A-Za-z0-9_]+)(?=\())"))
  , m_defTypePattern(QRegularExpression(
        R"(\b([A-Za-z0-9_]+)\s+[A-Za-z]{1}[A-Za-z0-9_]+\s*[;=])"))
{
  Q_INIT_RESOURCE(qcodeeditor_resources);
  QFile fl(":/languages/python.xml");

  if (!fl.open(QIODevice::ReadOnly))
  {
    return;
  }

  QLanguage language(&fl);

  if (!language.isLoaded())
  {
    return;
  }

  auto keys = language.keys();
  for (auto &&key : keys)
  {
    auto names = language.names(key);
    for (auto &&name : names)
    {
      m_highlightRules.append(
          {QRegularExpression(QString(R"(\b%1\b)").arg(name)), key});
    }
  }

  // Following rules has higher priority to display
  // than language specific keys
  // So they must be applied at last.
  // Numbers
  m_highlightRules.append(
      {QRegularExpression(R"(\b(0b|0x){0,1}[\d.']+\b)"), "Number"});

  // Strings
  m_highlightRules.append({QRegularExpression(R"("[^\n"]*")"), "String"});
  m_highlightRules.append({QRegularExpression(R"('[^\n"]*')"), "String"});

  // Single line comment
  m_highlightRules.append({QRegularExpression("#[^\n]*"), "Comment"});

  // Multiline string
  m_highlightBlockRules.append(
      {QRegularExpression("(''')"), QRegularExpression("(''')"), "String"});
  m_highlightBlockRules.append({QRegularExpression("(\"\"\")"),
                                QRegularExpression("(\"\"\")"), "String"});
}

void QPythonHighlighter::highlightBlock(const QString &text)
{
  // Checking for function
  {
    auto matchIterator = m_functionPattern.globalMatch(text);

    while (matchIterator.hasNext())
    {
      auto match = matchIterator.next();

      setFormat(match.capturedStart(), match.capturedLength(),
                syntaxStyle()->getFormat("Type"));

      setFormat(match.capturedStart(2), match.capturedLength(2),
                syntaxStyle()->getFormat("Function"));
    }
  }

  for (auto &rule : m_highlightRules)
  {
    auto matchIterator = rule.pattern.globalMatch(text);

    while (matchIterator.hasNext())
    {
      auto match = matchIterator.next();

      setFormat(match.capturedStart(), match.capturedLength(),
                syntaxStyle()->getFormat(rule.formatName));
    }
  }

  setCurrentBlockState(0);
  int startIndex = 0;
  int highlightRuleId = previousBlockState();
  if (highlightRuleId < 1 || highlightRuleId > m_highlightBlockRules.size())
  {
    for (int i = 0; i < m_highlightBlockRules.size(); ++i)
    {
      startIndex = text.indexOf(m_highlightBlockRules.at(i).startPattern);

      if (startIndex >= 0)
      {
        highlightRuleId = i + 1;
        break;
      }
    }
  }

  while (startIndex >= 0)
  {
    const auto &blockRules = m_highlightBlockRules.at(highlightRuleId - 1);
    auto match = blockRules.endPattern.match(
        text, startIndex + 1); // Should be + length of start pattern

    int endIndex = match.capturedStart();
    int matchLength = 0;

    if (endIndex == -1)
    {
      setCurrentBlockState(highlightRuleId);
      matchLength = text.length() - startIndex;
    }
    else
    {
      matchLength = endIndex - startIndex + match.capturedLength();
    }

    setFormat(startIndex, matchLength,
              syntaxStyle()->getFormat(blockRules.formatName));
    startIndex
        = text.indexOf(blockRules.startPattern, startIndex + matchLength);
  }
}
