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
#include "API/Handlers/TransformLibraryHandler.h"

#include <optional>

#include "API/CommandRegistry.h"
#include "API/SchemaBuilder.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/LuaCompat.h"
#include "DataModel/Scripting/ScriptDryRun.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Resolves the optional `language` parameter: Lua by default, JavaScript for "js" or
 *        "javascript". Returns std::nullopt for anything else.
 */
[[nodiscard]] static std::optional<bool> libraryIsJs(const QJsonObject& params)
{
  const QString language = params.value(QStringLiteral("language")).toString().trimmed().toLower();
  if (language.isEmpty() || language == QLatin1String("lua"))
    return false;

  if (language == QLatin1String("js") || language == QLatin1String("javascript"))
    return true;

  return std::nullopt;
}

/**
 * @brief Registers the shared-library commands with the registry.
 */
void API::Handlers::TransformLibraryHandler::registerCommands(CommandRegistry& registry)
{
  const API::SchemaProp languageParam{QStringLiteral("language"),
                                      QStringLiteral("string"),
                                      QStringLiteral("Which library: \"lua\" (default) or \"js\"")};
  const auto languageSchema = makeSchema({}, {languageParam});
  const auto codeSchema     = makeSchema(
    {
      {QStringLiteral("code"), QStringLiteral("string"), QStringLiteral("Library source")}
  },
    {languageParam});

  registry.registerCommand(
    QStringLiteral("project.transformLibrary.get"),
    QStringLiteral("Get a project's shared transform library (params: [language], lua by "
                   "default or js): one chunk whose top-level functions every dataset transform "
                   "of that language can call."),
    languageSchema,
    &getLibrary);

  registry.registerCommand(
    QStringLiteral("project.transformLibrary.set"),
    QStringLiteral("Replace a shared transform library (params: code, [language]; empty code "
                   "clears it). Persisted in the project; every transform engine recompiles "
                   "immediately. Validate first with project.transformLibrary.dryRun."),
    codeSchema,
    &setLibrary);

  registry.registerCommand(
    QStringLiteral("project.transformLibrary.dryRun"),
    QStringLiteral("Load and run library source in a sandboxed engine WITHOUT installing it "
                   "(params: code, [language]). Reports the error when the chunk fails to "
                   "compile or run. Use before project.transformLibrary.set."),
    codeSchema,
    &dryRun);
}

//--------------------------------------------------------------------------------------------------
// Handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current library source.
 */
API::CommandResponse API::Handlers::TransformLibraryHandler::getLibrary(const QString& id,
                                                                        const QJsonObject& params)
{
  const auto js = libraryIsJs(params);
  if (!js)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("language must be \"lua\" or \"js\""));

  auto& projectModel = DataModel::pipelineModules().projectModel;

  QJsonObject result;
  result[QStringLiteral("language")] = *js ? QStringLiteral("js") : QStringLiteral("lua");
  result[QStringLiteral("code")] =
    *js ? projectModel.transformLibraryJsCode() : projectModel.transformLibraryCode();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Replaces the library source through the project model.
 */
API::CommandResponse API::Handlers::TransformLibraryHandler::setLibrary(const QString& id,
                                                                        const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("code")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));

  const auto js = libraryIsJs(params);
  if (!js)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("language must be \"lua\" or \"js\""));

  const QString code = params.value(QStringLiteral("code")).toString();
  auto& projectModel = DataModel::pipelineModules().projectModel;
  if (*js)
    projectModel.setTransformLibraryJsCode(code);
  else
    projectModel.setTransformLibraryCode(code);

  QJsonObject result;
  result[QStringLiteral("language")] = *js ? QStringLiteral("js") : QStringLiteral("lua");
  result[QStringLiteral("length")]   = code.size();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief JavaScript half of the dry-run: evaluates the chunk in a throwaway QJSEngine under the
 *        dry-run deadline and reports the first error with its line.
 */
[[nodiscard]] static API::CommandResponse dryRunJs(const QString& id, const QString& code)
{
  using API::CommandResponse;

  DataModel::ScriptDryRun session(DataModel::ScriptDryRun::Language::JavaScript,
                                  DataModel::kScriptDryRunBudgetMs,
                                  "project.transformLibrary.dryRun");
  if (!session.valid())
    return CommandResponse::makeError(
      id, API::ErrorCode::ExecutionError, QStringLiteral("Failed to create the dry-run engine"));

  const auto result = session.evaluate(code, QStringLiteral("library.js"));
  const bool valid  = code.trimmed().isEmpty() || (!session.timedOut() && !result.isError());

  QJsonObject out;
  out[QStringLiteral("language")] = QStringLiteral("js");
  out[QStringLiteral("valid")]    = valid;
  if (!valid)
    out[QStringLiteral("error")] = session.timedOut()
                                   ? QStringLiteral("library timed out")
                                   : QStringLiteral("Line %1: %2")
                                       .arg(result.property(QStringLiteral("lineNumber")).toInt())
                                       .arg(result.toString());

  return CommandResponse::makeSuccess(id, out);
}

/**
 * @brief Runs the chunk in a throwaway sandbox (interpreter, deadline hook) and reports whether
 *        it loaded and ran; nothing is installed.
 */
API::CommandResponse API::Handlers::TransformLibraryHandler::dryRun(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("code")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));

  const auto js = libraryIsJs(params);
  if (!js)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("language must be \"lua\" or \"js\""));

  const QString code = params.value(QStringLiteral("code")).toString();
  if (*js)
    return dryRunJs(id, code);

  DataModel::ScriptDryRun session(DataModel::ScriptDryRun::Language::Lua,
                                  DataModel::kScriptDryRunBudgetMs,
                                  "project.transformLibrary.dryRun");
  if (!session.valid())
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("Failed to create the dry-run engine"));

  DataModel::installLuaCompat(session.luaState());
  const bool valid = code.trimmed().isEmpty() || session.runLuaChunk(code, "library") == LUA_OK;

  QJsonObject result;
  result[QStringLiteral("language")] = QStringLiteral("lua");
  result[QStringLiteral("valid")]    = valid;
  if (!valid)
    result[QStringLiteral("error")] =
      session.timedOut() ? QStringLiteral("library timed out") : session.luaError();

  return CommandResponse::makeSuccess(id, result);
}
