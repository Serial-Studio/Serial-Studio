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

#pragma once

// clang-format off
extern "C" {
#include <lua.h>
}
// clang-format on

#include <array>
#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <QByteArrayView>
#include <QDeadlineTimer>
#include <QHash>
#include <QJSEngine>
#include <QJSValue>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QThread>
#include <QTimer>
#include <QVariant>
#include <unordered_map>
#include <vector>

#include "DataModel/DataBlock.h"
#include "DataModel/DataTable.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameBuilder/QuickPlotBuilder.h"
#include "DataModel/FrameBuilder/TableScriptBridge.h"
#include "DataModel/FrameBuilder/TransformCompiler.h"
#include "DataModel/FramePoolPolicy.h"
#include "DataModel/ParseBudget.h"
#include "DataModel/RepublishGate.h"
#include "IO/HAL_Driver.h"
#include "IO/PipelineHost.h"
#include "SerialStudio.h"
#include "ThirdParty/readerwriterqueue.h"

class SessionContext;

namespace Misc {
class TimerEvents;
}  // namespace Misc

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
  void structureGenerationChanged(quint64 generation);
  void structurePublished(int sourceId, const DataModel::Frame& frame);
  void sessionStructureReady(const DataModel::Frame& frame);

private:
  friend class ::SessionContext;
  explicit FrameBuilder();
  FrameBuilder(FrameBuilder&&)                 = delete;
  FrameBuilder(const FrameBuilder&)            = delete;
  FrameBuilder& operator=(FrameBuilder&&)      = delete;
  FrameBuilder& operator=(const FrameBuilder&) = delete;

public:
  /**
   * @brief Latest received frame snapshot for script/API consumers: the raw chunk (a retained
   *        FrameReader pool reference), the parser's channel tokens, and capture sequence
   *        numbers. Channel tokens are valid only when channelsSequence == sequence.
   */
  struct LatestFrameInfo {
    LatestFrameInfo();
    int sourceId;
    quint64 sequence;
    qint64 timestampMs;
    quint64 channelsSequence;
    QStringList channels;
    IO::CapturedDataPtr chunk;
  };

  [[nodiscard]] static FrameBuilder& instance();

  [[nodiscard]] const DataModel::Frame& frame() const noexcept;
  [[nodiscard]] const DataModel::Frame& quickPlotFrame() const noexcept;
  [[nodiscard]] const DataModel::DataTableStore& tableStore() const noexcept;
  [[nodiscard]] DataModel::DataTableStore& tableStore() noexcept;
  [[nodiscard]] const LatestFrameInfo* latestFrame(int sourceId) const noexcept;

  using ParseLoad = DataModel::ParseBudget::Load;

  void resetFrameCounters();
  void setParseBudgetEnabled(bool enabled);
  [[nodiscard]] bool parseBudgetThinning() const noexcept;
  [[nodiscard]] std::vector<ParseLoad> parseLoadSnapshot();
  [[nodiscard]] LatestFrameInfo latestFrameSnapshot(int sourceId);
  [[nodiscard]] quint64 parsedFrameCount() const noexcept;
  [[nodiscard]] quint64 skippedFrameCount() const noexcept;
  [[nodiscard]] quint64 transformErrorCount() const noexcept;
  [[nodiscard]] int lastTransformDataset() const noexcept;
  [[nodiscard]] const QString& lastTransformError() const noexcept;

  /**
   * @brief One replay cell for the typed lane: a borrowed text pointer for string channels,
   *        or a native double (text == nullptr) for numeric channels.
   */
  struct ReplayCell {
    const QString* text;
    double number;
  };

  [[nodiscard]] const DataModel::TableApiContext& guiTableApiContext();

  void injectTableApiLua(lua_State* L);
  void injectTableApiJS(QJSEngine* js);
  void refreshTableStoreFromProjectModel();
  void setReplayColumnMap(std::unordered_map<int, std::unordered_map<int, int>> map);
  void replayChannels(int sourceId,
                      const QStringList& channels,
                      const DataModel::TimestampedFrame::SteadyTimePoint& timestamp);
  void replayChannelSpans(int sourceId,
                          const QByteArrayView* cells,
                          qsizetype count,
                          const DataModel::TimestampedFrame::SteadyTimePoint& timestamp);
  void replayChannelsTyped(int sourceId,
                           const ReplayCell* cells,
                           qsizetype count,
                           const DataModel::TimestampedFrame::SteadyTimePoint& timestamp);
  void replayBlock(const DataModel::DataBlockPtr& block);

  [[nodiscard]] bool reprocessFrames();
  [[nodiscard]] bool dashboardTick();
  void drainTableSnapshot();
  void drainLatestFrameSnapshot();

  /**
   * @brief Runs @p fn on the frame builder's owning thread, fire-and-forget: direct when the
   *        caller is already there (headless paths keep synchronous semantics), queued
   *        otherwise (spec 0051 M3 -- the builder lives on the pipeline thread).
   */
  template<typename Fn>
  void invokeOnBuilderThread(Fn&& fn)
  {
    if (QThread::currentThread() == thread()) {
      fn();
      return;
    }

    QMetaObject::invokeMethod(this, std::forward<Fn>(fn), Qt::QueuedConnection);
  }

  /**
   * @brief Blocking twin of invokeOnBuilderThread for callers that need the result before
   *        returning (ControlScript verbs, API table commands, editor dry runs). Delegates to
   *        the deadlock-aware marshal (IO::PipelineHost::runOnObjectThread).
   */
  template<typename Fn>
  void invokeOnBuilderThreadBlocking(Fn&& fn)
  {
    IO::PipelineHost::runOnObjectThread(this, std::forward<Fn>(fn));
  }

public slots:
  void prepareShutdown();
  void setupExternalConnections();
  void syncFromProjectModel();

  void registerQuickPlotHeaders(const QStringList& headers);

  void hotpathRxFrame(const IO::CapturedDataPtr& data);
  void hotpathRxSourceFrame(int sourceId, const IO::CapturedDataPtr& data);

  void publishSourceTemplate(int sourceId);
  void publishQuickPlotAudioTemplate(int channels);
  void setStreamSourceIds(const QSet<int>& sourceIds);
  void ingestStreamValues(int sourceId, const QList<QPair<int, double>>& values);
  void ingestStreamBlock(const DataModel::DataBlockPtr& block);
  void refreshStreamDrivenFrames();
  void refreshAsyncSinks();
  void forgetPublishedStructures();
  void releaseReplayPoolStorage();
  void onPlayerOpenChanged();

  void collectTransformEngineGarbage();
  void releaseTransformEngines();
  void rebuildTransformEngines();
  void flushOpenBlocks();

private slots:
  void onSourceRemoved();
  void onConnectedChanged();
  void onOperationModeChanged();
  void refreshProjectSourceSnapshot();

private:
  using BudgetClock                              = DataModel::ParseBudget::Clock;
  static constexpr double kMillisecondsToSeconds = 1.0 / 1000.0;

  struct TransformFrameInfo {
    quint64 frameNumber = 0;
    int sourceId        = 0;
    qint64 timestampMs  = 0;
  };

  /**
   * @brief Change-driven dependency state for one transform dataset: the union-over-history set
   *        of store slots its transform has read, and the write clock at its last run. Re-run
   *        only when changedSince(readSlots, lastRunClock) is true. hasRun distinguishes "never
   *        profiled" (readSlots empty, must run once to capture) from "profiled and reads no
   *        tables" (readSlots empty, safe to skip in the synthetic reprocess pass for
   *        channel-fed datasets).
   */
  struct DatasetDeps {
    std::vector<int> readSlots;
    quint64 lastRunClock = 0;
    bool hasRun          = false;
  };

  int m_quickPlotChannels;
  bool m_parseBudgetEnabled;
  bool m_lastConnectedState;
  bool m_playerOpen;
  bool m_anyAsyncSink;
  bool m_captureDatasetValues;
  bool m_captureFlagsDirty;
  bool m_externalTableApiUsers;
  bool m_captureLatestFrame;
  bool m_changeDriven;
  bool m_shuttingDown;
  int m_seenEngineEpoch;
  SerialStudio::OperationMode m_operationMode;
  SerialStudio::DecoderMethod m_projectDecoderMethod;
  DataModel::ParseBudget m_parseBudget;

  quint64 m_parsedFrameCount;
  quint64 m_skippedFrameCount;

  bool m_jsTransformTimedOut;
  QStringList m_channelScratch;

  DataModel::Frame m_frame;
  DataModel::DataTableStore m_tableStore;

  static constexpr size_t kTableMirrorSlots = 4;

  int m_publishedTableGeneration;
  quint64 m_publishedTableClock;

  // code-verify off
  // One is written once at bridge injection, the other toggles once per display tick. No
  // steady-state cross-core write traffic, so sharing a cache line is harmless.
  // Latches a project sync already in flight: its blocking marshal pumps the GUI loop, and a
  // nested sync delivered there would spin another loop and recurse without bound
  std::atomic<bool> m_projectSyncInFlight;
  std::atomic<bool> m_guiTableApiUsers;
  std::atomic<bool> m_tableSnapshotRequested;
  // code-verify on

  DataModel::DataTableSnapshotPtr m_guiTableSnapshot;
  moodycamel::ReaderWriterQueue<DataModel::DataTableSnapshotPtr> m_tableMirrorRing;

  static constexpr size_t kTableSnapshotPoolSlots = kTableMirrorSlots + 4;

  std::vector<std::shared_ptr<DataModel::DataTableSnapshot>> m_tableSnapshotPool;
  std::size_t m_tableSnapshotPoolHint;

  // Concern sub-objects: their references bind members declared above, so addresses never move
  DataModel::QuickPlotBuilder m_quickPlot;
  DataModel::TableScriptBridge m_tableApi;
  DataModel::TransformCompiler m_transforms;

  void noteGuiTableApiUser();
  void publishTableSnapshot();
  [[nodiscard]] std::shared_ptr<DataModel::DataTableSnapshot> claimTableSnapshotSlot();

  bool m_streamValuesDirty;
  QSet<int> m_streamSourceIds;
  QSet<int> m_streamDatasetIds;
  DataModel::RepublishGate m_republishGate;
  QMap<int, DataModel::Frame> m_sourceFrames;
  std::map<int, quint64> m_sourceFrameCounters;
  std::unordered_map<int, std::unordered_map<int, int>> m_replayColumnMap;
  std::unordered_map<int, DatasetDeps> m_datasetDeps;

  int m_latestFrameSourceId;
  quint64 m_latestFrameSeq;
  QHash<int, LatestFrameInfo> m_latestFrames;

  /**
   * @brief GUI-side copy of every source's latest capture, published by the builder thread at
   *        display-tick rate so an API handler serving a script never marshals into the pipeline.
   */
  struct LatestFrameMirror {
    int newestSourceId;
    QHash<int, LatestFrameInfo> frames;
  };

  using LatestFrameMirrorPtr = std::shared_ptr<const LatestFrameMirror>;

  static constexpr size_t kLatestFrameMirrorSlots = 4;

  quint64 m_publishedLatestFrameSeq;

  // code-verify off
  // One is armed once by the first GUI-thread reader, the other toggles once per display tick.
  // No steady-state cross-core write traffic, so sharing a cache line is harmless.
  std::atomic<bool> m_guiLatestFrameUsers;
  std::atomic<bool> m_latestFrameSnapshotRequested;
  // code-verify on

  LatestFrameMirrorPtr m_guiLatestFrameMirror;
  moodycamel::ReaderWriterQueue<LatestFrameMirrorPtr> m_latestFrameMirrorRing;

  [[nodiscard]] LatestFrameInfo guiLatestFrame(int sourceId);
  void publishLatestFrameSnapshot();

  using ParseLoadsPtr = std::shared_ptr<const std::vector<ParseLoad>>;

  static constexpr size_t kParseLoadMirrorSlots = 4;

  ParseLoadsPtr m_guiParseLoads;
  moodycamel::ReaderWriterQueue<ParseLoadsPtr> m_parseLoadMirrorRing;

  [[nodiscard]] std::vector<ParseLoad> guiParseLoads();
  void publishParseLoads();

  int m_engineCacheSourceId;
  DataModel::TransformEngine* m_luaEngineForSource;
  DataModel::TransformEngine* m_jsEngineForSource;
  DataModel::TransformEngine* m_exprEngineForSource;

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
  static constexpr size_t kInvalidSlotIdx = DataModel::FramePoolPolicy::kInvalidSlot;

  // Memory ceiling for materialised slots; kFramePoolSize alone assumes small frames
  static constexpr size_t kFramePoolBudgetBytes = 192ULL * 1024ULL * 1024ULL;

  std::vector<std::shared_ptr<PooledFrameSlot>> m_framePool;
  DataModel::FramePoolPolicy m_poolPolicy;
  quint64 m_framePoolGeneration;

  /**
   * @brief Recyclable pool slot holding one staged DataBlock plus the generation and source it
   *        is bound to. A slot is free exactly when the pool's shared_ptr is its only reference,
   *        so the builder keeps its own reference while a block is open and hands out an aliasing
   *        shared_ptr on publish -- no per-block control block, and no other source can steal a
   *        slot that is still filling.
   */
  struct PooledBlockSlot {
    PooledBlockSlot();
    DataModel::DataBlock block;
    quint64 generation;
    quint64 flushEpoch;
    int sourceId;
  };

  static constexpr int kBlockPoolSlots = 64;

  // Frame-lane flush cap (spec 0055 D1/D6); low because these columns carry a string per sample
  static constexpr qsizetype kFrameBlockSampleCap = 64;

  // Memory ceiling for materialised block slots; a slot's storage scales with the dataset count
  static constexpr size_t kBlockPoolBudgetBytes = 192ULL * 1024ULL * 1024ULL;

  std::vector<std::shared_ptr<PooledBlockSlot>> m_blockPool;
  std::map<int, std::shared_ptr<PooledBlockSlot>> m_openBlocks;
  std::map<int, quint64> m_blockNumbers;
  std::size_t m_blockPoolHint;
  int m_blockSlotsUsable;
  bool m_maskSinks;
  std::map<int, quint64> m_publishedStructureGeneration;

  /**
   * @brief ProjectModel state carried from the GUI thread to the builder thread, so the builder
   *        never has to reach back for it mid-sync.
   */
  struct ProjectSnapshot {
    QString title;
    std::vector<DataModel::Group> groups;
    std::vector<DataModel::Action> actions;
    std::vector<DataModel::Source> sources;
    SerialStudio::DecoderMethod decoder = SerialStudio::PlainText;
  };

  [[nodiscard]] static ProjectSnapshot collectProjectSnapshot();
  void applyProjectSnapshot(ProjectSnapshot snapshot);
  [[nodiscard]] std::shared_ptr<PooledBlockSlot> claimBlockSlot(int sourceId) noexcept;
  void refreshBlockPoolBudget(const DataModel::Frame& src) noexcept;
  [[nodiscard]] DataModel::StructureSnapshotPtr buildStructureSnapshot(const DataModel::Frame& src);
  [[nodiscard]] bool structureIsCurrent(int sourceId) const noexcept;
  void noteStructurePublished(int sourceId) noexcept;

  void ensureStructurePublished(int sourceId, const DataModel::Frame& src);
  void bindBlockToFrame(PooledBlockSlot& slot, const DataModel::Frame& src, bool uniform);
  [[nodiscard]] PooledBlockSlot* openBlockFor(int sourceId, const DataModel::Frame& src);
  SS_HOT void stageFrameValues(int sourceId,
                               const DataModel::Frame& src,
                               const DataModel::TimestampedFrame::SteadyTimePoint& ts);
  void flushBlock(int sourceId);
  void publishBlock(const DataModel::DataBlockPtr& block);

  void invalidateFramePool() noexcept;
  void refreshFramePoolBudget(const DataModel::Frame& src) noexcept;
  SS_COLD void notePoolExhausted();
  [[nodiscard]] size_t claimPoolSlot(int sourceId, bool hintedOnly = false) noexcept;
  bool republishOneFrame(DataModel::Frame& frame, int key, bool feedExports);
  bool emitRepublishedFrame(const DataModel::Frame& frame, int key, bool feedExports);
  void bindSlotTemplate(PooledFrameSlot* slot, const DataModel::Frame& src);
  [[nodiscard]] bool preparePooledSlot(PooledFrameSlot* slot, const DataModel::Frame& src);

private:
  // code-verify off
  // Parse pipeline
  static constexpr qsizetype kMaxSpanFields = 128;
  std::array<QByteArrayView, kMaxSpanFields> m_spanScratch;

  DataModel::Frame& ensureSourceFrame(int sourceId);
  SerialStudio::DecoderMethod resolveDecoderMethod(int sourceId, bool applyPerSourceOverride) const;
  void parseProjectFrame(const IO::CapturedDataPtr& data);
  void parseProjectFrame(int sourceId, const IO::CapturedDataPtr& data);
  void parseQuickPlotFrame(const IO::CapturedDataPtr& data);
  void publishReplayValues(int sourceId,
                           const DataModel::Frame& src,
                           const DataModel::TimestampedFrame::SteadyTimePoint& ts);
  void publishSourceTemplateFrame(const DataModel::Source& src);
  [[nodiscard]] bool republishFrames(bool feedExports);
  void wireDisplayTickHooks(Misc::TimerEvents& timers, IO::PipelineHost& pipeline);
  void refreshAnyAsyncSink();
  void refreshDatasetCaptureFlag();
  void refreshLatestFrameCapture();
  void clearLatestFrames();
  void captureLatestChunk(int sourceId, const IO::CapturedDataPtr& data);
  void captureLatestChannels(int sourceId, const QStringList& channels);
  void captureLatestChannelSpans(int sourceId, const QByteArrayView* spans, qsizetype count);
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
  [[nodiscard]] bool reprocessDatasetValues(DataModel::Frame& frame);
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
                         const std::unordered_map<int, int>* replayColumns,
                         bool finalValueReplay);
  [[nodiscard]] const std::unordered_map<int, int>* replayColumnsFor(int sourceId) const;
  void applyReplaySpanValue(Dataset& dataset,
                            const QByteArrayView* cells,
                            qsizetype count,
                            const std::unordered_map<int, int>* columns);
  void applyReplayTypedValue(Dataset& dataset,
                             const ReplayCell* cells,
                             qsizetype count,
                             const std::unordered_map<int, int>* columns);
  SS_HOT void applyDatasetValueSpan(Dataset& dataset,
                                    const QByteArrayView* spans,
                                    qsizetype count,
                                    const TransformFrameInfo& info);

  // Parser-load budget guard
  [[nodiscard]] bool parseBudgetSkipFrame(int sourceId);
  void parseBudgetAccount(int sourceId, BudgetClock::time_point startedAt);
  SS_COLD void noteParseBudgetThinning(int sourceId);
  void parseBudgetReset() noexcept;

  // Transform compile + dispatch
  QVariant applyTransform(int language,
                          int uniqueId,
                          const QVariant& rawValue,
                          const TransformFrameInfo& info);
  QVariant applyTransformLua(DataModel::TransformEngine& engine,
                             int uniqueId,
                             const QVariant& rawValue,
                             const TransformFrameInfo& info);
  QVariant applyTransformExpr(DataModel::TransformEngine& engine,
                              int uniqueId,
                              const QVariant& rawValue,
                              const TransformFrameInfo& info);
  QVariant applyTransformJs(DataModel::TransformEngine& engine,
                            int uniqueId,
                            const QVariant& rawValue,
                            const TransformFrameInfo& info);

  void compileTransforms();
  void destroyTransformEngines();
  void initializeTableStore();
  void rebuildTransformsForPlayback();
  // code-verify on
};

}  // namespace DataModel
