// QCodeEditor
#include <QXMLHighlighter>
#include <QSyntaxStyle>

QXMLHighlighter::QXMLHighlighter(QTextDocument *document)
  : QStyleSyntaxHighlighter(document)
  , m_xmlKeywordRegexes()
  , m_xmlElementRegex(R"(<[\s]*[/]?[\s]*([^\n][a-zA-Z-_:]*)(?=[\s/>]))")
  , m_xmlAttributeRegex(R"(\w+(?=\=))")
  , m_xmlValueRegex(R"("[^\n"]+"(?=\??[\s/>]))")
  , m_xmlCommentBeginRegex(R"(<!--)")
  , m_xmlCommentEndRegex(R"(-->)")
{
  m_xmlKeywordRegexes << QRegularExpression("<\\?") << QRegularExpression("/>")
                      << QRegularExpression(">") << QRegularExpression("<")
                      << QRegularExpression("</") << QRegularExpression("\\?>");
}

void QXMLHighlighter::highlightBlock(const QString &text)
{
  // Special treatment for xml element regex as we use captured text to emulate
  // lookbehind
  auto matchIterator = m_xmlElementRegex.globalMatch(text);
  while (matchIterator.hasNext())
  {
    auto match = matchIterator.next();

    setFormat(match.capturedStart(), match.capturedLength(),
              syntaxStyle()->getFormat("Keyword") // XML ELEMENT FORMAT
    );
  }

  // Highlight xml keywords *after* xml elements to fix any occasional /
  // captured into the enclosing element

  for (auto &&regex : m_xmlKeywordRegexes)
  {
    highlightByRegex(syntaxStyle()->getFormat("Keyword"), regex, text);
  }

  highlightByRegex(syntaxStyle()->getFormat("Text"), m_xmlAttributeRegex, text);

  setCurrentBlockState(0);

  int startIndex = 0;
  if (previousBlockState() != 1)
  {
    startIndex = text.indexOf(m_xmlCommentBeginRegex);
  }

  while (startIndex >= 0)
  {
    auto match = m_xmlCommentEndRegex.match(text, startIndex);

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
        = text.indexOf(m_xmlCommentBeginRegex, startIndex + commentLength);
  }

  highlightByRegex(syntaxStyle()->getFormat("String"), m_xmlValueRegex, text);
}

void QXMLHighlighter::highlightByRegex(const QTextCharFormat &format,
                                       const QRegularExpression &regex,
                                       const QString &text)
{
  auto matchIterator = regex.globalMatch(text);

  while (matchIterator.hasNext())
  {
    auto match = matchIterator.next();

    setFormat(match.capturedStart(), match.capturedLength(), format);
  }
}
