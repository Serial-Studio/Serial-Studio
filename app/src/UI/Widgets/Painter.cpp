/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * Pro feature -- requires the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "UI/Widgets/Painter.h"

#  include <chrono>
#  include <cmath>
#  include <QDateTime>
#  include <QFileInfo>
#  include <QPainter>
#  include <QQuickWindow>

#  include "AppState.h"
#  include "DataModel/Frame.h"
#  include "DataModel/FrameBuilder.h"
#  include "DataModel/NotificationCenter.h"
#  include "DataModel/Scripting/DashboardApi.h"
#  include "DataModel/Scripting/DeviceWriteApi.h"
#  include "DataModel/Scripting/ScriptApiCall.h"
#  include "Misc/ThemeManager.h"
#  include "Misc/TimerEvents.h"
#  include "SerialStudio.h"
#  include "UI/Dashboard.h"
#  include "UI/Widgets/PainterContext.h"
#  include "UI/Widgets/PainterDataBridge.h"

static constexpr int kPainterWatchdogMs = 250;
static constexpr int kSlowPaintMs       = 30;

/**
 * @brief Returns the JS bootstrap that exposes bridge globals + redirects console.
 */
[[nodiscard]] static QString jsBootstrap()
{
  return QStringLiteral(
    "function __ssDataset(i) {"
    "  return Object.freeze({"
    "    get index()      { return __pp.dsIndex(i); },"
    "    get uniqueId()   { return __pp.dsUniqueId(i); },"
    "    get title()      { return __pp.dsTitle(i); },"
    "    get units()      { return __pp.dsUnits(i); },"
    "    get widget()     { return __pp.dsWidget(i); },"
    "    get value()      { return __pp.dsValue(i); },"
    "    get rawValue()   { return __pp.dsRawValue(i); },"
    "    get text()       { return __pp.dsValueStr(i); },"
    "    get rawText()    { return __pp.dsRawValueStr(i); },"
    "    get min()        { return __pp.dsMin(i); },"
    "    get max()        { return __pp.dsMax(i); },"
    "    get widgetMin()  { return __pp.dsWidgetMin(i); },"
    "    get widgetMax()  { return __pp.dsWidgetMax(i); },"
    "    get plotMin()    { return __pp.dsPlotMin(i); },"
    "    get plotMax()    { return __pp.dsPlotMax(i); },"
    "    get fftMin()     { return __pp.dsFftMin(i); },"
    "    get fftMax()     { return __pp.dsFftMax(i); },"
    "    get alarmLow()   { return __pp.dsAlarmLow(i); },"
    "    get alarmHigh()  { return __pp.dsAlarmHigh(i); },"
    "    get alarmBands() { return __pp.dsAlarmBands(i); },"
    "    get ledHigh()    { return __pp.dsLedHigh(i); },"
    "    get hasPlot()    { return __pp.dsHasPlot(i); },"
    "    get hasFft()     { return __pp.dsHasFft(i); },"
    "    get hasLed()     { return __pp.dsHasLed(i); }"
    "  });"
    "}"
    "var group = Object.freeze({"
    "  get id()       { return __pp.gId(); },"
    "  get title()    { return __pp.gTitle(); },"
    "  get columns()  { return __pp.gColumns(); },"
    "  get sourceId() { return __pp.gSourceId(); }"
    "});"
    "var datasets = Object.freeze(new Proxy({}, {"
    "  get: function(_, k) {"
    "    if (k === 'length') return __pp.dsCount();"
    "    var i = Number(k);"
    "    if (Number.isInteger(i) && i >= 0 && i < __pp.dsCount()) return __ssDataset(i);"
    "    return undefined;"
    "  },"
    "  has: function(_, k) {"
    "    if (k === 'length') return true;"
    "    var i = Number(k);"
    "    return Number.isInteger(i) && i >= 0 && i < __pp.dsCount();"
    "  },"
    "  set: function() { return false; }"
    "}));"
    "var frame = Object.freeze({"
    "  get number()      { return __pp.frameNumber(); },"
    "  get timestampMs() { return __pp.frameTimestampMs(); }"
    "});"
    "function __ssFmt(args) {"
    "  return Array.prototype.slice.call(args).map(function(a) {"
    "    if (a === undefined) return 'undefined';"
    "    if (a === null)      return 'null';"
    "    try { return typeof a === 'string' ? a : JSON.stringify(a); }"
    "    catch (e) { return String(a); }"
    "  }).join(' ');"
    "}"
    "try {"
    "  console = {"
    "    log:   function() { __pp.log(__ssFmt(arguments)); },"
    "    info:  function() { __pp.log(__ssFmt(arguments)); },"
    "    debug: function() { __pp.log(__ssFmt(arguments)); },"
    "    warn:  function() { __pp.warn(__ssFmt(arguments)); },"
    "    error: function() { __pp.error(__ssFmt(arguments)); }"
    "  };"
    "} catch (e) { /* QJSEngine console is non-writable in some builds; leave default */ }");
}

//--------------------------------------------------------------------------------------------------
// Construction / destruction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the painter widget bound to a dashboard slot.
 */
Widgets::Painter::Painter(int index, QQuickItem* parent)
  : QQuickPaintedItem(parent)
  , m_index(index)
  , m_compileDirty(true)
  , m_runtimeOk(false)
  , m_bootstrapOk(false)
  , m_hasOnFrame(false)
  , m_previewMode(false)
  , m_slowPaintWarned(false)
  , m_slowPaintStreak(0)
  , m_frameSeq(0)
  , m_watchdog(&m_engine, kPainterWatchdogMs, QStringLiteral("Painter"))
  , m_ctx(new PainterContext(this))
  , m_bridge(new PainterDataBridge(this))
{
  setOpaquePainting(false);
  setFlag(ItemHasContents, true);
  setAcceptedMouseButtons(Qt::NoButton);

  m_engine.installExtensions(QJSEngine::ConsoleExtension | QJSEngine::GarbageCollectionExtension);
  DataModel::NotificationCenter::installScriptApi(&m_engine);
  DataModel::FrameBuilder::instance().injectTableApiJS(&m_engine);

  // Closed-loop control APIs -- deviceWrite() default source is refreshed in updateData()
  DataModel::DeviceWriteApi::installJS(&m_engine, /*defaultSourceId=*/0);
  DataModel::ActionFireApi::installJS(&m_engine);

  // Dashboard helpers (clearPlots, setPlotPoints, UI toggles, setActiveWorkspace)
  DataModel::DashboardApi::installJS(&m_engine);

  // Generic API gateway (apiCall) -- painter widgets share the source-0 rate bucket
  DataModel::ScriptApiCall::installJS(&m_engine, 0);

  const auto projectPath = AppState::instance().projectFilePath();
  if (!projectPath.isEmpty())
    m_ctx->setProjectDirectory(QFileInfo(projectPath).absolutePath());

  // Expose the per-widget bridge as __pp
  auto bridgeJsValue = m_engine.newQObject(m_bridge);
  m_engine.globalObject().setProperty(QStringLiteral("__pp"), bridgeJsValue);

  auto ctxJsValue = m_engine.newQObject(m_ctx);
  m_engine.globalObject().setProperty(QStringLiteral("__ctx"), ctxJsValue);
  m_ctxValue = ctxJsValue;

  // Install the JS-side bridge wrappers (datasets/group/frame/console) before any user code runs
  installBootstrap();
  installTheme();

  connect(m_bridge, &PainterDataBridge::consoleLine, this, &Painter::consoleLine);

  // Re-inject theme and force recompile on palette change
  connect(&Misc::ThemeManager::instance(), &Misc::ThemeManager::themeChanged, this, [this]() {
    installTheme();
    m_compileDirty = true;
  });

  connect(&UI::Dashboard::instance(), &UI::Dashboard::updated, this, &Painter::updateData);
  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::uiTimeout, this, [this] {
    if (!isVisible())
      return;

    if (m_previewMode)
      tickPreview();
    else if (m_runtimeOk)
      update();
  });
}

/**
 * @brief Destroys the painter widget; QJSEngine + child QObjects clean up automatically.
 */
Widgets::Painter::~Painter() = default;

//--------------------------------------------------------------------------------------------------
// QQuickPaintedItem hook
//--------------------------------------------------------------------------------------------------

/**
 * @brief Blits the cached QImage produced by the last script execution.
 */
void Widgets::Painter::paint(QPainter* painter)
{
  if (m_cache.isNull())
    return;

  painter->drawImage(QRectF(0.0, 0.0, width(), height()), m_cache);
}

//--------------------------------------------------------------------------------------------------
// Public surface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true once paint() has compiled and at least one frame succeeded.
 */
bool Widgets::Painter::runtimeOk() const noexcept
{
  return m_runtimeOk;
}

/**
 * @brief Returns the most recent compile or runtime error message.
 */
QString Widgets::Painter::lastError() const
{
  return m_lastError;
}

/**
 * @brief Replaces the user JS and recompiles on the next tick.
 */
void Widgets::Painter::setUserCode(const QString& code)
{
  if (code == m_userCode)
    return;

  m_userCode = code;
  invalidateCompilation();
}

/**
 * @brief Switches between live-dashboard mode and synthetic preview mode.
 */
void Widgets::Painter::setPreviewMode(bool enabled)
{
  if (m_previewMode == enabled)
    return;

  m_previewMode = enabled;
  if (m_previewMode) {
    if (m_previewGroup.title.isEmpty())
      m_previewGroup.title = QStringLiteral("Preview");

    m_bridge->setGroup(&m_previewGroup);
  } else {
    m_bridge->setGroup(nullptr);
  }
}

/**
 * @brief Decodes a QVariantList of alarm-band maps into a typed AlarmBand vector.
 */
static void readBandsFromVariantList(const QVariantList& list,
                                     std::vector<DataModel::AlarmBand>& out)
{
  out.reserve(list.size());
  for (const auto& b : list) {
    const auto m = b.toMap();
    DataModel::AlarmBand band;
    band.min   = m.value(QStringLiteral("min")).toDouble();
    band.max   = m.value(QStringLiteral("max")).toDouble();
    band.color = m.value(QStringLiteral("color")).toString();
    band.label = m.value(QStringLiteral("label")).toString();
    const int sev =
      m.value(QStringLiteral("severity"), static_cast<int>(DataModel::AlarmSeverity::Warning))
        .toInt();
    band.severity = static_cast<DataModel::AlarmSeverity>(qBound(0, sev, 3));
    if (band.max > band.min)
      out.push_back(std::move(band));
  }
}

/**
 * @brief Synthesises Warning bands from legacy alarmLow / alarmHigh entries on a preview dataset.
 */
static void synthesizeLegacyAlarmBands(const QVariantMap& entry, DataModel::Dataset& ds)
{
  const double lo       = entry.value(QStringLiteral("alarmLow"), ds.wgtMin).toDouble();
  const double hi       = entry.value(QStringLiteral("alarmHigh"), ds.wgtMax).toDouble();
  const double rangeMin = qMin(ds.wgtMin, ds.wgtMax);
  const double rangeMax = qMax(ds.wgtMin, ds.wgtMax);
  if (lo > rangeMin && lo < rangeMax) {
    DataModel::AlarmBand low;
    low.min      = rangeMin;
    low.max      = lo;
    low.severity = DataModel::AlarmSeverity::Warning;
    ds.alarmBands.push_back(std::move(low));
  }

  if (hi > rangeMin && hi < rangeMax) {
    DataModel::AlarmBand high;
    high.min      = hi;
    high.max      = rangeMax;
    high.severity = DataModel::AlarmSeverity::Warning;
    ds.alarmBands.push_back(std::move(high));
  }
}

/**
 * @brief Pushes a list of synthetic dataset values into the preview group.
 */
void Widgets::Painter::setSimulatedDatasets(const QVariantList& datasets)
{
  m_previewGroup.datasets.clear();
  m_previewGroup.datasets.reserve(static_cast<size_t>(datasets.size()));

  for (int i = 0; i < datasets.size(); ++i) {
    const auto entry = datasets.at(i).toMap();

    DataModel::Dataset ds;
    ds.index    = i;
    ds.uniqueId = i + 1;
    ds.title = entry.value(QStringLiteral("title"), QStringLiteral("DS%1").arg(i + 1)).toString();
    ds.units = entry.value(QStringLiteral("units")).toString();
    ds.numericValue    = entry.value(QStringLiteral("value"), 0.0).toDouble();
    ds.rawNumericValue = entry.value(QStringLiteral("rawValue"), ds.numericValue).toDouble();
    ds.value           = QString::number(ds.numericValue);
    ds.rawValue        = QString::number(ds.rawNumericValue);
    ds.wgtMin          = entry.value(QStringLiteral("min"), 0.0).toDouble();
    ds.wgtMax          = entry.value(QStringLiteral("max"), 100.0).toDouble();

    // Accept bands explicitly; otherwise synthesise from legacy alarmLow/alarmHigh entries.
    const auto bandsVar = entry.value(QStringLiteral("alarmBands"));
    if (bandsVar.canConvert<QVariantList>())
      readBandsFromVariantList(bandsVar.toList(), ds.alarmBands);

    else if (entry.contains(QStringLiteral("alarmLow"))
             || entry.contains(QStringLiteral("alarmHigh")))
      synthesizeLegacyAlarmBands(entry, ds);

    m_previewGroup.datasets.push_back(std::move(ds));
  }
}

/**
 * @brief Sets the title surfaced as `group.title` while in preview mode.
 */
void Widgets::Painter::setPreviewGroupTitle(const QString& title)
{
  m_previewGroup.title = title;
}

//--------------------------------------------------------------------------------------------------
// Per-tick refresh
//--------------------------------------------------------------------------------------------------

/**
 * @brief Pulls the latest group snapshot, optionally calls onFrame(), repaints cache.
 */
void Widgets::Painter::updateData()
{
  if (m_previewMode)
    return;

  if (m_index < 0)
    return;

  const auto* dashboard = &UI::Dashboard::instance();
  const auto count      = dashboard->widgetCount(SerialStudio::DashboardPainter);
  if (m_index >= count)
    return;

  const auto& group = dashboard->getGroupWidget(SerialStudio::DashboardPainter, m_index);
  m_bridge->setGroup(&group);
  m_bridge->setFrame(++m_frameSeq, QDateTime::currentMSecsSinceEpoch());

  // Default deviceWrite() sourceId follows the painter's group
  DataModel::DeviceWriteApi::setJSDefaultSourceId(&m_engine, group.sourceId);

  // Lazy-compile when user code changed; failed compile leaves runtimeOk false
  if (m_compileDirty) {
    if (!compile(m_userCode))
      return;
  }

  if (!m_runtimeOk)
    return;

  if (m_hasOnFrame) {
    QJSValueList noArgs;
    auto r = m_watchdog.call(m_onFrameFn, noArgs);
    if (r.isError()) [[unlikely]] {
      setLastError(QStringLiteral("onFrame: ") + r.property(QStringLiteral("message")).toString());
      setRuntimeOk(false);
      return;
    }
  }

  renderFrame();
}

/**
 * @brief Preview-mode tick: same as updateData() but feeds from m_previewGroup.
 */
void Widgets::Painter::tickPreview()
{
  if (!m_previewMode)
    return;

  m_bridge->setFrame(++m_frameSeq, QDateTime::currentMSecsSinceEpoch());

  if (m_compileDirty) {
    if (!compile(m_userCode))
      return;
  }

  if (!m_runtimeOk)
    return;

  if (m_hasOnFrame) {
    QJSValueList noArgs;
    auto r = m_watchdog.call(m_onFrameFn, noArgs);
    if (r.isError()) [[unlikely]] {
      setLastError(QStringLiteral("onFrame: ") + r.property(QStringLiteral("message")).toString());
      setRuntimeOk(false);
      return;
    }
  }

  renderFrame();
  update();
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Installs the JS bridge wrappers and verifies they reached globalThis.
 */
void Widgets::Painter::installBootstrap()
{
  m_bootstrapOk = false;

  auto result = m_engine.evaluate(jsBootstrap(), QStringLiteral("painter_bootstrap.js"));
  if (result.isError()) [[unlikely]] {
    setLastError(QStringLiteral("bootstrap: ")
                 + result.property(QStringLiteral("message")).toString() + QStringLiteral(" (line ")
                 + result.property(QStringLiteral("lineNumber")).toString() + QStringLiteral(")"));
    return;
  }

  // Defensive sanity check
  auto global       = m_engine.globalObject();
  const auto bridge = global.property(QStringLiteral("__pp"));
  const auto ds     = global.property(QStringLiteral("datasets"));
  const auto grp    = global.property(QStringLiteral("group"));
  const auto frm    = global.property(QStringLiteral("frame"));
  if (!bridge.isObject() || !ds.isObject() || !grp.isObject() || !frm.isObject()) [[unlikely]] {
    setLastError(QStringLiteral("bootstrap: bridge globals missing after evaluate"));
    return;
  }

  m_bootstrapOk = true;
}

/**
 * @brief Snapshots the ThemeManager palette into a frozen `theme` global.
 */
void Widgets::Painter::installTheme()
{
  const auto& palette = Misc::ThemeManager::instance().colors();
  auto themeObject    = m_engine.newObject();
  for (auto it = palette.constBegin(); it != palette.constEnd(); ++it) {
    const auto v = it.value();
    if (v.typeId() == QMetaType::QVariantList || v.typeId() == QMetaType::QStringList) {
      // Expose array-typed palette entries as real JS arrays
      const auto list = v.toList();
      auto jsArray    = m_engine.newArray(static_cast<quint32>(list.size()));
      for (int i = 0; i < list.size(); ++i)
        jsArray.setProperty(static_cast<quint32>(i), list.at(i).toString());

      themeObject.setProperty(it.key(), jsArray);
    } else {
      themeObject.setProperty(it.key(), v.toString());
    }
  }
  m_engine.globalObject().setProperty(QStringLiteral("theme"), themeObject);
}

/**
 * @brief Returns a tiny built-in script used when the group has no painterCode.
 */
QString Widgets::Painter::fallbackTemplate()
{
  return QStringLiteral("function paint(ctx, w, h) {"
                        "  ctx.fillStyle = '#0d1117';"
                        "  ctx.fillRect(0, 0, w, h);"
                        "  ctx.fillStyle = '#94a3b8';"
                        "  ctx.font = '13px sans-serif';"
                        "  ctx.fillText((group.title || 'Painter') + ' (no script)', 12, 22);"
                        "}");
}

/**
 * @brief Compiles the user JS, looks up paint() and (optional) onFrame().
 */
bool Widgets::Painter::compile(const QString& code)
{
  m_compileDirty = false;
  m_paintFn      = QJSValue();
  m_onFrameFn    = QJSValue();
  m_hasOnFrame   = false;

  // Bootstrap must have succeeded; otherwise datasets/group/frame globals are unreachable
  if (!m_bootstrapOk) [[unlikely]] {
    setRuntimeOk(false);
    return false;
  }

  // Empty painterCode falls back to a stub so the widget always renders something
  const QString effectiveCode = code.isEmpty() ? fallbackTemplate() : code;
  auto result                 = m_engine.evaluate(effectiveCode, QStringLiteral("painter.js"));
  if (result.isError()) {
    setLastError(QStringLiteral("compile: ") + result.property(QStringLiteral("message")).toString()
                 + QStringLiteral(" (line ")
                 + result.property(QStringLiteral("lineNumber")).toString() + QStringLiteral(")"));
    setRuntimeOk(false);
    return false;
  }

  auto paintFn = m_engine.globalObject().property(QStringLiteral("paint"));
  if (!paintFn.isCallable()) {
    setLastError(QStringLiteral("missing paint(ctx, w, h) function"));
    setRuntimeOk(false);
    return false;
  }

  m_paintFn = paintFn;

  auto onFrameFn = m_engine.globalObject().property(QStringLiteral("onFrame"));
  if (onFrameFn.isCallable()) {
    m_onFrameFn  = onFrameFn;
    m_hasOnFrame = true;
  }

  setRuntimeOk(true);
  setLastError(QString());
  return true;
}

/**
 * @brief Drops compiled state so the next updateData() recompiles from m_userCode.
 */
void Widgets::Painter::invalidateCompilation()
{
  m_compileDirty    = true;
  m_paintFn         = QJSValue();
  m_onFrameFn       = QJSValue();
  m_hasOnFrame      = false;
  m_slowPaintStreak = 0;
  m_slowPaintWarned = false;
  setRuntimeOk(false);
}

/**
 * @brief Resizes the cache to the widget size and runs the user paint() into it.
 */
void Widgets::Painter::renderFrame()
{
  const qreal dpr = qMax<qreal>(1.0, window() ? window()->effectiveDevicePixelRatio() : 1.0);
  const QSize logical(qMax(1, int(width())), qMax(1, int(height())));
  const QSize physical(qMax(1, int(std::ceil(logical.width() * dpr))),
                       qMax(1, int(std::ceil(logical.height() * dpr))));

  if (physical != m_cacheSize || !qFuzzyCompare(m_cache.devicePixelRatio(), dpr)) {
    m_cache = QImage(physical, QImage::Format_ARGB32_Premultiplied);
    m_cache.setDevicePixelRatio(dpr);
    m_cacheSize = physical;
  }

  m_cache.fill(Qt::transparent);

  QPainter qp(&m_cache);
  m_ctx->beginFrame(&qp, logical.width(), logical.height());

  QJSValueList args;
  args << m_ctxValue << QJSValue(double(logical.width())) << QJSValue(double(logical.height()));
  const auto t0 = std::chrono::steady_clock::now();
  auto r        = m_watchdog.call(m_paintFn, args);
  const auto dt =
    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0)
      .count();

  m_ctx->endFrame();
  qp.end();

  if (r.isError()) [[unlikely]] {
    setLastError(QStringLiteral("paint: ") + r.property(QStringLiteral("message")).toString());
    setRuntimeOk(false);
    return;
  }

  if (dt > kSlowPaintMs) [[unlikely]] {
    if (m_slowPaintStreak < 2)
      ++m_slowPaintStreak;

    if (m_slowPaintStreak >= 2 && !m_slowPaintWarned) {
      m_slowPaintWarned = true;
      setLastError(QStringLiteral("warning: paint() took %1 ms (>%2 ms budget)")
                     .arg(QString::number(dt), QString::number(kSlowPaintMs)));
    }
  } else {
    m_slowPaintStreak = 0;
  }
}

//--------------------------------------------------------------------------------------------------
// Notification helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Updates the runtimeOk flag and notifies QML.
 */
void Widgets::Painter::setRuntimeOk(bool ok)
{
  if (m_runtimeOk == ok)
    return;

  m_runtimeOk = ok;
  Q_EMIT runtimeOkChanged();
}

/**
 * @brief Updates the lastError message and notifies QML.
 */
void Widgets::Painter::setLastError(const QString& error)
{
  if (m_lastError == error)
    return;

  m_lastError = error;
  Q_EMIT lastErrorChanged();
}

#endif  // BUILD_COMMERCIAL
