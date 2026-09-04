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

#include <functional>
#include <map>
#include <memory>
#include <QDeadlineTimer>
#include <QJSEngine>
#include <QJSValue>
#include <QString>
#include <vector>

#include "Core/HotpathOptimization.h"
#include "DataModel/DataTable.h"
#include "DataModel/Frame.h"
#include "DataModel/Scripting/ExpressionTransform.h"
#include "DataModel/Scripting/JsWatchdog.h"

namespace DataModel {

// Per-call budget for one dataset transform; also the JS watchdog's interrupt period
inline constexpr int kTransformWatchdogMs = 100;

/**
 * @brief One dataset's transform source, paired with the dataset it belongs to.
 */
struct TransformEntry {
  int uniqueId;
  QString code;
};

/**
 * @brief Registry reference to a compiled Lua transform plus its arity probe result.
 */
struct LuaTransformRef {
  int ref;
  bool acceptsInfo;
};

/**
 * @brief Compiled JavaScript transform function plus its arity probe result.
 */
struct JsTransformRef {
  QJSValue fn;
  bool acceptsInfo;
};

/**
 * @brief One (source, language) transform engine: the interpreter state, the per-dataset
 *        compiled references, and the watchdog state the execution path arms per frame.
 */
struct TransformEngine {
  lua_State* luaState = nullptr;
  QJSEngine* jsEngine = nullptr;
  std::unique_ptr<JsWatchdog> jsWatchdog;
  std::map<int, LuaTransformRef> luaRefs;
  std::map<int, JsTransformRef> jsRefs;
  QDeadlineTimer luaDeadline{QDeadlineTimer::Forever};
  std::unique_ptr<Expression::SlotTable> exprSlots;
  std::map<int, Expression::Runtime> exprRefs;
};

/**
 * @brief Engine map key: transforms of one source and one language share a single engine.
 */
struct EngineKey {
  int sourceId;
  int language;

  bool operator<(const EngineKey& other) const noexcept
  {
    return sourceId < other.sourceId || (sourceId == other.sourceId && language < other.language);
  }
};

/**
 * @brief Owns the per-dataset transform engines and everything that builds them: the Lua sandbox,
 *        the JavaScript engines, the compiled-expression programs, and the transform-error stats
 *        the 1 Hz diagnostics pull. Compile-time only. The Lua bootstrap installs the table API
 *        through an injected installer, the facade method that also arms the cached capture flags.
 */
class TransformCompiler {
public:
  using LuaApiInstaller = std::function<void(lua_State*)>;

  TransformCompiler(const DataModel::Frame& frame,
                    DataModel::DataTableStore& store,
                    LuaApiInstaller installLuaTableApi);
  ~TransformCompiler();
  TransformCompiler(TransformCompiler&&)                 = delete;
  TransformCompiler(const TransformCompiler&)            = delete;
  TransformCompiler& operator=(TransformCompiler&&)      = delete;
  TransformCompiler& operator=(const TransformCompiler&) = delete;

  /**
   * @brief True when no engine is compiled. Read once per frame by the parse lanes, so it is
   *        inline here to stay the same load it was as a member of the frame builder.
   */
  [[nodiscard]] bool empty() const noexcept { return m_engines.empty(); }

  [[nodiscard]] quint64 errorCount() const noexcept { return m_transformErrors; }

  [[nodiscard]] int lastErrorDataset() const noexcept { return m_lastTransformDatasetUniqueId; }

  [[nodiscard]] const QString& lastError() const noexcept { return m_lastTransformError; }

  [[nodiscard]] bool hasScriptEngines() const noexcept;
  [[nodiscard]] TransformEngine* engineFor(int sourceId, int language) noexcept;

  void compile();
  void destroy();
  void collectGarbage();

  SS_COLD void noteTransformError(int uniqueId, const char* message);
  SS_COLD void noteTransformError(int uniqueId, const QString& message);

private:
  void compileLua(TransformEngine& engine,
                  int sourceId,
                  const std::vector<TransformEntry>& entries);
  void compileLuaEntry(lua_State* L, TransformEngine& engine, const TransformEntry& entry);
  void compileExpr(TransformEngine& engine,
                   int sourceId,
                   const std::vector<TransformEntry>& entries);
  void compileJs(TransformEngine& engine, int sourceId, const std::vector<TransformEntry>& entries);

private:
  const DataModel::Frame& m_frame;
  DataModel::DataTableStore& m_store;
  LuaApiInstaller m_installLuaTableApi;

  quint64 m_transformErrors;
  int m_lastTransformDatasetUniqueId;
  QString m_lastTransformError;

  std::map<EngineKey, TransformEngine> m_engines;
};

}  // namespace DataModel
