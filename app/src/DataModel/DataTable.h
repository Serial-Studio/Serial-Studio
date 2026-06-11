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

#include <QHash>
#include <QMap>
#include <QObject>
#include <QPair>
#include <QSet>
#include <QString>
#include <QVariant>
#include <vector>

#include "DataModel/Frame.h"

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
 * @brief Flat pre-allocated store for all data table registers; central data
 *        bus for cross-dataset communication and transforms.
 */
class DataTableStore {
public:
  DataTableStore();

  void initialize(const std::vector<TableDef>& userTables, const Frame& templateFrame);

  void clear();

  [[nodiscard]] bool isInitialized() const noexcept;

  [[nodiscard]] const RegisterValue* get(const QString& table, const QString& reg) const;

  [[nodiscard]] const RegisterValue* getByInternedKey(const char* table, const char* reg) const;
  bool setByInternedKey(const char* table, const char* reg, const RegisterValue& val);

  void clearLookupCache() const;

  bool set(const QString& table, const QString& reg, const RegisterValue& val);

  void setDatasetRaw(int uniqueId, double numeric, const QString& str, bool isNum);

  void setDatasetFinal(int uniqueId, double numeric, const QString& str, bool isNum);

  [[nodiscard]] const RegisterValue* getDatasetRaw(int uniqueId) const;
  [[nodiscard]] const RegisterValue* getDatasetFinal(int uniqueId) const;

  [[nodiscard]] QMap<QString, QMap<QString, RegisterValue>> snapshot() const;

private:
  void addRegister(const QString& table,
                   const QString& reg,
                   const RegisterValue& defaultVal,
                   RegisterType type);

  [[nodiscard]] int indexOf(const QString& table, const QString& reg) const;

  void noteMissingLookup(const QString& table, const QString& reg) const;
  void noteMissingDataset(int uniqueId, const char* kind) const;

private:
  bool m_initialized;

  std::vector<RegisterValue> m_storage;
  QHash<QPair<QString, QString>, int> m_index;
  QHash<int, std::pair<int, int>> m_datasetIndex;

  std::vector<bool> m_isComputed;

  std::vector<std::pair<QString, std::vector<QString>>> m_tableRegNames;

  mutable QSet<QPair<QString, QString>> m_warnedMissing;
  mutable QSet<int> m_warnedMissingDatasets;

  static constexpr int kInternedKeyCacheSize = 32;

  struct InternedKeyCacheEntry {
    const char* tablePtr = nullptr;
    const char* regPtr   = nullptr;
    int storeIndex       = -1;
  };

  mutable InternedKeyCacheEntry m_internedKeyCache[kInternedKeyCacheSize];
  mutable int m_internedKeyCacheNext = 0;
};

/**
 * @brief QObject bridge exposing DataTableStore to QJSEngine for JS transforms.
 */
class TableApiBridge : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

public:
  explicit TableApiBridge(QObject* parent = nullptr) : QObject(parent), store(nullptr) {}

  DataTableStore* store;

public:
  Q_INVOKABLE [[nodiscard]] QVariant tableGet(const QString& t, const QString& r);
  Q_INVOKABLE [[nodiscard]] QVariant datasetGetRaw(int uid);
  Q_INVOKABLE [[nodiscard]] QVariant datasetGetFinal(int uid);
#ifdef BUILD_COMMERCIAL
  Q_INVOKABLE qint64 mqttPublish(const QString& topic,
                                 const QByteArray& payload,
                                 int qos     = 0,
                                 bool retain = false);
#endif
public slots:
  void tableSet(const QString& t, const QString& r, const QVariant& v);
};

}  // namespace DataModel
