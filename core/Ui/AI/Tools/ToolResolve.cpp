/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Tools/ToolResolve.h"

#include <QJsonArray>
#include <QString>

#include "AI/Tools/ToolSupport.h"
#include "DataModel/Frame.h"

namespace AI::ToolDetail {

/**
 * @brief Resolves a dataset from uniqueId, path, or title and returns the canonical row.
 */
QJsonObject resolveDataset(const QJsonObject& args)
{
  QJsonArray attempts;

  auto tryResolve = [&attempts](const QString& command, const QJsonObject& params) {
    QJsonObject attempt;
    attempt[QStringLiteral("command")]   = command;
    attempt[QStringLiteral("arguments")] = params;
    const auto reply                     = runCommand(command, params);
    attempt[QStringLiteral("ok")]        = reply.value(QStringLiteral("ok")).toBool();
    attempts.append(attempt);
    return reply;
  };

  QJsonObject resolved;
  QString method;
  if (args.contains(Keys::UniqueId)) {
    QJsonObject params;
    params[Keys::UniqueId] = args.value(Keys::UniqueId);
    const auto reply       = tryResolve(QStringLiteral("project.dataset.getByUniqueId"), params);
    if (reply.value(QStringLiteral("ok")).toBool()) {
      resolved = reply.value(QStringLiteral("result")).toObject();
      method   = Keys::UniqueId;
    }
  }

  if (resolved.isEmpty() && !args.value(QStringLiteral("path")).toString().isEmpty()) {
    QJsonObject params;
    params[QStringLiteral("path")] = args.value(QStringLiteral("path")).toString();
    const auto reply = tryResolve(QStringLiteral("project.dataset.getByPath"), params);
    if (reply.value(QStringLiteral("ok")).toBool()) {
      resolved = reply.value(QStringLiteral("result")).toObject();
      method   = QStringLiteral("path");
    }
  }

  const auto path = args.value(QStringLiteral("path")).toString();
  QString title   = args.value(QStringLiteral("title")).toString();
  if (title.isEmpty() && !path.isEmpty() && !path.contains(QLatin1Char('/')))
    title = path;

  if (resolved.isEmpty() && !title.isEmpty()) {
    QJsonObject params;
    params[QStringLiteral("title")] = title;
    if (args.contains(QStringLiteral("groupId")))
      params[QStringLiteral("groupId")] = args.value(QStringLiteral("groupId")).toInt();

    if (args.contains(Keys::SourceId))
      params[Keys::SourceId] = args.value(Keys::SourceId).toInt();

    const auto reply = tryResolve(QStringLiteral("project.dataset.getByTitle"), params);
    if (reply.value(QStringLiteral("ok")).toBool()) {
      resolved = reply.value(QStringLiteral("result")).toObject();
      method   = QStringLiteral("title");
    }
  }

  QJsonObject out;
  if (resolved.isEmpty()) {
    out[QStringLiteral("ok")]       = false;
    out[QStringLiteral("error")]    = QStringLiteral("dataset_not_resolved");
    out[QStringLiteral("attempts")] = attempts;
    out[QStringLiteral("hint")] =
      QStringLiteral("path expects 'Group/Dataset' or 'Source/Group/Dataset'; a bare name is "
                     "also matched as an exact title. Call assistant.snapshot to see every "
                     "group and dataset, or pass uniqueId.");
    return out;
  }

  out[QStringLiteral("ok")]      = true;
  out[QStringLiteral("method")]  = method;
  out[QStringLiteral("dataset")] = resolved;
  out[QStringLiteral("identity")] =
    QStringLiteral("Use groupId + datasetId for dataset mutations; use uniqueId only as an opaque "
                   "stable resolver; use index only for parser output order.");
  return out;
}

/**
 * @brief Resolves a workspace by id or title against project.workspace.list.
 */
QJsonObject resolveWorkspace(const QJsonObject& args)
{
  const auto listReply = runCommand(QStringLiteral("project.workspace.list"));
  if (!listReply.value(QStringLiteral("ok")).toBool())
    return listReply;

  const auto list      = listReply.value(QStringLiteral("result")).toObject();
  const auto rows      = list.value(QStringLiteral("workspaces")).toArray();
  const int wantedId   = args.value(QStringLiteral("workspaceId")).toInt(-1);
  const QString title  = args.value(QStringLiteral("title")).toString();
  const auto titleFold = title.toCaseFolded();

  if (wantedId < 0 && title.isEmpty() && !rows.isEmpty()) {
    QJsonObject out;
    out[QStringLiteral("ok")]        = true;
    out[QStringLiteral("workspace")] = rows.first().toObject();
    out[QStringLiteral("defaulted")] = true;
    out[QStringLiteral("hint")] =
      QStringLiteral("No workspace was specified, so the first active workspace was selected.");
    return out;
  }

  for (const auto& value : rows) {
    const auto row = value.toObject();
    if (wantedId >= 0 && row.value(QStringLiteral("id")).toInt() == wantedId) {
      QJsonObject out;
      out[QStringLiteral("ok")]        = true;
      out[QStringLiteral("workspace")] = row;
      return out;
    }

    if (!title.isEmpty()
        && row.value(QStringLiteral("title")).toString().toCaseFolded() == titleFold) {
      QJsonObject out;
      out[QStringLiteral("ok")]        = true;
      out[QStringLiteral("workspace")] = row;
      return out;
    }
  }

  QJsonObject out;
  out[QStringLiteral("ok")]         = false;
  out[QStringLiteral("error")]      = QStringLiteral("workspace_not_resolved");
  out[QStringLiteral("workspaces")] = rows;
  return out;
}

/**
 * @brief Resolves a group by id or title against project.group.list.
 */
QJsonObject resolveGroup(const QJsonObject& args)
{
  const auto listReply = runCommand(QStringLiteral("project.group.list"));
  if (!listReply.value(QStringLiteral("ok")).toBool())
    return listReply;

  const auto rows =
    listReply.value(QStringLiteral("result")).toObject().value(QStringLiteral("groups")).toArray();
  const int wantedId = args.value(QStringLiteral("groupId")).toInt(-1);
  const auto title   = args.value(QStringLiteral("group")).toString().toCaseFolded();

  for (const auto& value : rows) {
    const auto row = value.toObject();
    if (wantedId >= 0
        && (row.value(QStringLiteral("groupId")).toInt(-1) == wantedId
            || row.value(QStringLiteral("id")).toInt(-1) == wantedId)) {
      QJsonObject out;
      out[QStringLiteral("ok")]    = true;
      out[QStringLiteral("group")] = row;
      return out;
    }

    if (!title.isEmpty() && row.value(QStringLiteral("title")).toString().toCaseFolded() == title) {
      QJsonObject out;
      out[QStringLiteral("ok")]    = true;
      out[QStringLiteral("group")] = row;
      return out;
    }
  }

  QJsonObject out;
  out[QStringLiteral("ok")]     = false;
  out[QStringLiteral("error")]  = QStringLiteral("group_not_resolved");
  out[QStringLiteral("groups")] = rows;
  return out;
}

}  // namespace AI::ToolDetail
