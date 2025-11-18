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
#include <QInputDialog>
#include <QLineNumberArea>
#include <QDesktopServices>
#include <QRegularExpression>
#include <QJavascriptHighlighter>

#include "JSON/FrameParser.h"
#include "JSON/ProjectModel.h"
#include <QtGui/qshortcut.h>

#include "Misc/Utilities.h"
#include "Misc/Translator.h"
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
  , m_testDialog(this, nullptr)
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

  // Set widget palette
  onThemeChanged();
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged,
          this, &JSON::FrameParser::onThemeChanged);

  // Load template names
  loadTemplateNames();
  loadDefaultTemplate();
  connect(&Misc::Translator::instance(), &Misc::Translator::languageChanged,
          this, &JSON::FrameParser::loadTemplateNames);

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

  // Load template code
  reload();
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
 * @brief Retrieves the JavaScript code for the currently selected template
 *
 * Loads and returns the JavaScript parser code from the template file
 * corresponding to the current template index.
 *
 * The template files are stored in the application's resource system under
 * ":/rcc/scripts/".
 *
 * The function constructs the file path using the template filename, reads
 * the entire file contents, and returns it as a QString.
 *
 * @return QString containing the JavaScript parser code from the selected
 *         template file.
 */
QString JSON::FrameParser::templateCode() const
{
  QString code;
  auto path = QString(":/rcc/scripts/%1").arg(m_templateFiles[m_templateIdx]);

  QFile file(path);
  if (file.open(QFile::ReadOnly))
  {
    code = QString::fromUtf8(file.readAll());
    file.close();
  }

  return code;
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
 * @brief Validates and loads the JavaScript frame parser code
 *
 * Performs comprehensive validation of the JavaScript parser code to ensure it:
 * - Contains no syntax exceptionStackTrace
 * - Declares a callable 'parse' function
 * - Uses the correct function signature (single parameter)
 * - Can be executed by the JavaScript engine
 *
 * The validation process checks for:
 * 1. JavaScript syntax exceptionStackTrace during compilation
 * 2. Existence of the 'parse' function in global scope
 * 3. Correct function signature (parse(frame) not parse(frame, separator))
 * 4. Function callability
 *
 * If validation succeeds, stores the parse function for later use.
 * If validation fails, displays an appropriate error message to the user.
 *
 * @param script The JavaScript parser code to validate and load
 *
 * @return true if the code is valid and the parse function is ready to use,
 *         false if validation failed
 *
 * @note Legacy two-parameter parse functions (parse(frame, separator)) are
 *       no longer supported and will trigger a warning
 *
 * @see evaluate()
 * @see m_parseFunction
 */
bool JSON::FrameParser::loadScript(const QString &script)
{
  // Reset any previously loaded function
  m_parseFunction = QJSValue();

  // Ensure that engine is configured correctly
  m_engine.installExtensions(QJSEngine::AllExtensions);

  // Check for JavaScript syntax errors
  QStringList exceptionStackTrace;
  auto result = m_engine.evaluate(script, "parser.js", 1, &exceptionStackTrace);
  if (result.isError())
  {
    QString errorMsg = result.property("message").toString();
    int lineNumber = result.property("lineNumber").toInt();
    Misc::Utilities::showMessageBox(
        tr("JavaScript Syntax Error"),
        tr("The parser code contains a syntax error at line %1:\n\n%2")
            .arg(lineNumber)
            .arg(errorMsg),
        QMessageBox::Critical);
    return false;
  }

  // Get list of exceptions
  if (!exceptionStackTrace.isEmpty())
  {
    Misc::Utilities::showMessageBox(
        tr("JavaScript Exception Occurred"),
        tr("The parser code triggered the following exceptions:\n\n%1")
            .arg(exceptionStackTrace.join("\n")),
        QMessageBox::Critical);
    return false;
  }

  // Verify that the 'parse' function exists
  auto parseFunction = m_engine.globalObject().property("parse");
  if (parseFunction.isNull())
  {
    Misc::Utilities::showMessageBox(
        tr("Missing Parse Function"),
        tr("The 'parse' function is not defined in the script.\n\n"
           "Please ensure your code includes:\n"
           "function parse(frame) { ... }"),
        QMessageBox::Critical);
    return false;
  }

  // Verify that the 'parse' function is callable
  if (!parseFunction.isCallable())
  {
    Misc::Utilities::showMessageBox(
        tr("Invalid Parse Function"),
        tr("The 'parse' property exists but is not a callable function.\n\n"
           "Please ensure 'parse' is declared as a function."),
        QMessageBox::Critical);
    return false;
  }

  // Validate function signature using regex
  static QRegularExpression functionRegex(
      R"(\bfunction\s+parse\s*\(\s*([a-zA-Z_$][a-zA-Z0-9_$]*)(\s*,\s*([a-zA-Z_$][a-zA-Z0-9_$]*))?\s*\))");
  auto match = functionRegex.match(script);
  if (!match.hasMatch())
  {
    Misc::Utilities::showMessageBox(
        tr("Invalid Function Declaration"),
        tr("No valid 'parse' function declaration found.\n\n"
           "Expected format:\n"
           "function parse(frame) { ... }"),
        QMessageBox::Critical);
    return false;
  }

  // Extract function parameters
  QString firstArg = match.captured(1);
  QString secondArg = match.captured(3);

  // Check for empty or invalid first argument
  if (firstArg.isEmpty())
  {
    Misc::Utilities::showMessageBox(
        tr("Invalid Function Parameter"),
        tr("The 'parse' function must have at least one parameter.\n\n"
           "Expected format:\n"
           "function parse(frame) { ... }"),
        QMessageBox::Critical);
    return false;
  }

  // Check for deprecated two-parameter format
  if (!secondArg.isEmpty())
  {
    Misc::Utilities::showMessageBox(
        tr("Deprecated Function Signature"),
        tr("The 'parse' function uses the old two-parameter format: parse(%1, "
           "%2)\n\n"
           "This format is no longer supported. Please update to the new "
           "single-parameter format:\n"
           "function parse(%1) { ... }\n\n"
           "The separator parameter is no longer needed.")
            .arg(firstArg, secondArg),
        QMessageBox::Warning);
    return false;
  }

  // Test execute the parse function with sample data to catch runtime errors
  QJSValueList args;
  args << QJSValue("");
  auto testResult = parseFunction.call(args);
  if (testResult.isError())
  {
    QString errorMsg = testResult.property("message").toString();
    int lineNumber = testResult.property("lineNumber").toInt();

    Misc::Utilities::showMessageBox(
        tr("Parse Function Runtime Error"),
        tr("The parse function contains an error at line %1:\n\n%2\n\n"
           "Please fix the error in the function body.")
            .arg(lineNumber)
            .arg(errorMsg),
        QMessageBox::Critical);

    return false;
  }

  // All validations passed, store the function for use
  m_parseFunction = parseFunction;
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
  if (save(true))
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
        tr("The document has been modified."),
        tr("Are you sure you want to continue?"), QMessageBox::Question,
        qAppName(), QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::No)
      return;
  }

  // Load default template
  loadDefaultTemplate();
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
 * @brief Evaluates the JavaScript parser code for syntax errors
 *
 * Attempts to load and compile the current JavaScript parser code to detect
 * any syntax errors before the code is used for actual frame parsing.
 * If successful, displays a confirmation message to the user.
 *
 * This validation step helps users identify coding mistakes early, preventing
 * runtime errors when processing incoming data frames.
 *
 * @note This only validates JavaScript syntax, not logical correctness or
 *       runtime behavior. The parser may still fail during actual use if
 *       the logic is incorrect. Use the @c testWithSampleData() function
 *       to validate logic.
 *
 * @see loadScript()
 * @see text()
 */
void JSON::FrameParser::evaluate()
{
  if (loadScript(text()))
  {
    Misc::Utilities::showMessageBox(
        tr("Code Validation Successful"),
        tr("No syntax errors detected in the parser code."),
        QMessageBox::Information);
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
 * @brief Sets the frame template index, prompting user confirmation if
 *        the document is modified.
 */
void JSON::FrameParser::selectTemplate()
{
  // Show combobox dialog to get template selection
  bool ok;
  auto name
      = QInputDialog::getItem(Q_NULLPTR, tr("Select Frame Parser Template"),
                              tr("Choose a template to load:"), m_templateNames,
                              m_templateIdx, false, &ok);

  // User cancelled the dialog
  if (!ok)
    return;

  // Get the selected index
  int idx = m_templateNames.indexOf(name);
  if (idx < 0)
    return;

  // Load template
  setTemplateIdx(idx);
}

/**
 * @brief Opens the frame parser test dialog with the current script.
 *
 * Loads the current script text and displays the test dialog if loading
 * succeeds. Clears any previous test results before showing.
 */
void JSON::FrameParser::testWithSampleData()
{
  if (loadScript(text()))
  {
    m_testDialog.clear();
    m_testDialog.showNormal();
  }
}

/**
 * @brief Loads the default comma-separated values (CSV) template
 *
 * Sets the current template to the comma-separated parser by finding its index
 * in the template files list and updating the template index accordingly.
 *
 * This is typically called during initialization or when resetting the parser
 * to default settings.
 *
 * The CSV template is chosen as the default because it's one of the most common
 * and simple data formats for serial communication.
 *
 * @note If "comma_separated.js" is not found in m_templateFiles, the index will
 *       be -1, which may result in undefined behavior. Ensure loadTemplates()
 *       is called before this function.
 *
 * @see setTemplateIdx()
 * @see loadTemplates()
 */
void JSON::FrameParser::loadDefaultTemplate()
{
  const auto idx = m_templateFiles.indexOf("comma_separated.js");
  setTemplateIdx(idx);
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
 * @brief Loads the available JavaScript frame parser templates
 *
 * Initializes the list of template files and their corresponding user-friendly
 * display names. The templates provide pre-written JavaScript code for parsing
 * various serial communication protocols and data formats.
 *
 * The template files are loaded from the application resources and include
 * parsers for common protocols like NMEA, Modbus, CAN bus, MAVLink, as well
 * as generic formats like CSV, JSON, XML, and various binary encodings.
 *
 * Template files and display names are stored in parallel arrays with matching
 * indices, allowing UI components to display localized names while referencing
 * the correct template file.
 *
 * @note Template names are marked for translation using tr() to support
 *       internationalization
 *
 * @note The order of m_templateFiles and m_templateNames must be kept in sync
 */
void JSON::FrameParser::loadTemplateNames()
{
  m_templateFiles.clear();
  m_templateFiles = {"at_commands.js",
                     "base64_encoded.js",
                     "binary_tlv.js",
                     "can_bus.js",
                     "cobs_encoded.js",
                     "comma_separated.js",
                     "fixed_width_fields.js",
                     "hexadecimal_bytes.js",
                     "ini_config.js",
                     "json_data.js",
                     "key_value_pairs.js",
                     "mavlink.js",
                     "messagepack.js",
                     "modbus_ascii.js",
                     "modbus_rtu.js",
                     "nmea_0183.js",
                     "nmea_2000.js",
                     "pipe_delimited.js",
                     "raw_bytes.js",
                     "rtcm_corrections.js",
                     "semicolon_separated.js",
                     "sirf_binary.js",
                     "slip_encoded.js",
                     "tab_separated.js",
                     "ubx_ublox.js",
                     "url_encoded.js",
                     "xml_data.js",
                     "yaml_data.js"};

  m_templateNames.clear();
  m_templateNames = {tr("AT command responses"),
                     tr("Base64-encoded data"),
                     tr("Binary TLV (Tag-Length-Value)"),
                     tr("CAN bus frames"),
                     tr("COBS-encoded frames"),
                     tr("Comma-separated data"),
                     tr("Fixed-width fields"),
                     tr("Hexadecimal bytes"),
                     tr("INI/config format"),
                     tr("JSON data"),
                     tr("Key-value pairs"),
                     tr("MAVLink messages"),
                     tr("MessagePack data"),
                     tr("Modbus ASCII frames"),
                     tr("Modbus RTU frames"),
                     tr("NMEA 0183 sentences"),
                     tr("NMEA 2000 messages"),
                     tr("Pipe-delimited data"),
                     tr("Raw bytes"),
                     tr("RTCM corrections"),
                     tr("Semicolon-separated data"),
                     tr("SiRF binary protocol"),
                     tr("SLIP-encoded frames"),
                     tr("Tab-separated data"),
                     tr("UBX protocol (u-blox)"),
                     tr("URL-encoded data"),
                     tr("XML data"),
                     tr("YAML data")};
}

/**
 * @brief Sets the frame template index, prompting user confirmation if
 *        modified.
 *
 * If the current document has been modified, prompts the user before switching
 * templates. Loads the new template code and saves it automatically.
 *
 * @param idx The template index to set
 */
void JSON::FrameParser::setTemplateIdx(const int idx)
{
  // Get current code and template code
  auto code = m_widget.toPlainText().simplified();
  auto currentTemplateCode = templateCode().simplified();

  // Don't do anything if the template index is the same
  if (idx == m_templateIdx && code == currentTemplateCode)
    return;

  // Document has been modified, ask user if he/she wants to continue
  if (isModified() || (!code.isEmpty() && code != currentTemplateCode))
  {
    auto ret = Misc::Utilities::showMessageBox(
        tr("Loading a template will replace your current code."),
        tr("Are you sure you want to continue?"), QMessageBox::Question,
        qAppName(), QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::No)
      return;
  }

  // Load template code
  m_templateIdx = idx;
  m_widget.setPlainText(templateCode());
  (void)save(true);

  // Mark the project file as modified
  JSON::ProjectModel::instance().setModified(true);
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
