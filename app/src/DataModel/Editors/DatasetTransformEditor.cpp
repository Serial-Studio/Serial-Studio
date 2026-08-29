/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/Editors/DatasetTransformEditor.h"

// clang-format off
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}
// clang-format on

#include <QDir>
#include <QFile>
#include <QHash>
#include <QHBoxLayout>
#include <QIcon>
#include <QJavascriptHighlighter>
#include <QJSEngine>
#include <QLuaHighlighter>
#include <QMenu>
#include <QMessageBox>
#include <QShortcut>

#include "DataModel/Editors/EditorFormatting.h"
#include "DataModel/Editors/ExpressionHighlighter.h"
#include "DataModel/Editors/SerialStudioCompleter.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/Scripting/ExpressionTransform.h"
#include "DataModel/Scripting/LuaCompat.h"
#include "DataModel/Scripting/LuaCompatJIT.h"
#include "DataModel/Scripting/ScriptApiCall.h"
#include "DataModel/Scripting/ScriptTemplates.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the dataset value-transform editor dialog.
 */
DataModel::DatasetTransformEditor::DatasetTransformEditor(QWidget* parent)
  : QDialog(parent)
  , m_language(SerialStudio::Lua)
  , m_targetGroupId(-1)
  , m_targetDatasetId(-1)
  , m_commonFonts(Misc::CommonFonts::instance())
  , m_themeManager(Misc::ThemeManager::instance())
  , m_translator(Misc::Translator::instance())
  , m_frameBuilder(DataModel::FrameBuilder::instance())
{
  setWindowTitle(tr("Dataset Value Transform"));
  setMinimumSize(640, 520);
  resize(720, 580);

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
  applyLanguage(SerialStudio::Lua);
}

/**
 * @brief Allocates the editor + combobox widgets and seeds the template list.
 */
void DataModel::DatasetTransformEditor::buildEditorWidgets()
{
  m_editor = new QCodeEditor(this);
  m_editor->setTabReplace(true);
  m_editor->setTabReplaceSize(2);
  m_editor->setAutoIndentation(true);
  m_editor->setFont(m_commonFonts.monoFont());
  m_editor->setMinimumHeight(200);
  m_editor->setLayoutDirection(Qt::LeftToRight);

  m_languageCombo = new QComboBox(this);
  m_languageCombo->addItems({tr("Lua"), tr("JavaScript"), tr("Expression")});

  m_templateCombo   = new QComboBox(this);
  m_templateOpacity = new QGraphicsOpacityEffect(m_templateCombo);
  m_templateOpacity->setOpacity(1.0);
  m_templateCombo->setGraphicsEffect(m_templateOpacity);
  buildTemplates();

  m_testInput = new QLineEdit(this);
  m_testInput->setPlaceholderText(tr("Enter raw value (e.g., 1024)"));
  m_testOutput = new QLineEdit(QStringLiteral("--.--"), this);
  m_testOutput->setReadOnly(true);
  m_testOutput->setMinimumWidth(120);

  m_testButton  = new QPushButton(tr("Test"), this);
  m_clearButton = new QPushButton(tr("Clear"), this);

  m_applyButton  = new QPushButton(tr("Apply"), this);
  m_cancelButton = new QPushButton(tr("Cancel"), this);
}

/**
 * @brief Builds the language + template selector toolbar row.
 */
QHBoxLayout* DataModel::DatasetTransformEditor::buildToolbarLayout()
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
 * @brief Builds the input/output test row used to dry-run a transform.
 */
QHBoxLayout* DataModel::DatasetTransformEditor::buildTestLayout()
{
  auto* testLayout = new QHBoxLayout();
  testLayout->addWidget(new QLabel(tr("Input:"), this));
  testLayout->addWidget(m_testInput, 1);
  testLayout->addWidget(m_testButton);
  testLayout->addWidget(new QLabel(tr("Output:"), this));
  testLayout->addWidget(m_testOutput, 1);
  testLayout->addWidget(m_clearButton);
  return testLayout;
}

/**
 * @brief Builds the Apply/Cancel button row.
 */
QHBoxLayout* DataModel::DatasetTransformEditor::buildButtonLayout()
{
  auto* buttonLayout = new QHBoxLayout();
  buttonLayout->addStretch();
  buttonLayout->addWidget(m_cancelButton);
  buttonLayout->addWidget(m_applyButton);
  return buttonLayout;
}

/**
 * @brief Wires every widget signal needed by the dialog.
 */
void DataModel::DatasetTransformEditor::wireSignals()
{
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
  connect(&m_themeManager,
          &Misc::ThemeManager::themeChanged,
          this,
          &DatasetTransformEditor::onThemeChanged);
  connect(&m_translator,
          &Misc::Translator::languageChanged,
          this,
          &DatasetTransformEditor::buildTemplates);
}

/**
 * @brief Installs the format/format-line shortcuts and the custom context menu.
 */
void DataModel::DatasetTransformEditor::installShortcuts()
{
  auto* formatShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_I), m_editor);
  formatShortcut->setContext(Qt::WidgetShortcut);
  connect(formatShortcut, &QShortcut::activated, this, &DatasetTransformEditor::onFormatLine);
  auto* formatAllShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_I), m_editor);
  formatAllShortcut->setContext(Qt::WidgetShortcut);
  connect(formatAllShortcut, &QShortcut::activated, this, &DatasetTransformEditor::onFormat);

  m_editor->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_editor,
          &QWidget::customContextMenuRequested,
          this,
          &DatasetTransformEditor::showEditorContextMenu);
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
  m_targetGroupId   = groupId;
  m_targetDatasetId = datasetId;

  setWindowTitle(tr("Transform — %1").arg(datasetTitle));

  const int comboIdx = comboIndexForLanguage(language);
  m_languageCombo->blockSignals(true);
  m_languageCombo->setCurrentIndex(comboIdx);
  m_languageCombo->blockSignals(false);

  const int resolvedLanguage = languageForComboIndex(comboIdx);
  applyLanguage(resolvedLanguage);

  if (currentCode.isEmpty())
    m_editor->setPlainText(defaultPlaceholder(resolvedLanguage));
  else
    m_editor->setPlainText(currentCode);

  const int tmplIdx = detectTemplate();
  m_templateCombo->setCurrentIndex(tmplIdx >= 0 ? tmplIdx + 1 : 0);

  m_testOutput->setText(QStringLiteral("--.--"));

  showNormal();
  raise();
  activateWindow();
  m_editor->setFocus();
}

/**
 * @brief Returns the current editor text.
 */
QString DataModel::DatasetTransformEditor::code() const
{
  return m_editor->toPlainText();
}

/**
 * @brief Returns the active scripting language (Lua or JavaScript).
 */
int DataModel::DatasetTransformEditor::language() const
{
  return m_language;
}

/**
 * @brief Returns the group ID the editor is currently targeting.
 */
int DataModel::DatasetTransformEditor::targetGroupId() const noexcept
{
  return m_targetGroupId;
}

/**
 * @brief Returns the dataset ID the editor is currently targeting.
 */
int DataModel::DatasetTransformEditor::targetDatasetId() const noexcept
{
  return m_targetDatasetId;
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies valid code; on a syntax error or missing transform() it warns and keeps the
 *        dialog open so the user does not lose their edits.
 */
void DataModel::DatasetTransformEditor::onApply()
{
  const QString code = m_editor->toPlainText();

  if (code.trimmed().isEmpty() || isDefaultPlaceholder(code, m_language)) {
    Q_EMIT transformApplied(QString(), m_language, m_targetGroupId, m_targetDatasetId);
    QDialog::accept();
    return;
  }

  QString error;
  const TransformStatus status = validateTransform(code, m_language, error);
  if (status == TransformStatus::SyntaxError) {
    Misc::Utilities::showMessageBox(
      tr("The value transform has a syntax error and was not applied."),
      error,
      QMessageBox::Warning,
      windowTitle());
    return;
  }

  if (status == TransformStatus::NoFunction) {
    Misc::Utilities::showMessageBox(
      tr("The value transform must define a transform(value) function."),
      tr("Define a transform(value) function that returns a number, or use Clear to remove the "
         "transform."),
      QMessageBox::Warning,
      windowTitle());
    return;
  }

  Q_EMIT transformApplied(code, m_language, m_targetGroupId, m_targetDatasetId);
  QDialog::accept();
}

/**
 * @brief Runs the transform against the test input and shows the result.
 */
void DataModel::DatasetTransformEditor::onTest()
{
  const QString inputStr = m_testInput->text().trimmed();
  if (inputStr.isEmpty()) {
    m_testOutput->setText(tr("Enter a value"));
    return;
  }

  bool ok               = false;
  const double inputVal = SerialStudio::toDouble(inputStr, &ok);
  if (!ok) {
    m_testOutput->setText(tr("Invalid number"));
    return;
  }

  const QString result = testTransform(m_editor->toPlainText(), m_language, inputVal);
  m_testOutput->setText(result);
}

/**
 * @brief Reformats the entire transform script.
 */
void DataModel::DatasetTransformEditor::onFormat()
{
  if (m_language == SerialStudio::Expression)
    return;

  const auto lang = (m_language == SerialStudio::Lua) ? CodeFormatter::Language::Lua
                                                      : CodeFormatter::Language::JavaScript;
  EditorFormatting::formatDocument(*m_editor, lang);
}

/**
 * @brief Reformats the selected lines, or the current line when nothing is selected.
 */
void DataModel::DatasetTransformEditor::onFormatLine()
{
  if (m_language == SerialStudio::Expression)
    return;

  const auto lang = (m_language == SerialStudio::Lua) ? CodeFormatter::Language::Lua
                                                      : CodeFormatter::Language::JavaScript;
  EditorFormatting::formatSelection(*m_editor, lang);
}

/**
 * @brief Builds and shows the editor's right-click menu with format entries.
 */
void DataModel::DatasetTransformEditor::showEditorContextMenu(const QPoint& localPos)
{
  QMenu* menu = m_editor->createStandardContextMenu();
  menu->addSeparator();
  menu->addAction(tr("Format Document\tCtrl+Shift+I"), this, &DatasetTransformEditor::onFormat);
  menu->addAction(tr("Format Selection\tCtrl+I"), this, &DatasetTransformEditor::onFormatLine);
  menu->exec(m_editor->viewport()->mapToGlobal(localPos));
  menu->deleteLater();
}

/**
 * @brief Clears test fields and resets the editor to the default placeholder.
 */
void DataModel::DatasetTransformEditor::onClear()
{
  m_editor->setPlainText(defaultPlaceholder(m_language));
  m_testInput->clear();
  m_testOutput->setText(QStringLiteral("--.--"));
  m_templateCombo->setCurrentIndex(0);
}

/**
 * @brief Loads the selected template's code (Lua or JS) into the editor.
 */
void DataModel::DatasetTransformEditor::onTemplateSelected(int index)
{
  if (index <= 0 || index > m_templates.size() || m_language == SerialStudio::Expression)
    return;

  const auto& tmpl   = m_templates[index - 1];
  const QString code = (m_language == SerialStudio::Lua) ? tmpl.luaCode : tmpl.jsCode;
  m_editor->setPlainText(code);
}

/**
 * @brief Swaps the highlighter and, when recognised, the template or placeholder.
 */
void DataModel::DatasetTransformEditor::onLanguageChanged(int index)
{
  const int newLang = languageForComboIndex(index);
  if (newLang == m_language)
    return;

  const int tmplIdx         = detectTemplate();
  const bool wasPlaceholder = isDefaultPlaceholder(m_editor->toPlainText(), m_language);

  applyLanguage(newLang);

  if (newLang == SerialStudio::Expression) {
    QString error;
    const bool compiles =
      validateTransform(m_editor->toPlainText(), newLang, error) == TransformStatus::Ok;
    if (!compiles)
      m_editor->setPlainText(defaultPlaceholder(newLang));

    m_templateCombo->setCurrentIndex(0);
  }

  else if (tmplIdx >= 0) {
    const auto& tmpl   = m_templates[tmplIdx];
    const QString code = (newLang == SerialStudio::Lua) ? tmpl.luaCode : tmpl.jsCode;
    m_editor->setPlainText(code);
    m_templateCombo->setCurrentIndex(tmplIdx + 1);
  }

  else if (wasPlaceholder) {
    m_editor->setPlainText(defaultPlaceholder(newLang));
  }
}

/**
 * @brief Reapplies the editor colour scheme and re-tints the button icons for the active theme.
 */
void DataModel::DatasetTransformEditor::onThemeChanged()
{
  const auto name =
    m_themeManager.parameters().value(QStringLiteral("code-editor-theme")).toString();
  const auto path =
    QDir::isAbsolutePath(name) ? name : QStringLiteral(":/themes/code-editor/%1.xml").arg(name);

  QFile file(path);
  if (file.open(QFile::ReadOnly)) {
    m_style.load(QString::fromUtf8(file.readAll()));
    m_editor->setSyntaxStyle(&m_style);
    file.close();
  }

  const QColor c = m_themeManager.getColor(QStringLiteral("button_text"));
  m_testButton->setIcon(Misc::Utilities::coloredSvgIcon(":/icons/buttons/test.svg", c));
  m_clearButton->setIcon(Misc::Utilities::coloredSvgIcon(":/icons/buttons/clear.svg", c));
  m_applyButton->setIcon(Misc::Utilities::coloredSvgIcon(":/icons/buttons/apply.svg", c));
  m_cancelButton->setIcon(Misc::Utilities::coloredSvgIcon(":/icons/buttons/close.svg", c));
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Switches the syntax highlighter and auto-completer for the language, releasing the
 *        previous pair (the editor widget swaps without taking ownership).
 */
void DataModel::DatasetTransformEditor::applyLanguage(int language)
{
  m_language = language;

  const bool scripted = language != SerialStudio::Expression;
  m_templateCombo->setEnabled(scripted);
  m_templateOpacity->setOpacity(scripted ? 1.0 : 0.5);
  if (!scripted)
    m_templateCombo->setCurrentIndex(0);

  auto* old_completer   = m_editor->completer();
  auto* old_highlighter = m_editor->highlighter();

  if (language == SerialStudio::Lua) {
    m_editor->setHighlighter(new QLuaHighlighter());
    m_editor->setLanguageHint(QCodeEditor::LanguageHint::Lua);
    m_editor->setCompleter(new DataModel::SerialStudioCompleter(true, m_editor));
  } else if (language == SerialStudio::Expression) {
    m_editor->setHighlighter(new DataModel::ExpressionHighlighter());
    m_editor->setLanguageHint(QCodeEditor::LanguageHint::JavaScript);
    m_editor->setCompleter(nullptr);
  } else {
    m_editor->setHighlighter(new QJavascriptHighlighter());
    m_editor->setLanguageHint(QCodeEditor::LanguageHint::JavaScript);
    m_editor->setCompleter(new DataModel::SerialStudioCompleter(false, m_editor));
  }

  if (old_completer)
    old_completer->deleteLater();

  if (old_highlighter)
    old_highlighter->deleteLater();
}

/**
 * @brief Returns the default editor placeholder comment for the language.
 */
QString DataModel::DatasetTransformEditor::defaultPlaceholder(int language)
{
  if (language == SerialStudio::Expression)
    return tr("#\n"
              "# An arithmetic expression evaluated once per sample. No function,\n"
              "# no statements: the value of the expression is the new reading.\n"
              "# Lines starting with # are comments.\n"
              "#\n"
              "# Inputs:\n"
              "#    v     this sample's raw value\n"
              "#    t     seconds since the frame stream started\n"
              "#    dt    seconds since the previous sample\n"
              "#    n     sample index, starting at 0\n"
              "#    pi, e, nan, inf\n"
              "#\n"
              "# Other datasets of the same source by Script Alias, or by\n"
              "# dataset id in braces, plus history up to 256 samples back:\n"
              "#    v * shunt_current            by alias\n"
              "#    v * {12}                     by dataset id\n"
              "#    v - sample(shunt_current, 1) one sample ago\n"
              "#\n"
              "# Data-table registers are readable (never writable):\n"
              "#    v * table(calibration, scale)\n"
              "#\n"
              "# Operators: + - * / % ^ < <= > >= == != && || ! and a ? b : c\n"
              "# Functions: abs floor ceil round sqrt cbrt exp log log10 log2\n"
              "#            sin cos tan asin acos atan sinh cosh tanh deg rad\n"
              "#            min max pow atan2 hypot clamp lerp\n"
              "#\n"
              "# Examples:\n"
              "#    v * 0.01 + 273.15            scale and offset\n"
              "#    clamp(v, 0, 100)             limit to a range\n"
              "#    v > 50 ? 50 : v              conditional\n"
              "#\n"
              "v\n");

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
            " * stateful transform -- just like a frame parser script:\n"
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

  if (trimmed == defaultPlaceholder(language).trimmed())
    return true;

  return trimmed == defaultPlaceholder(SerialStudio::Lua).trimmed()
      || trimmed == defaultPlaceholder(SerialStudio::JavaScript).trimmed()
      || trimmed == defaultPlaceholder(SerialStudio::Expression).trimmed();
}

/**
 * @brief Combo row for a transform language: Lua, JavaScript, Expression (spec 0060).
 */
int DataModel::DatasetTransformEditor::comboIndexForLanguage(int language)
{
  if (language == SerialStudio::Lua)
    return 0;

  return (language == SerialStudio::Expression) ? 2 : 1;
}

/**
 * @brief Transform language behind a combo row.
 */
int DataModel::DatasetTransformEditor::languageForComboIndex(int index)
{
  if (index == 0)
    return SerialStudio::Lua;

  return (index == 2) ? SerialStudio::Expression : SerialStudio::JavaScript;
}

/**
 * @brief Table resolver for the editor's expression compile: handles come from the same store the
 *        pipeline reads, refreshed from the project model before validation.
 */
DataModel::Expression::TableResolver DataModel::DatasetTransformEditor::expressionTables()
{
  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  return [](QStringView table, QStringView reg) -> qint64 {
    return frameBuilder.tableStore().handleOf(table.toString(), reg.toString());
  };
}

/**
 * @brief Name resolver for the editor's expression compile: every dataset title of the live
 *        frame resolves (to a throwaway slot), so a sibling reference validates without a device.
 *        The titles are read once through the builder-thread marshal, never from the GUI thread.
 *        Static because validateTransform() is static and needs the same resolver.
 */
DataModel::Expression::NameResolver DataModel::DatasetTransformEditor::expressionResolver(
  DataModel::Expression::SlotTable& table)
{
  static auto& frameBuilder = DataModel::FrameBuilder::instance();

  QHash<QString, int> aliases;
  QSet<int> uniqueIds;
  frameBuilder.invokeOnBuilderThreadBlocking([&aliases, &uniqueIds] {
    for (const auto& group : frameBuilder.frame().groups)
      for (const auto& dataset : group.datasets) {
        uniqueIds.insert(dataset.uniqueId);
        if (!dataset.alias.isEmpty())
          aliases.insert(dataset.alias, dataset.uniqueId);
      }
  });

  return [aliases, uniqueIds, &table](QStringView name) -> int {
    const auto it = aliases.constFind(name.toString());
    if (it != aliases.cend())
      return table.slotFor(it.value());

    bool ok               = false;
    const int resolved_id = name.toInt(&ok);
    if (ok && uniqueIds.contains(resolved_id))
      return table.slotFor(resolved_id);

    return -1;
  };
}

/**
 * @brief Compiles the code and reports whether it is valid, errored, or lacks transform().
 */
DataModel::DatasetTransformEditor::TransformStatus DataModel::DatasetTransformEditor::
  validateTransform(const QString& code, int language, QString& error)
{
  error.clear();
  if (code.trimmed().isEmpty())
    return TransformStatus::NoFunction;

  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  frameBuilder.refreshTableStoreFromProjectModel();

  if (language == SerialStudio::Expression) {
    DataModel::Expression::SlotTable table;
    DataModel::Expression::Program program;
    return DataModel::Expression::compile(
             code, expressionResolver(table), expressionTables(), program, error)
           ? TransformStatus::Ok
           : TransformStatus::SyntaxError;
  }

  if (language == SerialStudio::Lua) {
    lua_State* L = luaL_newstate();
    if (!L) {
      error = tr("Failed to create the Lua engine.");
      return TransformStatus::SyntaxError;
    }

    static const luaL_Reg kSafeLibs[] = {
      {    "_G",   luaopen_base},
      { "table",  luaopen_table},
      {"string", luaopen_string},
      {  "math",   luaopen_math},
      {   "bit",    luaopen_bit},
      { nullptr,        nullptr}
    };

    for (const luaL_Reg* lib = kSafeLibs; lib->func; ++lib) {
      luaL_requiref(L, lib->name, lib->func, 1);
      lua_pop(L, 1);
    }

    DataModel::installLuaCompat(L);
    DataModel::NotificationCenter::installScriptApi(L);
    frameBuilder.injectTableApiLua(L);

    const QByteArray utf8 = code.toUtf8();
    if (luaL_dostring(L, utf8.constData()) != LUA_OK) {
      error = QString::fromUtf8(lua_tostring(L, -1));
      lua_close(L);
      return TransformStatus::SyntaxError;
    }

    lua_getglobal(L, "transform");
    const bool hasFn = lua_isfunction(L, -1);
    lua_close(L);
    return hasFn ? TransformStatus::Ok : TransformStatus::NoFunction;
  }

  QJSEngine jsEngine;
  DataModel::ScriptApiCall::installAll(&jsEngine, 0);
  auto evalResult = jsEngine.evaluate(code);
  if (evalResult.isError()) {
    error = tr("Line %1: %2")
              .arg(evalResult.property(QStringLiteral("lineNumber")).toInt())
              .arg(evalResult.toString());
    return TransformStatus::SyntaxError;
  }

  auto transformFn = jsEngine.globalObject().property(QStringLiteral("transform"));
  return transformFn.isCallable() ? TransformStatus::Ok : TransformStatus::NoFunction;
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

  if (language == SerialStudio::Expression) {
    DataModel::Expression::SlotTable table;
    DataModel::Expression::Runtime runtime;
    QString error;
    if (!DataModel::Expression::compile(
          code, expressionResolver(table), expressionTables(), runtime.program, error))
      return tr("Error: %1").arg(error);

    return QString::number(runtime.run(inputValue, 0.0, table), 'g', 15);
  }

  m_frameBuilder.refreshTableStoreFromProjectModel();

  if (language == SerialStudio::Lua) {
    lua_State* L = luaL_newstate();
    if (!L)
      return tr("Engine error");

    static const luaL_Reg kSafeLibs[] = {
      {    "_G",   luaopen_base},
      { "table",  luaopen_table},
      {"string", luaopen_string},
      {  "math",   luaopen_math},
      {   "bit",    luaopen_bit},
      { nullptr,        nullptr}
    };

    for (const luaL_Reg* lib = kSafeLibs; lib->func; ++lib) {
      luaL_requiref(L, lib->name, lib->func, 1);
      lua_pop(L, 1);
    }

    DataModel::installLuaCompat(L);
    DataModel::NotificationCenter::installScriptApi(L);
    m_frameBuilder.injectTableApiLua(L);

    const QByteArray utf8 = code.toUtf8();
    if (luaL_dostring(L, utf8.constData()) != LUA_OK) {
      QString err = QString::fromUtf8(lua_tostring(L, -1));
      lua_close(L);
      return tr("Error: %1").arg(err);
    }

    lua_getglobal(L, "transform");
    if (!lua_isfunction(L, -1)) {
      lua_close(L);
      return tr("Error: transform() not defined");
    }

    lua_pushnumber(L, inputValue);
    if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
      QString err = QString::fromUtf8(lua_tostring(L, -1));
      lua_close(L);
      return tr("Error: %1").arg(err);
    }

    if (!lua_isnumber(L, -1)) {
      lua_close(L);
      return tr("Error: transform() must return a number");
    }

    const double result = lua_tonumber(L, -1);
    lua_close(L);
    return QString::number(result, 'g', 15);
  }

  QJSEngine jsEngine;
  DataModel::ScriptApiCall::installAll(&jsEngine, 0);
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
  QString selectedFile;
  if (m_templateCombo) {
    const int currentIndex = m_templateCombo->currentIndex();
    if (currentIndex > 0 && currentIndex <= m_templates.size())
      selectedFile = m_templates[currentIndex - 1].file;
  }

  const auto definitions =
    loadScriptTemplateManifest(QStringLiteral(":/scripts/transforms/templates.json"));

  m_templates.clear();
  m_templates.reserve(definitions.size());

  for (const auto& definition : definitions) {
    m_templates.append(
      {definition.file,
       definition.name,
       readTextResource(templateResourcePath(
         QStringLiteral(":/scripts/transforms/lua"), definition.file, QStringLiteral(".lua"))),
       readTextResource(templateResourcePath(
         QStringLiteral(":/scripts/transforms/js"), definition.file, QStringLiteral(".js")))});
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
  }

  else {
    const int detectedIndex = detectTemplate();
    if (detectedIndex >= 0)
      selectedIndex = detectedIndex + 1;
  }

  m_templateCombo->setCurrentIndex(selectedIndex);
  m_templateCombo->blockSignals(false);
}
