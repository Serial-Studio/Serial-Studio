// QCodeEditor
#include <QLuaHighlighter>
#include <QSyntaxStyle>
#include <QLanguage>

// Qt
#include <QFile>

namespace
{

// Lua long brackets: [[ ]] / [=[ ]=] / [==[ ]==] ... The block state encodes
// the construct type and the bracket level so a multiline comment or string
// continues across blocks until the close token of the SAME level appears.
constexpr int kStateNone = 0;

int makeBlockState(bool comment, int level)
{
  return 1 + level * 2 + (comment ? 0 : 1);
}

bool stateIsComment(int state)
{
  return state > kStateNone && ((state - 1) % 2) == 0;
}

int stateLevel(int state)
{
  return (state - 1) / 2;
}

bool tryLongBracketOpen(const QString &text, int pos, int &level,
                        int &consumed)
{
  if (pos >= text.length() || text.at(pos) != QLatin1Char('['))
  {
    return false;
  }

  int i = pos + 1;
  while (i < text.length() && text.at(i) == QLatin1Char('='))
  {
    ++i;
  }

  if (i >= text.length() || text.at(i) != QLatin1Char('['))
  {
    return false;
  }

  level = i - pos - 1;
  consumed = i - pos + 1;
  return true;
}

int findLongBracketClose(const QString &text, int from, int level,
                         int &closeEnd)
{
  const QString token = QStringLiteral("]") + QString(level, QLatin1Char('='))
                        + QStringLiteral("]");

  const int idx = text.indexOf(token, from);
  if (idx >= 0)
  {
    closeEnd = idx + token.length();
  }

  return idx;
}

int skipQuotedString(const QString &text, int pos)
{
  const QChar quote = text.at(pos);

  int i = pos + 1;
  while (i < text.length())
  {
    const QChar c = text.at(i);
    if (c == QLatin1Char('\\'))
    {
      i += 2;
      continue;
    }

    if (c == quote)
    {
      return i + 1;
    }

    ++i;
  }

  return text.length();
}

} // namespace

QLuaHighlighter::QLuaHighlighter(QTextDocument *document)
  : QStyleSyntaxHighlighter(document)
  , m_highlightRules()
  , m_requirePattern(
        QRegularExpression(R"(require\s*([("'][a-zA-Z0-9*._]+['")]))"))
  , m_functionPattern(QRegularExpression(
        R"(\b([A-Za-z0-9_]+(?:\s+|::))*([A-Za-z0-9_]+)(?=\())"))
  , m_defTypePattern(QRegularExpression(
        R"(\b([A-Za-z0-9_]+)\s+[A-Za-z]{1}[A-Za-z0-9_]+\s*[=])"))
{
  Q_INIT_RESOURCE(qcodeeditor_resources);
  QFile fl(":/languages/lua.xml");

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
          {QRegularExpression(QString(R"(\b\s{0,1}%1\s{0,1}\b)").arg(name)),
           key});
    }
  }

  // Numbers
  m_highlightRules.append(
      {QRegularExpression(R"(\b(0b|0x){0,1}[\d.']+\b)"), "Number"});

  // Strings
  m_highlightRules.append({QRegularExpression(R"(["'][^\n"]*["'])"), "String"});

  // Preprocessor
  m_highlightRules.append(
      {QRegularExpression(R"(#\![a-zA-Z_]+)"), "Preprocessor"});

  // Comments (line and block) and long strings are handled by the
  // long-bracket scanner in highlightBlock(), not by regex rules.
}

void QLuaHighlighter::highlightBlock(const QString &text)
{
  { // Checking for require
    auto matchIterator = m_requirePattern.globalMatch(text);

    while (matchIterator.hasNext())
    {
      auto match = matchIterator.next();

      setFormat(match.capturedStart(), match.capturedLength(),
                syntaxStyle()->getFormat("Preprocessor"));

      setFormat(match.capturedStart(1), match.capturedLength(1),
                syntaxStyle()->getFormat("String"));
    }
  }
  { // Checking for function
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
  { // checking for type
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

  setCurrentBlockState(kStateNone);

  int pos = 0;
  const int prevState = previousBlockState();
  if (prevState > kStateNone)
  {
    const char *format = stateIsComment(prevState) ? "Comment" : "String";

    int closeEnd = 0;
    if (findLongBracketClose(text, 0, stateLevel(prevState), closeEnd) < 0)
    {
      setCurrentBlockState(prevState);
      setFormat(0, text.length(), syntaxStyle()->getFormat(format));
      return;
    }

    setFormat(0, closeEnd, syntaxStyle()->getFormat(format));
    pos = closeEnd;
  }

  while (pos < text.length())
  {
    const QChar c = text.at(pos);

    if (c == QLatin1Char('"') || c == QLatin1Char('\''))
    {
      pos = skipQuotedString(text, pos);
      continue;
    }

    int level = 0;
    int consumed = 0;

    if (c == QLatin1Char('-') && pos + 1 < text.length()
        && text.at(pos + 1) == QLatin1Char('-'))
    {
      if (tryLongBracketOpen(text, pos + 2, level, consumed))
      {
        int closeEnd = 0;
        if (findLongBracketClose(text, pos + 2 + consumed, level, closeEnd)
            < 0)
        {
          setCurrentBlockState(makeBlockState(true, level));
          setFormat(pos, text.length() - pos,
                    syntaxStyle()->getFormat("Comment"));
          return;
        }

        setFormat(pos, closeEnd - pos, syntaxStyle()->getFormat("Comment"));
        pos = closeEnd;
        continue;
      }

      setFormat(pos, text.length() - pos, syntaxStyle()->getFormat("Comment"));
      return;
    }

    if (tryLongBracketOpen(text, pos, level, consumed))
    {
      int closeEnd = 0;
      if (findLongBracketClose(text, pos + consumed, level, closeEnd) < 0)
      {
        setCurrentBlockState(makeBlockState(false, level));
        setFormat(pos, text.length() - pos, syntaxStyle()->getFormat("String"));
        return;
      }

      setFormat(pos, closeEnd - pos, syntaxStyle()->getFormat("String"));
      pos = closeEnd;
      continue;
    }

    ++pos;
  }
}
