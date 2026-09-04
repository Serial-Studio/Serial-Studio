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

#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <QObject>
#include <vector>

#include "DataModel/DataTable.h"
#include "ThirdParty/readerwriterqueue.h"

namespace DataModel {

/**
 * @brief GUI-ward mirror of the data-table store (spec 0051 M5): the snapshot pool, the SPSC ring
 *        the builder thread publishes through, and the snapshot the GUI thread serves to
 *        painter/output scripts and API handlers. Pool and ring are one unit: a published snapshot
 *        aliases a pool slot, so a slot frees itself only once the GUI released it.
 */
class TableSnapshotChannel {
public:
  TableSnapshotChannel(const QObject& owner, DataModel::DataTableStore& store);
  TableSnapshotChannel(TableSnapshotChannel&&)                 = delete;
  TableSnapshotChannel(const TableSnapshotChannel&)            = delete;
  TableSnapshotChannel& operator=(TableSnapshotChannel&&)      = delete;
  TableSnapshotChannel& operator=(const TableSnapshotChannel&) = delete;

  /**
   * @brief The snapshot the GUI thread reads through. Its address is stable for the channel's
   *        lifetime, so the table-API context can bind it once at construction.
   */
  [[nodiscard]] const DataModel::DataTableSnapshotPtr& guiSnapshot() const noexcept
  {
    return m_guiSnapshot;
  }

  [[nodiscard]] bool drainForGui();

  void noteGuiUser();
  void publish();

private:
  [[nodiscard]] std::shared_ptr<DataModel::DataTableSnapshot> claimSlot();

private:
  static constexpr std::size_t kMirrorSlots = 4;
  static constexpr std::size_t kPoolSlots   = kMirrorSlots + 4;

  const QObject& m_owner;
  DataModel::DataTableStore& m_store;

  int m_publishedGeneration;
  quint64 m_publishedClock;
  std::size_t m_poolHint;

  // code-verify off
  // One is written once at bridge injection, the other toggles once per display tick. No
  // steady-state cross-core write traffic, so sharing a cache line is harmless.
  std::atomic<bool> m_guiUsers;
  std::atomic<bool> m_publishRequested;
  // code-verify on

  DataModel::DataTableSnapshotPtr m_guiSnapshot;
  moodycamel::ReaderWriterQueue<DataModel::DataTableSnapshotPtr> m_mirrorRing;
  std::vector<std::shared_ptr<DataModel::DataTableSnapshot>> m_pool;
};

}  // namespace DataModel
