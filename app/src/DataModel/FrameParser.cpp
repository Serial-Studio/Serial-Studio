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

#include "DataModel/FrameParser.h"

#include <QCoreApplication>
#include <QFile>
#include <QThread>

#include "DataModel/IScriptEngine.h"
#include "DataModel/JsScriptEngine.h"
#include "DataModel/LuaScriptEngine.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/ScriptTemplates.h"
#include "Misc/TimerEvents.h"
#include "Misc/Translator.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initialises the engine map (source 0 = global) and loads template
 * names.
 *
 * Cross-singleton connections to ProjectModel are deferred to
 * @c setupExternalConnections() to avoid static-init-order deadlocks.
 */
DataModel::FrameParser::FrameParser() : m_suppressMessageBoxes(false)
{
  (void)engineForSource(0);

  connect(&Misc::TimerEvents::instance(),
          &Misc::TimerEvents::timeout1Hz,
          this,
          &DataModel::FrameParser::collectGarbage);

  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &DataModel::FrameParser::loadTemplateNames);

  // Tear down engines before static destruction — QJSEngine depends on the
  // live QCoreApplication.
  if (auto* app = QCoreApplication::instance()) {
    connect(app, &QCoreApplication::aboutToQuit, this, [this]() {
      Q_ASSERT(QThread::currentThread() == this->thread());
      m_engines.clear();
    });
  }

  loadTemplateNames();
}

/**
 * @brief Returns the singleton instance.
 */
DataModel::FrameParser& DataModel::FrameParser::instance()
{
  static FrameParser singleton;
  return singleton;
}

//--------------------------------------------------------------------------------------------------
// External connections
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires cross-singleton connections and performs the initial script load.
 *
 * Called by ModuleManager after all singletons are constructed so that
 * accessing ProjectModel::instance() here cannot deadlock.
 */
void DataModel::FrameParser::setupExternalConnections()
{
  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::frameParserCodeChanged,
          this,
          &DataModel::FrameParser::readCode);

  readCode();
}

//--------------------------------------------------------------------------------------------------
// Default template code
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the source code of the default frame parser template for the
 * given @p language.
 *
 * @param language SerialStudio::ScriptLanguage value (0 = JS, 1 = Lua).
 * @return The default template source code, or an empty string if the
 *         resource could not be read.
 */
QString DataModel::FrameParser::defaultTemplateCode(int language)
{
  const auto templates = loadScriptTemplateManifest(
    QStringLiteral(":/rcc/scripts/parser/templates.json"), "DataModel::FrameParser");

  QString defaultFile;
  for (const auto& tmpl : templates) {
    if (tmpl.isDefault) {
      defaultFile = tmpl.file;
      break;
    }
  }

  if (defaultFile.isEmpty() && !templates.isEmpty())
    defaultFile = templates.constFirst().file;

  if (defaultFile.isEmpty())
    return {};

  const bool isLua = (language == SerialStudio::Lua);
  const auto directory =
    isLua ? QStringLiteral(":/rcc/scripts/parser/lua") : QStringLiteral(":/rcc/scripts/parser/js");
  const auto suffix = isLua ? QStringLiteral(".lua") : QStringLiteral(".js");
  return readTextResource(templateResourcePath(directory, defaultFile, suffix));
}

//--------------------------------------------------------------------------------------------------
// Template accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the source code of the currently selected template for
 * @p sourceId.
 *
 * @param sourceId Source whose stored template index is used (default: 0).
 * @return Template source code, or an empty string if no template is selected.
 */
QString DataModel::FrameParser::templateCode(int sourceId) const
{
  auto it       = m_engines.find(sourceId);
  const int idx = (it != m_engines.end()) ? it->second->templateIdx : -1;

  if (idx < 0 || idx >= m_templateFiles.count())
    return {};

  // Resolve template path for the configured language
  const int lang   = languageForSource(sourceId);
  const bool isLua = (lang == SerialStudio::Lua);
  const auto directory =
    isLua ? QStringLiteral(":/rcc/scripts/parser/lua") : QStringLiteral(":/rcc/scripts/parser/js");
  const auto suffix = isLua ? QStringLiteral(".lua") : QStringLiteral(".js");
  return readTextResource(templateResourcePath(directory, m_templateFiles.at(idx), suffix));
}

/**
 * @brief Checks if @p code matches any known template (Lua or JS variant).
 *
 * Compares the trimmed code against both language variants of every
 * template. This enables language switching to find the equivalent
 * template even when the code was loaded from a previous session.
 *
 * @param code The frame parser code to match.
 * @return Template index (0-based into templateFiles) if found, or -1.
 */
int DataModel::FrameParser::detectTemplate(const QString& code) const
{
  const QString trimmed = code.trimmed();
  if (trimmed.isEmpty())
    return -1;

  for (int i = 0; i < m_templateFiles.size(); ++i) {
    const auto& file = m_templateFiles[i];

    // Check Lua variant
    const auto luaPath = templateResourcePath(
      QStringLiteral(":/rcc/scripts/parser/lua"), file, QStringLiteral(".lua"));
    const QString luaCode = readTextResource(luaPath).trimmed();
    if (!luaCode.isEmpty() && luaCode == trimmed)
      return i;

    // Check JS variant
    const auto jsPath =
      templateResourcePath(QStringLiteral(":/rcc/scripts/parser/js"), file, QStringLiteral(".js"));
    const QString jsCode = readTextResource(jsPath).trimmed();
    if (!jsCode.isEmpty() && jsCode == trimmed)
      return i;
  }

  return -1;
}

/**
 * @brief Returns the list of localised template display names.
 */
const QStringList& DataModel::FrameParser::templateNames() const
{
  return m_templateNames;
}

/**
 * @brief Returns the list of template file basenames.
 */
const QStringList& DataModel::FrameParser::templateFiles() const
{
  return m_templateFiles;
}

//--------------------------------------------------------------------------------------------------
// Engine management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the scripting language configured for @p sourceId.
 *
 * @param sourceId Source identifier.
 * @return SerialStudio::ScriptLanguage value (0 = JS, 1 = Lua).
 */
int DataModel::FrameParser::languageForSource(int sourceId) const
{
  const auto& sources = ProjectModel::instance().sources();
  for (const auto& src : sources)
    if (src.sourceId == sourceId)
      return src.frameParserLanguage;

  return SerialStudio::JavaScript;
}

/**
 * @brief Returns (or lazily creates) the script engine for @p sourceId.
 *
 * The engine type is determined by the source's configured language.
 * Source 0 is the global engine shared by all single-source projects.
 *
 * @param sourceId Source identifier (0 = global).
 * @return Reference to the IScriptEngine for this source.
 */
DataModel::IScriptEngine& DataModel::FrameParser::engineForSource(int sourceId)
{
  Q_ASSERT(sourceId >= 0);

  auto it = m_engines.find(sourceId);
  if (it != m_engines.end())
    return *it->second;

  // Create a new engine for the configured language
  const int lang = languageForSource(sourceId);
  std::unique_ptr<IScriptEngine> engine;
  if (lang == SerialStudio::Lua)
    engine = std::make_unique<LuaScriptEngine>();
  else
    engine = std::make_unique<JsScriptEngine>();

  auto& ref           = *engine;
  m_engines[sourceId] = std::move(engine);
  Q_ASSERT(m_engines.count(sourceId));
  return ref;
}

/**
 * @brief Sets per-source code, loading it into the source engine.
 *
 * @param sourceId Source identifier (0 = global).
 * @param code     Script source code.
 */
void DataModel::FrameParser::setSourceCode(int sourceId, const QString& code)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(m_engines.count(0));

  if (code.isEmpty()) {
    clearSourceEngine(sourceId);
    return;
  }

  if (!loadScript(sourceId, code, false))
    qWarning() << "[FrameParser] Failed to load script for source" << sourceId;
}

/**
 * @brief Removes and destroys the engine for @p sourceId.
 *
 * For source 0 this resets the global engine to a clean, unloaded state
 * rather than removing it from the map (source 0 is always present).
 *
 * @param sourceId Source identifier (0 = global).
 */
void DataModel::FrameParser::clearSourceEngine(int sourceId)
{
  auto it = m_engines.find(sourceId);
  if (it == m_engines.end())
    return;

  if (sourceId == 0) {
    it->second->reset();
    return;
  }

  m_engines.erase(it);
}

//--------------------------------------------------------------------------------------------------
// Parsing dispatch
//--------------------------------------------------------------------------------------------------

/**
 * @brief Executes the engine for @p sourceId over text data, returning one
 * or more frames.
 *
 * Falls back to source 0 when the source has no dedicated engine loaded.
 *
 * @param frame    Decoded UTF-8 string frame.
 * @param sourceId Source identifier whose engine should be used.
 * @return List of QStringList, each representing one frame.
 */
QList<QStringList> DataModel::FrameParser::parseMultiFrame(const QString& frame, int sourceId)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(!frame.isEmpty());

  // Runtime guard for release builds (assertions stripped)
  if (sourceId < 0 || frame.isEmpty()) [[unlikely]]
    return {};

  auto it = m_engines.find(sourceId);
  if (it == m_engines.end() || !it->second->isLoaded()) {
    if (sourceId == 0)
      return {};

    return parseMultiFrame(frame, 0);
  }

  return it->second->parseString(frame);
}

/**
 * @brief Executes the engine for @p sourceId over binary data, returning
 * one or more frames.
 *
 * Falls back to source 0 when the source has no dedicated engine loaded.
 *
 * @param frame    Binary frame data.
 * @param sourceId Source identifier whose engine should be used.
 * @return List of QStringList, each representing one frame.
 */
QList<QStringList> DataModel::FrameParser::parseMultiFrame(const QByteArray& frame, int sourceId)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(!frame.isEmpty());

  // Runtime guard for release builds (assertions stripped)
  if (sourceId < 0 || frame.isEmpty()) [[unlikely]]
    return {};

  auto it = m_engines.find(sourceId);
  if (it == m_engines.end() || !it->second->isLoaded()) {
    if (sourceId == 0)
      return {};

    return parseMultiFrame(frame, 0);
  }

  return it->second->parseBinary(frame);
}

//--------------------------------------------------------------------------------------------------
// Script loading
//--------------------------------------------------------------------------------------------------

/**
 * @brief Validates and loads a frame parser script into the engine for
 * @p sourceId.
 *
 * Delegates all language-specific validation to the concrete IScriptEngine.
 *
 * @param sourceId         Target source engine (0 = global).
 * @param script           The script source to validate and load.
 * @param showMessageBoxes When @c false, errors are logged instead of shown.
 * @return @c true on success, @c false if validation failed.
 */
bool DataModel::FrameParser::loadScript(int sourceId, const QString& script, bool showMessageBoxes)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(!script.isEmpty());

  // Recreate engine if the language changed since last load
  auto it = m_engines.find(sourceId);
  if (it != m_engines.end()) {
    const int lang              = languageForSource(sourceId);
    const bool isLua            = (lang == SerialStudio::Lua);
    const bool engineIsLua      = (dynamic_cast<LuaScriptEngine*>(it->second.get()) != nullptr);
    const bool languageMismatch = (isLua != engineIsLua);
    if (languageMismatch) {
      m_engines.erase(it);
      it = m_engines.end();
    }
  }

  auto& engine = engineForSource(sourceId);
  return engine.loadScript(script, sourceId, showMessageBoxes);
}

/**
 * @brief Enables or disables UI message boxes (suppressed during API calls).
 */
void DataModel::FrameParser::setSuppressMessageBoxes(const bool suppress)
{
  m_suppressMessageBoxes = suppress;
}

//--------------------------------------------------------------------------------------------------
// Code reload
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads the code stored in the project model into all engines.
 *
 * Destroys all engines except source 0 (which is reset in-place), then
 * reloads source 0 from the project and lazily loads each per-source engine
 * from its stored parser code.
 */
void DataModel::FrameParser::readCode()
{
  // Remove all per-source engines (source 0 is recreated below)
  for (auto it = m_engines.begin(); it != m_engines.end();)
    if (it->first != 0)
      it = m_engines.erase(it);
    else
      ++it;

  // Recreate source 0 engine if language changed
  const int lang0  = languageForSource(0);
  auto it0         = m_engines.find(0);
  const bool isLua = (lang0 == SerialStudio::Lua);
  const bool engineIsLua =
    it0 != m_engines.end() && dynamic_cast<LuaScriptEngine*>(it0->second.get()) != nullptr;
  if (it0 != m_engines.end() && isLua != engineIsLua)
    m_engines.erase(0);

  // Load global script and per-source scripts
  const auto& model   = ProjectModel::instance();
  const bool suppress = m_suppressMessageBoxes || model.suppressMessageBoxes();

  const auto& sources = model.sources();
  const QString code  = sources.empty() ? QString() : sources[0].frameParserCode;

  if (!code.isEmpty())
    (void)loadScript(0, code, !suppress);

  for (const auto& src : sources)
    if (src.sourceId > 0 && !src.frameParserCode.isEmpty())
      (void)loadScript(src.sourceId, src.frameParserCode, false);

  Q_EMIT modifiedChanged();
}

/**
 * @brief Resets the execution context by re-loading all current code.
 */
void DataModel::FrameParser::clearContext()
{
  // Remove all per-source engines (source 0 is recreated below)
  for (auto it = m_engines.begin(); it != m_engines.end();)
    if (it->first != 0)
      it = m_engines.erase(it);
    else
      ++it;

  // Recreate source 0 engine if language changed
  const int lang0  = languageForSource(0);
  auto it0         = m_engines.find(0);
  const bool isLua = (lang0 == SerialStudio::Lua);
  const bool engineIsLua =
    it0 != m_engines.end() && dynamic_cast<LuaScriptEngine*>(it0->second.get()) != nullptr;
  if (it0 != m_engines.end() && isLua != engineIsLua)
    m_engines.erase(0);

  const auto& sources = ProjectModel::instance().sources();
  const QString code  = sources.empty() ? QString() : sources[0].frameParserCode;

  if (!code.isEmpty())
    (void)loadScript(0, code, !m_suppressMessageBoxes);

  for (const auto& src : sources)
    if (src.sourceId > 0 && !src.frameParserCode.isEmpty())
      (void)loadScript(src.sourceId, src.frameParserCode, false);
}

/**
 * @brief Runs one cycle of garbage collection on all engines.
 */
void DataModel::FrameParser::collectGarbage()
{
  for (auto it = m_engines.begin(); it != m_engines.end(); ++it)
    it->second->collectGarbage();
}

//--------------------------------------------------------------------------------------------------
// Template management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the list of available template files and display names.
 */
void DataModel::FrameParser::loadTemplateNames()
{
  m_defaultTemplateFile.clear();
  m_templateFiles.clear();
  m_templateNames.clear();

  const auto templates = loadScriptTemplateManifest(
    QStringLiteral(":/rcc/scripts/parser/templates.json"), "DataModel::FrameParser");

  for (const auto& tmpl : templates) {
    m_templateFiles.append(tmpl.file);
    m_templateNames.append(tmpl.name);
    if (m_defaultTemplateFile.isEmpty() && tmpl.isDefault)
      m_defaultTemplateFile = tmpl.file;
  }

  if (m_defaultTemplateFile.isEmpty() && !m_templateFiles.isEmpty())
    m_defaultTemplateFile = m_templateFiles.constFirst();

  Q_EMIT templateNamesChanged();
}

/**
 * @brief Sets the active template index and saves the resulting code to the
 * project model.
 *
 * @param sourceId Target source engine (0 = global).
 * @param idx      Template index into @c m_templateFiles.
 */
void DataModel::FrameParser::setTemplateIdx(int sourceId, int idx)
{
  if (idx < 0 || idx >= m_templateFiles.size())
    return;

  engineForSource(sourceId).templateIdx = idx;
  const QString code                    = templateCode(sourceId);

  if (loadScript(sourceId, code, !m_suppressMessageBoxes)) {
    auto& model = DataModel::ProjectModel::instance();
    if (sourceId == 0)
      model.setFrameParserCode(code);
    else
      model.updateSourceFrameParser(sourceId, code);

    model.setModified(true);
  }
}

/**
 * @brief Loads the default comma-separated-values template for @p sourceId.
 *
 * @param sourceId   Target source engine (0 = global).
 * @param guiTrigger When @c false, clears the project's modified flag after
 *                   loading (silent default reset).
 */
void DataModel::FrameParser::loadDefaultTemplate(int sourceId, bool guiTrigger)
{
  const auto idx = m_templateFiles.indexOf(m_defaultTemplateFile);
  setTemplateIdx(sourceId, idx);

  if (!guiTrigger)
    DataModel::ProjectModel::instance().setModified(false);
}
