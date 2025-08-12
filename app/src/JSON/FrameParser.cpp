/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
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
#include <QtGui/qshortcut.h>

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
          [=, this] { Q_EMIT modifiedChanged(); });

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

  // Collect JS garbage at 1 Hz
  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::timeout1Hz,
          &m_engine, &QJSEngine::collectGarbage);

  // Configure render loop
  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::uiTimeout, this,
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
 * @brief Executes the current frame parser function over UTF-8 text data.
 *
 * This version is used when the frame is already a decoded QString
 * (e.g., ASCII or UTF-8 text). It avoids any binary conversion and
 * sends the string directly to the JavaScript parser function.
 *
 * @param frame Decoded UTF-8 string frame (e.g., "value1,value2,value3").
 * @return An array of strings returned by the JS parser.
 */
QStringList JSON::FrameParser::parse(const QString &frame)
{
  QJSValueList args;
  args << frame;

  return m_parseFunction.call(args).toVariant().toStringList();
}

/**
 * @brief Executes the current frame parser function over the input binary data.
 *
 * @param frame Binary frame data (e.g., from serial input).
 * @return Parsed string fields returned by the JS frame parser.
 */
QStringList JSON::FrameParser::parse(const QByteArray &frame)
{
  QJSValue jsArray = m_engine.newArray(frame.size());
  const auto *data = reinterpret_cast<const quint8 *>(frame.constData());
  for (int i = 0; i < frame.size(); ++i)
    jsArray.setProperty(i, data[i]);

  QJSValueList args;
  args << jsArray;

  return m_parseFunction.call(args).toVariant().toStringList();
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
    auto &model = ProjectModel::instance();
    bool prevModif = model.modified();
    model.setFrameParserCode(text());
    m_widget.document()->setModified(false);
    m_widget.document()->clearUndoRedoStacks();

    // Show save messagebox
    if (!silent)
      Misc::Utilities::showMessageBox(
          tr("Frame parser code updated successfully!"),
          tr("No errors have been detected in the code."),
          QMessageBox::Information);
    else
      model.setModified(prevModif);

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
        tr("The 'parse' function is not declared or is not callable!"),
        QMessageBox::Critical);
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
          tr("No valid 'parse' function declaration found in the script!"),
          QMessageBox::Critical);
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
              .arg(firstArg, secondArg),
          QMessageBox::Warning);
      return false;
    }
  }

  // Function doesn't match the expected declaration
  else
  {
    Misc::Utilities::showMessageBox(
        tr("Frame parser error!"),
        tr("No valid 'parse' function declaration found in the script!"),
        QMessageBox::Critical);
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
        tr("Are you sure you want to continue?"), QMessageBox::Question,
        qAppName(), QMessageBox::Yes | QMessageBox::No);
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
        tr("Are you sure you want to continue?"), QMessageBox::Question,
        qAppName(), QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::No)
      return;
  }

  auto *dialog
      = new QFileDialog(nullptr, tr("Select Javascript file to import"),
                        QDir::homePath(), "*.js");

  dialog->setFileMode(QFileDialog::ExistingFile);
  dialog->setOption(QFileDialog::DontUseNativeDialog);
  connect(dialog, &QFileDialog::fileSelected, this,
          [this, dialog](const QString &path) {
            if (path.isEmpty())
            {
              dialog->deleteLater();
              return;
            }

            QFile file(path);
            if (file.open(QFile::ReadOnly))
            {
              m_widget.setPlainText(QString::fromUtf8(file.readAll()));
              file.close();
              (void)save(true);
            }

            dialog->deleteLater();
          });

  dialog->open();
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
  const auto name = t->parameters().value("code-editor-theme").toString();
  const auto path = QString(":/rcc/themes/code-editor/%1.xml").arg(name);

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
  forceActiveFocus();
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
