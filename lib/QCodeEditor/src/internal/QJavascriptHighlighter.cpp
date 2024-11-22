// QCodeEditor
#include <QJavascriptHighlighter>
#include <QSyntaxStyle>
#include <QLanguage>

// Qt
#include <QFile>

QJavascriptHighlighter::QJavascriptHighlighter(QTextDocument *document)
  : QStyleSyntaxHighlighter(document)
  , m_highlightRules()
  , m_functionPattern(QRegularExpression(
        R"(\bfunction\b\s+([_a-zA-Z][_a-zA-Z0-9]*)\s*(?=\())"))
  , m_varPattern(
        QRegularExpression(R"(\b(var|let|const)\b\s+([_a-zA-Z][_a-zA-Z0-9]*))"))
  , m_commentStartPattern(QRegularExpression(R"(/\*)"))
  , m_commentEndPattern(QRegularExpression(R"(\*/)"))
{
  Q_INIT_RESOURCE(qcodeeditor_resources);
  QFile fl(":/languages/javascript.xml");

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
      {QRegularExpression(R"(\b\d+(\.\d+)?([eE][+-]?\d+)?\b)"), "Number"});

  // Strings (single and double quotes)
  m_highlightRules.append(
      {QRegularExpression(R"("(?:\\.|[^"\\])*"|'(?:\\.|[^'\\])*')"), "String"});

  // Template Literals
  m_highlightRules.append(
      {QRegularExpression(R"(`(?:\\.|[^`\\])*`)"), "TemplateString"});

  // Keywords
  m_highlightRules.append(
      {QRegularExpression(
           R"(\b(?:break|case|catch|class|const|continue|debugger|default|delete|do|else|enum|export|extends|false|finally|for|function|if|import|in|instanceof|let|new|null|return|super|switch|this|throw|true|try|typeof|var|void|while|with|yield)\b)"),
       "Keyword"});

  // Operators
  m_highlightRules.append(
      {QRegularExpression(R"([+\-*/%=<>&|^!?~:]+)"), "Operator"});

  // Single-line Comments
  m_highlightRules.append({QRegularExpression(R"(//[^\n]*)"), "Comment"});

  // Multi-line Comments
  m_highlightRules.append({QRegularExpression(R"(/\*.*?\*/)"), "Comment"});
}

void QJavascriptHighlighter::highlightBlock(const QString &text)
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
    auto matchIterator = m_varPattern.globalMatch(text);

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
