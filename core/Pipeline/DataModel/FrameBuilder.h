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
#include <optional>
#include <QByteArrayView>
#include <QDeadlineTimer>
#include <QHash>
#include <QJSEngine>
#include <QJSValue>
#include <QList>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QThread>
#include <QTimer>
#include <QVariant>
#include <span>
#include <unordered_map>
#include <vector>

#include "Core/DataModel/DataBlock.h"
#include "Core/DataModel/Frame.h"
#include "Core/IO/HAL_Driver.h"
#include "Core/ParseBudget.h"
#include "Core/SerialStudio.h"
#include "Core/ThirdParty/readerwriterqueue.h"
#include "DataModel/DataTable.h"
#include "DataModel/FrameBuilder/BlockPublisher.h"
#include "DataModel/FrameBuilder/BlockStager.h"
#include "DataModel/FrameBuilder/ExternalWiring.h"
#include "DataModel/FrameBuilder/LatestFrameTap.h"
#include "DataModel/FrameBuilder/QuickPlotBuilder.h"
#include "DataModel/FrameBuilder/ReplayIngest.h"
#include "DataModel/FrameBuilder/TableScriptBridge.h"
#include "DataModel/FrameBuilder/TableSnapshotChannel.h"
#include "DataModel/FrameBuilder/TransformCompiler.h"
#include "DataModel/FrameBuilder/TransformDispatch.h"
#include "DataModel/FramePoolPolicy.h"
#include "DataModel/RepublishGate.h"
#include "DataModel/Scripting/ScriptCells.h"
#include "IO/PipelineHost.h"

namespace Core::Bus {
class MessageBus;
}  // namespace Core::Bus

class SessionContext;

namespace Misc {
class TimerEvents;
}  // namespace Misc

namespace DataModel {

class ControlScript;

/**
 * @brief Assembles a DataModel::Frame from raw I/O bytes and distributes it to the dashboard and
 * export workers.
 */
class FrameBuilder
  : public QObject
  , public DataModel::BlockStagerHost {
  // clang-format off
  Q_OBJECT
  // clang-format on

signals:
  void jsonFileMapChanged();
  void structureGenerationChanged(quint64 generation);
  void structurePublished(int sourceId, const DataModel::Frame& frame);
  void sessionStructureReady(const DataModel::Frame& frame);
  void sessionBoundary(bool connected, bool paused);

private:
  friend class ::SessionContext;

  static void bindInstance(FrameBuilder* instance) noexcept { s_instance = instance; }

  static FrameBuilder* s_instance;
  explicit FrameBuilder(Core::Bus::MessageBus& bus);
  friend class ExternalWiring;

  [[nodiscard]] bool anyPlayerOpen() const
  {
    return m_playerOpenMask[0] || m_playerOpenMask[1] || m_playerOpenMask[2];
  }

  FrameBuilder(FrameBuilder&&)                 = delete;
  FrameBuilder(const FrameBuilder&)            = delete;
  FrameBuilder& operator=(FrameBuilder&&)      = delete;
  FrameBuilder& operator=(const FrameBuilder&) = delete;

public:
  using LatestFrameInfo = DataModel::LatestFrameInfo;

  static constexpr qsizetype kMaxSpanFields = 128;

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

  using ReplayCell = DataModel::ReplayIngest::Cell;

  [[nodiscard]] const DataModel::TableApiContext& guiTableApiContext();

  void injectTableApiLua(lua_State* L);
  void injectTableApiJS(QJSEngine* js);
  void acquireTableApiUser();
  void releaseTableApiUser();

  void installTableApiNames(QJSEngine* js);
  void installTableApiNamesLua(lua_State* L);

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

  void bindBlockSinks(std::span<DataModel::IBlockSink* const> sinks,
                      DataModel::IBlockSink* server,
                      DataModel::IBlockSink* grpc);

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
  void onPausedChanged();
  void onPipelineParkedChanged(bool parked);
  void onConnectedChanged();
  void onOperationModeChanged();
  void refreshProjectSourceSnapshot();

private:
  using BudgetClock        = DataModel::ParseBudget::Clock;
  using TransformFrameInfo = DataModel::TransformFrameInfo;

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

  Core::Bus::MessageBus& m_bus;
  int m_quickPlotChannels;
  bool m_parseBudgetEnabled;
  bool m_lastConnectedState;
  bool m_lastPausedState;
  bool m_playerOpen;
  std::array<bool, DataModel::ExternalWiring::kPlayerSlots> m_playerOpenMask;
  bool m_captureDatasetValues;
  bool m_captureFlagsDirty;
  int m_externalTableUsers;
  bool m_captureLatestFrame;
  bool m_changeDriven;
  bool m_shuttingDown;
  int m_seenEngineEpoch;
  SerialStudio::OperationMode m_operationMode;
  SerialStudio::DecoderMethod m_projectDecoderMethod;
  DataModel::ParseBudget m_parseBudget;

  quint64 m_parsedFrameCount;
  quint64 m_skippedFrameCount;

  DataModel::Frame m_frame;
  DataModel::DataTableStore m_tableStore;

  // code-verify off
  // Latches a project sync already in flight: its blocking marshal pumps the GUI loop, and a
  // nested sync delivered there would spin another loop and recurse without bound
  std::atomic<bool> m_projectSyncInFlight;
  // code-verify on

  // Concern sub-objects: their references bind members declared above, so addresses never move
  DataModel::TableSnapshotChannel m_tableChannel;
  DataModel::QuickPlotBuilder m_quickPlot;
  DataModel::TableScriptBridge m_tableApi;
  DataModel::TransformCompiler m_transforms;
  DataModel::TransformDispatch m_dispatch;
  DataModel::ReplayIngest m_replay;

  bool m_streamValuesDirty;
  QSet<int> m_streamSourceIds;
  QSet<int> m_streamDatasetIds;
  DataModel::RepublishGate m_republishGate;
  QMap<int, DataModel::Frame> m_sourceFrames;
  std::map<int, quint64> m_sourceFrameCounters;
  std::unordered_map<int, DatasetDeps> m_datasetDeps;

  int m_latestFrameSourceId;
  quint64 m_latestFrameSeq;
  QHash<int, LatestFrameInfo> m_latestFrames;

  // Mirrors the capture state declared above GUI-ward; the capture writes stay in this TU
  DataModel::LatestFrameTap m_latestTap;

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

  std::vector<std::shared_ptr<PooledFrameSlot>> m_framePool;
  DataModel::FramePoolPolicy m_poolPolicy;
  quint64 m_framePoolGeneration;

  bool m_maskSinks;
  std::map<int, quint64> m_publishedStructureGeneration;

  // Bind m_framePoolGeneration and m_maskSinks above, so their addresses never move
  DataModel::BlockStager m_stager;
  DataModel::BlockPublisher m_publisher;

  /**
   * @brief ProjectModel state carried from the GUI thread to the builder thread, so the builder
   *        never has to reach back for it mid-sync.
   */
  struct ProjectSnapshot {
    QString title;
    QString transformLibrary;
    QString transformLibraryJs;
    std::vector<DataModel::Group> groups;
    std::vector<DataModel::Action> actions;
    std::vector<DataModel::Source> sources;
    SerialStudio::DecoderMethod decoder = SerialStudio::PlainText;
  };

  // Holds a sync that arrived while the pipeline was parked; applied once it is running again
  std::optional<ProjectSnapshot> m_deferredProjectSnapshot;
  DataModel::ExternalWiring m_wiring;

  [[nodiscard]] static ProjectSnapshot collectProjectSnapshot();
  void applyProjectSnapshot(ProjectSnapshot snapshot);
  void applyDeferredProjectSnapshot();
  [[nodiscard]] DataModel::StructureSnapshotPtr buildStructureSnapshot(const DataModel::Frame& src);
  [[nodiscard]] bool structureIsCurrent(int sourceId) const noexcept;
  void noteStructurePublished(int sourceId) noexcept;

  void ensureStructurePublished(int sourceId, const DataModel::Frame& src);
  void emitSessionBoundary();

  void noteStagingPoolExhausted() final;
  void publishStagedBlock(const DataModel::DataBlockPtr& block) final;
  void announceStructure(int sourceId, const DataModel::Frame& src) final;
  [[nodiscard]] quint64 stagingFlushEpoch() const final;

  void invalidateFramePool() noexcept;
  SS_COLD void notePoolExhausted();
  [[nodiscard]] size_t claimPoolSlot(int sourceId, bool hintedOnly = false) noexcept;
  bool republishOneFrame(DataModel::Frame& frame, int key, bool feedExports);
  bool emitRepublishedFrame(const DataModel::Frame& frame, int key, bool feedExports);
  void bindSlotTemplate(PooledFrameSlot* slot, const DataModel::Frame& src);
  [[nodiscard]] bool preparePooledSlot(PooledFrameSlot* slot, const DataModel::Frame& src);

private:
  // code-verify off
  // Parse pipeline
  std::array<QByteArrayView, kMaxSpanFields> m_spanScratch;
  DataModel::ScriptCellRows m_cellRows;
  QList<QStringList> m_cellFallback;

  DataModel::Frame& ensureSourceFrame(int sourceId);
  SerialStudio::DecoderMethod resolveDecoderMethod(int sourceId, bool applyPerSourceOverride) const;
  void parseProjectFrameFor(int sourceId, bool perSource, const IO::CapturedDataPtr& data);
  void parseProjectFrame(const IO::CapturedDataPtr& data);
  void parseProjectFrame(int sourceId, const IO::CapturedDataPtr& data);
  void parseQuickPlotFrame(const IO::CapturedDataPtr& data);
  void publishReplayValues(int sourceId,
                           const DataModel::Frame& src,
                           const DataModel::TimestampedFrame::SteadyTimePoint& ts);
  void publishSourceTemplateFrame(const DataModel::Source& src);
  [[nodiscard]] bool republishFrames(bool feedExports);
  void wireDisplayTickHooks(Misc::TimerEvents& timers, IO::PipelineHost& pipeline);
  void wireAsyncSinkHooks();
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
  int tryCellLane(int sourceId,
                  bool applyPerSourceOverride,
                  DataModel::Frame& frame,
                  const IO::CapturedDataPtr& data);
  void captureLatestCells(int sourceId, const DataModel::ScriptCell* cells, qsizetype count);
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
  SS_HOT void applyDatasetValueSpan(Dataset& dataset,
                                    const QByteArrayView* spans,
                                    qsizetype count,
                                    const TransformFrameInfo& info);
  SS_HOT void applyDatasetValuesCells(DataModel::Frame& frame,
                                      const DataModel::ScriptCell* cells,
                                      qsizetype count,
                                      const TransformFrameInfo& info);
  SS_HOT void applyDatasetValueCell(Dataset& dataset,
                                    const DataModel::ScriptCell* cells,
                                    qsizetype count,
                                    const TransformFrameInfo& info);
  SS_HOT void applyDatasetToken(Dataset& dataset,
                                const QByteArrayView* token,
                                const double* number,
                                const TransformFrameInfo& info);

  // Parser-load budget guard
  [[nodiscard]] bool parseBudgetSkipFrame(int sourceId);
  void parseBudgetAccount(int sourceId, BudgetClock::time_point startedAt);
  SS_COLD void noteParseBudgetThinning(int sourceId);
  void parseBudgetReset() noexcept;

  // Transform compile + dispatch

  void compileTransforms();
  void destroyTransformEngines();
  void initializeTableStore();
  void rebuildTransformsForPlayback();
  // code-verify on
};

/**
 * @brief Scope guard for an external table-API user (spec 0086): the owner arms capture through
 *        injectTableApiLua/JS and this releases it when the engine's scope ends.
 */
class TableApiUserLease {
public:
  explicit TableApiUserLease(FrameBuilder& builder) : m_builder(builder) {}

  ~TableApiUserLease() { m_builder.releaseTableApiUser(); }

  TableApiUserLease(TableApiUserLease&&)                 = delete;
  TableApiUserLease(const TableApiUserLease&)            = delete;
  TableApiUserLease& operator=(TableApiUserLease&&)      = delete;
  TableApiUserLease& operator=(const TableApiUserLease&) = delete;

private:
  FrameBuilder& m_builder;
};

}  // namespace DataModel
