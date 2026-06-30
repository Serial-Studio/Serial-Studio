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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/DataTable.h"

#include <QDebug>

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
  m_isComputed.clear();
  m_version.clear();
  m_tableRegNames.clear();
  m_warnedMissing.clear();
  m_warnedMissingDatasets.clear();
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
  Q_ASSERT(m_initialized);

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
 * @brief Hot-path write keyed by Lua's interned string pointers.
 */
bool DataModel::DataTableStore::setByInternedKey(const char* table,
                                                 const char* reg,
                                                 const RegisterValue& val)
{
  Q_ASSERT(m_initialized);
  Q_ASSERT(m_isComputed.size() == m_storage.size());

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
  Q_ASSERT(m_initialized);

  const int idx = indexOf(table, reg);
  if (idx < 0) [[unlikely]] {
    noteMissingLookup(table, reg);
    return nullptr;
  }

  captureRead(idx);
  return &m_storage[static_cast<size_t>(idx)];
}

/**
 * @brief Writes to a computed register; no-op for constant or system registers.
 */
bool DataModel::DataTableStore::set(const QString& table,
                                    const QString& reg,
                                    const RegisterValue& val)
{
  Q_ASSERT(m_initialized);
  Q_ASSERT(m_isComputed.size() == m_storage.size());

  const int idx = indexOf(table, reg);
  if (idx < 0) [[unlikely]]
    return false;

  if (!m_isComputed[static_cast<size_t>(idx)]) [[unlikely]] {
    qWarning() << "[DataTableStore] Cannot write to non-computed register" << table << "/" << reg;
    return false;
  }

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
  Q_ASSERT(m_initialized);

  const int idx = indexOf(table, reg);
  if (idx < 0) [[unlikely]]
    return -1;

  return (static_cast<qint64>(m_generation) << kHandleIndexBits) | static_cast<qint64>(idx);
}

/**
 * @brief Hot-path read by handle; a stale, out-of-range, or -1 handle is a safe nullptr.
 */
const DataModel::RegisterValue* DataModel::DataTableStore::getByHandle(qint64 handle) const
{
  Q_ASSERT(m_initialized);

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
 */
bool DataModel::DataTableStore::setByHandle(qint64 handle, const RegisterValue& val)
{
  Q_ASSERT(m_initialized);
  Q_ASSERT(m_isComputed.size() == m_storage.size());

  if (handle < 0) [[unlikely]]
    return false;

  const int gen   = static_cast<int>(handle >> kHandleIndexBits);
  const int index = static_cast<int>(handle & kHandleIndexMask);
  if (gen != m_generation || index >= static_cast<int>(m_storage.size())) [[unlikely]]
    return false;

  if (!m_isComputed[static_cast<size_t>(index)]) [[unlikely]]
    return false;

  m_storage[static_cast<size_t>(index)] = val;
  m_version[static_cast<size_t>(index)] = ++m_writeClock;
  return true;
}

//--------------------------------------------------------------------------------------------------
// System dataset table
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes raw (pre-transform) values for a dataset.
 */
void DataModel::DataTableStore::setDatasetRaw(int uniqueId,
                                              double numeric,
                                              const QString& str,
                                              bool isNum)
{
  Q_ASSERT(m_initialized);

  const auto it = m_datasetIndex.constFind(uniqueId);
  if (it == m_datasetIndex.constEnd()) [[unlikely]]
    return;

  auto& rv                                  = m_storage[static_cast<size_t>(it->first)];
  rv.numericValue                           = numeric;
  rv.stringValue                            = str;
  rv.isNumeric                              = isNum;
  m_version[static_cast<size_t>(it->first)] = ++m_writeClock;
}

/**
 * @brief Writes final (post-transform) values for a dataset.
 */
void DataModel::DataTableStore::setDatasetFinal(int uniqueId,
                                                double numeric,
                                                const QString& str,
                                                bool isNum)
{
  Q_ASSERT(m_initialized);

  const auto it = m_datasetIndex.constFind(uniqueId);
  if (it == m_datasetIndex.constEnd()) [[unlikely]]
    return;

  auto& rv                                   = m_storage[static_cast<size_t>(it->second)];
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
  Q_ASSERT(m_initialized);

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
  Q_ASSERT(m_initialized);

  const auto it = m_datasetIndex.constFind(uniqueId);
  if (it == m_datasetIndex.constEnd()) [[unlikely]] {
    noteMissingDataset(uniqueId, "final");
    return nullptr;
  }

  captureRead(it->second);
  return &m_storage[static_cast<size_t>(it->second)];
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

//--------------------------------------------------------------------------------------------------
// TableApiBridge: QJSEngine bridge methods
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a register value from a user-defined table.
 */
QVariant DataModel::TableApiBridge::tableGet(const QString& t, const QString& r)
{
  Q_ASSERT(store);

  const auto* val = store->get(t, r);
  if (!val)
    return QVariant();

  if (val->isNumeric)
    return QVariant(val->numericValue);

  return QVariant(val->stringValue);
}

/**
 * @brief Writes a value to a computed register.
 */
void DataModel::TableApiBridge::tableSet(const QString& t, const QString& r, const QVariant& v)
{
  Q_ASSERT(store);

  DataModel::RegisterValue rv;
  rv.numericValue = SerialStudio::toDouble(v, &rv.isNumeric);
  if (!rv.isNumeric)
    rv.stringValue = v.toString();

  store->set(t, r, rv);
}

/**
 * @brief Resolves a (table, register) pair to a reusable handle for the fast read/write path.
 */
qint64 DataModel::TableApiBridge::tableHandle(const QString& t, const QString& r)
{
  Q_ASSERT(store);
  return store->handleOf(t, r);
}

/**
 * @brief Resolves many register names against one table in a single call; -1 for unknown ones.
 */
QVariantList DataModel::TableApiBridge::tableHandleMany(const QString& t, const QVariantList& regs)
{
  Q_ASSERT(store);

  QVariantList handles;
  handles.reserve(regs.size());
  for (const QVariant& reg : regs)
    handles.append(QVariant(store->handleOf(t, reg.toString())));

  return handles;
}

/**
 * @brief Returns a register value by handle; empty QVariant for a stale or invalid handle.
 */
QVariant DataModel::TableApiBridge::tableGetH(qint64 h)
{
  Q_ASSERT(store);

  const auto* val = store->getByHandle(h);
  if (!val)
    return QVariant();

  return val->isNumeric ? QVariant(val->numericValue) : QVariant(val->stringValue);
}

/**
 * @brief Writes a value to a computed register by handle.
 */
void DataModel::TableApiBridge::tableSetH(qint64 h, const QVariant& v)
{
  Q_ASSERT(store);

  DataModel::RegisterValue rv;
  rv.numericValue = SerialStudio::toDouble(v, &rv.isNumeric);
  if (!rv.isNumeric)
    rv.stringValue = v.toString();

  store->setByHandle(h, rv);
}

/**
 * @brief Returns the raw (pre-transform) value of a dataset.
 */
QVariant DataModel::TableApiBridge::datasetGetRaw(int uid)
{
  Q_ASSERT(store);

  const auto* val = store->getDatasetRaw(uid);
  if (!val)
    return QVariant();

  return val->isNumeric ? QVariant(val->numericValue) : QVariant(val->stringValue);
}

/**
 * @brief Returns the final (post-transform) value of a dataset.
 */
QVariant DataModel::TableApiBridge::datasetGetFinal(int uid)
{
  Q_ASSERT(store);

  const auto* val = store->getDatasetFinal(uid);
  if (!val)
    return QVariant();

  return val->isNumeric ? QVariant(val->numericValue) : QVariant(val->stringValue);
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
  return MQTT::Publisher::instance().mqttPublish(topic, payload, qos, retain);
}
#endif
