// QCodeEditor
#include <QCXXHighlighter>
#include <QSyntaxStyle>
#include <QLanguage>

// Qt
#include <QFile>

QCXXHighlighter::QCXXHighlighter(QTextDocument *document)
  : QStyleSyntaxHighlighter(document)
  , m_highlightRules()
  , m_includePattern(
        QRegularExpression(R"(^\s*#\s*include\s*([<"][^:?"<>\|]+[">]))"))
  , m_functionPattern(QRegularExpression(
        R"(\b([_a-zA-Z][_a-zA-Z0-9]*\s+)?((?:[_a-zA-Z][_a-zA-Z0-9]*\s*::\s*)*[_a-zA-Z][_a-zA-Z0-9]*)(?=\s*\())"))
  , m_defTypePattern(QRegularExpression(
        R"(\b([_a-zA-Z][_a-zA-Z0-9]*)\s+[_a-zA-Z][_a-zA-Z0-9]*\s*[;=])"))
  , m_commentStartPattern(QRegularExpression(R"(/\*)"))
  , m_commentEndPattern(QRegularExpression(R"(\*/)"))
{
  Q_INIT_RESOURCE(qcodeeditor_resources);
  QFile fl(":/languages/cpp.xml");

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

  // Numbers
  m_highlightRules.append(
      {QRegularExpression(
           R"((?<=\b|\s|^)(?i)(?:(?:(?:(?:(?:\d+(?:'\d+)*)?\.(?:\d+(?:'\d+)*)(?:e[+-]?(?:\d+(?:'\d+)*))?)|(?:(?:\d+(?:'\d+)*)\.(?:e[+-]?(?:\d+(?:'\d+)*))?)|(?:(?:\d+(?:'\d+)*)(?:e[+-]?(?:\d+(?:'\d+)*)))|(?:0x(?:[0-9a-f]+(?:'[0-9a-f]+)*)?\.(?:[0-9a-f]+(?:'[0-9a-f]+)*)(?:p[+-]?(?:\d+(?:'\d+)*)))|(?:0x(?:[0-9a-f]+(?:'[0-9a-f]+)*)\.?(?:p[+-]?(?:\d+(?:'\d+)*))))[lf]?)|(?:(?:(?:[1-9]\d*(?:'\d+)*)|(?:0[0-7]*(?:'[0-7]+)*)|(?:0x[0-9a-f]+(?:'[0-9a-f]+)*)|(?:0b[01]+(?:'[01]+)*))(?:u?l{0,2}|l{0,2}u?)))(?=\b|\s|$))"),
       "Number"});

  // Strings
  m_highlightRules.append({QRegularExpression(R"("[^\n"]*")"), "String"});

  // Define
  m_highlightRules.append(
      {QRegularExpression(R"(#[a-zA-Z_]+)"), "Preprocessor"});

  // Single line
  m_highlightRules.append({QRegularExpression(R"(//[^\n]*)"), "Comment"});
}

void QCXXHighlighter::highlightBlock(const QString &text)
{
  // Checking for include
  {
    auto matchIterator = m_includePattern.globalMatch(text);

    while (matchIterator.hasNext())
    {
      auto match = matchIterator.next();

      setFormat(match.capturedStart(), match.capturedLength(),
                syntaxStyle()->getFormat("Preprocessor"));

      setFormat(match.capturedStart(1), match.capturedLength(1),
                syntaxStyle()->getFormat("String"));
    }
  }
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
  {
    auto matchIterator = m_defTypePattern.globalMatch(text);

    while (matchIterator.hasNext())
    {
      auto match = matchIterator.next();

      setFormat(match.capturedStart(1), match.capturedLength(1),
                syntaxStyle()->getFormat("Type"));
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
  if (previousBlockState() != 1)
  {
    startIndex = text.indexOf(m_commentStartPattern);
  }

  while (startIndex >= 0)
  {
    auto match = m_commentEndPattern.match(text, startIndex);

    int endIndex = match.capturedStart();
    int commentLength = 0;

    if (endIndex == -1)
    {
      setCurrentBlockState(1);
      commentLength = text.length() - startIndex;
    }
    else
    {
      commentLength = endIndex - startIndex + match.capturedLength();
    }

    setFormat(startIndex, commentLength, syntaxStyle()->getFormat("Comment"));
    startIndex
        = text.indexOf(m_commentStartPattern, startIndex + commentLength);
  }
}
