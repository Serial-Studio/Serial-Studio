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

#pragma once

#include <lua.h>

#include <array>
#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <QByteArrayView>
#include <QDeadlineTimer>
#include <QJSEngine>
#include <QJSValue>
#include <QMap>
#include <QObject>
#include <QTimer>
#include <QVariant>
#include <unordered_map>
#include <vector>

#include "DataModel/DataTable.h"
#include "DataModel/Frame.h"
#include "DataModel/Scripting/JsWatchdog.h"
#include "IO/HAL_Driver.h"
#include "SerialStudio.h"

namespace DataModel {

/**
 * @brief Assembles a DataModel::Frame from raw I/O bytes and distributes it to the dashboard and
 * export workers.
 */
class FrameBuilder : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

signals:
  void jsonFileMapChanged();
  void frameChanged(const DataModel::Frame& frame);

private:
  explicit FrameBuilder();
  FrameBuilder(FrameBuilder&&)                 = delete;
  FrameBuilder(const FrameBuilder&)            = delete;
  FrameBuilder& operator=(FrameBuilder&&)      = delete;
  FrameBuilder& operator=(const FrameBuilder&) = delete;

public:
  [[nodiscard]] static FrameBuilder& instance();

  [[nodiscard]] const DataModel::Frame& frame() const noexcept;
  [[nodiscard]] const DataModel::Frame& quickPlotFrame() const noexcept;
  [[nodiscard]] const DataModel::DataTableStore& tableStore() const noexcept;

  void resetFrameCounters() noexcept;
  void setParseBudgetEnabled(bool enabled) noexcept;
  [[nodiscard]] quint64 parsedFrameCount() const noexcept;
  [[nodiscard]] quint64 skippedFrameCount() const noexcept;

  void injectTableApiLua(lua_State* L);
  void injectTableApiJS(QJSEngine* js);
  void refreshTableStoreFromProjectModel();
  void setReplayColumnMap(std::unordered_map<int, std::unordered_map<int, int>> map);

public slots:
  void setupExternalConnections();
  void syncFromProjectModel();
  void registerQuickPlotHeaders(const QStringList& headers);

  void hotpathRxFrame(const IO::CapturedDataPtr& data);
  void hotpathRxSourceFrame(int sourceId, const IO::CapturedDataPtr& data);

  void collectTransformEngineGarbage();

private slots:
  void onSourceRemoved();
  void onConnectedChanged();
  void onOperationModeChanged();

private:
  using BudgetClock                             = std::chrono::steady_clock;
  static constexpr int kTransformWatchdogMs     = 100;
  static constexpr int kTransformHookInstrCount = 10000;
  static constexpr int kParseBudgetWindowMs     = 1000;
  static constexpr int kParseBudgetWarnLimitMs  = 800;

  struct TransformEntry {
    int uniqueId;
    QString code;
  };

  struct TransformFrameInfo {
    quint64 frameNumber = 0;
    int sourceId        = 0;
    qint64 timestampMs  = 0;
  };

  struct LuaTransformRef {
    int ref;
    bool acceptsInfo;
  };

  struct JsTransformRef {
    QJSValue fn;
    bool acceptsInfo;
  };

  struct TransformEngine {
    lua_State* luaState = nullptr;
    QJSEngine* jsEngine = nullptr;
    std::unique_ptr<JsWatchdog> jsWatchdog;
    std::map<int, LuaTransformRef> luaRefs;
    std::map<int, JsTransformRef> jsRefs;
    QDeadlineTimer luaDeadline{QDeadlineTimer::Forever};
  };

  struct EngineKey {
    int sourceId;
    int language;

    bool operator<(const EngineKey& other) const noexcept
    {
      return sourceId < other.sourceId || (sourceId == other.sourceId && language < other.language);
    }
  };

  int m_quickPlotChannels;
  bool m_quickPlotHasHeader;
  bool m_parseBudgetSkipping;
  bool m_parseBudgetWarned;
  bool m_parseBudgetEnabled;
  bool m_lastConnectedState;
  bool m_playerOpen;
  bool m_anyAsyncSink;
  bool m_captureDatasetValues;
  bool m_captureFlagsDirty;
  bool m_externalTableApiUsers;
  int m_seenEngineEpoch;
  SerialStudio::OperationMode m_operationMode;
  qint64 m_parseBudgetUsedNs;
  BudgetClock::time_point m_parseBudgetWindowStart;

  quint64 m_parsedFrameCount;
  quint64 m_skippedFrameCount;

  bool m_jsTransformTimedOut;
  QStringList m_channelScratch;
  QStringList m_quickPlotChannelNames;

  DataModel::Frame m_frame;
  DataModel::Frame m_quickPlotFrame;
  DataModel::DataTableStore m_tableStore;

  QMap<int, DataModel::Frame> m_sourceFrames;
  std::map<int, quint64> m_sourceFrameCounters;
  std::map<EngineKey, TransformEngine> m_transformEngines;
  std::unordered_map<int, std::unordered_map<int, int>> m_replayColumnMap;

  int m_engineCacheSourceId;
  TransformEngine* m_luaEngineForSource;
  TransformEngine* m_jsEngineForSource;

  int m_compileGuard;
  bool m_compilePending;

  /**
   * @brief Recyclable pool slot holding one TimestampedFrame, the template generation + source
   *        frame that last bound it, and a flattened dataset table for the span lane. A slot is
   *        free exactly when the pool's shared_ptr is the only reference; hand-outs alias that
   *        shared_ptr, so no per-frame control block exists.
   */
  struct PooledFrameSlot {
    PooledFrameSlot();
    DataModel::TimestampedFrame frame;
    quint64 generation;
    const DataModel::Frame* matchedSrc;
    std::vector<DataModel::Dataset*> flat;
  };

  static constexpr int kFramePoolSize     = 8192;
  static constexpr size_t kInvalidSlotIdx = static_cast<size_t>(-1);
  std::vector<std::shared_ptr<PooledFrameSlot>> m_framePool;
  std::atomic<size_t> m_framePoolHint;
  quint64 m_framePoolGeneration;

  void invalidateFramePool() noexcept;
  SS_COLD void notePoolExhausted();
  [[nodiscard]] size_t claimPoolSlot() noexcept;
  void bindSlotTemplate(PooledFrameSlot* slot, const DataModel::Frame& src);
  [[nodiscard]] bool preparePooledSlot(PooledFrameSlot* slot, const DataModel::Frame& src);
  [[nodiscard]] SS_HOT DataModel::TimestampedFramePtr acquireFrame(const DataModel::Frame& src);
  [[nodiscard]] SS_HOT DataModel::TimestampedFramePtr acquireFrame(
    const DataModel::Frame& src, const DataModel::TimestampedFrame::SteadyTimePoint& ts);

private:
  // code-verify off
  // Parse pipeline
  static constexpr qsizetype kMaxSpanFields = 64;
  std::array<QByteArrayView, kMaxSpanFields> m_spanScratch;

  DataModel::Source makeQuickPlotSource() const;
  DataModel::Frame& ensureSourceFrame(int sourceId);
  SerialStudio::DecoderMethod resolveDecoderMethod(int sourceId, bool applyPerSourceOverride) const;
  void parseProjectFrame(const IO::CapturedDataPtr& data);
  void parseProjectFrame(int sourceId, const IO::CapturedDataPtr& data);
  void parseQuickPlotFrame(const IO::CapturedDataPtr& data);
  void buildQuickPlotFrame(const QStringList& channels);
  void buildQuickPlotAudioFrame(const QStringList& channels);
  void hotpathTxFrame(const DataModel::TimestampedFramePtr& frame);
  void publishSourceTemplateFrame(const DataModel::Source& src);
  void refreshAnyAsyncSink();
  void refreshDatasetCaptureFlag();
  int trySpanLane(int sourceId,
                  bool applyPerSourceOverride,
                  DataModel::Frame& frame,
                  const IO::CapturedDataPtr& data);
  void decodeProjectChannels(int sourceId,
                             bool applyPerSourceOverride,
                             const IO::CapturedDataPtr& data,
                             QList<QStringList>& outChannels);
  bool beginDatasetPass(const TransformFrameInfo& info);
  void endDatasetPass(bool armedJsWatchdog);
  void applyDatasetValues(DataModel::Frame& frame,
                          const QStringList& channels,
                          const TransformFrameInfo& info);
  void applyDatasetValuesSpans(DataModel::Frame& frame,
                               const QByteArrayView* spans,
                               qsizetype count,
                               const TransformFrameInfo& info);
  SS_HOT void applyDatasetValuesSpans(DataModel::Dataset* const* SS_RESTRICT datasets,
                                      qsizetype datasetCount,
                                      const QByteArrayView* SS_RESTRICT spans,
                                      qsizetype count,
                                      const TransformFrameInfo& info);
  void applyDatasetValue(Dataset& dataset,
                         const QString* channelData,
                         int channelCount,
                         const TransformFrameInfo& info,
                         const std::unordered_map<int, int>* replayColumns);
  SS_HOT SS_FLATTEN void applyDatasetValueSpan(Dataset& dataset,
                                               const QByteArrayView* spans,
                                               qsizetype count,
                                               const TransformFrameInfo& info);

  // Parser-load budget guard
  bool parseBudgetSkipFrame();
  void parseBudgetAccount(BudgetClock::time_point startedAt);
  void parseBudgetReset() noexcept;

  // Transform compile + dispatch
  QVariant applyTransform(int language,
                          int uniqueId,
                          const QVariant& rawValue,
                          const TransformFrameInfo& info);
  QVariant applyTransformLua(TransformEngine& engine,
                             int uniqueId,
                             const QVariant& rawValue,
                             const TransformFrameInfo& info);
  QVariant applyTransformJs(TransformEngine& engine,
                            int uniqueId,
                            const QVariant& rawValue,
                            const TransformFrameInfo& info);

  void compileTransforms();
  void destroyTransformEngines();
  void initializeTableStore();
  void rebuildTransformsForPlayback();
  void compileTransformsLua(TransformEngine& engine,
                            int sourceId,
                            const std::vector<TransformEntry>& entries);
  void compileTransformsLuaEntry(lua_State* L,
                                 TransformEngine& engine,
                                 const TransformEntry& entry);
  void compileTransformsJS(TransformEngine& engine,
                           int sourceId,
                           const std::vector<TransformEntry>& entries);

  static void transformLuaWatchdogHook(lua_State* L, lua_Debug* ar);
  // code-verify on
};

}  // namespace DataModel
