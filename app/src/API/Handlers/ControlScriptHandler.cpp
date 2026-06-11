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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/ControlScriptHandler.h"

#include <QFile>
#include <QJSEngine>

#include "API/CommandRegistry.h"
#include "API/SchemaBuilder.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/ControlScript.h"
#include "DataModel/Scripting/JsWatchdog.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static constexpr int kDryRunWatchdogMs = 2000;

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all control-script commands with the registry.
 */
void API::Handlers::ControlScriptHandler::registerCommands()
{
  auto& registry   = CommandRegistry::instance();
  const auto empty = emptySchema();

  const auto codeSchema = makeSchema({
    {QStringLiteral("code"), QStringLiteral("string"), QStringLiteral("Control script source")}
  });

  registry.registerCommand(
    QStringLiteral("controlscript.get"),
    QStringLiteral("Get the project's setup()/loop() control script source code."),
    empty,
    &getScript);

  registry.registerCommand(
    QStringLiteral("controlscript.getCode"),
    QStringLiteral("Get the control script source (alias of controlscript.get; matches the "
                   "project.frameParser.getCode naming convention)."),
    empty,
    &getScript);

  registry.registerCommand(
    QStringLiteral("controlscript.set"),
    QStringLiteral("Replace the project's control script source (params: code). The script is "
                   "persisted in the project and applied to the live runtime; if a device is "
                   "connected it is recompiled and restarted immediately. Validate first with "
                   "controlscript.dryRun."),
    codeSchema,
    &setScript);

  registry.registerCommand(
    QStringLiteral("controlscript.setCode"),
    QStringLiteral("Replace the control script source (alias of controlscript.set; params: "
                   "code; matches the project.frameParser.setCode naming convention)."),
    codeSchema,
    &setScript);

  registry.registerCommand(
    QStringLiteral("controlscript.dryRun"),
    QStringLiteral("Validate control-script source WITHOUT installing or running it (params: "
                   "code). Compiles the script in a sandboxed engine with the SDK prelude, "
                   "reports syntax errors with line numbers, and checks that setup() and/or "
                   "loop() are defined. setup()/loop() are never executed and apiCall/tableSet "
                   "have no effect. Use before controlscript.set."),
    codeSchema,
    &dryRun);

  registry.registerCommand(
    QStringLiteral("controlscript.getStatus"),
    QStringLiteral("Returns whether the control script is currently running."),
    empty,
    &getStatus);
}

//--------------------------------------------------------------------------------------------------
// Handlers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current control-script source.
 */
API::CommandResponse API::Handlers::ControlScriptHandler::getScript(const QString& id,
                                                                    const QJsonObject& params)
{
  Q_UNUSED(params)

  QJsonObject result;
  result[QStringLiteral("code")] = DataModel::ProjectModel::instance().controlScriptCode();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Replaces the control-script source through the project model.
 */
API::CommandResponse API::Handlers::ControlScriptHandler::setScript(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("code"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));
  }

  const QString code = params.value(QStringLiteral("code")).toString();
  DataModel::ProjectModel::instance().setControlScriptCode(code);

  QJsonObject result;
  result[QStringLiteral("running")] = DataModel::ControlScript::instance().running();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Returns the running state of the control script.
 */
API::CommandResponse API::Handlers::ControlScriptHandler::getStatus(const QString& id,
                                                                    const QJsonObject& params)
{
  Q_UNUSED(params)

  QJsonObject result;
  result[QStringLiteral("running")] = DataModel::ControlScript::instance().running();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Compile-checks control-script source in a throwaway engine, mirroring the worker's
 *        compile step: stub bridge + SDK prelude + watchdogged top-level evaluation, then a
 *        setup()/loop() presence check. Nothing is installed or executed.
 */
API::CommandResponse API::Handlers::ControlScriptHandler::dryRun(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("code")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));

  const QString code = params.value(QStringLiteral("code")).toString();

  QJsonObject result;
  if (code.trimmed().isEmpty()) {
    result[QStringLiteral("valid")] = false;
    result[QStringLiteral("error")] = QStringLiteral("Script is empty");
    return CommandResponse::makeSuccess(id, result);
  }

  QJSEngine engine;
  engine.installExtensions(QJSEngine::ConsoleExtension);
  DataModel::JsWatchdog watchdog(
    &engine, kDryRunWatchdogMs, QStringLiteral("controlscript.dryRun"));

  engine.evaluate(
    QStringLiteral("var __ss_bridge = { call: function() { return { ok: false, error: 'dryRun' "
                   "}; }, listCommands: function() { return []; }, delay: function() {} };\n"
                   "var __ss_control = true;"));

  QFile sdkFile(QStringLiteral(":/api/SerialStudio.js"));
  if (sdkFile.open(QFile::ReadOnly))
    engine.evaluate(QString::fromUtf8(sdkFile.readAll()));

  watchdog.arm();
  const auto evalResult = engine.evaluate(code, QStringLiteral("control-script.js"));
  watchdog.disarm();

  if (engine.isInterrupted()) {
    engine.setInterrupted(false);
    result[QStringLiteral("valid")] = false;
    result[QStringLiteral("error")] =
      QStringLiteral("Script did not finish evaluating within %1 ms (infinite loop at the top "
                     "level?)")
        .arg(kDryRunWatchdogMs);
    return CommandResponse::makeSuccess(id, result);
  }

  if (evalResult.isError()) {
    result[QStringLiteral("valid")] = false;
    result[QStringLiteral("line")]  = evalResult.property(QStringLiteral("lineNumber")).toInt();
    result[QStringLiteral("error")] = evalResult.toString();
    return CommandResponse::makeSuccess(id, result);
  }

  const bool hasSetup = engine.globalObject().property(QStringLiteral("setup")).isCallable();
  const bool hasLoop  = engine.globalObject().property(QStringLiteral("loop")).isCallable();

  result[QStringLiteral("valid")]    = hasSetup || hasLoop;
  result[QStringLiteral("hasSetup")] = hasSetup;
  result[QStringLiteral("hasLoop")]  = hasLoop;
  if (!hasSetup && !hasLoop)
    result[QStringLiteral("error")] = QStringLiteral("Script must define setup() and/or loop().");

  return CommandResponse::makeSuccess(id, result);
}
