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

#include <QDeadlineTimer>

#include "DataModel/Scripting/IScriptEngine.h"

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
  [[nodiscard]] QList<QStringList> parseUtf8(const QByteArray& frame) override;
  [[nodiscard]] QList<QStringList> parseBinary(const QByteArray& frame) override;

  [[nodiscard]] bool isLoaded() const noexcept override;
  [[nodiscard]] int language() const noexcept override;

  [[nodiscard]] bool disabled() const noexcept override;
  [[nodiscard]] QString lastError() const override;
  [[nodiscard]] quint64 errorCount() const noexcept override;
  [[nodiscard]] int consecutiveTimeouts() const noexcept override;

  void reset() override;
  void collectGarbage() override;
  void resetErrorStats() override;

private:
  void createState();
  void destroyState();

  [[nodiscard]] bool runLoadedChunk(int sourceId, bool showMessageBoxes);
  [[nodiscard]] bool ensureParseFunction(int sourceId, bool showMessageBoxes);
  [[nodiscard]] bool probeParseFunction(int sourceId, bool showMessageBoxes);

  [[nodiscard]] QList<QStringList> parseLuaText(const char* data, qsizetype len);
  [[nodiscard]] QList<QStringList> convertResult();
  [[nodiscard]] QList<QStringList> classifyTable(int len);
  [[nodiscard]] QList<QStringList> unzipMixedTable(int len);
  [[nodiscard]] QString luaValueToString();
  [[nodiscard]] QStringList tableToStringList(int tableIndex);
  [[nodiscard]] QStringList scalarToStringList();
  void appendMixedElement(QStringList& scalars,
                          QList<QStringList>& vectors,
                          qsizetype& maxVectorLength);

  static void watchdogHook(lua_State* L, lua_Debug* ar);

private:
  static constexpr int kHookInstructionCount   = 10000;
  static constexpr int kMaxElements            = 10000;
  static constexpr int kRuntimeWatchdogMs      = 500;
  static constexpr int kMaxConsecutiveTimeouts = 3;

  [[nodiscard]] bool noteTimeoutAndCheckDisabled(int sourceId);
  void noteError(const QString& message);
  void resetTimeoutCounter() noexcept;

  lua_State* m_state;
  bool m_loaded;
  bool m_disabled;
  int m_sourceId;
  int m_parseRef;
  int m_consecutiveTimeouts;
  quint64 m_errorCount;
  QString m_lastError;
  QDeadlineTimer m_deadline;
};

}  // namespace DataModel
