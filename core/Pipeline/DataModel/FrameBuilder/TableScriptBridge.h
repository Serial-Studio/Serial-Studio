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

// clang-format off
extern "C" {
#include <lua.h>
}
// clang-format on

#include <QJSEngine>
#include <QObject>

#include "DataModel/DataTable.h"
#include "DataModel/Frame.h"

namespace DataModel {

/**
 * @brief Publishes the data-table API into script engines and owns the store initialization those
 *        engines' handles resolve against. The Lua closures and the JS bridge share one
 *        TableApiContext whose `owner` is the frame builder, so every off-thread access routes
 *        through DataModel::readTableView / writeTableStore and lands on the store's own thread.
 */
class TableScriptBridge {
public:
  TableScriptBridge(QObject& owner,
                    DataModel::DataTableStore& store,
                    const DataModel::DataTableSnapshotPtr& guiMirror);
  TableScriptBridge(TableScriptBridge&&)                 = delete;
  TableScriptBridge(const TableScriptBridge&)            = delete;
  TableScriptBridge& operator=(TableScriptBridge&&)      = delete;
  TableScriptBridge& operator=(const TableScriptBridge&) = delete;

  [[nodiscard]] const DataModel::TableApiContext& context() const noexcept { return m_context; }

  void installLua(lua_State* L);
  void installJs(QJSEngine* js);

  void initializeStore(const DataModel::Frame& frame);
  void refreshStoreFromProject();

private:
  DataModel::DataTableStore& m_store;
  DataModel::TableApiContext m_context;
};

}  // namespace DataModel
