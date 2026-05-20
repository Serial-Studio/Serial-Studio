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

#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <QDeadlineTimer>
#include <QJSEngine>
#include <QJSValue>
#include <QMap>
#include <QObject>
#include <QTimer>
#include <QVariant>
#include <vector>

#include "DataModel/DataTable.h"
#include "DataModel/Frame.h"
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

  void injectTableApiLua(lua_State* L);
  void injectTableApiJS(QJSEngine* js);
  void refreshTableStoreFromProjectModel();

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
  bool m_lastConnectedState;
  qint64 m_parseBudgetUsedNs;
  BudgetClock::time_point m_parseBudgetWindowStart;

  QTimer m_jsTransformWatchdog;
  QStringList m_channelScratch;
  QStringList m_quickPlotChannelNames;

  DataModel::Frame m_frame;
  DataModel::Frame m_quickPlotFrame;
  DataModel::DataTableStore m_tableStore;

  QMap<int, DataModel::Frame> m_sourceFrames;
  std::map<int, quint64> m_sourceFrameCounters;
  std::map<EngineKey, TransformEngine> m_transformEngines;

  int m_engineCacheSourceId;
  TransformEngine* m_luaEngineForSource;
  TransformEngine* m_jsEngineForSource;

  int m_compileGuard;
  bool m_compilePending;

  /**
   * @brief Recyclable pool slot holding one TimestampedFrame and its in-use flag.
   */
  struct PooledFrameSlot {
    PooledFrameSlot();
    DataModel::TimestampedFrame frame;
    std::atomic<bool> inUse;
  };

  static constexpr int kFramePoolSize = 1024;
  std::vector<std::shared_ptr<PooledFrameSlot>> m_framePool;
  std::atomic<size_t> m_framePoolHint;

  [[nodiscard]] DataModel::TimestampedFramePtr acquireFrame(const DataModel::Frame& src);
  [[nodiscard]] DataModel::TimestampedFramePtr acquireFrame(
    const DataModel::Frame& src, const DataModel::TimestampedFrame::SteadyTimePoint& ts);

private:
  // code-verify off
  // Parse pipeline
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
  void decodeProjectChannels(int sourceId,
                             bool applyPerSourceOverride,
                             const IO::CapturedDataPtr& data,
                             QList<QStringList>& outChannels);
  void applyDatasetValues(DataModel::Frame& frame,
                          const QStringList& channels,
                          const TransformFrameInfo& info);
  void applyDatasetValue(Dataset& dataset,
                         const QString* channelData,
                         int channelCount,
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
