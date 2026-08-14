/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#pragma once

// clang-format off
extern "C" {
#include <lua.h>
}
// clang-format on

#include <atomic>
#include <chrono>
#include <memory>
#include <QDeadlineTimer>
#include <QJSEngine>
#include <QJSValue>
#include <QObject>
#include <QString>
#include <QThread>
#include <vector>

#include "IO/HAL_Driver.h"
#include "ThirdParty/readerwriterqueue.h"

namespace DataModel {
class FrameBuilder;
}  // namespace DataModel

namespace IO {

/**
 * @brief Resolves the effective stream lane for a source: the per-source project override wins,
 *        otherwise the driver decides (stream-capable drivers stream by default, spec 0051 R6).
 */
[[nodiscard]] inline bool streamLaneOn(const HAL_Driver* driver, const QString& lane)
{
  if (lane == QLatin1String("on"))
    return true;

  if (lane == QLatin1String("off"))
    return false;

  return driver && driver->isStreamCapable();
}

/**
 * @brief One dataset bound to a stream channel: dashboard identity, channel index, display
 *        reductions to run, and the optional per-dataset transform.
 */
struct StreamChannelConfig {
  int uniqueId          = -1;
  int channel           = 0;
  bool plot             = false;
  bool fft              = false;
  int fftSamples        = 0;
  int transformLanguage = 0;
  QString transformCode;
};

/**
 * @brief Immutable per-source stream configuration handed to a worker at creation.
 */
struct StreamConfig {
  int sourceId      = 0;
  int channels      = 1;
  double sampleRate = 0.0;
  bool luaFastMode  = false;
  std::vector<StreamChannelConfig> datasets;
};

/**
 * @brief Bounded display payload published per processed block: O(pixels + fftSize + datasets),
 *        never O(samples). Envelope pairs accumulate across updates (apply every update); the
 *        FFT window is a full snapshot (apply only the newest update per drain).
 */
struct StreamDisplayUpdate {
  struct ChannelUpdate {
    int uniqueId  = -1;
    double latest = 0.0;
    bool hasFft   = false;
    std::vector<std::pair<double, double>> envelope;
    std::vector<double> fftWindow;
  };

  int sourceId        = 0;
  quint64 blockNumber = 0;
  SampleBlock::SteadyTimePoint t0;
  std::chrono::nanoseconds dt{1};
  qsizetype frames = 0;
  std::vector<ChannelUpdate> channels;
};

/**
 * @typedef StreamDisplayUpdatePtr
 * @brief Shared immutable pointer to a display update.
 */
typedef std::shared_ptr<const StreamDisplayUpdate> StreamDisplayUpdatePtr;

/**
 * @brief Per-block export payload for the typed sinks (spec 0051 M5): full-rate post-transform
 *        planar samples with per-sample timing derivable as t0 + i * dt.
 */
struct StreamBlockItem {
  int sourceId        = 0;
  quint64 blockNumber = 0;
  SampleBlock::SteadyTimePoint t0;
  std::chrono::nanoseconds dt{1};
  qsizetype frames = 0;
  std::vector<int> uniqueIds;
  std::vector<std::vector<double>> channels;
};

/**
 * @typedef StreamBlockItemPtr
 * @brief Shared immutable pointer to a full-rate export block.
 */
typedef std::shared_ptr<const StreamBlockItem> StreamBlockItemPtr;

/**
 * @brief Worker-thread half of a stream source (spec 0051 M4): consumes typed SampleBlocks at
 *        block rate, runs block/per-sample transforms in a worker-owned script engine, reduces
 *        to per-pixel envelopes + FFT windows + latest values, and fans out full-rate typed
 *        export blocks. Lives on the StreamWorker's thread; all members are worker-affine.
 */
class StreamProcessor : public QObject {
  Q_OBJECT

signals:
  void blockReady(const IO::StreamBlockItemPtr& block);
  void latestValuesReady(int sourceId, const QList<QPair<int, double>>& values);

public:
  explicit StreamProcessor(const StreamConfig& config,
                           moodycamel::ReaderWriterQueue<StreamDisplayUpdatePtr>* displayOut,
                           std::atomic<int>* pixelWidth,
                           std::atomic<double>* windowSec,
                           std::atomic<bool>* exportActive,
                           std::atomic<bool>* paused             = nullptr,
                           DataModel::FrameBuilder* frameBuilder = nullptr);
  ~StreamProcessor() override;

  StreamProcessor(StreamProcessor&&)                 = delete;
  StreamProcessor(const StreamProcessor&)            = delete;
  StreamProcessor& operator=(StreamProcessor&&)      = delete;
  StreamProcessor& operator=(const StreamProcessor&) = delete;

  [[nodiscard]] quint64 samplesProcessed() const noexcept { return m_samplesProcessed; }

  [[nodiscard]] quint64 blocksProcessed() const noexcept { return m_blocksProcessed; }

  [[nodiscard]] quint64 transformErrorCount() const noexcept { return m_transformErrors; }

  [[nodiscard]] quint64 displayDropCount() const noexcept { return m_displayDrops; }

public slots:
  void onSampleBlock(const IO::SampleBlockPtr& block);
  void compileEngines();
  void teardownEngines();

private:
  struct ChannelState {
    StreamChannelConfig config;
    double latest     = 0.0;
    int luaRef        = -1;
    int luaBlockRef   = -1;
    bool jsValid      = false;
    bool jsBlockValid = false;
    QJSValue jsFn;
    QJSValue jsBlockFn;
    quint64 firstSampleIndex = 0;
    std::vector<double> fftRing;
    std::size_t fftFill = 0;
    std::size_t fftHead = 0;
    std::vector<std::pair<double, double>> envelope;
  };

  void processChannel(ChannelState& state,
                      const IO::SampleBlock& block,
                      quint64 blockNumber,
                      StreamBlockItem* exportItem);
  void compileLuaEntry(ChannelState& state);
  void compileJsEntry(ChannelState& state);
  [[nodiscard]] bool runBlockTransform(ChannelState& state, quint64 blockNumber, double t0Ms);
  [[nodiscard]] bool runLuaBlockTransform(ChannelState& state, quint64 blockNumber, double t0Ms);
  [[nodiscard]] bool runJsBlockTransform(ChannelState& state, quint64 blockNumber, double t0Ms);
  void runSampleTransform(ChannelState& state);
  void reduceChannel(ChannelState& state, const IO::SampleBlock& block);
  void publishDisplayUpdate(const IO::SampleBlock& block, quint64 blockNumber);
  [[nodiscard]] std::shared_ptr<StreamDisplayUpdate> claimUpdateSlot();
  void setupLuaState();
  void setupJsEngine();
  static void luaWatchdogHook(lua_State* L, lua_Debug* ar);

private:
  static constexpr int kWatchdogMs     = 100;
  static constexpr int kHookInstrCount = 10000;
  static constexpr int kDefaultBuckets = 512;

  StreamConfig m_config;
  moodycamel::ReaderWriterQueue<StreamDisplayUpdatePtr>* m_displayOut;
  std::atomic<int>* m_pixelWidth;
  std::atomic<double>* m_windowSec;
  std::atomic<bool>* m_exportActive;
  std::atomic<bool>* m_paused;
  DataModel::FrameBuilder* m_frameBuilder;

  lua_State* m_lua;
  QJSEngine* m_js;
  QDeadlineTimer m_luaDeadline;
  bool m_inBlock;

  quint64 m_samplesProcessed;
  quint64 m_blocksProcessed;
  quint64 m_transformErrors;
  quint64 m_displayDrops;

  std::vector<double> m_scratch;
  std::vector<ChannelState> m_channels;

  std::vector<std::shared_ptr<StreamDisplayUpdate>> m_updatePool;
  std::size_t m_updatePoolHint;
};

/**
 * @brief GUI-side facade of one stream source's worker (spec 0051 M4): owns the thread, the
 *        display SPSC ring and the resize atomics; created/destroyed by ConnectionManager
 *        beside the DeviceManager. Teardown is stop(): disconnect feed, quit, bounded wait,
 *        warn-and-abandon on a hung Fast-mode script (R21).
 */
class StreamWorker : public QObject {
  Q_OBJECT

public:
  explicit StreamWorker(HAL_Driver* driver,
                        const StreamConfig& config,
                        DataModel::FrameBuilder* frameBuilder = nullptr,
                        QObject* parent                       = nullptr);
  ~StreamWorker() override;

  StreamWorker(StreamWorker&&)                 = delete;
  StreamWorker(const StreamWorker&)            = delete;
  StreamWorker& operator=(StreamWorker&&)      = delete;
  StreamWorker& operator=(const StreamWorker&) = delete;

  [[nodiscard]] int sourceId() const noexcept;
  [[nodiscard]] bool abandoned() const noexcept;
  [[nodiscard]] const StreamConfig& config() const noexcept;
  [[nodiscard]] int displayRingCapacity() const noexcept;
  [[nodiscard]] bool dequeueDisplayUpdate(StreamDisplayUpdatePtr& out);
  [[nodiscard]] StreamProcessor* processor() const noexcept;

  void setPixelWidth(int px) noexcept;
  void setWindowSec(double seconds) noexcept;
  void setExportActive(bool active) noexcept;
  void setPaused(bool paused) noexcept;

  void stop();

private:
  static constexpr int kJoinTimeoutMs     = 5000;
  static constexpr int kDisplayRingSlots  = 256;
  static constexpr int kDefaultPixelWidth = 512;

  StreamConfig m_config;
  bool m_abandoned;
  QThread m_thread;
  QMetaObject::Connection m_feed;
  StreamProcessor* m_processor;

  // code-verify off
  // All written by the GUI at resize/config rate only; the worker reads per block. No
  // steady-state cross-core write traffic, so sharing a cache line is harmless.
  std::atomic<int> m_pixelWidth;
  std::atomic<double> m_windowSec;
  std::atomic<bool> m_exportActive;
  std::atomic<bool> m_paused;
  // code-verify on
  moodycamel::ReaderWriterQueue<StreamDisplayUpdatePtr> m_displayRing;
};

}  // namespace IO
