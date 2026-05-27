/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "MQTT/PublisherScriptEditor.h"

#  include <lauxlib.h>
#  include <lua.h>
#  include <lualib.h>

#  include <QFile>
#  include <QHBoxLayout>
#  include <QJavascriptHighlighter>
#  include <QJSEngine>
#  include <QLuaCompleter>
#  include <QLuaHighlighter>
#  include <QMenu>
#  include <QRegularExpression>
#  include <QShortcut>
#  include <QTextCursor>

#  include "DataModel/Editors/CodeFormatter.h"
#  include "DataModel/Scripting/LuaCompat.h"
#  include "DataModel/Scripting/ScriptTemplates.h"
#  include "Misc/CommonFonts.h"
#  include "Misc/ThemeManager.h"
#  include "Misc/Translator.h"
#  include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the MQTT publisher script editor dialog.
 */
MQTT::PublisherScriptEditor::PublisherScriptEditor(QWidget* parent)
  : QDialog(parent), m_language(SerialStudio::JavaScript), m_hexInput(false)
{
  setWindowTitle(tr("MQTT Publisher Script"));
  setMinimumSize(720, 560);
  resize(820, 620);

  buildEditorWidgets();

  auto* toolbarLayout = buildToolbarLayout();
  auto* testLayout    = buildTestLayout();
  auto* buttonLayout  = buildButtonLayout();

  auto* mainLayout = new QVBoxLayout(this);
  mainLayout->addLayout(toolbarLayout);
  mainLayout->addWidget(m_editor, 1);
  mainLayout->addLayout(testLayout);
  mainLayout->addLayout(buttonLayout);

  wireSignals();
  installShortcuts();

  onThemeChanged();
  applyLanguage(SerialStudio::JavaScript);
}

/**
 * @brief Allocates the editor + combobox widgets and seeds the template list.
 */
void MQTT::PublisherScriptEditor::buildEditorWidgets()
{
  m_editor = new QCodeEditor(this);
  m_editor->setTabReplace(true);
  m_editor->setTabReplaceSize(2);
  m_editor->setAutoIndentation(true);
  m_editor->setFont(Misc::CommonFonts::instance().monoFont());
  m_editor->setMinimumHeight(220);
  m_editor->setLayoutDirection(Qt::LeftToRight);

  m_languageCombo = new QComboBox(this);
  m_languageCombo->addItems({tr("JavaScript"), tr("Lua")});

  m_templateCombo = new QComboBox(this);
  buildTemplates();

  m_testInput = new QLineEdit(this);
  m_testInput->setPlaceholderText(tr("Sample frame bytes (text or hex)"));
  m_testOutput = new QLineEdit(QStringLiteral("--"), this);
  m_testOutput->setReadOnly(true);
  m_testOutput->setMinimumWidth(160);

  m_hexCheckBox = new QCheckBox(tr("Hex"), this);
  m_testButton  = new QPushButton(tr("Test"), this);
  m_clearButton = new QPushButton(tr("Clear"), this);

  m_applyButton  = new QPushButton(tr("Apply"), this);
  m_cancelButton = new QPushButton(tr("Cancel"), this);
}

/**
 * @brief Builds the language + template selector toolbar row.
 */
QHBoxLayout* MQTT::PublisherScriptEditor::buildToolbarLayout()
{
  auto* toolbarLayout = new QHBoxLayout();
  toolbarLayout->addWidget(new QLabel(tr("Language:"), this));
  toolbarLayout->addWidget(m_languageCombo);
  toolbarLayout->addSpacing(16);
  toolbarLayout->addWidget(new QLabel(tr("Template:"), this));
  toolbarLayout->addWidget(m_templateCombo, 1);
  return toolbarLayout;
}

/**
 * @brief Builds the input/output test row used to dry-run the script.
 */
QHBoxLayout* MQTT::PublisherScriptEditor::buildTestLayout()
{
  auto* testLayout = new QHBoxLayout();
  testLayout->addWidget(new QLabel(tr("Frame:"), this));
  testLayout->addWidget(m_testInput, 1);
  testLayout->addWidget(m_hexCheckBox);
  testLayout->addWidget(m_testButton);
  testLayout->addWidget(new QLabel(tr("Output:"), this));
  testLayout->addWidget(m_testOutput, 1);
  testLayout->addWidget(m_clearButton);
  return testLayout;
}

/**
 * @brief Builds the Apply/Cancel button row.
 */
QHBoxLayout* MQTT::PublisherScriptEditor::buildButtonLayout()
{
  auto* buttonLayout = new QHBoxLayout();
  buttonLayout->addStretch();
  buttonLayout->addWidget(m_applyButton);
  buttonLayout->addWidget(m_cancelButton);
  return buttonLayout;
}

/**
 * @brief Wires every widget signal needed by the dialog.
 */
void MQTT::PublisherScriptEditor::wireSignals()
{
  connect(m_applyButton, &QPushButton::clicked, this, &PublisherScriptEditor::onApply);
  connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
  connect(m_testButton, &QPushButton::clicked, this, &PublisherScriptEditor::onTest);
  connect(m_clearButton, &QPushButton::clicked, this, &PublisherScriptEditor::onClear);
  connect(m_testInput, &QLineEdit::returnPressed, this, &PublisherScriptEditor::onTest);
  connect(m_testInput, &QLineEdit::textChanged, this, &PublisherScriptEditor::onInputChanged);
  connect(m_hexCheckBox, &QCheckBox::checkStateChanged, this, &PublisherScriptEditor::onHexToggled);
  connect(m_templateCombo,
          QOverload<int>::of(&QComboBox::activated),
          this,
          &PublisherScriptEditor::onTemplateSelected);
  connect(m_languageCombo,
          QOverload<int>::of(&QComboBox::activated),
          this,
          &PublisherScriptEditor::onLanguageChanged);
  connect(&Misc::ThemeManager::instance(),
          &Misc::ThemeManager::themeChanged,
          this,
          &PublisherScriptEditor::onThemeChanged);
  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &PublisherScriptEditor::buildTemplates);
}

/**
 * @brief Installs the format/format-line shortcuts and the custom context menu.
 */
void MQTT::PublisherScriptEditor::installShortcuts()
{
  auto* formatShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_I), m_editor);
  formatShortcut->setContext(Qt::WidgetShortcut);
  connect(formatShortcut, &QShortcut::activated, this, &PublisherScriptEditor::onFormatLine);
  auto* formatAllShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_I), m_editor);
  formatAllShortcut->setContext(Qt::WidgetShortcut);
  connect(formatAllShortcut, &QShortcut::activated, this, &PublisherScriptEditor::onFormat);

  m_editor->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_editor,
          &QWidget::customContextMenuRequested,
          this,
          &PublisherScriptEditor::showEditorContextMenu);
}

//--------------------------------------------------------------------------------------------------
// Public interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens the dialog pre-populated with existing script code.
 */
void MQTT::PublisherScriptEditor::displayDialog(const QString& currentCode, int language)
{
  const int comboIdx = (language == SerialStudio::Lua) ? 1 : 0;
  m_languageCombo->blockSignals(true);
  m_languageCombo->setCurrentIndex(comboIdx);
  m_languageCombo->blockSignals(false);

  const int resolvedLanguage = (comboIdx == 1) ? SerialStudio::Lua : SerialStudio::JavaScript;
  applyLanguage(resolvedLanguage);

  if (currentCode.isEmpty())
    m_editor->setPlainText(defaultPlaceholder(resolvedLanguage));
  else
    m_editor->setPlainText(currentCode);

  const int tmplIdx = detectTemplate();
  m_templateCombo->setCurrentIndex(tmplIdx >= 0 ? tmplIdx + 1 : 0);

  m_testOutput->setText(QStringLiteral("--"));

  showNormal();
  raise();
  activateWindow();
  m_editor->setFocus();
}

/**
 * @brief Returns the current editor text.
 */
QString MQTT::PublisherScriptEditor::code() const
{
  return m_editor->toPlainText();
}

/**
 * @brief Returns the active scripting language (JavaScript or Lua).
 */
int MQTT::PublisherScriptEditor::language() const
{
  return m_language;
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Emits the script if mqtt() is defined, otherwise emits empty.
 */
void MQTT::PublisherScriptEditor::onApply()
{
  QString code = m_editor->toPlainText();
  if (!definesMqttFunction(code, m_language))
    code.clear();

  Q_EMIT scriptApplied(code, m_language);
  QDialog::accept();
}

/**
 * @brief Runs the script against the test input and shows the result.
 */
void MQTT::PublisherScriptEditor::onTest()
{
  const QString inputStr = m_testInput->text();
  if (inputStr.isEmpty()) {
    m_testOutput->setText(tr("Enter a frame"));
    return;
  }

  QByteArray frame;
  if (m_hexInput) {
    if (!validateHexInput(inputStr)) {
      m_testOutput->setText(tr("Invalid hex"));
      return;
    }

    frame = QByteArray::fromHex(inputStr.toLatin1());
  } else {
    frame = inputStr.toUtf8();
  }

  QString error;
  const QString result = runScript(m_editor->toPlainText(), m_language, frame, error);
  if (!error.isEmpty()) {
    m_testOutput->setText(error);
    return;
  }

  m_testOutput->setText(result);
}

/**
 * @brief Reformats the entire script.
 */
void MQTT::PublisherScriptEditor::onFormat()
{
  const auto lang         = (m_language == SerialStudio::Lua)
                            ? DataModel::CodeFormatter::Language::Lua
                            : DataModel::CodeFormatter::Language::JavaScript;
  const QString original  = m_editor->toPlainText();
  const QString formatted = DataModel::CodeFormatter::formatDocument(original, lang);
  if (formatted == original)
    return;

  QTextCursor cursor = m_editor->textCursor();
  const int savedPos = cursor.position();
  cursor.beginEditBlock();
  cursor.select(QTextCursor::Document);
  cursor.insertText(formatted);
  cursor.endEditBlock();

  cursor.setPosition(qMin(savedPos, formatted.size()));
  m_editor->setTextCursor(cursor);
}

/**
 * @brief Reformats the selected lines, or the current line if nothing is selected.
 */
void MQTT::PublisherScriptEditor::onFormatLine()
{
  const auto lang        = (m_language == SerialStudio::Lua)
                           ? DataModel::CodeFormatter::Language::Lua
                           : DataModel::CodeFormatter::Language::JavaScript;
  const QString original = m_editor->toPlainText();

  QTextCursor cursor = m_editor->textCursor();
  QTextCursor first(m_editor->document());
  first.setPosition(qMin(cursor.selectionStart(), cursor.selectionEnd()));
  QTextCursor last(m_editor->document());
  last.setPosition(qMax(cursor.selectionStart(), cursor.selectionEnd()));

  const int firstLine = first.blockNumber();
  const int lastLine  = last.blockNumber();
  const QString formatted =
    DataModel::CodeFormatter::formatLineRange(original, lang, firstLine, lastLine);
  if (formatted == original)
    return;

  const int savedPos = cursor.position();
  cursor.beginEditBlock();
  cursor.select(QTextCursor::Document);
  cursor.insertText(formatted);
  cursor.endEditBlock();

  cursor.setPosition(qMin(savedPos, formatted.size()));
  m_editor->setTextCursor(cursor);
}

/**
 * @brief Builds and shows the editor's right-click menu with format entries.
 */
void MQTT::PublisherScriptEditor::showEditorContextMenu(const QPoint& localPos)
{
  QMenu* menu = m_editor->createStandardContextMenu();
  menu->addSeparator();
  menu->addAction(tr("Format Document\tCtrl+Shift+I"), this, &PublisherScriptEditor::onFormat);
  menu->addAction(tr("Format Selection\tCtrl+I"), this, &PublisherScriptEditor::onFormatLine);
  menu->exec(m_editor->viewport()->mapToGlobal(localPos));
  menu->deleteLater();
}

/**
 * @brief Clears test fields and resets the editor to the default placeholder.
 */
void MQTT::PublisherScriptEditor::onClear()
{
  m_editor->setPlainText(defaultPlaceholder(m_language));
  m_testInput->clear();
  m_testOutput->setText(QStringLiteral("--"));
  m_templateCombo->setCurrentIndex(0);
}

/**
 * @brief Loads the selected template's code (Lua or JS) into the editor.
 */
void MQTT::PublisherScriptEditor::onTemplateSelected(int index)
{
  if (index <= 0 || index > m_templates.size())
    return;

  const auto& tmpl   = m_templates[index - 1];
  const QString code = (m_language == SerialStudio::Lua) ? tmpl.luaCode : tmpl.jsCode;
  m_editor->setPlainText(code);
}

/**
 * @brief Swaps the highlighter and, when recognised, the template variant.
 */
void MQTT::PublisherScriptEditor::onLanguageChanged(int index)
{
  const int newLang = (index == 1) ? SerialStudio::Lua : SerialStudio::JavaScript;
  if (newLang == m_language)
    return;

  const int tmplIdx = detectTemplate();

  applyLanguage(newLang);

  if (tmplIdx >= 0) {
    const auto& tmpl   = m_templates[tmplIdx];
    const QString code = (newLang == SerialStudio::Lua) ? tmpl.luaCode : tmpl.jsCode;
    m_editor->setPlainText(code);
    m_templateCombo->setCurrentIndex(tmplIdx + 1);
    return;
  }

  // No template match: swap the placeholder if the editor is at one
  const QString trimmed = m_editor->toPlainText().trimmed();
  const int other = (newLang == SerialStudio::Lua) ? SerialStudio::JavaScript : SerialStudio::Lua;
  if (trimmed == defaultPlaceholder(other).trimmed())
    m_editor->setPlainText(defaultPlaceholder(newLang));
}

/**
 * @brief Toggles hex-mode framing of the test input.
 */
void MQTT::PublisherScriptEditor::onHexToggled(Qt::CheckState state)
{
  m_hexInput = (state == Qt::Checked);
  m_testOutput->setText(QStringLiteral("--"));
}

/**
 * @brief Live-formats hex input as the user types.
 */
void MQTT::PublisherScriptEditor::onInputChanged(const QString& text)
{
  if (!m_hexInput)
    return;

  const QString formatted = formatHexInput(text);
  if (formatted == text)
    return;

  const int pos = m_testInput->cursorPosition();
  m_testInput->blockSignals(true);
  m_testInput->setText(formatted);
  m_testInput->setCursorPosition(qMin(pos, formatted.size()));
  m_testInput->blockSignals(false);
}

/**
 * @brief Updates the editor colour scheme to match the active theme.
 */
void MQTT::PublisherScriptEditor::onThemeChanged()
{
  static const auto* t = &Misc::ThemeManager::instance();
  const auto name      = t->parameters().value(QStringLiteral("code-editor-theme")).toString();
  const auto path =
    name.startsWith('/') ? name : QStringLiteral(":/themes/code-editor/%1.xml").arg(name);

  QFile file(path);
  if (file.open(QFile::ReadOnly)) {
    m_style.load(QString::fromUtf8(file.readAll()));
    m_editor->setSyntaxStyle(&m_style);
    file.close();
  }
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Switches the syntax highlighter and auto-completer for the language.
 */
void MQTT::PublisherScriptEditor::applyLanguage(int language)
{
  m_language = language;
  if (language == SerialStudio::Lua) {
    m_editor->setHighlighter(new QLuaHighlighter());
    m_editor->setCompleter(new QLuaCompleter(m_editor));
  } else {
    m_editor->setHighlighter(new QJavascriptHighlighter());
    m_editor->setCompleter(nullptr);
  }
}

/**
 * @brief Returns the default editor placeholder comment for the language.
 */
QString MQTT::PublisherScriptEditor::defaultPlaceholder(int language)
{
  if (language == SerialStudio::Lua)
    return tr("--\n"
              "-- Define a mqtt(frame) function that receives the raw bytes\n"
              "-- of one parsed frame and returns the payload to publish to\n"
              "-- the broker, or nil to skip this frame.\n"
              "--\n"
              "-- The frame argument matches what the Frame Parser script\n"
              "-- sees: a Lua string holding the bytes between FrameReader\n"
              "-- delimiters.\n"
              "--\n"
              "-- Example:\n"
              "--   function mqtt(frame)\n"
              "--     return frame    -- forward as-is\n"
              "--   end\n"
              "--\n"
              "-- Use the Template dropdown for ready-made examples.\n"
              "--\n");

  return tr("/*\n"
            " * Define a mqtt(frame) function that receives the raw bytes\n"
            " * of one parsed frame and returns the payload to publish to\n"
            " * the broker, or null to skip this frame.\n"
            " *\n"
            " * The frame argument matches what the Frame Parser script\n"
            " * sees: a string holding the bytes between FrameReader\n"
            " * delimiters.\n"
            " *\n"
            " * Example:\n"
            " *   function mqtt(frame) {\n"
            " *     return frame;   // forward as-is\n"
            " *   }\n"
            " *\n"
            " * Use the Template dropdown for ready-made examples.\n"
            " */");
}

/**
 * @brief Returns true if the code defines a callable mqtt() global.
 */
bool MQTT::PublisherScriptEditor::definesMqttFunction(const QString& code, int language)
{
  if (code.trimmed().isEmpty())
    return false;

  if (language == SerialStudio::Lua) {
    lua_State* L = luaL_newstate();
    if (!L)
      return false;

    static const luaL_Reg kSafeLibs[] = {
      {    "_G",   luaopen_base},
      { "table",  luaopen_table},
      {"string", luaopen_string},
      {  "math",   luaopen_math},
      { nullptr,        nullptr}
    };

    for (const luaL_Reg* lib = kSafeLibs; lib->func; ++lib) {
      luaL_requiref(L, lib->name, lib->func, 1);
      lua_pop(L, 1);
    }

    DataModel::installLuaCompat(L);

    const QByteArray utf8 = code.toUtf8();
    if (luaL_dostring(L, utf8.constData()) != LUA_OK) {
      lua_close(L);
      return false;
    }

    lua_getglobal(L, "mqtt");
    const bool hasFn = lua_isfunction(L, -1);
    lua_close(L);
    return hasFn;
  }

  QJSEngine jsEngine;
  jsEngine.installExtensions(QJSEngine::ConsoleExtension | QJSEngine::GarbageCollectionExtension);
  const auto evalResult = jsEngine.evaluate(code);
  if (evalResult.isError())
    return false;

  const auto fn = jsEngine.globalObject().property(QStringLiteral("mqtt"));
  return fn.isCallable();
}

/**
 * @brief Returns the template index matching the editor text, or -1.
 */
int MQTT::PublisherScriptEditor::detectTemplate() const
{
  const QString current = m_editor->toPlainText().trimmed();
  if (current.isEmpty())
    return -1;

  for (int i = 0; i < m_templates.size(); ++i) {
    if (m_templates[i].luaCode.trimmed() == current)
      return i;

    if (m_templates[i].jsCode.trimmed() == current)
      return i;
  }

  return -1;
}

/**
 * @brief Runs mqtt(frame) in a disposable engine and returns the published payload.
 */
QString MQTT::PublisherScriptEditor::runScript(const QString& code,
                                               int language,
                                               const QByteArray& frame,
                                               QString& errorOut)
{
  if (code.trimmed().isEmpty()) {
    errorOut = tr("Script is empty");
    return {};
  }

  if (language == SerialStudio::Lua) {
    lua_State* L = luaL_newstate();
    if (!L) {
      errorOut = tr("Lua engine error");
      return {};
    }

    static const luaL_Reg kSafeLibs[] = {
      {    "_G",   luaopen_base},
      { "table",  luaopen_table},
      {"string", luaopen_string},
      {  "math",   luaopen_math},
      {    "os",     luaopen_os},
      { nullptr,        nullptr}
    };

    for (const luaL_Reg* lib = kSafeLibs; lib->func; ++lib) {
      luaL_requiref(L, lib->name, lib->func, 1);
      lua_pop(L, 1);
    }

    DataModel::installLuaCompat(L);

    const QByteArray utf8 = code.toUtf8();
    if (luaL_dostring(L, utf8.constData()) != LUA_OK) {
      errorOut = tr("Error: %1").arg(QString::fromUtf8(lua_tostring(L, -1)));
      lua_close(L);
      return {};
    }

    lua_getglobal(L, "mqtt");
    if (!lua_isfunction(L, -1)) {
      lua_close(L);
      errorOut = tr("mqtt() is not defined");
      return {};
    }

    lua_pushlstring(L, frame.constData(), static_cast<size_t>(frame.size()));
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
      errorOut = tr("Error: %1").arg(QString::fromUtf8(lua_tostring(L, -1)));
      lua_close(L);
      return {};
    }

    QString out;
    if (lua_isstring(L, -1)) {
      size_t len    = 0;
      const char* d = lua_tolstring(L, -1, &len);
      out           = QString::fromUtf8(d, static_cast<int>(len));
    } else if (lua_isnoneornil(L, -1)) {
      out = tr("(nil -- frame skipped)");
    } else {
      out = tr("(non-string return)");
    }

    lua_close(L);
    return out;
  }

  QJSEngine jsEngine;
  jsEngine.installExtensions(QJSEngine::ConsoleExtension | QJSEngine::GarbageCollectionExtension);
  const auto evalResult = jsEngine.evaluate(code);
  if (evalResult.isError()) {
    errorOut = tr("Error: %1").arg(evalResult.property("message").toString());
    return {};
  }

  auto fn = jsEngine.globalObject().property(QStringLiteral("mqtt"));
  if (!fn.isCallable()) {
    errorOut = tr("mqtt() is not defined");
    return {};
  }

  QJSValueList args;
  args << QJSValue(QString::fromUtf8(frame));
  const auto result = fn.call(args);
  if (result.isError()) {
    errorOut = tr("Error: %1").arg(result.property("message").toString());
    return {};
  }

  if (result.isNull() || result.isUndefined())
    return tr("(null -- frame skipped)");

  return result.toString();
}

/**
 * @brief Returns true when text contains only hex digits and whitespace separators.
 */
bool MQTT::PublisherScriptEditor::validateHexInput(const QString& text)
{
  if (text.isEmpty())
    return false;

  static const QRegularExpression re(QStringLiteral("^[0-9A-Fa-f\\s]+$"));
  return re.match(text).hasMatch();
}

/**
 * @brief Lower-cases hex digits and inserts a space between every byte pair.
 */
QString MQTT::PublisherScriptEditor::formatHexInput(const QString& text)
{
  QString stripped;
  stripped.reserve(text.size());
  for (const QChar c : text)
    if (c.isLetterOrNumber())
      stripped.append(c.toUpper());

  QString out;
  out.reserve(stripped.size() + stripped.size() / 2);
  for (int i = 0; i < stripped.size(); ++i) {
    if (i > 0 && (i % 2) == 0)
      out.append(' ');

    out.append(stripped.at(i));
  }

  return out;
}

//--------------------------------------------------------------------------------------------------
// Template loading
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads MQTT script templates from the JSON manifest and resource files.
 */
void MQTT::PublisherScriptEditor::buildTemplates()
{
  QString selectedFile;
  if (m_templateCombo) {
    const int currentIndex = m_templateCombo->currentIndex();
    if (currentIndex > 0 && currentIndex <= m_templates.size())
      selectedFile = m_templates[currentIndex - 1].file;
  }

  const auto definitions =
    DataModel::loadScriptTemplateManifest(QStringLiteral(":/scripts/mqtt/templates.json"));

  m_templates.clear();
  m_templates.reserve(definitions.size());

  for (const auto& def : definitions) {
    m_templates.append({
      def.file,
      def.name,
      DataModel::readTextResource(DataModel::templateResourcePath(
        QStringLiteral(":/scripts/mqtt/lua"), def.file, QStringLiteral(".lua"))),
      DataModel::readTextResource(DataModel::templateResourcePath(
        QStringLiteral(":/scripts/mqtt/js"), def.file, QStringLiteral(".js"))),
    });
  }

  if (!m_templateCombo)
    return;

  m_templateCombo->blockSignals(true);
  m_templateCombo->clear();
  m_templateCombo->addItem(tr("Select Template…"));
  for (const auto& tmpl : std::as_const(m_templates))
    m_templateCombo->addItem(tmpl.name);

  int selectedIndex = 0;
  if (!selectedFile.isEmpty()) {
    for (int i = 0; i < m_templates.size(); ++i) {
      if (m_templates.at(i).file == selectedFile) {
        selectedIndex = i + 1;
        break;
      }
    }
  } else {
    const int detected = detectTemplate();
    if (detected >= 0)
      selectedIndex = detected + 1;
  }

  m_templateCombo->setCurrentIndex(selectedIndex);
  m_templateCombo->blockSignals(false);
}

#endif  // BUILD_COMMERCIAL
