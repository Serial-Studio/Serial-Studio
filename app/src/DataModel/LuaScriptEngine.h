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

#include <lua.h>

#include <QDeadlineTimer>

#include "DataModel/IScriptEngine.h"

namespace DataModel {

/**
 * @brief Lua 5.4 script engine implementation with an instruction-count watchdog.
 */
class LuaScriptEngine final : public IScriptEngine {
public:
  LuaScriptEngine();
  ~LuaScriptEngine() override;

  [[nodiscard]] bool loadScript(const QString& script,
                                int sourceId,
                                bool showMessageBoxes) override;

  [[nodiscard]] QList<QStringList> parseString(const QString& frame) override;
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override;

  [[nodiscard]] bool isLoaded() const noexcept override;

  void collectGarbage() override;
  void reset() override;

private:
  void createState();
  void destroyState();

  [[nodiscard]] QList<QStringList> convertResult();
  [[nodiscard]] QStringList tableToStringList(int tableIndex);

  static void watchdogHook(lua_State* L, lua_Debug* ar);

private:
  static constexpr int kHookInstructionCount = 10000;
  static constexpr int kMaxElements          = 10000;
  static constexpr int kMaxVecLen            = 10000;
  static constexpr int kRuntimeWatchdogMs    = 1000;

  lua_State* m_state;
  bool m_loaded;
  QDeadlineTimer m_deadline;
};

}  // namespace DataModel
