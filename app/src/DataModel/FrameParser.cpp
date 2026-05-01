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
 * @brief Constructs the FrameParser singleton and seeds the source-0 engine.
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

  if (auto* app = QCoreApplication::instance()) {
    connect(app, &QCoreApplication::aboutToQuit, this, [this]() {
      Q_ASSERT(QThread::currentThread() == this->thread());
      m_engines.clear();
    });
  }

  loadTemplateNames();
}

/**
 * @brief Returns the singleton FrameParser instance.
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
 * @brief Wires ProjectModel signals and runs the initial script load.
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
 * @brief Returns the default frame parser template code for the language.
 */
QString DataModel::FrameParser::defaultTemplateCode(int language)
{
  const auto templates = loadScriptTemplateManifest(
    QStringLiteral(":/scripts/parser/templates.json"), "DataModel::FrameParser");

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
    isLua ? QStringLiteral(":/scripts/parser/lua") : QStringLiteral(":/scripts/parser/js");
  const auto suffix = isLua ? QStringLiteral(".lua") : QStringLiteral(".js");
  return readTextResource(templateResourcePath(directory, defaultFile, suffix));
}

//--------------------------------------------------------------------------------------------------
// Template accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the template code currently selected for the source.
 */
QString DataModel::FrameParser::templateCode(int sourceId) const
{
  auto it       = m_engines.find(sourceId);
  const int idx = (it != m_engines.end()) ? it->second->templateIdx : -1;

  if (idx < 0 || idx >= m_templateFiles.count())
    return {};

  const int lang   = languageForSource(sourceId);
  const bool isLua = (lang == SerialStudio::Lua);
  const auto directory =
    isLua ? QStringLiteral(":/scripts/parser/lua") : QStringLiteral(":/scripts/parser/js");
  const auto suffix = isLua ? QStringLiteral(".lua") : QStringLiteral(".js");
  return readTextResource(templateResourcePath(directory, m_templateFiles.at(idx), suffix));
}

/**
 * @brief Returns the template index matching the code in either language.
 */
int DataModel::FrameParser::detectTemplate(const QString& code) const
{
  const QString trimmed = code.trimmed();
  if (trimmed.isEmpty())
    return -1;

  for (int i = 0; i < m_templateFiles.size(); ++i) {
    const auto& file = m_templateFiles[i];

    const auto luaPath =
      templateResourcePath(QStringLiteral(":/scripts/parser/lua"), file, QStringLiteral(".lua"));
    const QString luaCode = readTextResource(luaPath).trimmed();
    if (!luaCode.isEmpty() && luaCode == trimmed)
      return i;

    const auto jsPath =
      templateResourcePath(QStringLiteral(":/scripts/parser/js"), file, QStringLiteral(".js"));
    const QString jsCode = readTextResource(jsPath).trimmed();
    if (!jsCode.isEmpty() && jsCode == trimmed)
      return i;
  }

  return -1;
}

/**
 * @brief Returns the localized display names of every available template.
 */
const QStringList& DataModel::FrameParser::templateNames() const
{
  return m_templateNames;
}

/**
 * @brief Returns the resource file basenames of every available template.
 */
const QStringList& DataModel::FrameParser::templateFiles() const
{
  return m_templateFiles;
}

//--------------------------------------------------------------------------------------------------
// Engine management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the scripting language configured for the source.
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
 * @brief Returns (or lazily creates) the script engine for the source.
 */
DataModel::IScriptEngine& DataModel::FrameParser::engineForSource(int sourceId)
{
  Q_ASSERT(sourceId >= 0);

  auto it = m_engines.find(sourceId);
  if (it != m_engines.end())
    return *it->second;

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
 * @brief Loads per-source code into the source engine.
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
 * @brief Destroys the engine for the source (source 0 is reset in place).
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
 * @brief Runs the source's engine over a text frame, falling back to source 0.
 */
QList<QStringList> DataModel::FrameParser::parseMultiFrame(const QString& frame, int sourceId)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(!frame.isEmpty());

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
 * @brief Runs the source's engine over a binary frame, falling back to source 0.
 */
QList<QStringList> DataModel::FrameParser::parseMultiFrame(const QByteArray& frame, int sourceId)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(!frame.isEmpty());

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
 * @brief Validates and loads a frame parser script into the source's engine.
 */
bool DataModel::FrameParser::loadScript(int sourceId, const QString& script, bool showMessageBoxes)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(!script.isEmpty());

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
 */
void DataModel::FrameParser::readCode()
{
  for (auto it = m_engines.begin(); it != m_engines.end();)
    if (it->first != 0)
      it = m_engines.erase(it);
    else
      ++it;

  const int lang0  = languageForSource(0);
  auto it0         = m_engines.find(0);
  const bool isLua = (lang0 == SerialStudio::Lua);
  const bool engineIsLua =
    it0 != m_engines.end() && dynamic_cast<LuaScriptEngine*>(it0->second.get()) != nullptr;

  if (it0 != m_engines.end() && isLua != engineIsLua)
    m_engines.erase(0);

  const auto& model   = ProjectModel::instance();
  const auto& sources = model.sources();
  const bool suppress = m_suppressMessageBoxes || model.suppressMessageBoxes();
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
  for (auto it = m_engines.begin(); it != m_engines.end();)
    if (it->first != 0)
      it = m_engines.erase(it);
    else
      ++it;

  auto it0         = m_engines.find(0);
  const int lang0  = languageForSource(0);
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
    QStringLiteral(":/scripts/parser/templates.json"), "DataModel::FrameParser");

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
 * @brief Loads the template at idx into the source and saves it to the project.
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
 * @brief Loads the default CSV template for the source.
 */
void DataModel::FrameParser::loadDefaultTemplate(int sourceId, bool guiTrigger)
{
  const auto idx = m_templateFiles.indexOf(m_defaultTemplateFile);
  setTemplateIdx(sourceId, idx);

  if (!guiTrigger)
    DataModel::ProjectModel::instance().setModified(false);
}
