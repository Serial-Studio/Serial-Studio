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
#include <QDesktopServices>

#include "JSON/FrameParser.h"
#include "JSON/ProjectModel.h"

#include "Misc/Utilities.h"
#include "Misc/CommonFonts.h"
#include "Misc/TimerEvents.h"
#include "Misc/ThemeManager.h"

/**
 * Creates a subclass of @c QPlainTextEdit that allows us to call the given
 * protected/private @a function and pass the given @a event as a parameter to
 * the @a function.
 */
#define DW_EXEC_EVENT(pointer, function, event)                                \
  class PwnedWidget : public QPlainTextEdit                                    \
  {                                                                            \
  public:                                                                      \
    using QPlainTextEdit::function;                                            \
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
  setAcceptTouchEvents(false);

  // Set item flags, we need these to forward Quick events to the widget
  setFlag(ItemHasContents, true);
  setFlag(ItemIsFocusScope, true);
  setFlag(ItemAcceptsInputMethod, true);
  setAcceptedMouseButtons(Qt::AllButtons);

  // Setup syntax highlighter
  m_highlighter = new QSourceHighlite::QSourceHighliter(m_textEdit.document());
  m_highlighter->setCurrentLanguage(QSourceHighlite::QSourceHighliter::CodeJs);

  // Setup text editor
  m_textEdit.setFont(Misc::CommonFonts::instance().monoFont());

  // Configure JavaScript engine
  m_engine.installExtensions(QJSEngine::ConsoleExtension
                             | QJSEngine::GarbageCollectionExtension);

  // Load template code
  reload();

  // Set widget palette
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &JSON::FrameParser::onThemeChanged);

  // Connect modification check signals
  connect(m_textEdit.document(), &QTextDocument::modificationChanged, this,
          [=] { Q_EMIT modifiedChanged(); });

  // Load code from JSON model automatically
  connect(&JSON::ProjectModel::instance(),
          &ProjectModel::frameParserCodeChanged, this,
          &JSON::FrameParser::readCode);

  // Bridge signals
  connect(&m_textEdit, &QPlainTextEdit::textChanged, this,
          &JSON::FrameParser::textChanged);

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
  return m_textEdit.document()->toPlainText();
}

/**
 * @brief Indicates whenever there are any unsaved changes in the code editor.
 * @return
 */
bool JSON::FrameParser::isModified() const
{
  return m_textEdit.document()->isModified();
}

/**
 * @brief Executes the current frame parser function over the input data.
 *
 * @param frame current/latest frame data.
 * @param separator separator sequence to use.
 *
 * @return An array of strings with the values returned by the JS frame parser.
 */
QStringList JSON::FrameParser::parse(const QString &frame,
                                     const QString &separator)
{
  // Construct function arguments
  QJSValueList args;
  args << frame << separator;

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
 * @brief Removes the selected text from the code editor widget and copies it
 *        into the system's clipboard.
 */
void JSON::FrameParser::cut()
{
  m_textEdit.cut();
}

/**
 * @brief Undoes the last user's action.
 */
void JSON::FrameParser::undo()
{
  m_textEdit.undo();
}

/**
 * @brief Redoes an undone action.
 */
void JSON::FrameParser::redo()
{
  m_textEdit.redo();
}

/**
 * @brief Opens a website with documentation relevant to the frame parser.
 */
void JSON::FrameParser::help()
{
  // clang-format off
  const auto url = QStringLiteral("https://github.com/Serial-Studio/Serial-Studio/wiki/Project-Editor#frame-parser-function-view");
  QDesktopServices::openUrl(QUrl(url));
  // clang-format on
}

/**
 * @brief Copies the selected text into the system's clipboard.
 */
void JSON::FrameParser::copy()
{
  m_textEdit.copy();
}

/**
 * @brief Pastes data from the system's clipboard into the code editor.
 */
void JSON::FrameParser::paste()
{
  if (m_textEdit.canPaste())
  {
    m_textEdit.paste();
    Q_EMIT modifiedChanged();
  }
}

/**
 * @brief Writes the current code into the project file.
 */
void JSON::FrameParser::apply()
{
  save(true);
}

/**
 * @brief Loads the default frame parser function into the code editor.
 */
void JSON::FrameParser::reload()
{
  // Document has been modified, ask user if he/she wants to continue
  if (m_textEdit.document()->isModified())
  {
    auto ret = Misc::Utilities::showMessageBox(
        tr("The document has been modified!"),
        tr("Are you sure you want to continue?"), qAppName(),
        QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::No)
      return;
  }

  // Load default template
  m_textEdit.setPlainText(defaultCode());
  save(true);
}

/**
 * @brief Prompts the user to select a JS file to import into the code editor.
 */
void JSON::FrameParser::import()
{
  // Document has been modified, ask user if he/she wants to continue
  if (m_textEdit.document()->isModified())
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
      auto data = file.readAll();
      m_textEdit.setPlainText(QString::fromUtf8(data));
      save(true);
    }
  }
}

/**
 * @brief Selects all the text present in the code editor widget.
 */
void JSON::FrameParser::selectAll()
{
  m_textEdit.selectAll();
}

/**
 * @brief Modifies the palette of the code editor to match the colorscheme
 *        of the currently loaded theme file.
 */
void JSON::FrameParser::onThemeChanged()
{
  QPalette palette;
  auto theme = &Misc::ThemeManager::instance();
  palette.setColor(QPalette::Text, theme->getColor("text"));
  palette.setColor(QPalette::Base, theme->getColor("base"));
  palette.setColor(QPalette::Button, theme->getColor("button"));
  palette.setColor(QPalette::Window, theme->getColor("window"));
  palette.setColor(QPalette::Highlight, theme->getColor("highlight"));
  palette.setColor(QPalette::HighlightedText,
                   theme->getColor("highlighted_text"));
  palette.setColor(QPalette::PlaceholderText,
                   theme->getColor("placeholder_text"));
  m_textEdit.setPalette(palette);
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
  if (loadScript(m_textEdit.toPlainText()))
  {
    m_textEdit.document()->setModified(false);
    ProjectModel::instance().setFrameParserCode(m_textEdit.toPlainText());

    // Show save messagebox
    if (!silent)
      Misc::Utilities::showMessageBox(
          tr("Frame parser code updated successfully!"),
          tr("No errors have been detected in the code."));

    // Everything good
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
  m_engine.installExtensions(QJSEngine::ConsoleExtension
                             | QJSEngine::GarbageCollectionExtension);

  // Check if there are no general JS errors
  QStringList errors;
  m_engine.evaluate(script, "", 1, &errors);

  // Check if parse() function exists
  auto fun = m_engine.globalObject().property("parse");
  if (fun.isNull() || !fun.isCallable())
  {
    Misc::Utilities::showMessageBox(
        tr("Frame parser error!"),
        tr("No parse() function has been declared!"));
    return false;
  }

  // Try to run parse() function
  QJSValueList args = {"", ","};
  auto ret = fun.call(args);

  // Error on engine evaluation
  if (!errors.isEmpty())
  {
    Misc::Utilities::showMessageBox(
        tr("Frame parser syntax error!"),
        tr("Error on line %1.").arg(errors.first()));
    return false;
  }

  // Error on function execution
  else if (ret.isError())
  {
    QString errorStr;
    switch (ret.errorType())
    {
      case QJSValue::GenericError:
        errorStr = tr("Generic error");
        break;
      case QJSValue::EvalError:
        errorStr = tr("Evaluation error");
        break;
      case QJSValue::RangeError:
        errorStr = tr("Range error");
        break;
      case QJSValue::ReferenceError:
        errorStr = tr("Reference error");
        break;
      case QJSValue::SyntaxError:
        errorStr = tr("Syntax error");
        break;
      case QJSValue::TypeError:
        errorStr = tr("Type error");
        break;
      case QJSValue::URIError:
        errorStr = tr("URI error");
        break;
      default:
        errorStr = tr("Unknown error");
        break;
    }

    Misc::Utilities::showMessageBox(tr("Frame parser error detected!"),
                                    errorStr);
    return false;
  }

  // We have reached this point without any errors, set function caller
  m_parseFunction = fun;
  return true;
}

/**
 * @brief Loads the code stored in the project file into the code editor.
 */
void JSON::FrameParser::readCode()
{
  m_textEdit.setPlainText(ProjectModel::instance().frameParserCode());
  m_textEdit.document()->setModified(false);
  loadScript(m_textEdit.toPlainText());
}

/**
 * @brief Renders the widget as a pixmap, which is then painted in the QML
 *        user interface.
 */
void JSON::FrameParser::renderWidget()
{
  if (isVisible())
  {
    m_pixmap = m_textEdit.grab();
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
    m_textEdit.setFixedSize(width(), height());
    renderWidget();
  }
}

/**
 * Displays the pixmap generated in the @c update() function in the QML
 * interface through the given @a painter pointer.
 */
void JSON::FrameParser::paint(QPainter *painter)
{
  if (painter)
    painter->drawPixmap(0, 0, m_pixmap);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::keyPressEvent(QKeyEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, keyPressEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::keyReleaseEvent(QKeyEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, keyReleaseEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::inputMethodEvent(QInputMethodEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, inputMethodEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::focusInEvent(QFocusEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, focusInEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::focusOutEvent(QFocusEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, focusOutEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::mousePressEvent(QMouseEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, mousePressEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::mouseMoveEvent(QMouseEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, mouseMoveEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::mouseReleaseEvent(QMouseEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, mouseReleaseEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::mouseDoubleClickEvent(QMouseEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, mouseDoubleClickEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::wheelEvent(QWheelEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, wheelEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::dragEnterEvent(QDragEnterEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, dragEnterEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::dragMoveEvent(QDragMoveEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, dragMoveEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::dragLeaveEvent(QDragLeaveEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, dragLeaveEvent, event);
}

/**
 * Passes the given @param event to the contained widget (if any).
 */
void JSON::FrameParser::dropEvent(QDropEvent *event)
{
  DW_EXEC_EVENT(&m_textEdit, dropEvent, event);
}
