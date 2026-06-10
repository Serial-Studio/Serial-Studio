// QCodeEditor
#include <QLineNumberArea>
#include <QSyntaxStyle>
#include <QCodeEditor>
#include <QStyleSyntaxHighlighter>
#include <QFramedTextAttribute>
#include <QCXXHighlighter>

// Qt
#include <QTextBlock>
#include <QPaintEvent>
#include <QFontDatabase>
#include <QScrollBar>
#include <QAbstractTextDocumentLayout>
#include <QTextCharFormat>
#include <QCursor>
#include <QCompleter>
#include <QAbstractItemView>
#include <QRegularExpression>
#include <QShortcut>
#include <QMimeData>
#include <QEvent>
#include <QTextOption>

static QVector<QPair<QString, QString>> parentheses
    = {{"(", ")"}, {"{", "}"}, {"[", "]"}, {"\"", "\""}, {"'", "'"}};

QCodeEditor::QCodeEditor(QWidget *widget)
  : QTextEdit(widget)
  , m_highlighter(nullptr)
  , m_syntaxStyle(nullptr)
  , m_lineNumberArea(new QLineNumberArea(this))
  , m_completer(nullptr)
  , m_framedAttribute(new QFramedTextAttribute(this))
  , m_autoIndentation(true)
  , m_autoParentheses(true)
  , m_replaceTab(true)
  , m_tabReplace(QString(4, ' '))
  , m_languageHint(LanguageHint::Generic)
{
  initDocumentLayoutHandlers();
  initFont();
  performConnections();
  enforceLeftToRight();

  setSyntaxStyle(QSyntaxStyle::defaultStyle());
}

void QCodeEditor::enforceLeftToRight()
{
  setLayoutDirection(Qt::LeftToRight);
  if (auto *doc = document())
  {
    QTextOption opt = doc->defaultTextOption();
    if (opt.textDirection() != Qt::LeftToRight)
    {
      opt.setTextDirection(Qt::LeftToRight);
      doc->setDefaultTextOption(opt);
    }
  }
}

void QCodeEditor::changeEvent(QEvent *e)
{
  QTextEdit::changeEvent(e);
  if (e && e->type() == QEvent::LayoutDirectionChange)
    enforceLeftToRight();
}

void QCodeEditor::initDocumentLayoutHandlers()
{
  document()->documentLayout()->registerHandler(QFramedTextAttribute::type(),
                                                m_framedAttribute);
}

void QCodeEditor::initFont()
{
  auto fnt = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  fnt.setFixedPitch(true);
  fnt.setPointSize(10);

  setFont(fnt);
}

void QCodeEditor::performConnections()
{
  connect(document(), &QTextDocument::blockCountChanged, this,
          &QCodeEditor::updateLineNumberAreaWidth);

  connect(verticalScrollBar(), &QScrollBar::valueChanged,
          [this](int) { m_lineNumberArea->update(); });

  connect(this, &QTextEdit::cursorPositionChanged, this,
          &QCodeEditor::updateExtraSelection);

  connect(this, &QTextEdit::selectionChanged, this,
          &QCodeEditor::onSelectionChanged);
}

void QCodeEditor::setHighlighter(QStyleSyntaxHighlighter *highlighter)
{
  if (m_highlighter)
  {
    m_highlighter->setDocument(nullptr);
  }

  m_highlighter = highlighter;

  if (m_highlighter)
  {
    m_highlighter->setSyntaxStyle(m_syntaxStyle);
    m_highlighter->setDocument(document());
  }
}

QStyleSyntaxHighlighter *QCodeEditor::highlighter() const
{
  return m_highlighter;
}

void QCodeEditor::setSyntaxStyle(QSyntaxStyle *style)
{
  m_syntaxStyle = style;

  m_framedAttribute->setSyntaxStyle(m_syntaxStyle);
  m_lineNumberArea->setSyntaxStyle(m_syntaxStyle);

  if (m_highlighter)
  {
    m_highlighter->setSyntaxStyle(m_syntaxStyle);
  }

  updateStyle();
}

void QCodeEditor::updateStyle()
{
  if (m_highlighter)
  {
    m_highlighter->rehighlight();
  }

  if (m_syntaxStyle)
  {
    auto currentPalette = palette();

    // Setting text format/color
    currentPalette.setColor(
        QPalette::ColorRole::Text,
        m_syntaxStyle->getFormat("Text").foreground().color());

    // Setting common background
    currentPalette.setColor(
        QPalette::Base, m_syntaxStyle->getFormat("Text").background().color());

    // Setting selection color
    currentPalette.setColor(
        QPalette::Highlight,
        m_syntaxStyle->getFormat("Selection").background().color());
    currentPalette.setColor(
        QPalette::HighlightedText,
        m_syntaxStyle->getFormat("Selection").foreground().color());

    setPalette(currentPalette);

    if (m_completer && m_completer->popup())
    {
      m_completer->popup()->setFont(font());
      m_completer->popup()->setPalette(currentPalette);
    }
  }

  updateExtraSelection();
}

void QCodeEditor::onSelectionChanged()
{
  auto selected = textCursor().selectedText();

  auto cursor = textCursor();

  // Cursor is null if setPlainText was called.
  if (cursor.isNull())
  {
    return;
  }

  cursor.movePosition(QTextCursor::MoveOperation::Left);
  cursor.select(QTextCursor::SelectionType::WordUnderCursor);

  QSignalBlocker blocker(this);
  m_framedAttribute->clear(cursor);

  if (selected.size() > 1 && cursor.selectedText() == selected)
  {
    auto backup = textCursor();

    // Perform search selecting
    handleSelectionQuery(cursor);

    setTextCursor(backup);
  }
}

void QCodeEditor::resizeEvent(QResizeEvent *e)
{
  QTextEdit::resizeEvent(e);

  updateLineGeometry();
}

void QCodeEditor::updateLineGeometry()
{
  QRect cr = contentsRect();
  m_lineNumberArea->setGeometry(QRect(
      cr.left(), cr.top(), m_lineNumberArea->sizeHint().width(), cr.height()));
}

void QCodeEditor::updateLineNumberAreaWidth(int)
{
  setViewportMargins(m_lineNumberArea->sizeHint().width(), 0, 0, 0);
}

void QCodeEditor::updateLineNumberArea(const QRect &rect)
{
  m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->sizeHint().width(),
                           rect.height());
  updateLineGeometry();

  if (rect.contains(viewport()->rect()))
  {
    updateLineNumberAreaWidth(0);
  }
}

void QCodeEditor::handleSelectionQuery(QTextCursor cursor)
{

  auto searchIterator = cursor;
  searchIterator.movePosition(QTextCursor::Start);
  searchIterator = document()->find(cursor.selectedText(), searchIterator);
  while (searchIterator.hasSelection())
  {
    m_framedAttribute->frame(searchIterator);

    searchIterator = document()->find(cursor.selectedText(), searchIterator);
  }
}

void QCodeEditor::updateExtraSelection()
{
  QList<QTextEdit::ExtraSelection> extra;

  highlightCurrentLine(extra);
  highlightParenthesis(extra);

  setExtraSelections(extra);
}

void QCodeEditor::highlightParenthesis(
    QList<QTextEdit::ExtraSelection> &extraSelection)
{
  auto currentSymbol = charUnderCursor();
  auto prevSymbol = charUnderCursor(-1);

  for (auto &pair : parentheses)
  {
    int direction;

    QChar counterSymbol;
    QChar activeSymbol;
    auto position = textCursor().position();

    if (pair.first == currentSymbol)
    {
      direction = 1;
      counterSymbol = pair.second[0];
      activeSymbol = currentSymbol;
    }
    else if (pair.second == prevSymbol)
    {
      direction = -1;
      counterSymbol = pair.first[0];
      activeSymbol = prevSymbol;
      position--;
    }
    else
    {
      continue;
    }

    auto counter = 1;

    while (counter != 0 && position > 0
           && position < (document()->characterCount() - 1))
    {
      // Moving position
      position += direction;

      auto character = document()->characterAt(position);
      // Checking symbol under position
      if (character == activeSymbol)
      {
        ++counter;
      }
      else if (character == counterSymbol)
      {
        --counter;
      }
    }

    auto format = m_syntaxStyle->getFormat("Parentheses");

    // Found
    if (counter == 0)
    {
      ExtraSelection selection{};

      auto directionEnum = direction < 0 ? QTextCursor::MoveOperation::Left
                                         : QTextCursor::MoveOperation::Right;

      selection.format = format;
      selection.cursor = textCursor();
      selection.cursor.clearSelection();
      selection.cursor.movePosition(
          directionEnum, QTextCursor::MoveMode::MoveAnchor,
          std::abs(textCursor().position() - position));

      selection.cursor.movePosition(QTextCursor::MoveOperation::Right,
                                    QTextCursor::MoveMode::KeepAnchor, 1);

      extraSelection.append(selection);

      selection.cursor = textCursor();
      selection.cursor.clearSelection();
      selection.cursor.movePosition(directionEnum,
                                    QTextCursor::MoveMode::KeepAnchor, 1);

      extraSelection.append(selection);
    }

    break;
  }
}

void QCodeEditor::highlightCurrentLine(
    QList<QTextEdit::ExtraSelection> &extraSelection)
{
  if (!isReadOnly())
  {
    QTextEdit::ExtraSelection selection{};

    selection.format = m_syntaxStyle->getFormat("CurrentLine");
    selection.format.setForeground(QBrush());
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();

    extraSelection.append(selection);
  }
}

void QCodeEditor::paintEvent(QPaintEvent *e)
{
  updateLineNumberArea(e->rect());
  QTextEdit::paintEvent(e);
}

int QCodeEditor::getFirstVisibleBlock()
{
  // Detect the first block for which bounding rect - once translated
  // in absolute coordinated - is contained by the editor's text area

  // Costly way of doing but since "blockBoundingGeometry(...)" doesn't
  // exists for "QTextEdit"...

  QTextCursor curs = QTextCursor(document());
  curs.movePosition(QTextCursor::Start);
  for (int i = 0; i < document()->blockCount(); ++i)
  {
    QTextBlock block = curs.block();

    QRect r1 = viewport()->geometry();
    QRect r2 = document()
                   ->documentLayout()
                   ->blockBoundingRect(block)
                   .translated(viewport()->geometry().x(),
                               viewport()->geometry().y()
                                   - verticalScrollBar()->sliderPosition())
                   .toRect();

    if (r1.intersects(r2))
    {
      return i;
    }

    curs.movePosition(QTextCursor::NextBlock);
  }

  return 0;
}

bool QCodeEditor::proceedCompleterBegin(QKeyEvent *e)
{
  if (m_completer && m_completer->popup()->isVisible())
  {
    switch (e->key())
    {
      case Qt::Key_Enter:
      case Qt::Key_Return:
      case Qt::Key_Escape:
      case Qt::Key_Tab:
      case Qt::Key_Backtab:
        e->ignore();
        return true; // let the completer do default behavior
      default:
        break;
    }
  }

  // todo: Replace with modifiable QShortcut
  auto isShortcut
      = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_Space);

  return !(!m_completer || !isShortcut);
}

void QCodeEditor::proceedCompleterEnd(QKeyEvent *e)
{
  auto ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);

  if (!m_completer || (ctrlOrShift && e->text().isEmpty())
      || e->key() == Qt::Key_Delete)
  {
    return;
  }

  // End-of-word characters: '.', '_' and '$' stay out so dotted
  // identifiers ("io.getLatestFrame") keep completing
  static QString eow(R"(~!@#%^&*()+{}|:"<>?,/;'[]\-=)");

  auto isShortcut
      = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_Space);
  auto prefix = completionPrefix();

  if (!isShortcut
      && (e->text().isEmpty() || prefix.length() < 2
          || eow.contains(e->text().right(1))))
  {
    m_completer->popup()->hide();
    return;
  }

  if (prefix != m_completer->completionPrefix())
  {
    m_completer->setCompletionPrefix(prefix);
    m_completer->popup()->setCurrentIndex(
        m_completer->completionModel()->index(0, 0));
  }

  if (m_completer->completionCount() == 0
      || (m_completer->completionCount() == 1
          && m_completer->currentCompletion() == prefix))
  {
    m_completer->popup()->hide();
    return;
  }

  auto cursRect = cursorRect();
  cursRect.setWidth(
      m_completer->popup()->sizeHintForColumn(0)
      + m_completer->popup()->verticalScrollBar()->sizeHint().width());

  m_completer->complete(cursRect);
}

void QCodeEditor::keyPressEvent(QKeyEvent *e)
{
  auto completerSkip = proceedCompleterBegin(e);

  if (!completerSkip)
  {
    const int tabSize = m_tabReplace.size() > 0 ? m_tabReplace.size() : 4;

    // Toggle line comments with Ctrl+/ when a language is set
    if (e->key() == Qt::Key_Slash
        && e->modifiers().testFlag(Qt::ControlModifier)
        && m_languageHint != LanguageHint::Generic)
    {
      toggleLineComment();
      return;
    }

    if (m_replaceTab && e->key() == Qt::Key_Tab
        && e->modifiers() == Qt::NoModifier)
    {
      // Indent the whole block when the selection spans multiple lines
      auto cursor = textCursor();
      if (cursor.hasSelection()
          && document()->findBlock(cursor.selectionStart())
                 != document()->findBlock(cursor.selectionEnd()))
      {
        changeBlockIndent(true);
        return;
      }

      insertPlainText(m_tabReplace);
      return;
    }

    // Shortcut for moving the selected lines (or current line) to the left
    if (m_replaceTab && e->key() == Qt::Key_Backtab)
    {
      changeBlockIndent(false);
      return;
    }

    // Auto indentation
    int indentationLevel = getIndentationSpaces();

    int tabCounts = indentationLevel / tabSize;

    // Have Qt Editor like behaviour, if {|} and enter is pressed indent the
    // two parenthesis
    if (m_autoIndentation
        && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
        && charUnderCursor() == '}' && charUnderCursor(-1) == '{')
    {
      int charsBack = 0;
      insertPlainText("\n");

      if (m_replaceTab)
        insertPlainText(QString(indentationLevel + tabSize, ' '));
      else
        insertPlainText(QString(tabCounts + 1, '\t'));

      insertPlainText("\n");
      charsBack++;

      if (m_replaceTab)
      {
        insertPlainText(QString(indentationLevel, ' '));
        charsBack += indentationLevel;
      }
      else
      {
        insertPlainText(QString(tabCounts, '\t'));
        charsBack += tabCounts;
      }

      while (charsBack--)
        moveCursor(QTextCursor::MoveOperation::Left);
      return;
    }

    // Compute the extra indentation level before the newline is inserted
    int indentBoost = 0;
    if (m_autoIndentation
        && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter))
      indentBoost = newLineIndentBoost();

    // Unindent a closing brace typed on a whitespace-only line
    if (m_autoIndentation && e->text() == "}")
      dedentClosingBrace();

    QTextEdit::keyPressEvent(e);

    if (m_autoIndentation
        && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter))
    {
      if (m_replaceTab)
        insertPlainText(QString(indentationLevel + indentBoost * tabSize, ' '));
      else
        insertPlainText(QString(tabCounts + indentBoost, '\t'));
    }

    if (m_autoParentheses)
    {
      for (auto &&el : parentheses)
      {
        // Inserting closed brace
        if (el.first == e->text())
        {
          insertPlainText(el.second);
          moveCursor(QTextCursor::MoveOperation::Left);
          break;
        }

        // If it's close brace - check parentheses
        if (el.second == e->text())
        {
          auto symbol = charUnderCursor();

          if (symbol == el.second)
          {
            textCursor().deletePreviousChar();
            moveCursor(QTextCursor::MoveOperation::Right);
          }

          break;
        }
      }
    }
  }

  proceedCompleterEnd(e);
}

void QCodeEditor::setAutoIndentation(bool enabled)
{
  m_autoIndentation = enabled;
}

bool QCodeEditor::autoIndentation() const
{
  return m_autoIndentation;
}

void QCodeEditor::setAutoParentheses(bool enabled)
{
  m_autoParentheses = enabled;
}

bool QCodeEditor::autoParentheses() const
{
  return m_autoParentheses;
}

void QCodeEditor::setTabReplace(bool enabled)
{
  m_replaceTab = enabled;
}

bool QCodeEditor::tabReplace() const
{
  return m_replaceTab;
}

void QCodeEditor::setTabReplaceSize(int val)
{
  m_tabReplace.clear();

  m_tabReplace.fill(' ', val);
}

int QCodeEditor::tabReplaceSize() const
{
  return m_tabReplace.size();
}

void QCodeEditor::setCompleter(QCompleter *completer)
{
  if (m_completer)
  {
    disconnect(m_completer, nullptr, this, nullptr);
  }

  m_completer = completer;

  if (!m_completer)
  {
    return;
  }

  m_completer->setWidget(this);
  m_completer->setCompletionMode(QCompleter::CompletionMode::PopupCompletion);

  // Match the editor's monospace font and colors so the popup looks native
  if (m_completer->popup())
  {
    m_completer->popup()->setFont(font());
    m_completer->popup()->setPalette(palette());
  }

  connect(m_completer, QOverload<const QString &>::of(&QCompleter::activated),
          this, &QCodeEditor::insertCompletion);
}

void QCodeEditor::focusInEvent(QFocusEvent *e)
{
  if (m_completer)
  {
    m_completer->setWidget(this);
  }

  QTextEdit::focusInEvent(e);
}

void QCodeEditor::insertCompletion(QString s)
{
  if (m_completer->widget() != this)
  {
    return;
  }

  // Replace the dotted prefix that produced the match, not just the
  // word under the cursor, so "io.getLat" -> "io.getLatestFrame"
  auto tc = textCursor();
  const auto prefixLength = m_completer->completionPrefix().length();
  tc.movePosition(QTextCursor::MoveOperation::Left,
                  QTextCursor::MoveMode::KeepAnchor,
                  static_cast<int>(prefixLength));
  tc.insertText(s);
  setTextCursor(tc);
}

QCompleter *QCodeEditor::completer() const
{
  return m_completer;
}

QLineNumberArea *QCodeEditor::lineNumberArea() const
{
  return m_lineNumberArea;
}

QChar QCodeEditor::charUnderCursor(int offset) const
{
  auto block = textCursor().blockNumber();
  auto index = textCursor().positionInBlock();
  auto text = document()->findBlockByNumber(block).text();

  index += offset;

  if (index < 0 || index >= text.size())
  {
    return {};
  }

  return text[index];
}

QString QCodeEditor::wordUnderCursor() const
{
  auto tc = textCursor();
  tc.select(QTextCursor::WordUnderCursor);
  return tc.selectedText();
}

void QCodeEditor::insertFromMimeData(const QMimeData *source)
{
  insertPlainText(source->text());
}

int QCodeEditor::getIndentationSpaces()
{
  auto blockText = textCursor().block().text();

  int indentationLevel = 0;

  for (auto i = 0;
       i < blockText.size() && QString("\t ").contains(blockText[i]); ++i)
  {
    if (blockText[i] == ' ')
    {
      indentationLevel++;
    }
    else
    {
      indentationLevel += 4;
    }
  }

  return indentationLevel;
}

void QCodeEditor::setLanguageHint(LanguageHint hint)
{
  m_languageHint = hint;
}

QCodeEditor::LanguageHint QCodeEditor::languageHint() const
{
  return m_languageHint;
}

bool QCodeEditor::isJsHangingHeader(const QString &line)
{
  // Strip simple string literals so quoted brackets don't skew the balance
  static const QRegularExpression strings(R"("[^"]*"|'[^']*')");
  static const QRegularExpression header(
      R"(^\}?\s*(?:else\s+)?(?:if|for|while)\b.*\)$)");
  static const QRegularExpression bare(R"(^\}?\s*(?:else|do)$)");

  QString code = line;
  code.replace(strings, QString());

  if (bare.match(code).hasMatch())
    return true;

  int balance = 0;
  for (const QChar &ch : code)
  {
    if (ch == '(')
      ++balance;
    else if (ch == ')')
      --balance;
  }

  return balance == 0 && header.match(code).hasMatch();
}

bool QCodeEditor::isLuaBlockHeader(const QString &line)
{
  static const QRegularExpression strings(R"("[^"]*"|'[^']*')");
  static const QRegularExpression tail(R"((?:^|.*\b)(?:then|do|else|repeat)$)");
  static const QRegularExpression func(R"(^(?:local\s+)?function\b.*\)$)");

  QString code = line;
  code.replace(strings, QString());

  return tail.match(code).hasMatch() || func.match(code).hasMatch();
}

int QCodeEditor::newLineIndentBoost() const
{
  auto cursor = textCursor();
  cursor.setPosition(qMin(cursor.anchor(), cursor.position()));

  const auto blockText = cursor.block().text();
  const auto left = blockText.left(cursor.positionInBlock()).trimmed();
  if (left.isEmpty())
    return 0;

  const QChar last = left.back();
  if (last == '{' || last == '(' || last == '[')
    return 1;

  if (m_languageHint == LanguageHint::JavaScript)
    return isJsHangingHeader(left) ? 1 : 0;

  if (m_languageHint == LanguageHint::Lua)
    return isLuaBlockHeader(left) ? 1 : 0;

  return 0;
}

void QCodeEditor::dedentClosingBrace()
{
  auto cursor = textCursor();
  if (cursor.hasSelection())
    return;

  const auto left = cursor.block().text().left(cursor.positionInBlock());
  if (left.isEmpty() || !left.trimmed().isEmpty())
    return;

  const int tabSize = m_tabReplace.size() > 0 ? m_tabReplace.size() : 4;
  const int remove
      = left.endsWith('\t') ? 1 : qMin(tabSize, static_cast<int>(left.size()));

  cursor.movePosition(QTextCursor::MoveOperation::Left,
                      QTextCursor::MoveMode::KeepAnchor, remove);
  cursor.removeSelectedText();
}

void QCodeEditor::changeBlockIndent(bool increase)
{
  auto cursor = textCursor();
  const int startPos = qMin(cursor.anchor(), cursor.position());
  const int endPos = qMax(cursor.anchor(), cursor.position());

  auto probe = QTextCursor(document());
  probe.setPosition(startPos);
  const int firstBlock = probe.blockNumber();
  probe.setPosition(endPos);
  int lastBlock = probe.blockNumber();

  // A selection ending at column 0 does not include that line
  if (lastBlock > firstBlock && cursor.hasSelection()
      && probe.positionInBlock() == 0)
    --lastBlock;

  probe.beginEditBlock();
  for (int i = firstBlock; i <= lastBlock; ++i)
  {
    const auto block = document()->findBlockByNumber(i);
    auto line = QTextCursor(block);

    if (increase)
    {
      if (!block.text().isEmpty())
        line.insertText(m_tabReplace);
      continue;
    }

    const auto text = block.text();
    int remove = 0;
    while (remove < m_tabReplace.size() && remove < text.size()
           && text[remove] == ' ')
      ++remove;

    if (remove == 0 && !text.isEmpty() && text[0] == '\t')
      remove = 1;

    if (remove > 0)
    {
      line.movePosition(QTextCursor::MoveOperation::Right,
                        QTextCursor::MoveMode::KeepAnchor, remove);
      line.removeSelectedText();
    }
  }
  probe.endEditBlock();
}

void QCodeEditor::toggleLineComment()
{
  const QString token
      = m_languageHint == LanguageHint::Lua ? "--" : "//";

  auto cursor = textCursor();
  const int startPos = qMin(cursor.anchor(), cursor.position());
  const int endPos = qMax(cursor.anchor(), cursor.position());

  auto probe = QTextCursor(document());
  probe.setPosition(startPos);
  const int firstBlock = probe.blockNumber();
  probe.setPosition(endPos);
  int lastBlock = probe.blockNumber();

  if (lastBlock > firstBlock && cursor.hasSelection()
      && probe.positionInBlock() == 0)
    --lastBlock;

  // Comment when at least one non-empty line is not commented yet
  bool addComment = false;
  for (int i = firstBlock; i <= lastBlock && !addComment; ++i)
  {
    const auto text = document()->findBlockByNumber(i).text().trimmed();
    if (!text.isEmpty() && !text.startsWith(token))
      addComment = true;
  }

  probe.beginEditBlock();
  for (int i = firstBlock; i <= lastBlock; ++i)
  {
    const auto block = document()->findBlockByNumber(i);
    const auto text = block.text();
    if (text.trimmed().isEmpty())
      continue;

    int indent = 0;
    while (indent < text.size()
           && (text[indent] == ' ' || text[indent] == '\t'))
      ++indent;

    auto line = QTextCursor(block);
    line.movePosition(QTextCursor::MoveOperation::Right,
                      QTextCursor::MoveMode::MoveAnchor, indent);

    if (addComment)
    {
      line.insertText(token + ' ');
      continue;
    }

    int remove = token.size();
    if (indent + remove < text.size() && text[indent + remove] == ' ')
      ++remove;

    line.movePosition(QTextCursor::MoveOperation::Right,
                      QTextCursor::MoveMode::KeepAnchor, remove);
    line.removeSelectedText();
  }
  probe.endEditBlock();
}

QString QCodeEditor::completionPrefix() const
{
  const auto cursor = textCursor();
  const auto blockText = cursor.block().text();
  const int end = cursor.positionInBlock();

  int start = end;
  while (start > 0)
  {
    const QChar ch = blockText[start - 1];
    if (!ch.isLetterOrNumber() && ch != '_' && ch != '$' && ch != '.')
      break;

    --start;
  }

  auto prefix = blockText.mid(start, end - start);
  while (prefix.startsWith('.'))
    prefix.remove(0, 1);

  return prefix;
}
