#pragma once

// Qt
#include <QTextEdit> // Required for inheritance

class QCompleter;
class QLineNumberArea;
class QSyntaxStyle;
class QStyleSyntaxHighlighter;
class QFramedTextAttribute;

/**
 * @brief Class, that describes code editor.
 */
class QCodeEditor : public QTextEdit
{
  Q_OBJECT

public:
  /**
   * @brief Language used to drive auto-indentation heuristics.
   */
  enum class LanguageHint
  {
    Generic,
    JavaScript,
    Lua
  };

  /**
   * @brief Constructor.
   * @param widget Pointer to parent widget.
   */
  explicit QCodeEditor(QWidget *widget = nullptr);

  // Disable copying
  QCodeEditor(const QCodeEditor &) = delete;
  QCodeEditor &operator=(const QCodeEditor &) = delete;

  /**
   * @brief Method for getting first visible block
   * index.
   * @return Index.
   */
  int getFirstVisibleBlock();

  /**
   * @brief Method for setting highlighter.
   * @param highlighter Pointer to syntax highlighter.
   */
  void setHighlighter(QStyleSyntaxHighlighter *highlighter);

  /**
   * @brief Method for getting the current highlighter.
   * The editor does not own it; callers replacing a
   * highlighter are responsible for releasing the old one.
   * @return Pointer to the active syntax highlighter.
   */
  QStyleSyntaxHighlighter *highlighter() const;

  /**
   * @brief Method for setting syntax sty.e.
   * @param style Pointer to syntax style.
   */
  void setSyntaxStyle(QSyntaxStyle *style);

  /**
   * @brief Method setting auto parentheses enabled.
   */
  void setAutoParentheses(bool enabled);

  /**
   * @brief Method for getting is auto parentheses enabled.
   * Default value: true
   */
  bool autoParentheses() const;

  /**
   * @brief Method for setting tab replacing
   * enabled.
   */
  void setTabReplace(bool enabled);

  /**
   * @brief Method for getting is tab replacing enabled.
   * Default value: true
   */
  bool tabReplace() const;

  /**
   * @brief Method for setting amount of spaces, that will
   * replace tab.
   * @param val Number of spaces.
   */
  void setTabReplaceSize(int val);

  /**
   * @brief Method for getting number of spaces, that will
   * replace tab if `tabReplace` is true.
   * Default: 4
   */
  int tabReplaceSize() const;

  /**
   * @brief Method for setting the language used by the
   * auto-indentation heuristics (brace-free control
   * statement headers, Lua block keywords).
   */
  void setLanguageHint(LanguageHint hint);

  /**
   * @brief Method for getting the auto-indentation language.
   * Default: LanguageHint::Generic
   */
  LanguageHint languageHint() const;

  /**
   * @brief Method for setting auto indentation enabled.
   */
  void setAutoIndentation(bool enabled);

  /**
   * @brief Method for getting is auto indentation enabled.
   * Default: true
   */
  bool autoIndentation() const;

  /**
   * @brief Method for setting completer.
   * @param completer Pointer to completer object.
   */
  void setCompleter(QCompleter *completer);

  /**
   * @brief Method for getting completer.
   * @return Pointer to completer.
   */
  QCompleter *completer() const;

  /**
   * @brief Method for accessing the linenumber area
   * @return Pointer to the @c QLineNumberArea used by the widget
   */
  QLineNumberArea *lineNumberArea() const;

public Q_SLOTS:

  /**
   * @brief Slot, that performs insertion of
   * completion info into code.
   * @param s Data.
   */
  void insertCompletion(QString s);

  /**
   * @brief Slot, that performs update of
   * internal editor viewport based on line
   * number area width.
   */
  void updateLineNumberAreaWidth(int);

  /**
   * @brief Slot, that performs update of some
   * part of line number area.
   * @param rect Area that has to be updated.
   */
  void updateLineNumberArea(const QRect &rect);

  /**
   * @brief Slot, that will proceed extra selection
   * for current cursor position.
   */
  void updateExtraSelection();

  /**
   * @brief Slot, that will update editor style.
   */
  void updateStyle();

  /**
   * @brief Slot, that will be called on selection
   * change.
   */
  void onSelectionChanged();

protected:
  /**
   * @brief Method, that's called on any text insertion of
   * mimedata into editor. If it's text - it inserts text
   * as plain text.
   */
  void insertFromMimeData(const QMimeData *source) override;

  /**
   * @brief Method, that's called on editor painting. This
   * method if overloaded for line number area redraw.
   */
  void paintEvent(QPaintEvent *e) override;

  /**
   * @brief Method, that's called on any widget resize.
   * This method if overloaded for line number area
   * resizing.
   */
  void resizeEvent(QResizeEvent *e) override;

  /**
   * @brief Method, that's called on any key press, posted
   * into code editor widget. This method is overloaded for:
   *
   * 1. Completion
   * 2. Tab to spaces
   * 3. Low indentation
   * 4. Auto parenthesis
   */
  void keyPressEvent(QKeyEvent *e) override;

  /**
   * @brief Method, that's called on focus into widget.
   * It's required for setting this widget to set
   * completer.
   */
  void focusInEvent(QFocusEvent *e) override;

  /**
   * @brief Intercepts LayoutDirectionChange events to keep the editor
   * pinned to left-to-right regardless of the host application's locale.
   * Source code reads left-to-right, so RTL inheritance from a parent
   * window must not flip the editor's layout or its text flow.
   */
  void changeEvent(QEvent *e) override;

private:
  /**
   * @brief Forces the widget layout direction and the document's default
   * text option to left-to-right. Called from the constructor and after
   * any inherited LayoutDirectionChange event.
   */
  void enforceLeftToRight();


  /**
   * @brief Method for initializing document
   * layout handlers.
   */
  void initDocumentLayoutHandlers();

  /**
   * @brief Method for initializing default
   * monospace font.
   */
  void initFont();

  /**
   * @brief Method for performing connection
   * of objects.
   */
  void performConnections();

  /**
   * @brief Method, that performs selection
   * frame selection.
   */
  void handleSelectionQuery(QTextCursor cursor);

  /**
   * @brief Method for updating geometry of line number area.
   */
  void updateLineGeometry();

  /**
   * @brief Method, that performs completer processing.
   * Returns true if event has to be dropped.
   * @param e Pointer to key event.
   * @return Shall event be dropped.
   */
  bool proceedCompleterBegin(QKeyEvent *e);
  void proceedCompleterEnd(QKeyEvent *e);

  /**
   * @brief Method for getting character under
   * cursor.
   * @param offset Offset to cursor.
   */
  QChar charUnderCursor(int offset = 0) const;

  /**
   * @brief Method for getting word under
   * cursor.
   * @return Word under cursor.
   */
  QString wordUnderCursor() const;

  /**
   * @brief Method, that adds highlighting of
   * currently selected line to extra selection list.
   */
  void highlightCurrentLine(QList<QTextEdit::ExtraSelection> &extraSelection);

  /**
   * @brief Method, that adds highlighting of
   * parenthesis if available.
   */
  void highlightParenthesis(QList<QTextEdit::ExtraSelection> &extraSelection);

  /**
   * @brief Method for getting number of indentation
   * spaces in current line. Tabs will be treated
   * as `tabWidth / spaceWidth`
   */
  int getIndentationSpaces();

  /**
   * @brief Returns the extra indentation levels (0 or 1) the
   * line following the cursor should receive when Return is
   * pressed: after an unclosed bracket or after a brace-free
   * control-statement header in the hinted language.
   */
  int newLineIndentBoost() const;

  /**
   * @brief Removes one indentation level before a '}' typed
   * on a line that only contains whitespace so far.
   */
  void dedentClosingBrace();

  /**
   * @brief Indents (or unindents) every line spanned by the
   * current selection by one tab-replace unit.
   */
  void changeBlockIndent(bool increase);

  /**
   * @brief Toggles the line comment token ("//" or "--",
   * from the language hint) on every selected line.
   */
  void toggleLineComment();

  /**
   * @brief Returns the dotted completion prefix left of the
   * cursor (identifier characters and '.', e.g. "io.getLat").
   */
  QString completionPrefix() const;

  /**
   * @brief Returns true when the given trimmed line is a
   * brace-free JS control-statement header ("if (x)", "else").
   */
  static bool isJsHangingHeader(const QString &line);

  /**
   * @brief Returns true when the given trimmed line opens a
   * Lua block ("if x then", "for ... do", "function f(...)").
   */
  static bool isLuaBlockHeader(const QString &line);

  QStyleSyntaxHighlighter *m_highlighter;
  QSyntaxStyle *m_syntaxStyle;
  QLineNumberArea *m_lineNumberArea;
  QCompleter *m_completer;

  QFramedTextAttribute *m_framedAttribute;

  bool m_autoIndentation;
  bool m_autoParentheses;
  bool m_replaceTab;
  QString m_tabReplace;
  LanguageHint m_languageHint;
};
