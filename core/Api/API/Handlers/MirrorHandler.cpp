/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/MirrorHandler.h"

#include <QJsonObject>

#include "API/CommandRegistry.h"
#include "API/Mirror/MirrorPublisher.h"
#include "API/SchemaBuilder.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires mirror.getInfo and mirror.getStructure into CommandRegistry. The subscribe,
 *        setRate, and unsubscribe commands are deliberately absent: they are connection-scoped
 *        and mirror.getInfo's `commands` field is where a client discovers them.
 */
void API::Handlers::MirrorHandler::registerCommands()
{
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("mirror.getInfo"),
    QStringLiteral("Report whether this instance can be watched by a remote dashboard, and with "
                   "what: mirror wire version, application version, current structure epoch and "
                   "layout hash, dataset and source counts, the viewer limit, and the names of "
                   "the connection-scoped commands (mirror.subscribe / setRate / unsubscribe) "
                   "that a viewer sends on its own socket."),
    emptySchema(),
    &getInfo);

  registry.registerCommand(
    QStringLiteral("mirror.getStructure"),
    QStringLiteral("Return the mirror structure for the current epoch: the ordered "
                   "(sourceId, uniqueId) dataset list every snapshot is positional against, the "
                   "layout hash over it, the remote's operation mode, plot window and frozen "
                   "flag, and the serialized project. Large projects are delivered as base64 "
                   "parts; read structureParts from mirror.getInfo and pass part."),
    makeSchema(
      {
  },
      {{QStringLiteral("part"),
        QStringLiteral("integer"),
        QStringLiteral("Zero-based part index when the structure is chunked. Omitted "
                       "returns the whole structure, which is valid only when "
                       "structureParts is 1.")}}),
    &getStructure);
}

//--------------------------------------------------------------------------------------------------
// Command implementations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Pre-flight probe: everything a viewer needs before deciding to attach.
 */
API::CommandResponse API::Handlers::MirrorHandler::getInfo(const QString& id,
                                                           const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& publisher = MirrorPublisher::instance();

  return CommandResponse::makeSuccess(id, publisher.info());
}

/**
 * @brief Returns the current epoch's structure payload, or one chunk of it.
 */
API::CommandResponse API::Handlers::MirrorHandler::getStructure(const QString& id,
                                                                const QJsonObject& params)
{
  static auto& publisher = MirrorPublisher::instance();

  const int parts = publisher.structureParts();
  if (parts < 1) {
    return CommandResponse::makeError(
      id,
      Mirror::ErrorCode::StructureTooLarge,
      QStringLiteral("Project is too large to mirror (over %1 structure parts)")
        .arg(Mirror::kMaxStructureParts));
  }

  if (parts == 1 && !params.contains(QStringLiteral("part")))
    return CommandResponse::makeSuccess(id, publisher.structure());

  const int part          = params.value(QStringLiteral("part")).toInt(0);
  const auto chunkPayload = publisher.structureChunk(part);
  if (chunkPayload.isEmpty()) {
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Structure part %1 does not exist (parts: %2)")
                                        .arg(QString::number(part), QString::number(parts)));
  }

  return CommandResponse::makeSuccess(id, chunkPayload);
}
