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

#include "DataModel/Scripting/FrameParser.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QThread>

#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/CFrameParser.h"
#include "DataModel/Scripting/IScriptEngine.h"
#include "DataModel/Scripting/JsScriptEngine.h"
#include "DataModel/Scripting/LuaScriptEngine.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "DataModel/Scripting/ScriptTemplates.h"
#include "Misc/TimerEvents.h"
#include "Misc/Translator.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the FrameParser singleton and seeds the source-0 engine.
 */
DataModel::FrameParser::FrameParser()
  : m_hasLuaEngine(false), m_suppressMessageBoxes(false), m_engine0Cache(nullptr)
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
      refreshEngineCaches();
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

  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::sourceFrameParserTemplateChanged,
          this,
          &DataModel::FrameParser::readCode);

  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::sourceFrameParserParamsChanged,
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
  if (language == SerialStudio::Native) {
    const auto* tmpl = nativeTemplateById(defaultNativeTemplateId());
    Q_ASSERT(tmpl != nullptr);
    return CFrameParser::buildDescriptor(tmpl->id(), nativeTemplateDefaults(*tmpl));
  }

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
 * @brief Maps a JS/Lua template file basename to its native template id and params.
 */
bool DataModel::FrameParser::nativeEquivalentForFile(const QString& file,
                                                     QString& templateId,
                                                     QJsonObject& params)
{
  // Delimited variants collapse into the parametrized native template
  struct DelimitedVariant {
    QLatin1StringView file;
    QLatin1StringView separator;
  };

  static constexpr DelimitedVariant kDelimited[] = {
    {    QLatin1StringView("comma_separated"),   QLatin1StringView(",")},
    {      QLatin1StringView("tab_separated"), QLatin1StringView("\\t")},
    {     QLatin1StringView("pipe_delimited"),   QLatin1StringView("|")},
    {QLatin1StringView("semicolon_separated"),   QLatin1StringView(";")},
  };

  for (const auto& variant : kDelimited) {
    if (file != variant.file)
      continue;

    const auto* tmpl = nativeTemplateById(QStringLiteral("delimited"));
    Q_ASSERT(tmpl != nullptr);
    if (!tmpl)
      return false;

    templateId = tmpl->id();
    params     = nativeTemplateDefaults(*tmpl);
    params.insert(QStringLiteral("separator"), QString(variant.separator));
    return true;
  }

  // Renamed equivalents; every other native id matches its script file basename
  QString id = file;
  if (file == QStringLiteral("fixed_width_fields"))
    id = QStringLiteral("fixed_width");
  else if (file == QStringLiteral("key_value_pairs"))
    id = QStringLiteral("key_value");

  const auto* tmpl = nativeTemplateById(id);
  if (!tmpl)
    return false;

  templateId = tmpl->id();
  params     = nativeTemplateDefaults(*tmpl);
  return true;
}

/**
 * @brief Maps a native template id (+ params) to the equivalent JS/Lua template file.
 */
QString DataModel::FrameParser::fileForNativeTemplate(const QString& templateId,
                                                      const QJsonObject& params)
{
  Q_ASSERT(!nativeTemplates().isEmpty());

  if (templateId == QStringLiteral("delimited")) {
    const QString separator = SerialStudio::resolveEscapeSequences(
      params.value(QStringLiteral("separator")).toString(QStringLiteral(",")));

    if (separator == QStringLiteral("\t"))
      return QStringLiteral("tab_separated");

    if (separator == QStringLiteral("|"))
      return QStringLiteral("pipe_delimited");

    if (separator == QStringLiteral(";"))
      return QStringLiteral("semicolon_separated");

    return QStringLiteral("comma_separated");
  }

  if (templateId == QStringLiteral("fixed_width"))
    return QStringLiteral("fixed_width_fields");

  if (templateId == QStringLiteral("key_value"))
    return QStringLiteral("key_value_pairs");

  // Every other native id matches its script file basename
  return templateId;
}

/**
 * @brief Returns the template code currently selected for the source.
 */
QString DataModel::FrameParser::templateCode(int sourceId) const
{
  auto it        = m_engines.find(sourceId);
  const int idx  = (it != m_engines.end()) ? it->second->templateIdx : -1;
  const int lang = languageForSource(sourceId);

  if (lang == SerialStudio::Native) {
    const auto& templates = nativeTemplates();
    if (idx < 0 || idx >= templates.size())
      return {};

    const auto* tmpl = templates.at(idx);
    return CFrameParser::buildDescriptor(tmpl->id(), nativeTemplateDefaults(*tmpl));
  }

  if (idx < 0 || idx >= m_templateFiles.count())
    return {};

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

  // Native descriptors are JSON objects; JS/Lua template files never start with a brace
  if (trimmed.startsWith(QLatin1Char('{')))
    return detectNativeTemplate(trimmed);

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
 * @brief Returns the native registry index matching a JSON descriptor, or -1.
 */
int DataModel::FrameParser::detectNativeTemplate(const QString& code) const
{
  Q_ASSERT(!code.isEmpty());

  const auto doc = QJsonDocument::fromJson(code.toUtf8());
  if (!doc.isObject())
    return -1;

  const QString id = doc.object().value(QStringLiteral("template")).toString();
  if (id.isEmpty())
    return -1;

  const auto& templates = nativeTemplates();
  for (int i = 0; i < templates.size(); ++i)
    if (templates.at(i)->id() == id)
      return i;

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
 * @brief Returns the localized template display names for the given language.
 */
const QStringList& DataModel::FrameParser::templateNames(int language) const
{
  if (language == SerialStudio::Native)
    return m_nativeTemplateNames;

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
  if (lang == SerialStudio::Native)
    engine = std::make_unique<CFrameParser>();
  else if (lang == SerialStudio::Lua)
    engine = std::make_unique<LuaScriptEngine>();
  else
    engine = std::make_unique<JsScriptEngine>();

  auto& ref           = *engine;
  m_engines[sourceId] = std::move(engine);
  Q_ASSERT(m_engines.count(sourceId));
  refreshEngineCaches();
  return ref;
}

/**
 * @brief Rebuilds the hot source-0 engine pointer and the table-API (Lua) engine flag.
 */
void DataModel::FrameParser::refreshEngineCaches() noexcept
{
  const auto it0 = m_engines.find(0);
  m_engine0Cache = (it0 != m_engines.end()) ? it0->second.get() : nullptr;

  m_hasLuaEngine = false;
  for (const auto& [id, engine] : m_engines) {
    if (engine->language() == SerialStudio::Lua) {
      m_hasLuaEngine = true;
      break;
    }
  }
}

/**
 * @brief Returns true while any live parser engine exposes the table/dataset script API.
 */
bool DataModel::FrameParser::hasTableApiEngines() const noexcept
{
  return m_hasLuaEngine;
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
  refreshEngineCaches();
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
    if (sourceId == 0 || languageForSource(sourceId) != languageForSource(0))
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
    if (sourceId == 0 || languageForSource(sourceId) != languageForSource(0))
      return {};

    return parseMultiFrame(frame, 0);
  }

  return it->second->parseBinary(frame);
}

/**
 * @brief Runs the source's engine over a UTF-8 text frame, skipping the QString round-trip.
 */
QList<QStringList> DataModel::FrameParser::parseMultiFrameUtf8(const QByteArray& frame,
                                                               int sourceId)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(!frame.isEmpty());

  if (sourceId < 0 || frame.isEmpty()) [[unlikely]]
    return {};

  // Hot single-source path: skip the engine-map walk
  if (sourceId == 0 && m_engine0Cache) [[likely]] {
    if (!m_engine0Cache->isLoaded()) [[unlikely]]
      return {};

    return m_engine0Cache->parseUtf8(frame);
  }

  auto it = m_engines.find(sourceId);
  if (it == m_engines.end() || !it->second->isLoaded()) {
    if (sourceId == 0 || languageForSource(sourceId) != languageForSource(0))
      return {};

    return parseMultiFrameUtf8(frame, 0);
  }

  return it->second->parseUtf8(frame);
}

/**
 * @brief Span fast-path dispatch mirroring parseMultiFrameUtf8's source-0 fallback semantics.
 *        Returns -1 when the source's engine has no span-capable parser.
 */
qsizetype DataModel::FrameParser::parseSpansUtf8(const QByteArray& frame,
                                                 int sourceId,
                                                 QByteArrayView* out,
                                                 qsizetype maxSpans)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(out != nullptr);

  if (sourceId == 0) [[likely]] {
    if (!m_engine0Cache || !m_engine0Cache->isLoaded()) [[unlikely]]
      return -1;

    return m_engine0Cache->parseUtf8Spans(QByteArrayView(frame), out, maxSpans);
  }

  auto it = m_engines.find(sourceId);
  if (it == m_engines.end() || !it->second->isLoaded()) {
    if (languageForSource(sourceId) != languageForSource(0))
      return -1;

    // Same source-0 fallback as the QList path; recursion depth is capped at one
    return parseSpansUtf8(frame, 0, out, maxSpans);
  }

  return it->second->parseUtf8Spans(QByteArrayView(frame), out, maxSpans);
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
  if (it != m_engines.end() && it->second->language() != languageForSource(sourceId)) {
    m_engines.erase(it);
    refreshEngineCaches();
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
 * @brief Returns the script (code or native descriptor) configured for the source.
 */
QString DataModel::FrameParser::scriptForSource(const DataModel::Source& src) const
{
  if (src.frameParserLanguage == SerialStudio::Native) {
    if (src.frameParserTemplate.isEmpty())
      return defaultTemplateCode(SerialStudio::Native);

    return CFrameParser::buildDescriptor(src.frameParserTemplate, src.frameParserParams);
  }

  return src.frameParserCode;
}

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

  auto it0 = m_engines.find(0);
  if (it0 != m_engines.end() && it0->second->language() != languageForSource(0))
    m_engines.erase(it0);

  refreshEngineCaches();

  const auto& model   = ProjectModel::instance();
  const auto& sources = model.sources();
  const bool suppress = m_suppressMessageBoxes || model.suppressMessageBoxes();
  const QString code  = sources.empty() ? QString() : scriptForSource(sources[0]);

  if (!code.isEmpty())
    (void)loadScript(0, code, !suppress);

  for (const auto& src : sources) {
    const QString script = (src.sourceId > 0) ? scriptForSource(src) : QString();
    if (!script.isEmpty())
      (void)loadScript(src.sourceId, script, false);
  }

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

  auto it0 = m_engines.find(0);
  if (it0 != m_engines.end() && it0->second->language() != languageForSource(0))
    m_engines.erase(it0);

  refreshEngineCaches();

  const auto& sources = ProjectModel::instance().sources();
  const QString code  = sources.empty() ? QString() : scriptForSource(sources[0]);

  if (!code.isEmpty())
    (void)loadScript(0, code, !m_suppressMessageBoxes);

  for (const auto& src : sources) {
    const QString script = (src.sourceId > 0) ? scriptForSource(src) : QString();
    if (!script.isEmpty())
      (void)loadScript(src.sourceId, script, false);
  }
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
  m_nativeTemplateNames.clear();

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

  // Native names come from tr() in the registry, so rebuild on language change too
  const auto& native = nativeTemplates();
  for (const auto* tmpl : native)
    m_nativeTemplateNames.append(tmpl->name());

  Q_EMIT templateNamesChanged();
}

/**
 * @brief Loads the template at idx into the source and saves it to the project.
 */
void DataModel::FrameParser::setTemplateIdx(int sourceId, int idx)
{
  if (languageForSource(sourceId) == SerialStudio::Native) {
    setNativeTemplateIdx(sourceId, idx);
    return;
  }

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
 * @brief Persists the native template at idx (with schema defaults) for the source.
 */
void DataModel::FrameParser::setNativeTemplateIdx(int sourceId, int idx)
{
  Q_ASSERT(sourceId >= 0);

  const auto& templates = nativeTemplates();
  if (idx < 0 || idx >= templates.size())
    return;

  const auto* tmpl = templates.at(idx);
  Q_ASSERT(tmpl != nullptr);

  engineForSource(sourceId).templateIdx = idx;

  // Params land first so the template-change reload never sees a stale custom config
  auto& model = DataModel::ProjectModel::instance();
  model.updateSourceFrameParserParams(sourceId, nativeTemplateDefaults(*tmpl));
  model.updateSourceFrameParserTemplate(sourceId, tmpl->id());
  model.setModified(true);
}

/**
 * @brief Loads the default CSV template for the source.
 */
void DataModel::FrameParser::loadDefaultTemplate(int sourceId, bool guiTrigger)
{
  const bool native = (languageForSource(sourceId) == SerialStudio::Native);
  const auto idx    = native ? 0 : m_templateFiles.indexOf(m_defaultTemplateFile);
  setTemplateIdx(sourceId, idx);

  if (!guiTrigger)
    DataModel::ProjectModel::instance().setModified(false);
}
