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

#ifdef BUILD_COMMERCIAL
#  include "MQTT/Publisher.h"
#endif

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static const QString kSystemTable = QStringLiteral("__datasets__");
static const QString kRawPrefix   = QStringLiteral("raw:");
static const QString kFinalPrefix = QStringLiteral("final:");

//--------------------------------------------------------------------------------------------------
// Initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an empty DataTableStore in the uninitialized state.
 */
DataModel::DataTableStore::DataTableStore() : m_initialized(false) {}

/**
 * @brief Pre-allocates registers for user tables and the system dataset table.
 */
void DataModel::DataTableStore::initialize(const std::vector<TableDef>& userTables,
                                           const Frame& templateFrame)
{
  // Reset all state
  clear();

  // Estimate total register count for pre-allocation
  size_t totalRegs = 0;
  for (const auto& table : userTables)
    totalRegs += table.registers.size();

  // Count datasets for system table (2 registers each: raw + final)
  size_t datasetCount = 0;
  for (const auto& group : templateFrame.groups)
    datasetCount += group.datasets.size();

  totalRegs += datasetCount * 2;
  m_storage.reserve(totalRegs);

  // Build user-defined tables
  for (const auto& table : userTables) {
    std::vector<QString> regNames;
    regNames.reserve(table.registers.size());

    for (const auto& reg : table.registers) {
      // Build default RegisterValue from definition
      RegisterValue defVal;
      if (reg.defaultValue.typeId() == QMetaType::Double) {
        defVal.numericValue = reg.defaultValue.toDouble();
        defVal.isNumeric    = true;
      } else {
        defVal.stringValue = reg.defaultValue.toString();
        defVal.isNumeric   = false;
      }

      addRegister(table.name, reg.name, defVal, reg.type);
      regNames.push_back(reg.name);
    }

    m_tableRegNames.emplace_back(table.name, std::move(regNames));
  }

  // Build system __datasets__ table
  std::vector<QString> sysRegNames;
  sysRegNames.reserve(datasetCount * 2);
  for (const auto& group : templateFrame.groups) {
    for (const auto& dataset : group.datasets) {
      const int uid        = dataset.uniqueId;
      const QString rawReg = kRawPrefix + QString::number(uid);
      const QString finReg = kFinalPrefix + QString::number(uid);

      // Raw register
      const int rawOffset = static_cast<int>(m_storage.size());
      addRegister(kSystemTable, rawReg, RegisterValue{}, RegisterType::System);

      // Final register
      const int finOffset = static_cast<int>(m_storage.size());
      addRegister(kSystemTable, finReg, RegisterValue{}, RegisterType::System);

      // Store dataset index mapping
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

      return &m_storage[static_cast<size_t>(entry.storeIndex)];
    }
  }

  // Cache miss: pay the QString conversion + hash probe once, then memoize.
  const QString tableStr = QString::fromUtf8(table);
  const QString regStr   = QString::fromUtf8(reg);
  const int idx          = indexOf(tableStr, regStr);

  m_internedKeyCache[m_internedKeyCacheNext] = {table, reg, idx};
  m_internedKeyCacheNext                     = (m_internedKeyCacheNext + 1) % kInternedKeyCacheSize;

  if (idx < 0) [[unlikely]] {
    noteMissingLookup(tableStr, regStr);
    return nullptr;
  }

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
  return true;
}

/**
 * @brief Returns true once initialize() has built the register storage.
 */
bool DataModel::DataTableStore::isInitialized() const noexcept
{
  return m_initialized;
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

  // O(1) read-only check; linear scan would dominate the transform hotpath.
  if (!m_isComputed[static_cast<size_t>(idx)]) [[unlikely]] {
    qWarning() << "[DataTableStore] Cannot write to non-computed register" << table << "/" << reg;
    return false;
  }

  m_storage[static_cast<size_t>(idx)] = val;
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

  auto& rv        = m_storage[static_cast<size_t>(it->first)];
  rv.numericValue = numeric;
  rv.stringValue  = str;
  rv.isNumeric    = isNum;
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

  auto& rv        = m_storage[static_cast<size_t>(it->second)];
  rv.numericValue = numeric;
  rv.stringValue  = str;
  rv.isNumeric    = isNum;
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
// TableApiBridge -- QJSEngine bridge methods
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
  if (v.typeId() == QMetaType::Double) {
    rv.numericValue = v.toDouble();
    rv.isNumeric    = true;
  } else {
    rv.stringValue = v.toString();
    rv.isNumeric   = false;
  }

  store->set(t, r, rv);
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
