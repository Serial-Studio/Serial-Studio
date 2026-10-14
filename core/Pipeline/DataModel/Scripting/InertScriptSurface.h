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

#pragma once

#include <QObject>

#include "DataModel/DataTable.h"
#include "DataModel/FrameBuilder/TableScriptBridge.h"

namespace DataModel {

/**
 * @brief A dry run's host surface (spec 0098 R5): every live global, none reaching past the
 *        engine. Bridges answer {ok:false, error:'dryRun'}, tables use a scratch store at the
 *        project's starting values, console output is dropped, verify() proves the install.
 *        GUI thread only; it must outlive the engine it was installed into.
 */
class InertScriptSurface {
public:
  InertScriptSurface();
  InertScriptSurface(InertScriptSurface&&)                 = delete;
  InertScriptSurface(const InertScriptSurface&)            = delete;
  InertScriptSurface& operator=(InertScriptSurface&&)      = delete;
  InertScriptSurface& operator=(const InertScriptSurface&) = delete;

public:
  [[nodiscard]] bool installJs(QJSEngine* js);
  [[nodiscard]] bool installLua(lua_State* L);
  [[nodiscard]] static bool verify(QJSEngine* js);
  [[nodiscard]] static bool verify(lua_State* L);

  void forgetLookupCache() const;

private:
  void seedStore();

  QObject m_owner;
  DataTableStore m_store;
  DataTableSnapshotPtr m_mirror;
  TableScriptBridge m_tableBridge;
};

}  // namespace DataModel
