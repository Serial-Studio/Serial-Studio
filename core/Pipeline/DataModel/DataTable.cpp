/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include "DataModel/DataTable.h"

#include <QCoreApplication>
#include <QDebug>
#include <QThread>

#include "Core/SSAssert.h"
#include "IO/PipelineHost.h"
#include "SerialStudio.h"

#ifdef BUILD_COMMERCIAL
#  include "MQTT/Publisher.h"
#endif

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static const QString kSystemTable = QStringLiteral("__datasets__");
static const QString kRawPrefix   = QStringLiteral("raw:");
static const QString kFinalPrefix = QStringLiteral("final:");

static constexpr int kHandleIndexBits    = 24;
static constexpr qint64 kHandleIndexMask = (static_cast<qint64>(1) << kHandleIndexBits) - 1;

/**
 * @brief Returns the reserved name of the internal per-dataset mirror table.
 */
const QString& DataModel::systemDataTableName()
{
  return kSystemTable;
}

/**
 * @brief Returns true when two register values are identical (same type and payload); identical
 *        rewrites must not bump slot versions or change-driven transforms would re-run on every
 *        parser write instead of on value changes.
 */
[[nodiscard]] static bool sameRegisterValue(const DataModel::RegisterValue& a,
                                            const DataModel::RegisterValue& b)
{
  if (a.isNumeric != b.isNumeric)
    return false;

  return a.isNumeric ? (a.numericValue == b.numericValue) : (a.stringValue == b.stringValue);
}

//--------------------------------------------------------------------------------------------------
// Initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an empty DataTableStore in the uninitialized state.
 */
DataModel::DataTableStore::DataTableStore()
  : m_initialized(false), m_generation(0), m_writeClock(0), m_captureTarget(nullptr)
{}

/**
 * @brief Pre-allocates registers for user tables and the system dataset table.
 */
void DataModel::DataTableStore::initialize(const std::vector<TableDef>& userTables,
                                           const std::vector<TableFolder>& tableFolders,
                                           const Frame& templateFrame)
{
  clear();
  ++m_generation;

  size_t totalRegs = 0;
  for (const auto& table : userTables)
    totalRegs += table.registers.size();

  size_t datasetCount = 0;
  for (const auto& group : templateFrame.groups)
    datasetCount += group.datasets.size();

  totalRegs += datasetCount * 2;
  m_storage.reserve(totalRegs);
  m_version.reserve(totalRegs);

  for (const auto& table : userTables) {
    const QString tablePath = tableFullPath(tableFolders, table.parentFolderId, table.name);

    std::vector<QString> regNames;
    regNames.reserve(table.registers.size());

    for (const auto& reg : table.registers) {
      RegisterValue defVal;
      defVal.numericValue = SerialStudio::toDouble(reg.defaultValue, &defVal.isNumeric);
      if (!defVal.isNumeric)
        defVal.stringValue = reg.defaultValue.toString();

      addRegister(tablePath, reg.name, defVal, reg.type);
      regNames.push_back(reg.name);
    }

    m_tableRegNames.emplace_back(tablePath, std::move(regNames));
  }

  std::vector<QString> sysRegNames;
  sysRegNames.reserve(datasetCount * 2);
  for (const auto& group : templateFrame.groups) {
    for (const auto& dataset : group.datasets) {
      const int uid        = dataset.uniqueId;
      const QString rawReg = kRawPrefix + QString::number(uid);
      const QString finReg = kFinalPrefix + QString::number(uid);

      const int rawOffset = static_cast<int>(m_storage.size());
      addRegister(kSystemTable, rawReg, RegisterValue{}, RegisterType::System);

      const int finOffset = static_cast<int>(m_storage.size());
      addRegister(kSystemTable, finReg, RegisterValue{}, RegisterType::System);

      m_datasetIndex.insert(uid, {rawOffset, finOffset});

      sysRegNames.push_back(rawReg);
      sysRegNames.push_back(finReg);
    }
  }

  m_tableRegNames.emplace_back(kSystemTable, std::move(sysRegNames));

  for (const auto& group : templateFrame.groups) {
    for (const auto& dataset : group.datasets)
      if (!dataset.alias.isEmpty())
        registerDatasetAlias(dataset.alias, dataset.uniqueId);
  }

  m_initialized = true;
}

/**
 * @brief Releases storage and resets the store to its uninitialized state.
 */
void DataModel::DataTableStore::clear()
{
  m_storage.clear();
  m_index.clear();
  m_datasetIndex.clear();
  m_aliasIndex.clear();
  m_isComputed.clear();
  m_version.clear();
  m_tableRegNames.clear();
  m_warnedMissing.clear();
  m_warnedMissingDatasets.clear();
  m_warnedMissingAliases.clear();
  clearLookupCache();
  m_initialized = false;
}

/**
 * @brief Invalidates the (table_ptr, reg_ptr) cache before any Lua state populating it closes.
 */
void DataModel::DataTableStore::clearLookupCache() const
{
  for (auto& entry : m_internedKeyCache)
    entry = InternedKeyCacheEntry{};

  m_internedKeyCacheNext = 0;

  for (auto& entry : m_internedAliasCache)
    entry = InternedAliasCacheEntry{};

  m_internedAliasCacheNext = 0;
}

/**
 * @brief Hot-path lookup keyed by Lua's interned string pointers.
 *
 * Falls back to the QString-keyed indexOf() on a cache miss and inserts the result
 * (positive index OR -1 for "not found") so subsequent calls with the same literal hit
 * the cache. The -1 entries dedupe the per-call warning for unknown registers; the
 * existing noteMissingLookup() still fires once on the miss path.
 */
const DataModel::RegisterValue* DataModel::DataTableStore::getByInternedKey(const char* table,
                                                                            const char* reg) const
{
  SS_ASSERT(m_initialized, return nullptr);

  for (const auto& entry : m_internedKeyCache) {
    if (entry.tablePtr == table && entry.regPtr == reg) {
      if (entry.storeIndex < 0)
        return nullptr;

      captureRead(entry.storeIndex);
      return &m_storage[static_cast<size_t>(entry.storeIndex)];
    }
  }

  const QString tableStr = QString::fromUtf8(table);
  const QString regStr   = QString::fromUtf8(reg);
  const int idx          = indexOf(tableStr, regStr);

  m_internedKeyCache[m_internedKeyCacheNext] = {table, reg, idx};
  m_internedKeyCacheNext                     = (m_internedKeyCacheNext + 1) % kInternedKeyCacheSize;

  if (idx < 0) [[unlikely]] {
    noteMissingLookup(tableStr, regStr);
    return nullptr;
  }

  captureRead(idx);
  return &m_storage[static_cast<size_t>(idx)];
}

/**
 * @brief Hot-path write keyed by Lua's interned string pointers; an identical value is a
 *        successful no-op that leaves the slot version untouched.
 */
bool DataModel::DataTableStore::setByInternedKey(const char* table,
                                                 const char* reg,
                                                 const RegisterValue& val)
{
  SS_ASSERT(m_initialized, return false);
  SS_ASSERT(m_isComputed.size() == m_storage.size(), return false);

  int idx = -1;
  for (const auto& entry : m_internedKeyCache) {
    if (entry.tablePtr == table && entry.regPtr == reg) {
      idx = entry.storeIndex;
      break;
    }
  }

  if (idx == -1) {
    const QString tableStr                     = QString::fromUtf8(table);
    const QString regStr                       = QString::fromUtf8(reg);
    idx                                        = indexOf(tableStr, regStr);
    m_internedKeyCache[m_internedKeyCacheNext] = {table, reg, idx};
    m_internedKeyCacheNext = (m_internedKeyCacheNext + 1) % kInternedKeyCacheSize;
  }

  if (idx < 0) [[unlikely]]
    return false;

  if (!m_isComputed[static_cast<size_t>(idx)]) [[unlikely]] {
    qWarning() << "[DataTableStore] Cannot write to non-computed register"
               << QLatin1StringView(table) << "/" << QLatin1StringView(reg);
    return false;
  }

  if (sameRegisterValue(m_storage[static_cast<size_t>(idx)], val))
    return true;

  m_storage[static_cast<size_t>(idx)] = val;
  m_version[static_cast<size_t>(idx)] = ++m_writeClock;
  return true;
}

/**
 * @brief Returns true once initialize() has built the register storage.
 */
bool DataModel::DataTableStore::isInitialized() const noexcept
{
  return m_initialized;
}

/**
 * @brief Returns the layout generation, bumped by every initialize(); handles carry it so a
 *        handle minted against an older layout resolves to nullptr instead of a wrong slot.
 */
int DataModel::DataTableStore::generation() const noexcept
{
  return m_generation;
}

/**
 * @brief Returns the monotonic write clock; a slot's version equals the clock value at its last
 *        write, so changedSince() can tell whether any slot moved after a reader's snapshot.
 */
quint64 DataModel::DataTableStore::writeClock() const noexcept
{
  return m_writeClock;
}

/**
 * @brief Points the read-capture accumulator at a caller-owned slot list (nullptr to stop). While
 *        set, every successful read records the slot it resolved, building a dataset's read-set.
 */
void DataModel::DataTableStore::setReadCaptureTarget(std::vector<int>* target) const noexcept
{
  m_captureTarget = target;
}

/**
 * @brief Returns true if any of the given storage slots was written after sinceClock.
 */
bool DataModel::DataTableStore::changedSince(const std::vector<int>& slotList,
                                             quint64 sinceClock) const
{
  for (const int slot : slotList) {
    if (slot < 0 || slot >= static_cast<int>(m_version.size())) [[unlikely]]
      continue;

    if (m_version[static_cast<size_t>(slot)] > sinceClock)
      return true;
  }

  return false;
}

/**
 * @brief Records a read of @p slot into the active capture target, deduped; inert when off.
 */
void DataModel::DataTableStore::captureRead(int slot) const
{
  if (m_captureTarget == nullptr)
    return;

  for (const int s : *m_captureTarget)
    if (s == slot)
      return;

  m_captureTarget->push_back(slot);
}

//--------------------------------------------------------------------------------------------------
// User table access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Looks up a register by table and register name.
 */
const DataModel::RegisterValue* DataModel::DataTableStore::get(const QString& table,
                                                               const QString& reg) const
{
  SS_ASSERT(m_initialized, return nullptr);

  const int idx = indexOf(table, reg);
  if (idx < 0) [[unlikely]] {
    noteMissingLookup(table, reg);
    return nullptr;
  }

  captureRead(idx);
  return &m_storage[static_cast<size_t>(idx)];
}

/**
 * @brief Writes to a computed register; no-op for constant or system registers, and an identical
 *        value is a successful no-op that leaves the slot version untouched.
 */
bool DataModel::DataTableStore::set(const QString& table,
                                    const QString& reg,
                                    const RegisterValue& val)
{
  SS_ASSERT(m_initialized, return false);
  SS_ASSERT(m_isComputed.size() == m_storage.size(), return false);

  const int idx = indexOf(table, reg);
  if (idx < 0) [[unlikely]]
    return false;

  if (!m_isComputed[static_cast<size_t>(idx)]) [[unlikely]] {
    qWarning() << "[DataTableStore] Cannot write to non-computed register" << table << "/" << reg;
    return false;
  }

  if (sameRegisterValue(m_storage[static_cast<size_t>(idx)], val))
    return true;

  m_storage[static_cast<size_t>(idx)] = val;
  m_version[static_cast<size_t>(idx)] = ++m_writeClock;
  return true;
}

//--------------------------------------------------------------------------------------------------
// Handle (pointer) fast path
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resolves (table, register) to a generation-tagged handle (gen<<24|index), or -1 if
 *        unknown; run once off the hot path. Silent on miss: -1 is the documented sentinel and
 *        probing for optional registers is a legitimate pattern, so (unlike get()) it does not
 *        warn -- a typo surfaces through the name-based get() path instead.
 */
qint64 DataModel::DataTableStore::handleOf(const QString& table, const QString& reg) const
{
  SS_ASSERT(m_initialized, return -1);

  const int idx = indexOf(table, reg);
  if (idx < 0) [[unlikely]]
    return -1;

  return (static_cast<qint64>(m_generation) << kHandleIndexBits) | static_cast<qint64>(idx);
}

/**
 * @brief True when the register exists and accepts a write. Constant registers exist and read
 *        back fine but silently ignore set(), so a caller that reports success from mere
 *        existence tells the user it wrote something it did not.
 */
bool DataModel::DataTableStore::isWritable(const QString& table, const QString& reg) const
{
  SS_ASSERT(m_initialized, return false);
  SS_ASSERT(m_isComputed.size() == m_storage.size(), return false);

  const int idx = indexOf(table, reg);
  if (idx < 0)
    return false;

  return m_isComputed[static_cast<size_t>(idx)];
}

/**
 * @brief Handle-keyed twin of isWritable(), matching setByHandle()'s generation and range guards.
 */
bool DataModel::DataTableStore::isWritableHandle(qint64 handle) const
{
  SS_ASSERT(m_initialized, return false);
  SS_ASSERT(m_isComputed.size() == m_storage.size(), return false);

  if (handle < 0)
    return false;

  const int gen   = static_cast<int>(handle >> kHandleIndexBits);
  const int index = static_cast<int>(handle & kHandleIndexMask);
  if (gen != m_generation || index >= static_cast<int>(m_storage.size()))
    return false;

  return m_isComputed[static_cast<size_t>(index)];
}

/**
 * @brief Hot-path read by handle; a stale, out-of-range, or -1 handle is a safe nullptr.
 */
const DataModel::RegisterValue* DataModel::DataTableStore::getByHandle(qint64 handle) const
{
  SS_ASSERT(m_initialized, return nullptr);

  if (handle < 0) [[unlikely]]
    return nullptr;

  const int gen   = static_cast<int>(handle >> kHandleIndexBits);
  const int index = static_cast<int>(handle & kHandleIndexMask);
  if (gen != m_generation || index >= static_cast<int>(m_storage.size())) [[unlikely]]
    return nullptr;

  captureRead(index);
  return &m_storage[static_cast<size_t>(index)];
}

/**
 * @brief Hot-path write by handle; non-computed (guard matches set()), stale, or invalid handles
 *        are ignored silently, with no warning so a misused handle cannot spam the log per frame.
 *        An identical value is a successful no-op that leaves the slot version untouched.
 */
bool DataModel::DataTableStore::setByHandle(qint64 handle, const RegisterValue& val)
{
  SS_ASSERT(m_initialized, return false);
  SS_ASSERT(m_isComputed.size() == m_storage.size(), return false);

  if (handle < 0) [[unlikely]]
    return false;

  const int gen   = static_cast<int>(handle >> kHandleIndexBits);
  const int index = static_cast<int>(handle & kHandleIndexMask);
  if (gen != m_generation || index >= static_cast<int>(m_storage.size())) [[unlikely]]
    return false;

  if (!m_isComputed[static_cast<size_t>(index)]) [[unlikely]]
    return false;

  if (sameRegisterValue(m_storage[static_cast<size_t>(index)], val))
    return true;

  m_storage[static_cast<size_t>(index)] = val;
  m_version[static_cast<size_t>(index)] = ++m_writeClock;
  return true;
}

//--------------------------------------------------------------------------------------------------
// System dataset table
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes raw (pre-transform) values for a dataset; an identical value is a no-op that
 *        leaves the slot version untouched, matching the computed-register write paths.
 */
void DataModel::DataTableStore::setDatasetRaw(int uniqueId,
                                              double numeric,
                                              const QString& str,
                                              bool isNum)
{
  SS_ASSERT(m_initialized, return);

  const auto it = m_datasetIndex.constFind(uniqueId);
  if (it == m_datasetIndex.constEnd()) [[unlikely]]
    return;

  auto& rv = m_storage[static_cast<size_t>(it->first)];
  if (rv.isNumeric == isNum && (isNum ? rv.numericValue == numeric : rv.stringValue == str))
    return;

  rv.numericValue                           = numeric;
  rv.stringValue                            = str;
  rv.isNumeric                              = isNum;
  m_version[static_cast<size_t>(it->first)] = ++m_writeClock;
}

/**
 * @brief Writes final (post-transform) values for a dataset; an identical value is a no-op that
 *        leaves the slot version untouched, matching the computed-register write paths.
 */
void DataModel::DataTableStore::setDatasetFinal(int uniqueId,
                                                double numeric,
                                                const QString& str,
                                                bool isNum)
{
  SS_ASSERT(m_initialized, return);

  const auto it = m_datasetIndex.constFind(uniqueId);
  if (it == m_datasetIndex.constEnd()) [[unlikely]]
    return;

  auto& rv = m_storage[static_cast<size_t>(it->second)];
  if (rv.isNumeric == isNum && (isNum ? rv.numericValue == numeric : rv.stringValue == str))
    return;

  rv.numericValue                            = numeric;
  rv.stringValue                             = str;
  rv.isNumeric                               = isNum;
  m_version[static_cast<size_t>(it->second)] = ++m_writeClock;
}

/**
 * @brief Returns the raw (pre-transform) value for a dataset.
 */
const DataModel::RegisterValue* DataModel::DataTableStore::getDatasetRaw(int uniqueId) const
{
  SS_ASSERT(m_initialized, return nullptr);

  const auto it = m_datasetIndex.constFind(uniqueId);
  if (it == m_datasetIndex.constEnd()) [[unlikely]] {
    noteMissingDataset(uniqueId, "raw");
    return nullptr;
  }

  captureRead(it->first);
  return &m_storage[static_cast<size_t>(it->first)];
}

/**
 * @brief Returns the final (post-transform) value for a dataset.
 */
const DataModel::RegisterValue* DataModel::DataTableStore::getDatasetFinal(int uniqueId) const
{
  SS_ASSERT(m_initialized, return nullptr);

  const auto it = m_datasetIndex.constFind(uniqueId);
  if (it == m_datasetIndex.constEnd()) [[unlikely]] {
    noteMissingDataset(uniqueId, "final");
    return nullptr;
  }

  captureRead(it->second);
  return &m_storage[static_cast<size_t>(it->second)];
}

/**
 * @brief Returns the raw (pre-transform) value for a dataset addressed by its alias.
 */
const DataModel::RegisterValue* DataModel::DataTableStore::getDatasetRawByAlias(
  const QString& alias) const
{
  SS_ASSERT(m_initialized, return nullptr);

  const auto it = m_aliasIndex.constFind(alias);
  if (it == m_aliasIndex.constEnd()) [[unlikely]] {
    noteMissingAlias(alias, "raw");
    return nullptr;
  }

  captureRead(it->first);
  return &m_storage[static_cast<size_t>(it->first)];
}

/**
 * @brief Returns the final (post-transform) value for a dataset addressed by its alias.
 */
const DataModel::RegisterValue* DataModel::DataTableStore::getDatasetFinalByAlias(
  const QString& alias) const
{
  SS_ASSERT(m_initialized, return nullptr);

  const auto it = m_aliasIndex.constFind(alias);
  if (it == m_aliasIndex.constEnd()) [[unlikely]] {
    noteMissingAlias(alias, "final");
    return nullptr;
  }

  captureRead(it->second);
  return &m_storage[static_cast<size_t>(it->second)];
}

/**
 * @brief Hot-path alias resolver keyed by Lua's interned string pointers; returns {rawSlot,
 *        finSlot} or {-1,-1} on miss, caching both so a repeated literal alias never re-hashes
 *        and an unknown alias warns only once. Cleared by clearLookupCache() before a Lua close.
 */
std::pair<int, int> DataModel::DataTableStore::resolveAliasSlotsInterned(const char* alias,
                                                                         const char* kind) const
{
  SS_ASSERT(m_initialized, return std::make_pair(-1, -1));

  for (const auto& entry : m_internedAliasCache)
    if (entry.aliasPtr == alias)
      return {entry.rawSlot, entry.finSlot};

  const auto it     = m_aliasIndex.constFind(QString::fromUtf8(alias));
  const bool found  = (it != m_aliasIndex.constEnd());
  const int rawSlot = found ? it->first : -1;
  const int finSlot = found ? it->second : -1;

  m_internedAliasCache[m_internedAliasCacheNext] = {alias, rawSlot, finSlot};
  m_internedAliasCacheNext = (m_internedAliasCacheNext + 1) % kInternedAliasCacheSize;

  if (!found) [[unlikely]]
    noteMissingAlias(QString::fromUtf8(alias), kind);

  return {rawSlot, finSlot};
}

/**
 * @brief Returns the raw (pre-transform) value for a dataset addressed by an interned alias.
 */
const DataModel::RegisterValue* DataModel::DataTableStore::getDatasetRawByAliasInterned(
  const char* alias) const
{
  const auto slotPair = resolveAliasSlotsInterned(alias, "raw");
  if (slotPair.first < 0) [[unlikely]]
    return nullptr;

  captureRead(slotPair.first);
  return &m_storage[static_cast<size_t>(slotPair.first)];
}

/**
 * @brief Returns the final (post-transform) value for a dataset addressed by an interned alias.
 */
const DataModel::RegisterValue* DataModel::DataTableStore::getDatasetFinalByAliasInterned(
  const char* alias) const
{
  const auto slotPair = resolveAliasSlotsInterned(alias, "final");
  if (slotPair.second < 0) [[unlikely]]
    return nullptr;

  captureRead(slotPair.second);
  return &m_storage[static_cast<size_t>(slotPair.second)];
}

//--------------------------------------------------------------------------------------------------
// Export support
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a snapshot of all table values for export logging.
 */
QMap<QString, QMap<QString, DataModel::RegisterValue>> DataModel::DataTableStore::snapshot() const
{
  QMap<QString, QMap<QString, RegisterValue>> result;

  for (const auto& [tableName, regNames] : m_tableRegNames) {
    QMap<QString, RegisterValue> tableMap;
    for (const auto& regName : regNames) {
      const int idx = indexOf(tableName, regName);
      if (idx >= 0)
        tableMap.insert(regName, m_storage[static_cast<size_t>(idx)]);
    }

    result.insert(tableName, tableMap);
  }

  return result;
}

//--------------------------------------------------------------------------------------------------
// Cross-thread mirror
//--------------------------------------------------------------------------------------------------

/**
 * @brief Copies the store into @p out for readers on another thread, reusing its buffers (the
 *        value vector keeps capacity, the lookup maps re-share); call from the owning thread
 *        only. An uninitialized store resets @p out to the empty generation -1 state, which is
 *        what lets a reader drop the previous project's values the tick after a clear().
 */
void DataModel::DataTableStore::snapshotInto(DataTableSnapshot& out) const
{
  if (!m_initialized) {
    out.generation = -1;
    out.writeClock = 0;
    out.values.clear();
    out.computed.clear();
    out.index.clear();
    out.datasetIndex.clear();
    out.aliasIndex.clear();
    return;
  }

  out.generation   = m_generation;
  out.writeClock   = m_writeClock;
  out.values       = m_storage;
  out.computed     = m_isComputed;
  out.index        = m_index;
  out.datasetIndex = m_datasetIndex;
  out.aliasIndex   = m_aliasIndex;
}

/**
 * @brief Bounds-checked slot read shared by every DataTableSnapshot accessor.
 */
[[nodiscard]] static const DataModel::RegisterValue* snapshotSlot(
  const DataModel::DataTableSnapshot& snapshot, int slot)
{
  if (slot < 0 || slot >= static_cast<int>(snapshot.values.size())) [[unlikely]]
    return nullptr;

  return &snapshot.values[static_cast<size_t>(slot)];
}

/**
 * @brief Looks up a register by table and register name; silent on miss (the live store already
 *        warned once for the same name when the parser or a transform read it).
 */
const DataModel::RegisterValue* DataModel::DataTableSnapshot::get(const QString& table,
                                                                  const QString& reg) const
{
  const auto it = index.constFind(qMakePair(table, reg));
  if (it == index.constEnd()) [[unlikely]]
    return nullptr;

  return snapshotSlot(*this, it.value());
}

/**
 * @brief Mirrors DataTableStore::isInitialized() so a reader templated over both views can ask
 *        the same question of a snapshot; an unpublished snapshot keeps generation -1.
 */
bool DataModel::DataTableSnapshot::isInitialized() const
{
  return generation >= 0;
}

/**
 * @brief True when the register exists and accepts a write. Mirrors the live store's rule so a
 *        GUI-thread caller reading through the snapshot gets the same verdict set() would give.
 */
bool DataModel::DataTableSnapshot::isWritable(const QString& table, const QString& reg) const
{
  const auto it = index.constFind(qMakePair(table, reg));
  if (it == index.constEnd())
    return false;

  const auto slot = static_cast<size_t>(it.value());
  return slot < computed.size() && computed[slot];
}

/**
 * @brief Handle-keyed twin of isWritable(), with getByHandle()'s generation and range guards.
 */
bool DataModel::DataTableSnapshot::isWritableHandle(qint64 handle) const
{
  if (handle < 0)
    return false;

  if (static_cast<int>(handle >> kHandleIndexBits) != generation)
    return false;

  const auto slot = static_cast<size_t>(handle & kHandleIndexMask);
  return slot < computed.size() && computed[slot];
}

/**
 * @brief Resolves (table, register) to the same generation-tagged handle the live store mints,
 *        so a handle stays interchangeable between the store and its snapshots.
 */
qint64 DataModel::DataTableSnapshot::handleOf(const QString& table, const QString& reg) const
{
  const auto it = index.constFind(qMakePair(table, reg));
  if (it == index.constEnd()) [[unlikely]]
    return -1;

  return (static_cast<qint64>(generation) << kHandleIndexBits) | static_cast<qint64>(it.value());
}

/**
 * @brief Reads by handle; a handle minted against another layout generation is a safe nullptr.
 */
const DataModel::RegisterValue* DataModel::DataTableSnapshot::getByHandle(qint64 handle) const
{
  if (handle < 0) [[unlikely]]
    return nullptr;

  if (static_cast<int>(handle >> kHandleIndexBits) != generation) [[unlikely]]
    return nullptr;

  return snapshotSlot(*this, static_cast<int>(handle & kHandleIndexMask));
}

/**
 * @brief Returns the raw (pre-transform) value for a dataset.
 */
const DataModel::RegisterValue* DataModel::DataTableSnapshot::getDatasetRaw(int uniqueId) const
{
  const auto it = datasetIndex.constFind(uniqueId);
  if (it == datasetIndex.constEnd()) [[unlikely]]
    return nullptr;

  return snapshotSlot(*this, it->first);
}

/**
 * @brief Returns the final (post-transform) value for a dataset.
 */
const DataModel::RegisterValue* DataModel::DataTableSnapshot::getDatasetFinal(int uniqueId) const
{
  const auto it = datasetIndex.constFind(uniqueId);
  if (it == datasetIndex.constEnd()) [[unlikely]]
    return nullptr;

  return snapshotSlot(*this, it->second);
}

/**
 * @brief Returns the raw (pre-transform) value for a dataset addressed by its alias.
 */
const DataModel::RegisterValue* DataModel::DataTableSnapshot::getDatasetRawByAlias(
  const QString& alias) const
{
  const auto it = aliasIndex.constFind(alias);
  if (it == aliasIndex.constEnd()) [[unlikely]]
    return nullptr;

  return snapshotSlot(*this, it->first);
}

/**
 * @brief Returns the final (post-transform) value for a dataset addressed by its alias.
 */
const DataModel::RegisterValue* DataModel::DataTableSnapshot::getDatasetFinalByAlias(
  const QString& alias) const
{
  const auto it = aliasIndex.constFind(alias);
  if (it == aliasIndex.constEnd()) [[unlikely]]
    return nullptr;

  return snapshotSlot(*this, it->second);
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Allocates a new register in the flat storage.
 */
void DataModel::DataTableStore::addRegister(const QString& table,
                                            const QString& reg,
                                            const RegisterValue& defaultVal,
                                            RegisterType type)
{
  const int offset      = static_cast<int>(m_storage.size());
  const bool isComputed = (type == RegisterType::Computed);

  m_storage.push_back(defaultVal);
  m_isComputed.push_back(isComputed);
  m_version.push_back(0);
  m_index.insert(qMakePair(table, reg), offset);
}

/**
 * @brief Mirrors a dataset alias onto its existing raw/final slots for by-alias and table lookups;
 *        a duplicate alias is skipped with a warning and a name that collides with a real register
 *        keeps the register, so the canonical uniqueId identity always wins. Adds no storage.
 */
void DataModel::DataTableStore::registerDatasetAlias(const QString& alias, int uniqueId)
{
  const auto ds = m_datasetIndex.constFind(uniqueId);
  if (ds == m_datasetIndex.constEnd()) [[unlikely]]
    return;

  if (m_aliasIndex.constFind(alias) != m_aliasIndex.constEnd()) [[unlikely]] {
    qWarning().noquote() << "[DataTableStore] Duplicate dataset alias" << alias
                         << "-- alias lookups resolve to the first dataset that declared it";
    return;
  }

  m_aliasIndex.insert(alias, ds.value());

  const auto rawKey = qMakePair(kSystemTable, kRawPrefix + alias);
  const auto finKey = qMakePair(kSystemTable, kFinalPrefix + alias);
  if (m_index.constFind(rawKey) == m_index.constEnd())
    m_index.insert(rawKey, ds->first);

  if (m_index.constFind(finKey) == m_index.constEnd())
    m_index.insert(finKey, ds->second);
}

/**
 * @brief Resolves a (table, register) pair to a storage offset, or -1.
 */
int DataModel::DataTableStore::indexOf(const QString& table, const QString& reg) const
{
  const auto it = m_index.constFind(qMakePair(table, reg));
  if (it == m_index.constEnd())
    return -1;

  return it.value();
}

/**
 * @brief Logs a one-shot warning for an unknown (table, register) lookup.
 */
void DataModel::DataTableStore::noteMissingLookup(const QString& table, const QString& reg) const
{
  const auto key = qMakePair(table, reg);
  if (m_warnedMissing.contains(key))
    return;

  m_warnedMissing.insert(key);
  qWarning().noquote() << "[DataTableStore] Missing register" << QString(table + "/" + reg)
                       << "-- check spelling; tableGet returns nil/empty for this register";
}

/**
 * @brief Logs a one-shot warning for an unknown dataset uniqueId lookup.
 */
void DataModel::DataTableStore::noteMissingDataset(int uniqueId, const char* kind) const
{
  if (m_warnedMissingDatasets.contains(uniqueId))
    return;

  m_warnedMissingDatasets.insert(uniqueId);
  qWarning().noquote() << "[DataTableStore] datasetGet" << kind << "called with unknown uniqueId"
                       << uniqueId << "-- returns nil/empty";
}

/**
 * @brief Logs a one-shot warning for an unknown dataset alias lookup.
 */
void DataModel::DataTableStore::noteMissingAlias(const QString& alias, const char* kind) const
{
  if (m_warnedMissingAliases.contains(alias))
    return;

  m_warnedMissingAliases.insert(alias);
  qWarning().noquote() << "[DataTableStore] datasetGet" << kind << "called with unknown alias"
                       << alias << "-- returns nil/empty";
}

//--------------------------------------------------------------------------------------------------
// TableApiBridge: QJSEngine bridge methods
//--------------------------------------------------------------------------------------------------

/**
 * @brief Converts a register value to the QVariant the JS bridge hands back; a missing register
 *        is an invalid QVariant, which reaches the script as undefined.
 */
[[nodiscard]] static QVariant registerVariant(const DataModel::RegisterValue* val)
{
  if (!val)
    return QVariant();

  return val->isNumeric ? QVariant(val->numericValue) : QVariant(val->stringValue);
}

/**
 * @brief Returns a register value from a user-defined table.
 */
QVariant DataModel::TableApiBridge::tableGet(const QString& t, const QString& r)
{
  SS_ASSERT(context.store != nullptr, return {});

  QVariant out;
  readTableView(context, [&](const auto& view) { out = registerVariant(view.get(t, r)); });
  return out;
}

/**
 * @brief Writes a value to a computed register. An undefined or null value is a safe no-op,
 *        matching the Lua bridge's nil handling.
 */
void DataModel::TableApiBridge::tableSet(const QString& t, const QString& r, const QVariant& v)
{
  SS_ASSERT(context.store != nullptr, return);

  if (!v.isValid() || v.typeId() == QMetaType::Nullptr)
    return;

  DataModel::RegisterValue rv;
  rv.numericValue = SerialStudio::toDouble(v, &rv.isNumeric);
  if (!rv.isNumeric)
    rv.stringValue = v.toString();

  auto* target = context.store;
  writeTableStore(context, [target, t, r, rv = std::move(rv)] { (void)target->set(t, r, rv); });
}

/**
 * @brief Resolves a (table, register) pair to a reusable handle for the fast read/write path.
 */
qint64 DataModel::TableApiBridge::tableHandle(const QString& t, const QString& r)
{
  SS_ASSERT(context.store != nullptr, return -1);

  qint64 handle = -1;
  readTableView(context, [&](const auto& view) { handle = view.handleOf(t, r); });
  return handle;
}

/**
 * @brief Resolves many register names against one table in a single call; -1 for unknown ones.
 */
QVariantList DataModel::TableApiBridge::tableHandleMany(const QString& t, const QVariantList& regs)
{
  SS_ASSERT(context.store != nullptr, return {});

  QVariantList handles;
  handles.reserve(regs.size());
  readTableView(context, [&](const auto& view) {
    for (const QVariant& reg : regs)
      handles.append(QVariant(view.handleOf(t, reg.toString())));
  });

  return handles;
}

/**
 * @brief Returns a register value by handle; empty QVariant for a stale or invalid handle.
 */
QVariant DataModel::TableApiBridge::tableGetH(qint64 h)
{
  SS_ASSERT(context.store != nullptr, return {});

  QVariant out;
  readTableView(context, [&](const auto& view) { out = registerVariant(view.getByHandle(h)); });
  return out;
}

/**
 * @brief Writes a value to a computed register by handle. An undefined or null value is a safe
 *        no-op, matching the Lua bridge's nil handling.
 */
void DataModel::TableApiBridge::tableSetH(qint64 h, const QVariant& v)
{
  SS_ASSERT(context.store != nullptr, return);

  if (!v.isValid() || v.typeId() == QMetaType::Nullptr)
    return;

  DataModel::RegisterValue rv;
  rv.numericValue = SerialStudio::toDouble(v, &rv.isNumeric);
  if (!rv.isNumeric)
    rv.stringValue = v.toString();

  auto* target = context.store;
  writeTableStore(context, [target, h, rv = std::move(rv)] { (void)target->setByHandle(h, rv); });
}

/**
 * @brief Returns the raw (pre-transform) value of a dataset selected by uniqueId (number) or alias
 *        (string). A string is never coerced to a number, so "128" and 128 are distinct lookups.
 */
QVariant DataModel::TableApiBridge::datasetGetRaw(const QJSValue& sel)
{
  SS_ASSERT(context.store != nullptr, return {});

  const bool isString = sel.isString();
  const bool isNumber = sel.isNumber();
  const QString alias = isString ? sel.toString() : QString();
  const int uniqueId  = isNumber ? sel.toInt() : -1;

  QVariant out;
  readTableView(context, [&](const auto& view) {
    if (isString)
      out = registerVariant(view.getDatasetRawByAlias(alias));
    else if (isNumber)
      out = registerVariant(view.getDatasetRaw(uniqueId));
  });

  return out;
}

/**
 * @brief Returns the final (post-transform) value of a dataset selected by uniqueId (number) or
 *        alias (string). A string is never coerced to a number, so "128" and 128 differ.
 */
QVariant DataModel::TableApiBridge::datasetGetFinal(const QJSValue& sel)
{
  SS_ASSERT(context.store != nullptr, return {});

  const bool isString = sel.isString();
  const bool isNumber = sel.isNumber();
  const QString alias = isString ? sel.toString() : QString();
  const int uniqueId  = isNumber ? sel.toInt() : -1;

  QVariant out;
  readTableView(context, [&](const auto& view) {
    if (isString)
      out = registerVariant(view.getDatasetFinalByAlias(alias));
    else if (isNumber)
      out = registerVariant(view.getDatasetFinal(uniqueId));
  });

  return out;
}

#ifdef BUILD_COMMERCIAL
/**
 * @brief Publishes an arbitrary payload through the project's MQTT publisher.
 */
qint64 DataModel::TableApiBridge::mqttPublish(const QString& topic,
                                              const QByteArray& payload,
                                              int qos,
                                              bool retain)
{
  static auto& publisher = MQTT::Publisher::instance();
  return publisher.mqttPublish(topic, payload, qos, retain);
}
#endif
