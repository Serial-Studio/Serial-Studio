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

#include "DataModel/DatasetTransformEditor.h"

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include <QFile>
#include <QHBoxLayout>
#include <QJavascriptHighlighter>
#include <QJSEngine>
#include <QLuaCompleter>
#include <QLuaHighlighter>

#include "DataModel/NotificationCenter.h"
#include "DataModel/ScriptTemplates.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/Translator.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

DataModel::DatasetTransformEditor::DatasetTransformEditor(QWidget* parent)
  : QDialog(parent), m_language(SerialStudio::Lua), m_targetGroupId(-1), m_targetDatasetId(-1)
{
  setWindowTitle(tr("Dataset Value Transform"));
  setMinimumSize(640, 520);
  resize(720, 580);

  // Syntax-highlighted code editor
  m_editor = new QCodeEditor(this);
  m_editor->setTabReplace(true);
  m_editor->setTabReplaceSize(2);
  m_editor->setAutoIndentation(true);
  m_editor->setFont(Misc::CommonFonts::instance().monoFont());
  m_editor->setMinimumHeight(200);

  // Language selector (Lua first — it's the project default)
  m_languageCombo = new QComboBox(this);
  m_languageCombo->addItems({tr("Lua"), tr("JavaScript")});

  // Template selector with placeholder entry
  m_templateCombo = new QComboBox(this);
  buildTemplates();

  // Toolbar row
  auto* toolbarLayout = new QHBoxLayout();
  toolbarLayout->addWidget(new QLabel(tr("Language:"), this));
  toolbarLayout->addWidget(m_languageCombo);
  toolbarLayout->addSpacing(16);
  toolbarLayout->addWidget(new QLabel(tr("Template:"), this));
  toolbarLayout->addWidget(m_templateCombo, 1);

  // Test area
  m_testInput = new QLineEdit(this);
  m_testInput->setPlaceholderText(tr("Enter raw value (e.g., 1024)"));

  m_testOutput = new QLabel(QStringLiteral("—"), this);
  m_testOutput->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
  m_testOutput->setMinimumWidth(120);

  m_testButton  = new QPushButton(tr("Test"), this);
  m_clearButton = new QPushButton(tr("Clear"), this);

  auto* testLayout = new QHBoxLayout();
  testLayout->addWidget(new QLabel(tr("Input:"), this));
  testLayout->addWidget(m_testInput, 1);
  testLayout->addWidget(m_testButton);
  testLayout->addWidget(new QLabel(tr("Output:"), this));
  testLayout->addWidget(m_testOutput, 1);
  testLayout->addWidget(m_clearButton);

  // Action buttons
  m_applyButton  = new QPushButton(tr("Apply"), this);
  m_cancelButton = new QPushButton(tr("Cancel"), this);

  auto* buttonLayout = new QHBoxLayout();
  buttonLayout->addStretch();
  buttonLayout->addWidget(m_applyButton);
  buttonLayout->addWidget(m_cancelButton);

  // Assemble main layout
  auto* mainLayout = new QVBoxLayout(this);
  mainLayout->addLayout(toolbarLayout);
  mainLayout->addWidget(m_editor, 1);
  mainLayout->addLayout(testLayout);
  mainLayout->addLayout(buttonLayout);

  // Wire signals
  connect(m_applyButton, &QPushButton::clicked, this, &DatasetTransformEditor::onApply);
  connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
  connect(m_testButton, &QPushButton::clicked, this, &DatasetTransformEditor::onTest);
  connect(m_clearButton, &QPushButton::clicked, this, &DatasetTransformEditor::onClear);
  connect(m_testInput, &QLineEdit::returnPressed, this, &DatasetTransformEditor::onTest);
  connect(m_templateCombo,
          QOverload<int>::of(&QComboBox::activated),
          this,
          &DatasetTransformEditor::onTemplateSelected);
  connect(m_languageCombo,
          QOverload<int>::of(&QComboBox::activated),
          this,
          &DatasetTransformEditor::onLanguageChanged);
  connect(&Misc::ThemeManager::instance(),
          &Misc::ThemeManager::themeChanged,
          this,
          &DatasetTransformEditor::onThemeChanged);
  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &DatasetTransformEditor::buildTemplates);

  // Apply initial theme and Lua highlighting
  onThemeChanged();
  applyLanguage(SerialStudio::Lua);
}

//--------------------------------------------------------------------------------------------------
// Public interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens the dialog pre-populated with existing transform code.
 */
void DataModel::DatasetTransformEditor::displayDialog(
  const QString& datasetTitle, const QString& currentCode, int language, int groupId, int datasetId)
{
  // Capture target dataset identity at open time
  m_targetGroupId   = groupId;
  m_targetDatasetId = datasetId;

  // Update window title with dataset name
  setWindowTitle(tr("Transform — %1").arg(datasetTitle));

  // Sync combo + derive canonical language so m_language and highlighter can't disagree
  const int comboIdx = (language == SerialStudio::Lua) ? 0 : 1;
  m_languageCombo->blockSignals(true);
  m_languageCombo->setCurrentIndex(comboIdx);
  m_languageCombo->blockSignals(false);

  const int resolvedLanguage = (comboIdx == 0) ? SerialStudio::Lua : SerialStudio::JavaScript;
  applyLanguage(resolvedLanguage);

  // Prefill the editor using the canonical language so placeholder matches highlighter
  if (currentCode.isEmpty())
    m_editor->setPlainText(defaultPlaceholder(resolvedLanguage));
  else
    m_editor->setPlainText(currentCode);

  // Try to detect which template matches the current code
  const int tmplIdx = detectTemplate();
  m_templateCombo->setCurrentIndex(tmplIdx >= 0 ? tmplIdx + 1 : 0);

  // Reset test output
  m_testOutput->setText(QStringLiteral("—"));

  // Display the window & focus the text editor
  showNormal();
  raise();
  activateWindow();
  m_editor->setFocus();
}

QString DataModel::DatasetTransformEditor::code() const
{
  return m_editor->toPlainText();
}

int DataModel::DatasetTransformEditor::language() const
{
  return m_language;
}

int DataModel::DatasetTransformEditor::targetGroupId() const noexcept
{
  return m_targetGroupId;
}

int DataModel::DatasetTransformEditor::targetDatasetId() const noexcept
{
  return m_targetDatasetId;
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Emits the transform if transform() is defined, otherwise emits empty.
 */
void DataModel::DatasetTransformEditor::onApply()
{
  // Drop the code entirely if no transform() function is defined
  QString code = m_editor->toPlainText();
  if (!definesTransformFunction(code, m_language))
    code.clear();

  // Accept the transform code
  Q_EMIT transformApplied(code, m_language, m_targetGroupId, m_targetDatasetId);
  QDialog::accept();
}

/**
 * @brief Runs the transform against the test input and shows the result.
 */
void DataModel::DatasetTransformEditor::onTest()
{
  // Validate input
  const QString inputStr = m_testInput->text().trimmed();
  if (inputStr.isEmpty()) {
    m_testOutput->setText(tr("Enter a value"));
    return;
  }

  bool ok               = false;
  const double inputVal = inputStr.toDouble(&ok);
  if (!ok) {
    m_testOutput->setText(tr("Invalid number"));
    return;
  }

  // Run the transform and display the result
  const QString result = testTransform(m_editor->toPlainText(), m_language, inputVal);
  m_testOutput->setText(result);
}

/**
 * @brief Clears test fields and resets the editor to the default placeholder.
 */
void DataModel::DatasetTransformEditor::onClear()
{
  m_editor->setPlainText(defaultPlaceholder(m_language));
  m_testInput->clear();
  m_testOutput->setText(QStringLiteral("—"));
  m_templateCombo->setCurrentIndex(0);
}

/**
 * @brief Loads the selected template's code (Lua or JS) into the editor.
 */
void DataModel::DatasetTransformEditor::onTemplateSelected(int index)
{
  // Index 0 is the "Select Template..." placeholder
  if (index <= 0 || index > m_templates.size())
    return;

  // Load the correct language variant
  const auto& tmpl   = m_templates[index - 1];
  const QString code = (m_language == SerialStudio::Lua) ? tmpl.luaCode : tmpl.jsCode;
  m_editor->setPlainText(code);
}

/**
 * @brief Swaps the highlighter and, when recognised, the template or placeholder.
 */
void DataModel::DatasetTransformEditor::onLanguageChanged(int index)
{
  const int newLang = (index == 0) ? SerialStudio::Lua : SerialStudio::JavaScript;
  if (newLang == m_language)
    return;

  // Detect template and placeholder match against the OLD language
  // before the highlighter is swapped
  const int tmplIdx         = detectTemplate();
  const bool wasPlaceholder = isDefaultPlaceholder(m_editor->toPlainText(), m_language);

  // Switch highlighter
  applyLanguage(newLang);

  // If we recognised a template, load its equivalent in the new language
  if (tmplIdx >= 0) {
    const auto& tmpl   = m_templates[tmplIdx];
    const QString code = (newLang == SerialStudio::Lua) ? tmpl.luaCode : tmpl.jsCode;
    m_editor->setPlainText(code);
    m_templateCombo->setCurrentIndex(tmplIdx + 1);
  }

  // If the editor held the default placeholder, swap to the new
  // language's version so the comment style matches the highlighter
  else if (wasPlaceholder) {
    m_editor->setPlainText(defaultPlaceholder(newLang));
  }
}

/**
 * @brief Updates the editor colour scheme to match the active theme.
 */
void DataModel::DatasetTransformEditor::onThemeChanged()
{
  static const auto* t = &Misc::ThemeManager::instance();
  const auto name      = t->parameters().value(QStringLiteral("code-editor-theme")).toString();
  const auto path =
    name.startsWith('/') ? name : QStringLiteral(":/rcc/themes/code-editor/%1.xml").arg(name);

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
void DataModel::DatasetTransformEditor::applyLanguage(int language)
{
  m_language = language;

  // Switch syntax highlighter and auto-completer
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
QString DataModel::DatasetTransformEditor::defaultPlaceholder(int language)
{
  if (language == SerialStudio::Lua)
    return tr("--\n"
              "-- Define a transform(value) function that receives the live\n"
              "-- dataset reading and returns a transformed number. If no\n"
              "-- transform() function is defined, the raw value is kept\n"
              "-- and nothing is saved with the project.\n"
              "--\n"
              "-- Example:\n"
              "--    function transform(value)\n"
              "--      return value * 0.01 + 273.15\n"
              "--    end\n"
              "--\n"
              "-- Globals declared at the top level persist between frames,\n"
              "-- which is useful for filters, accumulators, and any other\n"
              "-- stateful transform, just like a frame parser script:\n"
              "--\n"
              "--    local alpha = 0.1\n"
              "--    local ema\n"
              "--\n"
              "--    function transform(value)\n"
              "--      ema = ema or value\n"
              "--      ema = alpha * value + (1 - alpha) * ema\n"
              "--      return ema\n"
              "--    end\n"
              "--\n"
              "-- Use the Template dropdown for ready-made examples, or\n"
              "-- click Test to try your function.\n"
              "--\n");

  return tr("/*\n"
            " * Define a transform(value) function that receives the live\n"
            " * dataset reading and returns a transformed number. If no\n"
            " * transform() function is defined, the raw value is kept\n"
            " * and nothing is saved with the project.\n"
            " *\n"
            " * Example:\n"
            " *   function transform(value) {\n"
            " *     return value * 0.01 + 273.15;\n"
            " *   }\n"
            " *\n"
            " * Globals declared at the top level persist between frames,\n"
            " * which is useful for filters, accumulators, and any other\n"
            " * stateful transform — just like a frame parser script:\n"
            " *\n"
            " *   var alpha = 0.1;\n"
            " *   var ema;\n"
            " *\n"
            " *   function transform(value) {\n"
            " *     if (ema === undefined) ema = value;\n"
            " *     ema = alpha * value + (1 - alpha) * ema;\n"
            " *     return ema;\n"
            " *   }\n"
            " *\n"
            " * Use the Template dropdown for ready-made examples, or\n"
            " * click Test to try your function.\n"
            " */");
}

/**
 * @brief Returns true if code matches the default placeholder in either language.
 */
bool DataModel::DatasetTransformEditor::isDefaultPlaceholder(const QString& code, int language)
{
  const QString trimmed = code.trimmed();
  if (trimmed.isEmpty())
    return false;

  // Match the current language's placeholder exactly
  if (trimmed == defaultPlaceholder(language).trimmed())
    return true;

  // Also match the other language's placeholder — covers the rare case
  // where state got out of sync (e.g. the editor was re-opened on a
  // project saved with the opposite language)
  const int other = (language == SerialStudio::Lua) ? SerialStudio::JavaScript : SerialStudio::Lua;
  return trimmed == defaultPlaceholder(other).trimmed();
}

/**
 * @brief Returns true if the code defines a callable transform() global.
 */
bool DataModel::DatasetTransformEditor::definesTransformFunction(const QString& code, int language)
{
  // Empty or whitespace-only code obviously has no function
  if (code.trimmed().isEmpty())
    return false;

  // Lua: spin up a disposable state, run the code, check for transform()
  if (language == SerialStudio::Lua) {
    lua_State* L = luaL_newstate();
    if (!L)
      return false;

    // Only load libraries that the user's code might reference at load
    // time — we don't care about any side effects here, just whether
    // transform() ends up defined
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

    // Install notify* so user code referencing it at top level doesn't fail
    DataModel::NotificationCenter::installScriptApi(L);

    // Run the chunk — if it fails to even load, there's definitely no
    // valid transform() defined
    const QByteArray utf8 = code.toUtf8();
    if (luaL_dostring(L, utf8.constData()) != LUA_OK) {
      lua_close(L);
      return false;
    }

    // Check for a global named "transform" that is actually a function
    lua_getglobal(L, "transform");
    const bool hasFn = lua_isfunction(L, -1);
    lua_close(L);
    return hasFn;
  }

  // JavaScript: evaluate in a disposable engine and look up transform
  QJSEngine jsEngine;
  DataModel::NotificationCenter::installScriptApi(&jsEngine);
  auto evalResult = jsEngine.evaluate(code);
  if (evalResult.isError())
    return false;

  auto transformFn = jsEngine.globalObject().property(QStringLiteral("transform"));
  return transformFn.isCallable();
}

/**
 * @brief Returns the template index matching the editor text, or -1.
 */
int DataModel::DatasetTransformEditor::detectTemplate() const
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
 * @brief Runs the transform in a disposable engine and returns the result.
 */
QString DataModel::DatasetTransformEditor::testTransform(const QString& code,
                                                         int language,
                                                         double inputValue)
{
  if (code.trimmed().isEmpty())
    return QString::number(inputValue, 'g', 15);

  if (language == SerialStudio::Lua) {
    // Disposable Lua state for a single test run
    lua_State* L = luaL_newstate();
    if (!L)
      return tr("Engine error");

    // Open only safe libraries (no io, os, debug, package)
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

    // Install the Serial Studio notification API + level constants
    DataModel::NotificationCenter::installScriptApi(L);

    // Execute the user's code which defines transform(value)
    const QByteArray utf8 = code.toUtf8();
    if (luaL_dostring(L, utf8.constData()) != LUA_OK) {
      QString err = QString::fromUtf8(lua_tostring(L, -1));
      lua_close(L);
      return tr("Error: %1").arg(err);
    }

    // Verify transform() was defined
    lua_getglobal(L, "transform");
    if (!lua_isfunction(L, -1)) {
      lua_close(L);
      return tr("Error: transform() not defined");
    }

    // Call transform(inputValue) and read the result
    lua_pushnumber(L, inputValue);
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
      QString err = QString::fromUtf8(lua_tostring(L, -1));
      lua_close(L);
      return tr("Error: %1").arg(err);
    }

    // Validate the return type before converting
    if (!lua_isnumber(L, -1)) {
      lua_close(L);
      return tr("Error: transform() must return a number");
    }

    const double result = lua_tonumber(L, -1);
    lua_close(L);
    return QString::number(result, 'g', 15);
  }

  // JavaScript: execute user code then call transform(inputValue)
  QJSEngine jsEngine;
  DataModel::NotificationCenter::installScriptApi(&jsEngine);
  auto evalResult = jsEngine.evaluate(code);
  if (evalResult.isError())
    return tr("Error: %1").arg(evalResult.property("message").toString());

  auto transformFn = jsEngine.globalObject().property(QStringLiteral("transform"));
  if (!transformFn.isCallable())
    return tr("Error: transform() not defined");

  QJSValueList args;
  args << QJSValue(inputValue);
  auto result = transformFn.call(args);

  if (result.isError())
    return tr("Error: %1").arg(result.property("message").toString());

  if (result.isNumber())
    return QString::number(result.toNumber(), 'g', 15);

  return result.toString();
}

//--------------------------------------------------------------------------------------------------
// Template loading
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads transform templates from the JSON manifest and resource files.
 */
void DataModel::DatasetTransformEditor::buildTemplates()
{
  // Get name for currently selected file
  QString selectedFile;
  if (m_templateCombo) {
    const int currentIndex = m_templateCombo->currentIndex();
    if (currentIndex > 0 && currentIndex <= m_templates.size())
      selectedFile = m_templates[currentIndex - 1].file;
  }

  // Read tempaltes.json file
  const auto definitions =
    loadScriptTemplateManifest(QStringLiteral(":/rcc/scripts/transforms/templates.json"));

  // Clear internal members
  m_templates.clear();
  m_templates.reserve(definitions.size());

  // Parse definitions file
  for (const auto& definition : definitions) {
    m_templates.append(
      {definition.file,
       definition.name,
       readTextResource(templateResourcePath(
         QStringLiteral(":/rcc/scripts/transforms/lua"), definition.file, QStringLiteral(".lua"))),
       readTextResource(templateResourcePath(
         QStringLiteral(":/rcc/scripts/transforms/js"), definition.file, QStringLiteral(".js")))});
  }

  // Skip if no combo is available
  if (!m_templateCombo)
    return;

  // Rebuild template combo
  m_templateCombo->blockSignals(true);
  m_templateCombo->clear();
  m_templateCombo->addItem(tr("Select Template..."));
  for (const auto& tmpl : std::as_const(m_templates))
    m_templateCombo->addItem(tmpl.name);

  // Get current index based on previously selected file
  int selectedIndex = 0;
  if (!selectedFile.isEmpty()) {
    for (int i = 0; i < m_templates.size(); ++i) {
      if (m_templates.at(i).file == selectedFile) {
        selectedIndex = i + 1;
        break;
      }
    }
  }

  // Get index for current file
  else {
    const int detectedIndex = detectTemplate();
    if (detectedIndex >= 0)
      selectedIndex = detectedIndex + 1;
  }

  // Set current index
  m_templateCombo->setCurrentIndex(selectedIndex);
  m_templateCombo->blockSignals(false);
}
