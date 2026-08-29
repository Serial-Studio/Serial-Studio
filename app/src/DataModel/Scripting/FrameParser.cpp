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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/Scripting/FrameParser.h"

#include <QCoreApplication>
#include <QThread>

#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/CFrameParser.h"
#include "DataModel/Scripting/IScriptEngine.h"
#include "DataModel/Scripting/JsScriptEngine.h"
#include "DataModel/Scripting/LuaScriptEngine.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "IO/PipelineHost.h"
#include "Misc/TimerEvents.h"
#include "Misc/Translator.h"
#include "SerialStudio.h"
#include "SessionContext.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the FrameParser singleton and seeds the source-0 engine.
 */
DataModel::FrameParser::FrameParser()
  : m_hasLuaEngine(false)
  , m_suppressMessageBoxes(false)
  , m_languagesDirty(true)
  , m_engineEpoch(0)
  , m_engine0Cache(nullptr)
  , m_statsMirrorRing(kStatsMirrorSlots)
{
  (void)engineForSource(0);

  static auto& timerEvents = Misc::TimerEvents::instance();
  connect(
    &timerEvents, &Misc::TimerEvents::timeout1Hz, this, &DataModel::FrameParser::collectGarbage);
  connect(&timerEvents, &Misc::TimerEvents::timeout1Hz, this, [this] { publishScriptStats(); });

  static auto& translator = Misc::Translator::instance();
  connect(&translator,
          &Misc::Translator::languageChanged,
          this,
          &DataModel::FrameParser::loadTemplateNames);

  if (auto* app = qApp)
    connect(app, &QCoreApplication::aboutToQuit, this, &DataModel::FrameParser::prepareShutdown);

  loadTemplateNames();
}

/**
 * @brief Destroys every parser engine on the parser's own thread. Runs queued ahead of the
 *        pipeline thread's quit() (PipelineHost::shutdown) so Lua states and QJSEngines die on
 *        the thread that owns them; the aboutToQuit connection is the fallback for paths that
 *        never start the pipeline.
 */
void DataModel::FrameParser::prepareShutdown()
{
  releaseEngines();
}

/**
 * @brief Destroys every parser engine from the thread that owns them: a lua_State and a QJSEngine
 *        may only be used and destroyed on their creating thread, so every thread hand-off of the
 *        parser (PipelineHost::moveProcessingObjectsTo) drops the engines first and rebuilds them
 *        on the new owner.
 */
void DataModel::FrameParser::releaseEngines()
{
  SS_ASSERT(QThread::currentThread() == this->thread(), return);

  m_engines.clear();
  refreshEngineCaches();
  SS_ASSERT_LOG(m_engines.empty());
}

/**
 * @brief Returns this session's frame parser. The object is owned by the SessionContext and
 *        built by the composition root, so a reach before adoption is a named fatal instead of
 *        an out-of-order lazy construction (spec 0039 M2, wave B2).
 */
DataModel::FrameParser& DataModel::FrameParser::instance()
{
  return SessionContext::current().frameParser();
}

//--------------------------------------------------------------------------------------------------
// External connections
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires ProjectModel signals and runs the initial script load.
 */
void DataModel::FrameParser::setupExternalConnections()
{
  auto& model = DataModel::ProjectModel::instance();

  connect(&model,
          &DataModel::ProjectModel::frameParserCodeChanged,
          this,
          &DataModel::FrameParser::readCode);

  connect(&model,
          &DataModel::ProjectModel::sourceFrameParserTemplateChanged,
          this,
          &DataModel::FrameParser::reloadSourceCode);

  connect(&model,
          &DataModel::ProjectModel::sourceFrameParserParamsChanged,
          this,
          &DataModel::FrameParser::reloadSourceCode);

  connect(&model, &DataModel::ProjectModel::sourceStructureChanged, this, [this] {
    m_languagesDirty = true;
  });

  connect(&model, &DataModel::ProjectModel::sourceFrameParserLanguageChanged, this, [this] {
    m_languagesDirty = true;
  });

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
  return ParserTemplateCatalog::defaultCode(language);
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
  return ParserTemplateCatalog::nativeEquivalentForFile(file, templateId, params);
}

/**
 * @brief Maps a native template id (+ params) to the equivalent JS/Lua template file.
 */
QString DataModel::FrameParser::fileForNativeTemplate(const QString& templateId,
                                                      const QJsonObject& params)
{
  return ParserTemplateCatalog::fileForNativeTemplate(templateId, params);
}

/**
 * @brief Returns the template code currently selected for the source.
 */
QString DataModel::FrameParser::templateCode(int sourceId) const
{
  auto it       = m_engines.find(sourceId);
  const int idx = (it != m_engines.end()) ? it->second->templateIdx : -1;

  return m_templates.codeForIndex(idx, languageForSource(sourceId));
}

/**
 * @brief Returns the template index matching the code in either language.
 */
int DataModel::FrameParser::detectTemplate(const QString& code) const
{
  return m_templates.detect(code);
}

/**
 * @brief Returns the localized display names of every available template.
 */
const QStringList& DataModel::FrameParser::templateNames() const
{
  return m_templates.names();
}

/**
 * @brief Returns the localized template display names for the given language.
 */
const QStringList& DataModel::FrameParser::templateNames(int language) const
{
  if (language == SerialStudio::Native)
    return m_templates.nativeNames();

  return m_templates.names();
}

/**
 * @brief Returns the resource file basenames of every available template.
 */
const QStringList& DataModel::FrameParser::templateFiles() const
{
  return m_templates.files();
}

//--------------------------------------------------------------------------------------------------
// Engine management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the per-source language mirror from the GUI-owned source list. One marshal per
 *        structural/language change, never per frame: the multi-source parse fallback compares
 *        languages on every frame, and a blocking GUI hop there parks the whole pipeline.
 */
void DataModel::FrameParser::refreshSourceLanguages() const
{
  m_sourceLanguages.clear();
  IO::PipelineHost::runOnGuiThreadBlocking([this] {
    static auto& projectModel = ProjectModel::instance();
    for (const auto& src : projectModel.sources())
      m_sourceLanguages[src.sourceId] = src.frameParserLanguage;
  });

  m_languagesDirty = false;
}

/**
 * @brief Returns the scripting language configured for the source, from the mirror refreshed by
 *        the same project signals that rebuild the engines. Unknown sources answer JavaScript,
 *        matching the pre-mirror lookup's miss behaviour.
 */
int DataModel::FrameParser::languageForSource(int sourceId) const
{
  if (m_languagesDirty) [[unlikely]]
    refreshSourceLanguages();

  const auto it = m_sourceLanguages.find(sourceId);
  return (it != m_sourceLanguages.end()) ? it->second : SerialStudio::JavaScript;
}

/**
 * @brief Returns (or lazily creates) the script engine for the source.
 */
DataModel::IScriptEngine& DataModel::FrameParser::engineForSource(int sourceId)
{
  SS_ASSERT(sourceId >= 0, sourceId = 0);

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
  SS_ASSERT_LOG(m_engines.count(sourceId) > 0);
  refreshEngineCaches();
  return ref;
}

/**
 * @brief Rebuilds the hot source-0 engine pointer and the table-API (Lua) engine flag, then bumps
 *        m_engineEpoch as a cheap change signal: FrameBuilder re-derives its dataset capture flag
 *        per frame whenever this counter moves.
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

  ++m_engineEpoch;
}

/**
 * @brief Returns true while any live parser engine exposes the table/dataset script API.
 */
bool DataModel::FrameParser::hasTableApiEngines() const noexcept
{
  return m_hasLuaEngine;
}

/**
 * @brief Monotonic engine-set change counter; lets per-frame callers poll with one compare.
 */
int DataModel::FrameParser::engineEpoch() const noexcept
{
  return m_engineEpoch;
}

/**
 * @brief Snapshots every live parser engine's error statistics for the 1 Hz diagnostics sample.
 *        The engine map is rebuilt on the parser thread, so the GUI reads the mirror published on
 *        that thread's own 1 Hz tick and other threads marshal: a GUI marshal spins a nested loop,
 *        which macOS runs re-entrantly and which swallows window resize steps.
 */
QList<DataModel::ScriptStat> DataModel::FrameParser::scriptStats()
{
  if (QThread::currentThread() != thread()) {
    if (qApp && QThread::currentThread() == qApp->thread())
      return guiScriptStats();

    QList<ScriptStat> stats;
    IO::PipelineHost::runOnObjectThread(this, [&] { stats = scriptStats(); });
    return stats;
  }

  QList<ScriptStat> stats;
  stats.reserve(static_cast<qsizetype>(m_engines.size()));

  for (const auto& [sourceId, engine] : m_engines) {
    if (!engine)
      continue;

    ScriptStat stat{};
    stat.sourceId            = sourceId;
    stat.language            = engine->language();
    stat.disabled            = engine->disabled();
    stat.consecutiveTimeouts = engine->consecutiveTimeouts();
    stat.errorCount          = engine->errorCount();
    stat.lastError           = engine->lastError();
    stats.append(stat);
  }

  return stats;
}

/**
 * @brief Parser-thread half of the stats mirror: publishes the current sample for the GUI on the
 *        same 1 Hz tick the diagnostics run at, so the reader never has to marshal.
 */
void DataModel::FrameParser::publishScriptStats()
{
  SS_ASSERT(QThread::currentThread() == thread(), return);

  auto sample = std::make_shared<const QList<ScriptStat>>(scriptStats());
  (void)m_statsMirrorRing.try_enqueue(std::move(sample));
}

/**
 * @brief GUI-thread read of the parser stats: adopts the newest published sample and serves it.
 *        One tick of staleness is inherent to a 1 Hz diagnostic and costs nothing here.
 */
QList<DataModel::ScriptStat> DataModel::FrameParser::guiScriptStats()
{
  ScriptStatsPtr sample;
  // code-verify off
  // Ring drain: bounded by the mirror ring capacity (4), provably finite per call.
  while (m_statsMirrorRing.try_dequeue(sample))
    if (sample)
      m_guiScriptStats = sample;
  // code-verify on

  sample.reset();
  return m_guiScriptStats ? *m_guiScriptStats : QList<ScriptStat>();
}

/**
 * @brief Loads per-source code into the source engine.
 */
void DataModel::FrameParser::setSourceCode(int sourceId, const QString& code)
{
  SS_ASSERT(sourceId >= 0, return);

  if (QThread::currentThread() != thread()) {
    IO::PipelineHost::runOnObjectThread(this,
                                        [this, sourceId, &code] { setSourceCode(sourceId, code); });
    return;
  }

  SS_ASSERT_LOG(m_engines.count(0) > 0);

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
  if (QThread::currentThread() != thread()) {
    IO::PipelineHost::runOnObjectThread(this, [this, sourceId] { clearSourceEngine(sourceId); });
    return;
  }

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
  if (sourceId < 0 || frame.isEmpty()) [[unlikely]]
    return {};

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
  SS_ASSERT(sourceId >= 0, return -1);
  SS_ASSERT(out != nullptr, return -1);

  if (sourceId == 0) [[likely]] {
    if (!m_engine0Cache || !m_engine0Cache->isLoaded()) [[unlikely]]
      return -1;

    return m_engine0Cache->parseUtf8Spans(QByteArrayView(frame), out, maxSpans);
  }

  auto it = m_engines.find(sourceId);
  if (it == m_engines.end() || !it->second->isLoaded()) {
    if (languageForSource(sourceId) != languageForSource(0))
      return -1;

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
  SS_ASSERT(sourceId >= 0, return false);
  SS_ASSERT(!script.isEmpty(), return false);

  if (QThread::currentThread() != thread()) {
    bool loaded = false;
    IO::PipelineHost::runOnObjectThread(
      this, [&] { loaded = loadScript(sourceId, script, showMessageBoxes); });
    return loaded;
  }

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
  if (QThread::currentThread() != thread()) {
    IO::PipelineHost::runOnObjectThread(this,
                                        [this, suppress] { setSuppressMessageBoxes(suppress); });
    return;
  }

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
  if (QThread::currentThread() != thread()) {
    IO::PipelineHost::runOnObjectThread(this, [this] { readCode(); });
    return;
  }

  m_languagesDirty = true;

  for (auto it = m_engines.begin(); it != m_engines.end();)
    if (it->first != 0)
      it = m_engines.erase(it);
    else
      ++it;

  auto it0 = m_engines.find(0);
  if (it0 != m_engines.end() && it0->second->language() != languageForSource(0))
    m_engines.erase(it0);

  refreshEngineCaches();

  std::vector<DataModel::Source> sources;
  bool modelSuppress = false;
  IO::PipelineHost::runOnGuiThreadBlocking([&] {
    static auto& model = ProjectModel::instance();
    sources            = model.sources();
    modelSuppress      = model.suppressMessageBoxes();
  });

  const bool suppress = m_suppressMessageBoxes || modelSuppress;
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
 * @brief Reloads only @p sourceId's engine after that source's parser template/params change,
 *        leaving every other source's engine (and its accumulated cross-frame Native/latch state)
 *        untouched. Tearing down all engines on any single edit discarded live parser state.
 */
void DataModel::FrameParser::reloadSourceCode(int sourceId)
{
  if (sourceId < 0)
    return;

  if (QThread::currentThread() != thread()) {
    IO::PipelineHost::runOnObjectThread(this, [this, sourceId] { reloadSourceCode(sourceId); });
    return;
  }

  bool haveTarget    = false;
  bool modelSuppress = false;
  DataModel::Source target;
  IO::PipelineHost::runOnGuiThreadBlocking([&] {
    static auto& model  = ProjectModel::instance();
    modelSuppress       = model.suppressMessageBoxes();
    const auto& sources = model.sources();
    for (const auto& src : sources)
      if (src.sourceId == sourceId) {
        target     = src;
        haveTarget = true;
        return;
      }
  });

  if (!haveTarget) {
    const auto it = m_engines.find(sourceId);
    if (it != m_engines.end()) {
      m_engines.erase(it);
      refreshEngineCaches();
    }

    Q_EMIT modifiedChanged();
    return;
  }

  const bool suppress  = m_suppressMessageBoxes || modelSuppress;
  const QString script = scriptForSource(target);
  if (!script.isEmpty())
    (void)loadScript(sourceId, script, sourceId == 0 && !suppress);

  Q_EMIT modifiedChanged();
}

/**
 * @brief Resets the execution context by re-loading all current code.
 */
void DataModel::FrameParser::clearContext()
{
  if (QThread::currentThread() != thread()) {
    IO::PipelineHost::runOnObjectThread(this, [this] { clearContext(); });
    return;
  }

  for (auto it = m_engines.begin(); it != m_engines.end();)
    if (it->first != 0)
      it = m_engines.erase(it);
    else
      ++it;

  auto it0 = m_engines.find(0);
  if (it0 != m_engines.end() && it0->second->language() != languageForSource(0))
    m_engines.erase(it0);

  refreshEngineCaches();

  std::vector<DataModel::Source> sources;
  IO::PipelineHost::runOnGuiThreadBlocking([&] {
    static auto& projectModel = ProjectModel::instance();
    sources                   = projectModel.sources();
  });

  const QString code = sources.empty() ? QString() : scriptForSource(sources[0]);

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
  m_templates.reload();
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

  if (idx < 0 || idx >= m_templates.fileCount())
    return;

  bool loaded = false;
  QString code;
  IO::PipelineHost::runOnObjectThread(this, [&] {
    engineForSource(sourceId).templateIdx = idx;
    code                                  = templateCode(sourceId);
    loaded                                = loadScript(sourceId, code, !m_suppressMessageBoxes);
  });

  if (loaded) {
    IO::PipelineHost::runOnGuiThreadBlocking([&] {
      static auto& model = DataModel::ProjectModel::instance();
      if (sourceId == 0)
        model.setFrameParserCode(code);
      else
        model.updateSourceFrameParser(sourceId, code);

      model.setModified(true);
    });
  }
}

/**
 * @brief Persists the native template at idx (with schema defaults) for the source; params are
 *        written before the template id so the reload that the template change triggers never
 *        sees a stale custom config.
 */
void DataModel::FrameParser::setNativeTemplateIdx(int sourceId, int idx)
{
  SS_ASSERT(sourceId >= 0, return);

  const auto& templates = nativeTemplates();
  if (idx < 0 || idx >= templates.size())
    return;

  const auto* tmpl = templates.at(idx);
  SS_ASSERT(tmpl != nullptr, return);

  IO::PipelineHost::runOnObjectThread(this, [&] { engineForSource(sourceId).templateIdx = idx; });

  IO::PipelineHost::runOnGuiThreadBlocking([&] {
    static auto& model = DataModel::ProjectModel::instance();
    model.updateSourceFrameParserParams(sourceId, nativeTemplateDefaults(*tmpl));
    model.updateSourceFrameParserTemplate(sourceId, tmpl->id());
    model.setModified(true);
  });
}

/**
 * @brief Loads the default CSV template for the source.
 */
void DataModel::FrameParser::loadDefaultTemplate(int sourceId, bool guiTrigger)
{
  const bool native = (languageForSource(sourceId) == SerialStudio::Native);
  const int idx     = native ? 0 : m_templates.defaultFileIndex();
  setTemplateIdx(sourceId, idx);

  if (!guiTrigger) {
    IO::PipelineHost::runOnGuiThreadBlocking([] {
      static auto& projectModel = DataModel::ProjectModel::instance();
      projectModel.setModified(false);
    });
  }
}
