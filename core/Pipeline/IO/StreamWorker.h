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

#include "Core/DataModel/DataBlock.h"
#include "Core/HotpathOptimization.h"
#include "Core/IO/HAL_Driver.h"
#include "Core/IO/StreamConfig.h"
#include "Core/ThirdParty/readerwriterqueue.h"
#include "DataModel/Scripting/ExpressionTransform.h"
#include "DataModel/Scripting/JsWatchdog.h"

namespace DataModel {
class FrameBuilder;
}  // namespace DataModel

namespace IO {

/**
 * @brief Samples per dense-lane block before publication (spec 0055 D6). Far above the frame
 *        lane's cap: a dense column is numeric-only (D2), so it carries no per-sample strings.
 */
inline constexpr qsizetype kStreamBlockSampleCap = 4096;

/**
 * @brief Pooled blocks a stream processor keeps in flight. Bounded low on purpose: the consumers
 *        that queue blocks take a trimmed copy, so only the dashboard ring holds a pool slot.
 */
inline constexpr std::size_t kBlockPoolSlots = 8;

/**
 * @brief Worker-thread half of a stream source (spec 0051 M4): consumes typed SampleBlocks at
 *        block rate, runs block/per-sample transforms in a worker-owned script engine, publishes
 *        the transformed samples + FFT windows + latest values, and fans out full-rate typed
 *        export blocks. Lives on the StreamWorker's thread; all members are worker-affine.
 */
class StreamProcessor : public QObject {
  Q_OBJECT

signals:
  void blockReady(const DataModel::DataBlockPtr& block);
  void latestValuesReady(int sourceId, const QList<QPair<int, double>>& values);

public:
  explicit StreamProcessor(const StreamConfig& config,
                           std::atomic<bool>* paused             = nullptr,
                           DataModel::FrameBuilder* frameBuilder = nullptr);
  ~StreamProcessor() override;

  StreamProcessor(StreamProcessor&&)                 = delete;
  StreamProcessor(const StreamProcessor&)            = delete;
  StreamProcessor& operator=(StreamProcessor&&)      = delete;
  StreamProcessor& operator=(const StreamProcessor&) = delete;

  [[nodiscard]] quint64 samplesProcessed() const noexcept { return m_samplesProcessed; }

  [[nodiscard]] quint64 blocksProcessed() const noexcept { return m_blocksProcessed; }

  [[nodiscard]] int observedChannels() const noexcept { return m_observedChannels; }

  [[nodiscard]] quint64 transformErrorCount() const noexcept { return m_transformErrors; }

  [[nodiscard]] quint64 displayDropCount() const noexcept { return m_displayDrops; }

  [[nodiscard]] quint64 transformTimeoutCount() const noexcept { return m_transformTimeouts; }

  [[nodiscard]] bool transformsReferenceTableApi(int language) const;

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
    bool exprValid      = false;
    DataModel::Expression::Runtime expr;
  };

  void processChannel(ChannelState& state,
                      const IO::SampleBlock& block,
                      quint64 blockNumber,
                      DataModel::BlockColumn* column);
  void compileLuaEntry(ChannelState& state);
  void compileJsEntry(ChannelState& state);
  void compileExprEntry(ChannelState& state);
  void processExpressionChannels(const IO::SampleBlock& block, DataModel::DataBlock& out);
  void publishFftWindow(const ChannelState& state, DataModel::BlockColumn& column) const;
  [[nodiscard]] bool runBlockTransform(ChannelState& state, quint64 blockNumber, double t0Ms);
  [[nodiscard]] bool runLuaBlockTransform(ChannelState& state, quint64 blockNumber, double t0Ms);
  [[nodiscard]] bool runJsBlockTransform(ChannelState& state, quint64 blockNumber, double t0Ms);
  void runSampleTransform(ChannelState& state);
  SS_COLD void noteTransformTimeout(const ChannelState& state);
  void restoreRawSamples(const IO::SampleBlock& block, int index);
  void publishChannel(ChannelState& state, DataModel::BlockColumn& column);
  static void appendFftRing(ChannelState& state, const double* samples, std::size_t count);
  [[nodiscard]] std::shared_ptr<DataModel::DataBlock> claimBlockSlot();
  void bindBlockColumns(DataModel::DataBlock& block) const;
  void setupLuaState();
  void setupJsEngine();
  void installJsTablePrelude();
  static void luaWatchdogHook(lua_State* L, lua_Debug* ar);

private:
  static constexpr int kWatchdogMs     = 100;
  static constexpr int kHookInstrCount = 10000;

  StreamConfig m_config;
  std::atomic<bool>* m_paused;
  DataModel::FrameBuilder* m_frameBuilder;

  lua_State* m_lua;
  QJSEngine* m_js;
  std::unique_ptr<DataModel::JsWatchdog> m_jsWatchdog;
  QDeadlineTimer m_luaDeadline;
  bool m_inBlock;
  bool m_jsTimedOut;
  bool m_luaTableArmed;
  bool m_jsTableArmed;

  int m_observedChannels;
  quint64 m_samplesProcessed;
  quint64 m_blocksProcessed;
  quint64 m_transformErrors;
  quint64 m_transformTimeouts;
  quint64 m_displayDrops;

  std::vector<double> m_scratch;
  std::vector<ChannelState> m_channels;
  DataModel::Expression::SlotTable m_exprSlots;
  bool m_hasExpressions;

  std::vector<std::shared_ptr<DataModel::DataBlock>> m_updatePool;
  std::size_t m_updatePoolHint;
};

/**
 * @brief GUI-side facade of one stream source's worker (spec 0051 M4): owns the thread and the
 *        display SPSC ring; created/destroyed by ConnectionManager beside the DeviceManager.
 *        Teardown is stop(): disconnect feed, quit, bounded wait, warn-and-abandon on a hung
 *        Fast-mode script (R21).
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
  [[nodiscard]] StreamProcessor* processor() const noexcept;

  void setPaused(bool paused) noexcept;

  void stop();

private:
  static constexpr int kJoinTimeoutMs = 5000;

  StreamConfig m_config;
  bool m_abandoned;
  std::unique_ptr<QThread> m_thread;
  QMetaObject::Connection m_feed;
  StreamProcessor* m_processor;

  // GUI-written at command rate, worker-read per block; no steady-state cross-core traffic
  std::atomic<bool> m_paused;
};

}  // namespace IO
