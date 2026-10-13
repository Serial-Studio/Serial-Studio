/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include <QObject>
#include <QTest>

#include "DataModel/DataTable.h"
#include "IO/PipelineHost.h"

/**
 * @brief The two out-of-line statics DataTable.cpp reaches through readTableView/writeTableStore;
 *        stubbed so the link set stays at the store TU (same precedent as the snapshot channel).
 */
bool IO::PipelineHost::pipelineParkedOnGui() noexcept
{
  return false;
}

bool IO::PipelineHost::tearingDown() noexcept
{
  return false;
}

static const QString kTable = QStringLiteral("DAQ");
static const QString kReg   = QStringLiteral("rx_count");

/**
 * @brief One computed register in one table plus one aliased dataset: the smallest layout that
 *        exercises every part of the fingerprint (user registers, system slots, aliases).
 */
static std::vector<DataModel::TableDef> tables(bool extraRegister)
{
  DataModel::RegisterDef reg;
  reg.name         = kReg;
  reg.type         = DataModel::RegisterType::Computed;
  reg.defaultValue = 0;

  DataModel::TableDef table;
  table.name = kTable;
  table.registers.push_back(reg);
  if (extraRegister) {
    DataModel::RegisterDef extra;
    extra.name = QStringLiteral("extra");
    extra.type = DataModel::RegisterType::Computed;
    table.registers.push_back(extra);
  }

  return {table};
}

static DataModel::Frame frame()
{
  DataModel::Dataset dataset;
  dataset.uniqueId = 7;
  dataset.alias    = QStringLiteral("rx");

  DataModel::Group group;
  group.datasets.push_back(dataset);

  DataModel::Frame templateFrame;
  templateFrame.groups.push_back(group);
  return templateFrame;
}

static DataModel::RegisterValue number(double value)
{
  DataModel::RegisterValue rv;
  rv.numericValue = value;
  rv.isNumeric    = true;
  return rv;
}

/**
 * @brief Rebuild semantics of DataModel::DataTableStore::initialize(): an identical layout is a
 *        no-op that keeps the generation, the cached handles and the live values, while a changed
 *        layout or a clear() bumps the generation and retires every old handle.
 */
class TstDataTableLayout : public QObject {
  Q_OBJECT

private slots:
  void identicalRebuildKeepsHandlesAndValues();
  void changedLayoutRetiresOldHandles();
  void clearForcesRebuild();
};

void TstDataTableLayout::identicalRebuildKeepsHandlesAndValues()
{
  DataModel::DataTableStore store;
  store.initialize(tables(false), {}, frame());

  const int generation = store.generation();
  const qint64 handle  = store.handleOf(kTable, kReg);
  QVERIFY(handle >= 0);
  QVERIFY(store.setByHandle(handle, number(42)));

  store.initialize(tables(false), {}, frame());

  QCOMPARE(store.generation(), generation);
  QCOMPARE(store.handleOf(kTable, kReg), handle);
  QVERIFY(store.getByHandle(handle) != nullptr);
  QCOMPARE(store.getByHandle(handle)->numericValue, 42.0);
  QVERIFY(store.setByHandle(handle, number(43)));
  QVERIFY(store.getDatasetFinalByAlias(QStringLiteral("rx")) != nullptr);
}

void TstDataTableLayout::changedLayoutRetiresOldHandles()
{
  DataModel::DataTableStore store;
  store.initialize(tables(false), {}, frame());

  const int generation = store.generation();
  const qint64 handle  = store.handleOf(kTable, kReg);
  QVERIFY(handle >= 0);

  store.initialize(tables(true), {}, frame());

  QCOMPARE(store.generation(), generation + 1);
  QVERIFY(!store.setByHandle(handle, number(1)));
  QVERIFY(store.getByHandle(handle) == nullptr);
  QVERIFY(store.handleOf(kTable, kReg) >= 0);
  QVERIFY(store.handleOf(kTable, QStringLiteral("extra")) >= 0);
}

void TstDataTableLayout::clearForcesRebuild()
{
  DataModel::DataTableStore store;
  store.initialize(tables(false), {}, frame());
  const int generation = store.generation();

  store.clear();
  QVERIFY(!store.isInitialized());

  store.initialize(tables(false), {}, frame());
  QVERIFY(store.isInitialized());
  QCOMPARE(store.generation(), generation + 1);
}

QTEST_GUILESS_MAIN(TstDataTableLayout)

#include "tst_data_table_layout.moc"
