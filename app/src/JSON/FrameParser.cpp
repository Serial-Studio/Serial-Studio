/*
 * Copyright (c) 2022-2023 Alex Spataru <https: *github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <QFile>
#include <QJSEngine>
#include <QFileDialog>
#include <QLineNumberArea>
#include <QDesktopServices>
#include <QRegularExpression>
#include <QJavascriptHighlighter>

#include "JSON/FrameParser.h"
#include "JSON/ProjectModel.h"

#include "Misc/Utilities.h"
#include "Misc/CommonFonts.h"
#include "Misc/TimerEvents.h"
#include "Misc/ThemeManager.h"

/**
 * Creates a subclass of @c QCodeEditor that allows us to call the given
 * protected/private @a function and pass the given @a event as a parameter to
 * the @a function.
 */
#define DW_EXEC_EVENT(pointer, function, event)                                \
  class PwnedWidget : public QCodeEditor                                       \
  {                                                                            \
  public:                                                                      \
    using QCodeEditor::function;                                               \
  };                                                                           \
  static_cast<PwnedWidget *>(pointer)->function(event);

/**
 * @brief Constructor function for the code editor widget.
 */
JSON::FrameParser::FrameParser(QQuickItem *parent)
  : QQuickPaintedItem(parent)
{
  // Disable mipmap & antialiasing, we don't need them
  setMipmap(false);
  setAntialiasing(false);

  // Disable alpha channel
  setOpaquePainting(true);
  setFillColor(Misc::ThemeManager::instance().getColor(QStringLiteral("base")));

  // Widgets don't process touch events, disable it
  setAcceptTouchEvents(true);

  // Set item flags, we need these to forward Quick events to the widget
  setFlag(ItemHasContents, true);
  setFlag(ItemIsFocusScope, true);
  setFlag(ItemAcceptsInputMethod, true);
  setAcceptedMouseButtons(Qt::AllButtons);

  // Configure code editor
  m_widget.setTabReplace(true);
  m_widget.setTabReplaceSize(4);
  m_widget.setAutoIndentation(true);
  m_widget.setHighlighter(new QJavascriptHighlighter());
  m_widget.setFont(Misc::CommonFonts::instance().monoFont());

  // Configure JavaScript engine
  m_engine.installExtensions(QJSEngine::ConsoleExtension
                             | QJSEngine::GarbageCollectionExtension);

  // Load template code
  reload();

  // Set widget palettez
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &JSON::FrameParser::onThemeChanged);

  // Connect modification check signals
  connect(&m_widget, &QCodeEditor::textChanged, this,
          [=] { Q_EMIT modifiedChanged(); });

  // Make project editor modification status depend on code modification status
  connect(this, &JSON::FrameParser::modifiedChanged, &ProjectModel::instance(),
          &ProjectModel::modifiedChanged);

  // Load code from JSON model automatically
  connect(&JSON::ProjectModel::instance(),
          &ProjectModel::frameParserCodeChanged, this,
          &JSON::FrameParser::readCode);

  // Bridge signals
  connect(&m_widget, &QCodeEditor::textChanged, this,
          &FrameParser::textChanged);

  // Resize widget to fit QtQuick item
  connect(this, &QQuickPaintedItem::widthChanged, this,
          &JSON::FrameParser::resizeWidget);
  connect(this, &QQuickPaintedItem::heightChanged, this,
          &JSON::FrameParser::resizeWidget);

  // Configure render loop
  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::timeout24Hz, this,
          &JSON::FrameParser::renderWidget);
}

/**
 * @brief Returns a string with the default frame parser function code.
 * @return
 */
const QString &JSON::FrameParser::defaultCode()
{
  static QString code;
  if (code.isEmpty())
  {
    QFile file(":/rcc/scripts/frame-parser.js");
    if (file.open(QFile::ReadOnly))
    {
      code = QString::fromUtf8(file.readAll());
      file.close();
    }
  }

  return code;
}

/**
 * @brief Returns the current text/data displayed in the code editor.
 */
QString JSON::FrameParser::text() const
{
  return m_widget.toPlainText();
}

/**
 * @brief Indicates whenever there are any unsaved changes in the code editor.
 * @return
 */
bool JSON::FrameParser::isModified() const
{
  if (m_widget.document())
    return m_widget.document()->isModified();

  return false;
}

/**
 * @brief Executes the current frame parser function over the input data.
 *
 * @param frame current/latest frame data.
 *
 * @return An array of strings with the values returned by the JS frame parser.
 */
QStringList JSON::FrameParser::parse(const QString &frame)
{
  // Construct function arguments
  QJSValueList args;
  args << frame;

  // Evaluate frame parsing function
  auto out = m_parseFunction.call(args).toVariant().toStringList();

  // Convert output to QStringList
  QStringList list;
  for (auto i = 0; i < out.count(); ++i)
    list.append(out.at(i));

  // Return fields list
  return list;
}

/**
 * @brief Returns @c true whenever if there are any actions that can be undone.
 */
bool JSON::FrameParser::undoAvailable() const
{
  if (m_widget.document())
    return isModified() && m_widget.document()->isUndoAvailable();

  return false;
}
/**
 * @brief Returns @c true whenever if there are any actions that can be redone.
 */
bool JSON::FrameParser::redoAvailable() const
{
  if (m_widget.document())
    return isModified() && m_widget.document()->isRedoAvailable();

  return false;
}

/**
 * @brief Validates the script and stores it into the project model.
 * @param silent if set to @c false, the user shall be notified that the script
 *               data was processed correctly.
 *
 * @return @c true on success, @c false on failure
 */
bool JSON::FrameParser::save(const bool silent)
{
  // Update text edit
  if (loadScript(text()))
  {
    ProjectModel::instance().setFrameParserCode(text());
    m_widget.document()->setModified(false);
    m_widget.document()->clearUndoRedoStacks();

    // Show save messagebox
    if (!silent)
      Misc::Utilities::showMessageBox(
          tr("Frame parser code updated successfully!"),
          tr("No errors have been detected in the code."));

    // Everything good
    Q_EMIT modifiedChanged();
    return true;
  }

  // Something did not work quite right
  return false;
}

/**
 * @brief Generates a callable JS function given the script code.
 *
 * This function validates that the given @a script data does not contain any
 * syntax errors and that it can be executed by the JavaScript Engine.
 *
 * If the conditions required to execute the frame parser code are met, this
 * function proceeds to generate a callable JS function that can be used
 * by the rest of the application modules.
 *
 * @param script JavaScript code
 *
 * @return @a true if the JS code is valid and contains to errors
 */
bool JSON::FrameParser::loadScript(const QString &script)
{
  // Ensure that engine is configured correctly
  m_engine.installExtensions(QJSEngine::AllExtensions);

  // Check if there are no general JS errors
  QStringList errors;
  m_engine.evaluate(script, "", 1, &errors);

  // Check if the 'parse' function exists and is callable
  auto fun = m_engine.globalObject().property("parse");
  if (fun.isNull() || !fun.isCallable())
  {
    Misc::Utilities::showMessageBox(
        tr("Frame parser error!"),
        tr("The 'parse' function is not declared or is not callable!"));
    return false;
  }

  // Check if the script contains a valid parse function declaratio
  static QRegularExpression functionRegex(
      R"(\bfunction\s+parse\s*\(\s*([a-zA-Z_$][a-zA-Z0-9_$]*)(\s*,\s*([a-zA-Z_$][a-zA-Z0-9_$]*))?\s*\))");
  auto match = functionRegex.match(script);
  if (match.hasMatch())
  {
    // Extract argument names
    QString firstArg = match.captured(1);
    QString secondArg = match.captured(3);

    // Warn about empty first argument
    if (firstArg.isEmpty())
    {
      Misc::Utilities::showMessageBox(
          tr("Frame parser error!"),
          tr("No valid 'parse' function declaration found in the script!"));
      return false;
    }

    // Warn about deprecated second argument
    if (!secondArg.isEmpty())
    {
      Misc::Utilities::showMessageBox(
          tr("Legacy frame parser function detected"),
          tr("The 'parse' function has two arguments ('%1', '%2'), indicating "
             "use of the old format. Please update it to the new format, which "
             "only takes the frame data as an argument.")
              .arg(firstArg, secondArg));
      return false;
    }
  }

  // Function doesn't match the expected declaration
  else
  {
    Misc::Utilities::showMessageBox(
        tr("Frame parser error!"),
        tr("No valid 'parse' function declaration found in the script!"));
    return false;
  }

  // We have reached this point without any errors, set function caller
  m_parseFunction = fun;
  return true;
}

/**
 * @brief Removes the selected text from the code editor widget and copies it
 *        into the system's clipboard.
 */
void JSON::FrameParser::cut()
{
  m_widget.cut();
}

/**
 * @brief Undoes the last user's action.
 */
void JSON::FrameParser::undo()
{
  m_widget.undo();
}

/**
 * @brief Redoes an undone action.
 */
void JSON::FrameParser::redo()
{
  m_widget.redo();
}

/**
 * @brief Opens a website with documentation relevant to the frame parser.
 */
void JSON::FrameParser::help()
{
  // clang-format off
  const auto url = QStringLiteral("https://github.com/Serial-Studio/Serial-Studio/wiki/Data-Flow-in-Serial-Studio#frame-parser-function");
  QDesktopServices::openUrl(QUrl(url));
  // clang-format on
}

/**
 * @brief Copies the selected text into the system's clipboard.
 */
void JSON::FrameParser::copy()
{
  m_widget.copy();
}

/**
 * @brief Pastes data from the system's clipboard into the code editor.
 */
void JSON::FrameParser::paste()
{
  m_widget.paste();
}

/**
 * @brief Writes the current code into the project file.
 */
void JSON::FrameParser::apply()
{
  if (save(false))
  {
    auto &model = JSON::ProjectModel::instance();
    if (!model.jsonFilePath().isEmpty())
    {
      const bool modified = model.modified();
      const bool hasGroups = model.groupCount() > 0;
      const bool hasDatasets = model.datasetCount() > 0;
      if (modified && hasGroups && hasDatasets)
      {
        model.saveJsonFile();
        model.displayFrameParserView();
      }
    }
  }
}

/**
 * @brief Loads the default frame parser function into the code editor.
 */
void JSON::FrameParser::reload()
{
  // Document has been modified, ask user if he/she wants to continue
  if (isModified())
  {
    auto ret = Misc::Utilities::showMessageBox(
        tr("The document has been modified!"),
        tr("Are you sure you want to continue?"), qAppName(),
        QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::No)
      return;
  }

  // Load default template
  m_widget.setPlainText(defaultCode());
  (void)save(true);
}

/**
 * @brief Prompts the user to select a JS file to import into the code editor.
 */
void JSON::FrameParser::import()
{
  // Document has been modified, ask user if he/she wants to continue
  if (isModified())
  {
    auto ret = Misc::Utilities::showMessageBox(
        tr("The document has been modified!"),
        tr("Are you sure you want to continue?"), qAppName(),
        QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::No)
      return;
  }

  // Get file from system
  auto path = QFileDialog::getOpenFileName(
      nullptr, tr("Select Javascript file to import"), QDir::homePath(),
      "*.js");

  // Load file into code editor
  if (!path.isEmpty())
  {
    QFile file(path);
    if (file.open(QFile::ReadOnly))
    {
      m_widget.setPlainText(QString::fromUtf8(file.readAll()));
      file.close();
      (void)save(true);
    }
  }
}

/**
 * @brief Loads the code stored in the project file into the code editor.
 */
void JSON::FrameParser::readCode()
{
  const auto code = ProjectModel::instance().frameParserCode();
  if (text() != code)
  {
    m_widget.setPlainText(code);
    m_widget.document()->setModified(false);
    m_widget.document()->clearUndoRedoStacks();

    (void)loadScript(code);
    Q_EMIT modifiedChanged();
  }
}

/**
 * @brief Selects all the text present in the code editor widget.
 */
void JSON::FrameParser::selectAll()
{
  m_widget.selectAll();
}

/**
 * @brief Modifies the palette of the code editor to match the colorscheme
 *        of the currently loaded theme file.
 */
void JSON::FrameParser::onThemeChanged()
{
  static const auto *t = &Misc::ThemeManager::instance();
  const auto name = t->themeData().value("code-editor-theme").toString();
  const auto path = QString(":/rcc/styles/%1.xml").arg(name);

  QFile file(path);
  if (file.open(QFile::ReadOnly))
  {
    m_style.load(QString::fromUtf8(file.readAll()));
    m_widget.setSyntaxStyle(&m_style);
    file.close();
  }
}

/**
 * @brief Renders the widget as a pixmap, which is then painted in the QML
 *        user interface.
 */
void JSON::FrameParser::renderWidget()
{
  if (isVisible())
  {
    m_pixmap = m_widget.grab();
    update();
  }
}

/**
 * Resizes the widget to fit inside the QML painted item.
 */
void JSON::FrameParser::resizeWidget()
{
  if (width() > 0 && height() > 0)
  {
    m_widget.setFixedSize(width(), height());
    renderWidget();
  }
}

/**
 * Displays the pixmap generated in the @c update() function in the QML
 * interface through the given @a painter pointer.
 */
void JSON::FrameParser::paint(QPainter *painter)
{
  if (painter && isVisible())
    painter->drawPixmap(0, 0, m_pixmap);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::keyPressEvent(QKeyEvent *event)
{
  DW_EXEC_EVENT(&m_widget, keyPressEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::keyReleaseEvent(QKeyEvent *event)
{
  DW_EXEC_EVENT(&m_widget, keyReleaseEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::inputMethodEvent(QInputMethodEvent *event)
{
  DW_EXEC_EVENT(&m_widget, inputMethodEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::focusInEvent(QFocusEvent *event)
{
  DW_EXEC_EVENT(&m_widget, focusInEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::focusOutEvent(QFocusEvent *event)
{
  DW_EXEC_EVENT(&m_widget, focusOutEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::mousePressEvent(QMouseEvent *event)
{
  auto pos = event->position();
  pos.setX(pos.x() - m_widget.lineNumberArea()->sizeHint().width());
  QMouseEvent eventCopy(event->type(), pos, event->globalPosition(),
                        event->button(), event->buttons(), event->modifiers(),
                        event->pointingDevice());

  DW_EXEC_EVENT(&m_widget, mousePressEvent, &eventCopy);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::mouseMoveEvent(QMouseEvent *event)
{
  auto pos = event->position();
  pos.setX(pos.x() - m_widget.lineNumberArea()->sizeHint().width());
  QMouseEvent eventCopy(event->type(), pos, event->globalPosition(),
                        event->button(), event->buttons(), event->modifiers(),
                        event->pointingDevice());

  DW_EXEC_EVENT(&m_widget, mouseMoveEvent, &eventCopy);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::mouseReleaseEvent(QMouseEvent *event)
{
  auto pos = event->position();
  pos.setX(pos.x() - m_widget.lineNumberArea()->sizeHint().width());
  QMouseEvent eventCopy(event->type(), pos, event->globalPosition(),
                        event->button(), event->buttons(), event->modifiers(),
                        event->pointingDevice());

  DW_EXEC_EVENT(&m_widget, mouseReleaseEvent, &eventCopy);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::mouseDoubleClickEvent(QMouseEvent *event)
{
  auto pos = event->position();
  pos.setX(pos.x() - m_widget.lineNumberArea()->sizeHint().width());
  QMouseEvent eventCopy(event->type(), pos, event->globalPosition(),
                        event->button(), event->buttons(), event->modifiers(),
                        event->pointingDevice());

  DW_EXEC_EVENT(&m_widget, mouseDoubleClickEvent, &eventCopy);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::wheelEvent(QWheelEvent *event)
{
  DW_EXEC_EVENT(&m_widget, wheelEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::dragEnterEvent(QDragEnterEvent *event)
{
  DW_EXEC_EVENT(&m_widget, dragEnterEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::dragMoveEvent(QDragMoveEvent *event)
{
  DW_EXEC_EVENT(&m_widget, dragMoveEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::dragLeaveEvent(QDragLeaveEvent *event)
{
  DW_EXEC_EVENT(&m_widget, dragLeaveEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::dropEvent(QDropEvent *event)
{
  DW_EXEC_EVENT(&m_widget, dropEvent, event);
}
