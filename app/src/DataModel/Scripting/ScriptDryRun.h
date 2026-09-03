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

// clang-format off
extern "C" {
#include <lua.h>
}
// clang-format on

#include <memory>
#include <QJSEngine>
#include <QJSValue>
#include <QString>

#include "DataModel/Scripting/JsWatchdog.h"
#include "DataModel/Scripting/LuaDeadlineHook.h"

namespace DataModel {

// Budget for one validation, preview or dry-run step of user code on the GUI thread
inline constexpr int kScriptDryRunBudgetMs = 2000;

/**
 * @brief Outcome of a one-shot dry run: whether the code compiled and ran, whether it was cut off
 *        by the deadline, and the error plus its line number when it failed.
 */
struct ScriptDryRunResult {
  bool ok       = false;
  bool timedOut = false;
  int line      = 0;
  QString error;
};

/**
 * @brief A throwaway, deadline-guarded script session for every validate / test / preview / dryRun
 *        path: a QJSEngine behind a JsWatchdog, or a sandboxed lua_State behind a LuaDeadlineHook,
 *        so user code on the GUI thread errors out instead of freezing the app. The caller installs
 *        the API surface it needs on the exposed engine or state; the session owns both.
 */
class ScriptDryRun {
public:
  /**
   * @brief Which engine the session drives.
   */
  enum class Language {
    JavaScript,
    Lua,
  };

  ScriptDryRun(Language language, int budgetMs, const char* label);
  ~ScriptDryRun();

  ScriptDryRun(const ScriptDryRun&)            = delete;
  ScriptDryRun& operator=(const ScriptDryRun&) = delete;

  [[nodiscard]] bool valid() const noexcept;
  [[nodiscard]] QJSEngine* jsEngine() const noexcept;
  [[nodiscard]] lua_State* luaState() const noexcept;

  [[nodiscard]] QJSValue evaluate(const QString& code, const QString& fileName);
  [[nodiscard]] QJSValue call(QJSValue& fn, const QJSValueList& args);

  [[nodiscard]] int runLuaChunk(const QString& code, const char* chunkName);
  [[nodiscard]] int callLua(int nargs, int nresults);
  [[nodiscard]] QString luaError();

  [[nodiscard]] bool timedOut() const noexcept;
  [[nodiscard]] int budgetMs() const noexcept;

  [[nodiscard]] static ScriptDryRunResult runJsDryRun(const QString& code,
                                                      const QString& prelude,
                                                      int budgetMs);

private:
  void createLuaState();

private:
  Language m_language;
  int m_budgetMs;
  const char* m_label;
  bool m_timedOut;
  lua_State* m_luaState;
  LuaDeadlineHook m_luaHook;
  std::unique_ptr<QJSEngine> m_jsEngine;
  std::unique_ptr<JsWatchdog> m_jsWatchdog;
};

}  // namespace DataModel
