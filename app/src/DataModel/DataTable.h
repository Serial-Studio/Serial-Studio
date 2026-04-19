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
#include <QString>
#include <QVariant>
#include <vector>

#include "DataModel/Frame.h"

namespace DataModel {

//--------------------------------------------------------------------------------------------------
// Register value
//--------------------------------------------------------------------------------------------------

/**
 * @brief Runtime value of a single register. Supports both numeric and string
 *        data without heap allocation for the numeric case.
 */
struct RegisterValue {
  double numericValue = 0.0;
  QString stringValue;
  bool isNumeric = true;
};

//--------------------------------------------------------------------------------------------------
// DataTableStore
//--------------------------------------------------------------------------------------------------

/**
 * @brief Flat, pre-allocated store for all data table registers.
 *
 * Serves as the central data bus for cross-dataset communication. Holds:
 * - User-defined tables (constants and computed registers)
 * - A system table (__datasets__) with raw/final values for every dataset
 *
 * All storage is pre-allocated at initialization. Hotpath operations
 * (get/set/reset) perform only index lookups and value writes — no heap
 * allocation.
 */
class DataTableStore {
public:
  DataTableStore();

  //------------------------------------------------------------------------------------------------
  // Initialization
  //------------------------------------------------------------------------------------------------

  void initialize(const std::vector<TableDef>& userTables, const Frame& templateFrame);

  void clear();

  [[nodiscard]] bool isInitialized() const noexcept;

  //------------------------------------------------------------------------------------------------
  // User table access (for transforms)
  //------------------------------------------------------------------------------------------------

  [[nodiscard]] const RegisterValue* get(const QString& table, const QString& reg) const;

  bool set(const QString& table, const QString& reg, const RegisterValue& val);

  //------------------------------------------------------------------------------------------------
  // System dataset table (for FrameBuilder)
  //------------------------------------------------------------------------------------------------

  void setDatasetRaw(int uniqueId, double numeric, const QString& str, bool isNum);

  void setDatasetFinal(int uniqueId, double numeric, const QString& str, bool isNum);

  [[nodiscard]] const RegisterValue* getDatasetRaw(int uniqueId) const;
  [[nodiscard]] const RegisterValue* getDatasetFinal(int uniqueId) const;

  //------------------------------------------------------------------------------------------------
  // Per-frame lifecycle
  //------------------------------------------------------------------------------------------------

  void resetComputedRegisters();

  //------------------------------------------------------------------------------------------------
  // Export support
  //------------------------------------------------------------------------------------------------

  [[nodiscard]] QMap<QString, QMap<QString, RegisterValue>> snapshot() const;

private:
  void addRegister(const QString& table,
                   const QString& reg,
                   const RegisterValue& defaultVal,
                   RegisterType type);

  [[nodiscard]] int indexOf(const QString& table, const QString& reg) const;

private:
  bool m_initialized;

  // Flat contiguous storage — one RegisterValue per register
  std::vector<RegisterValue> m_storage;

  // Index: (table, register) → offset into m_storage
  QHash<QPair<QString, QString>, int> m_index;

  // System dataset index: uniqueId → (rawOffset, finalOffset)
  QHash<int, std::pair<int, int>> m_datasetIndex;

  // Computed register offsets and their defaults for per-frame reset
  std::vector<int> m_computedOffsets;
  std::vector<RegisterValue> m_computedDefaults;

  // Parallel to m_storage: true when the offset holds a computed (writable)
  // register. O(1) isComputed lookup on the transform hotpath.
  std::vector<bool> m_isComputed;

  // Schema reference for table names in snapshot
  std::vector<std::pair<QString, std::vector<QString>>> m_tableRegNames;
};

//--------------------------------------------------------------------------------------------------
// TableApiBridge — QObject bridge for QJSEngine table/dataset API
//--------------------------------------------------------------------------------------------------

/**
 * @brief QObject bridge exposing DataTableStore to QJSEngine.
 *
 * Parented to the QJSEngine so its lifetime matches the engine. JS wrapper
 * functions installed at compile time delegate to these invokable methods.
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
public slots:
  void tableSet(const QString& t, const QString& r, const QVariant& v);
};

}  // namespace DataModel
