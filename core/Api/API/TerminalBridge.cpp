/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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

#include "API/TerminalBridge.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QUuid>

#include "API/CommandHandler.h"
#include "API/CommandRegistry.h"
#include "Core/SSAssert.h"

/**
 * @brief Wraps a CommandResponse into the {ok, result | error} map shape shared with the
 *        script apiCall() bridge.
 */
static QVariantMap wrapResponse(const API::CommandResponse& response)
{
  QVariantMap out;
  out.insert(QStringLiteral("ok"), response.success);
  if (response.success) {
    if (!response.result.isEmpty())
      out.insert(QStringLiteral("result"), response.result.toVariantMap());
  } else {
    out.insert(QStringLiteral("error"), response.errorMessage);
    out.insert(QStringLiteral("errorCode"), response.errorCode);
    if (!response.errorData.isEmpty())
      out.insert(QStringLiteral("errorData"), response.errorData.toVariantMap());
  }

  return out;
}

/**
 * @brief Builds the local error map for failures that never produce a CommandResponse.
 */
static QVariantMap localError(const QString& message, const char* code)
{
  QVariantMap out;
  out.insert(QStringLiteral("ok"), false);
  out.insert(QStringLiteral("error"), message);
  out.insert(QStringLiteral("errorCode"), QString::fromLatin1(code));
  return out;
}

/**
 * @brief Constructs the terminal bridge owned by the QML window that instantiates it.
 */
API::TerminalBridge::TerminalBridge(QObject* parent)
  : QObject(parent), m_handler(CommandHandler::instance()), m_registry(CommandRegistry::instance())
{}

/**
 * @brief Parses "scope.verb {json}" and dispatches it through CommandHandler::processCommand()
 *        with Trusted origin, so responses (including unknown-command errors) match the wire
 *        shape exactly; only parameter-JSON syntax errors are reported without dispatching.
 */
QVariantMap API::TerminalBridge::run(const QString& input)
{
  const QString line = input.trimmed();
  if (line.isEmpty())
    return localError(tr("Empty command line"), ErrorCode::InvalidMessageType);

  qsizetype split = -1;
  for (qsizetype i = 0; i < line.size(); ++i) {
    if (line.at(i).isSpace()) {
      split = i;
      break;
    }
  }

  const QString command   = (split < 0) ? line : line.left(split);
  const QString params_in = (split < 0) ? QString() : line.mid(split + 1).trimmed();

  QJsonObject params;
  if (!params_in.isEmpty()) {
    QJsonParseError parse_error;
    const auto doc = QJsonDocument::fromJson(params_in.toUtf8(), &parse_error);
    if (parse_error.error != QJsonParseError::NoError)
      return localError(tr("Invalid parameter JSON: %1").arg(parse_error.errorString()),
                        ErrorCode::InvalidJson);

    if (!doc.isObject())
      return localError(tr("Parameters must be a JSON object"), ErrorCode::InvalidJson);

    params = doc.object();
  }

  CommandRequest request;
  request.id      = QUuid::createUuid().toString(QUuid::WithoutBraces);
  request.command = command;
  request.params  = params;
  SS_ASSERT(request.isValid(),
            return localError(tr("Empty command line"), ErrorCode::InvalidMessageType));

  try {
    return wrapResponse(m_handler.processCommand(request));
  } catch (const std::exception& e) {
    return localError(QString::fromUtf8(e.what()), ErrorCode::ExecutionError);
  } catch (...) {
    return localError(tr("Unknown exception while executing command"), ErrorCode::ExecutionError);
  }
}

/**
 * @brief Dumps the live command registry (the source that generates api-schema.json) as
 *        {name, scope, verb, description, params:[{name, type, description, required}]}
 *        entries for the discovery tree, completion popup, and docs panel.
 */
QVariantList API::TerminalBridge::catalog() const
{
  QVariantList out;
  const auto& commands = m_registry.commands();
  out.reserve(commands.size());

  for (auto it = commands.constBegin(); it != commands.constEnd(); ++it) {
    const auto& definition = it.value();
    SS_ASSERT_LOG(!definition.name.isEmpty());
    if (definition.name.isEmpty())
      continue;

    const qsizetype dot = definition.name.indexOf(QLatin1Char('.'));
    QVariantMap entry;
    entry.insert(QStringLiteral("name"), definition.name);
    entry.insert(QStringLiteral("scope"), (dot < 0) ? QString() : definition.name.left(dot));
    entry.insert(QStringLiteral("verb"),
                 (dot < 0) ? definition.name : definition.name.mid(dot + 1));
    entry.insert(QStringLiteral("description"), definition.description);

    const auto properties = definition.inputSchema.value(QStringLiteral("properties")).toObject();
    const auto required   = definition.inputSchema.value(QStringLiteral("required")).toArray();
    QVariantList params;
    for (auto prop = properties.constBegin(); prop != properties.constEnd(); ++prop) {
      const auto schema = prop.value().toObject();
      QVariantMap param;
      param.insert(QStringLiteral("name"), prop.key());
      param.insert(QStringLiteral("type"), schema.value(QStringLiteral("type")).toString());
      param.insert(QStringLiteral("description"),
                   schema.value(QStringLiteral("description")).toString());
      param.insert(QStringLiteral("required"), required.contains(QJsonValue(prop.key())));
      params.append(param);
    }

    entry.insert(QStringLiteral("params"), params);
    out.append(entry);
  }

  return out;
}
