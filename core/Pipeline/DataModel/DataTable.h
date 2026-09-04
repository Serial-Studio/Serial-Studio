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

#include <memory>
#include <QCoreApplication>
#include <QHash>
#include <QJSValue>
#include <QMap>
#include <QObject>
#include <QPair>
#include <QSet>
#include <QString>
#include <QThread>
#include <QVariant>
#include <QVariantList>
#include <vector>

#include "DataModel/Frame.h"
#include "IO/PipelineHost.h"

namespace DataModel {

/**
 * @brief Returns the reserved name of the internal per-dataset mirror table.
 */
[[nodiscard]] const QString& systemDataTableName();

/**
 * @brief Single register value (numeric or string) held in a data table.
 */
struct RegisterValue {
  double numericValue = 0.0;
  QString stringValue;
  bool isNumeric = true;
};

/**
 * @brief Immutable copy of a DataTableStore for readers off its owning thread (spec 0051 M5):
 *        GUI scripts read a snapshot instead of blocking the display tick on a marshal. Lookup
 *        maps are implicitly shared and only rebuilt by initialize(), so a snapshot costs three
 *        refcount bumps plus one copy of the value vector.
 */
struct DataTableSnapshot {
  int generation     = -1;
  quint64 writeClock = 0;
  std::vector<RegisterValue> values;
  std::vector<bool> computed;
  QHash<QPair<QString, QString>, int> index;
  QHash<int, std::pair<int, int>> datasetIndex;
  QHash<QString, std::pair<int, int>> aliasIndex;

  [[nodiscard]] bool isInitialized() const;
  [[nodiscard]] bool isWritable(const QString& table, const QString& reg) const;
  [[nodiscard]] bool isWritableHandle(qint64 handle) const;
  [[nodiscard]] qint64 handleOf(const QString& table, const QString& reg) const;
  [[nodiscard]] const RegisterValue* getByHandle(qint64 handle) const;
  [[nodiscard]] const RegisterValue* getDatasetRaw(int uniqueId) const;
  [[nodiscard]] const RegisterValue* getDatasetFinal(int uniqueId) const;
  [[nodiscard]] const RegisterValue* get(const QString& table, const QString& reg) const;
  [[nodiscard]] const RegisterValue* getDatasetRawByAlias(const QString& alias) const;
  [[nodiscard]] const RegisterValue* getDatasetFinalByAlias(const QString& alias) const;
};

/**
 * @typedef DataTableSnapshotPtr
 * @brief Shared immutable pointer to a published data-table snapshot.
 */
typedef std::shared_ptr<const DataTableSnapshot> DataTableSnapshotPtr;

/**
 * @brief Flat pre-allocated store for all data table registers; central data
 *        bus for cross-dataset communication and transforms.
 */
class DataTableStore {
public:
  DataTableStore();

  void initialize(const std::vector<TableDef>& userTables,
                  const std::vector<TableFolder>& tableFolders,
                  const Frame& templateFrame);

  void clear();

  [[nodiscard]] int generation() const noexcept;
  [[nodiscard]] bool isInitialized() const noexcept;
  [[nodiscard]] quint64 writeClock() const noexcept;
  void snapshotInto(DataTableSnapshot& out) const;

  [[nodiscard]] const RegisterValue* get(const QString& table, const QString& reg) const;

  [[nodiscard]] bool isWritable(const QString& table, const QString& reg) const;
  [[nodiscard]] bool isWritableHandle(qint64 handle) const;

  [[nodiscard]] qint64 handleOf(const QString& table, const QString& reg) const;
  [[nodiscard]] const RegisterValue* getByHandle(qint64 handle) const;
  bool setByHandle(qint64 handle, const RegisterValue& val);

  [[nodiscard]] const RegisterValue* getByInternedKey(const char* table, const char* reg) const;
  bool setByInternedKey(const char* table, const char* reg, const RegisterValue& val);

  void clearLookupCache() const;

  void setReadCaptureTarget(std::vector<int>* target) const noexcept;
  [[nodiscard]] bool changedSince(const std::vector<int>& slotList, quint64 sinceClock) const;

  bool set(const QString& table, const QString& reg, const RegisterValue& val);

  void setDatasetRaw(int uniqueId, double numeric, const QString& str, bool isNum);

  void setDatasetFinal(int uniqueId, double numeric, const QString& str, bool isNum);

  [[nodiscard]] const RegisterValue* getDatasetRaw(int uniqueId) const;
  [[nodiscard]] const RegisterValue* getDatasetFinal(int uniqueId) const;

  [[nodiscard]] const RegisterValue* getDatasetRawByAlias(const QString& alias) const;
  [[nodiscard]] const RegisterValue* getDatasetFinalByAlias(const QString& alias) const;

  [[nodiscard]] const RegisterValue* getDatasetRawByAliasInterned(const char* alias) const;
  [[nodiscard]] const RegisterValue* getDatasetFinalByAliasInterned(const char* alias) const;

  [[nodiscard]] QMap<QString, QMap<QString, RegisterValue>> snapshot() const;

private:
  void addRegister(const QString& table,
                   const QString& reg,
                   const RegisterValue& defaultVal,
                   RegisterType type);

  void registerDatasetAlias(const QString& alias, int uniqueId);

  [[nodiscard]] std::pair<int, int> resolveAliasSlotsInterned(const char* alias,
                                                              const char* kind) const;

  [[nodiscard]] int indexOf(const QString& table, const QString& reg) const;

  void captureRead(int slot) const;

  void noteMissingLookup(const QString& table, const QString& reg) const;
  void noteMissingDataset(int uniqueId, const char* kind) const;
  void noteMissingAlias(const QString& alias, const char* kind) const;

private:
  bool m_initialized;
  int m_generation;
  quint64 m_writeClock;

  std::vector<RegisterValue> m_storage;
  QHash<QPair<QString, QString>, int> m_index;
  QHash<int, std::pair<int, int>> m_datasetIndex;
  QHash<QString, std::pair<int, int>> m_aliasIndex;

  std::vector<bool> m_isComputed;
  std::vector<quint64> m_version;

  std::vector<std::pair<QString, std::vector<QString>>> m_tableRegNames;

  mutable QSet<QPair<QString, QString>> m_warnedMissing;
  mutable QSet<int> m_warnedMissingDatasets;
  mutable QSet<QString> m_warnedMissingAliases;
  mutable std::vector<int>* m_captureTarget;

  static constexpr int kInternedKeyCacheSize = 32;

  struct InternedKeyCacheEntry {
    const char* tablePtr = nullptr;
    const char* regPtr   = nullptr;
    int storeIndex       = -1;
  };

  mutable InternedKeyCacheEntry m_internedKeyCache[kInternedKeyCacheSize];
  mutable int m_internedKeyCacheNext = 0;

  static constexpr int kInternedAliasCacheSize = 16;

  struct InternedAliasCacheEntry {
    const char* aliasPtr = nullptr;
    int rawSlot          = -1;
    int finSlot          = -1;
  };

  mutable InternedAliasCacheEntry m_internedAliasCache[kInternedAliasCacheSize];
  mutable int m_internedAliasCacheNext = 0;
};

/**
 * @brief Everything a script bridge needs to reach the table store from any thread: the live
 *        store, the QObject whose thread owns it (the FrameBuilder), and the GUI-side snapshot
 *        slot. Shared by the JS bridge and the Lua C closures so the routing rule below has a
 *        single definition.
 */
struct TableApiContext {
  DataTableStore* store              = nullptr;
  QObject* owner                     = nullptr;
  const DataTableSnapshotPtr* mirror = nullptr;
};

/**
 * @brief Runs @p fn against the safe view here: the live store when the caller owns it, the GUI
 *        mirror snapshot on the GUI thread, the live store behind the marshal otherwise (also
 *        before the first snapshot lands, so a startup handle is never a spurious -1). The
 *        marshaled read re-checks isInitialized(), so a read racing a disconnect clear misses.
 */
template<typename Fn>
void readTableView(const TableApiContext& ctx, Fn&& fn)
{
  if (QThread::currentThread() == ctx.owner->thread()) {
    fn(*ctx.store);
    return;
  }

  if (qApp && QThread::currentThread() == qApp->thread() && ctx.mirror && *ctx.mirror) {
    fn(**ctx.mirror);
    return;
  }

  IO::PipelineHost::runOnObjectThread(ctx.owner, [&] {
    if (ctx.store->isInitialized())
      fn(*ctx.store);
  });
}

/**
 * @brief Applies a store mutation on the store's thread: direct when the caller owns it, queued
 *        fire-and-forget from the GUI (a wait would park the display tick and re-enter the
 *        script), blocking from other workers so write-then-read stays ordered. Cross-thread
 *        paths re-check isInitialized() on arrival, so a write racing a disconnect clear no-ops.
 */
template<typename Fn>
void writeTableStore(const TableApiContext& ctx, Fn&& fn)
{
  if (QThread::currentThread() == ctx.owner->thread()) {
    fn();
    return;
  }

  DataTableStore* store = ctx.store;
  auto guarded          = [store, fn = std::forward<Fn>(fn)] {
    if (store->isInitialized())
      fn();
  };

  if (qApp && QThread::currentThread() == qApp->thread() && ctx.mirror) {
    QMetaObject::invokeMethod(ctx.owner, std::move(guarded), Qt::QueuedConnection);
    return;
  }

  IO::PipelineHost::runOnObjectThread(ctx.owner, std::move(guarded));
}

/**
 * @brief QObject bridge exposing DataTableStore to QJSEngine for JS transforms.
 */
class TableApiBridge : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

public:
  explicit TableApiBridge(QObject* parent = nullptr) : QObject(parent), context() {}

  TableApiContext context;

public:
  Q_INVOKABLE [[nodiscard]] QVariant tableGet(const QString& t, const QString& r);
  Q_INVOKABLE [[nodiscard]] QVariant tableGetH(qint64 h);
  Q_INVOKABLE [[nodiscard]] qint64 tableHandle(const QString& t, const QString& r);
  Q_INVOKABLE [[nodiscard]] QVariantList tableHandleMany(const QString& t,
                                                         const QVariantList& regs);
  Q_INVOKABLE [[nodiscard]] QVariant datasetGetRaw(const QJSValue& sel);
  Q_INVOKABLE [[nodiscard]] QVariant datasetGetFinal(const QJSValue& sel);
#ifdef BUILD_COMMERCIAL
  Q_INVOKABLE qint64 mqttPublish(const QString& topic,
                                 const QByteArray& payload,
                                 int qos     = 0,
                                 bool retain = false);
#endif
public slots:
  void tableSet(const QString& t, const QString& r, const QVariant& v);
  void tableSetH(qint64 h, const QVariant& v);
};

}  // namespace DataModel
