/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/ScriptsHandler.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "API/CommandRegistry.h"
#include "API/SchemaBuilder.h"

namespace detail {

/**
 * @brief One script kind paired with its rcc base path and language tag.
 */
struct Kind {
  QString id;           // "painter", "frame_parser_js", ...
  QString manifest;     // qrc:/scripts/<dir>/templates.json
  QString bodyDir;      // qrc:/scripts/<dir>/<file>.<ext>
  QString extension;    // ".js", ".lua"
  QString description;  // shown in scripts.list output
};

}  // namespace detail

/**
 * @brief Static catalog of every script kind exposed via scripts.*.
 */
static const QList<detail::Kind>& kinds()
{
  static const QList<detail::Kind> kCatalog = {
    {         QStringLiteral("painter"),
     QStringLiteral(":/scripts/painter/templates.json"),
     QStringLiteral(":/scripts/painter/"),
     QStringLiteral(".js"),
     QStringLiteral("Painter widget JavaScript -- canvas-2D programs that "
     "render group widgets. Use with project.painter.setCode.")},
    { QStringLiteral("frame_parser_js"),
     QStringLiteral(":/scripts/parser/templates.json"),
     QStringLiteral(":/scripts/parser/js/"),
     QStringLiteral(".js"),
     QStringLiteral("Frame parser, JavaScript flavor. parse(frame) -> array "
     "of dataset values. Use with "
     "project.frameParser.setCode + setLanguage(0).")          },
    {QStringLiteral("frame_parser_lua"),
     QStringLiteral(":/scripts/parser/templates.json"),
     QStringLiteral(":/scripts/parser/lua/"),
     QStringLiteral(".lua"),
     QStringLiteral("Frame parser, Lua flavor. parse(frame) -> table of "
     "dataset values. Use with project.frameParser.setCode + "
     "setLanguage(1).")                                        },
    {    QStringLiteral("transform_js"),
     QStringLiteral(":/scripts/transforms/templates.json"),
     QStringLiteral(":/scripts/transforms/js/"),
     QStringLiteral(".js"),
     QStringLiteral("Per-dataset value transform, JavaScript flavor. "
     "transform(value) -> number. Use with "
     "project.dataset.setTransformCode.")                      },
    {   QStringLiteral("transform_lua"),
     QStringLiteral(":/scripts/transforms/templates.json"),
     QStringLiteral(":/scripts/transforms/lua/"),
     QStringLiteral(".lua"),
     QStringLiteral("Per-dataset value transform, Lua flavor. "
     "transform(value) -> number. Use with "
     "project.dataset.setTransformCode.")                      },
    {QStringLiteral("output_widget_js"),
     QStringLiteral(":/scripts/output/templates.json"),
     QStringLiteral(":/scripts/output/"),
     QStringLiteral(".js"),
     QStringLiteral("Output widget transmit script. Maps UI state to bytes "
     "for the device. Use with "
     "project.outputWidget.update {transmitFunction}.")        },
  };
  return kCatalog;
}

/**
 * @brief Looks up a kind by id; returns nullptr when unknown.
 */
static const detail::Kind* findKind(const QString& id)
{
  for (const auto& k : kinds())
    if (k.id == id)
      return &k;

  return nullptr;
}

/**
 * @brief Reads the manifest for one kind into a QJsonArray of entries.
 */
static QJsonArray loadManifest(const detail::Kind& kind)
{
  QFile f(kind.manifest);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    return {};

  return QJsonDocument::fromJson(f.readAll()).array();
}

/**
 * @brief Returns a brief catalog row for one manifest entry.
 */
static QJsonObject summarizeEntry(const detail::Kind& kind, const QJsonObject& entry)
{
  QJsonObject row;
  row[QStringLiteral("id")]   = entry.value(QStringLiteral("file")).toString();
  row[QStringLiteral("name")] = entry.value(QStringLiteral("name")).toString();
  if (entry.contains(QStringLiteral("default")))
    row[QStringLiteral("isDefault")] = entry.value(QStringLiteral("default")).toBool();

  if (entry.contains(QStringLiteral("datasets"))) {
    const auto ds                       = entry.value(QStringLiteral("datasets")).toArray();
    row[QStringLiteral("datasetCount")] = ds.size();
  }
  row[QStringLiteral("kind")] = kind.id;
  return row;
}

/**
 * @brief Wires scripts.list and scripts.get into CommandRegistry.
 */
void API::Handlers::ScriptsHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("scripts.list"),
    QStringLiteral("List bundled reference scripts the dashboard ships with. "
                   "Use this BEFORE writing painter / frame-parser / "
                   "transform / output-widget code so you can adapt real "
                   "templates instead of inventing patterns. Optional "
                   "params: kind (filter to one kind: painter, "
                   "frame_parser_js, frame_parser_lua, transform_js, "
                   "transform_lua, output_widget_js)."),
    makeSchema(
      {
  },
      {{QStringLiteral("kind"),
        QStringLiteral("string"),
        QStringLiteral("Optional kind filter -- one of: painter, "
                       "frame_parser_js, frame_parser_lua, transform_js, "
                       "transform_lua, output_widget_js. When omitted, all "
                       "kinds are listed.")}}),
    &list);

  registry.registerCommand(QStringLiteral("scripts.get"),
                           QStringLiteral("Read the body of one bundled reference script "
                                          "(params: kind, id). The id is the `file` field "
                                          "from scripts.list (no extension)."),
                           makeSchema({
                             {QStringLiteral("kind"),
                              QStringLiteral("string"),
                              QStringLiteral("Script kind, see scripts.list")                   },
                             {  QStringLiteral("id"),
                              QStringLiteral("string"),
                              QStringLiteral("Template id (e.g. \"progress_rings\", \"clamp\")")}
  }),
                           &get);
}

/**
 * @brief Returns the full catalog (or one kind's slice) of bundled scripts.
 */
API::CommandResponse API::Handlers::ScriptsHandler::list(const QString& id,
                                                         const QJsonObject& params)
{
  const auto kindFilter = params.value(QStringLiteral("kind")).toString();

  QJsonArray entries;
  QJsonArray kindList;
  int total = 0;

  for (const auto& k : kinds()) {
    if (!kindFilter.isEmpty() && k.id != kindFilter)
      continue;

    QJsonObject kobj;
    kobj[QStringLiteral("id")]          = k.id;
    kobj[QStringLiteral("description")] = k.description;
    kindList.append(kobj);

    const auto manifest = loadManifest(k);
    for (const auto& v : manifest) {
      const auto entry = v.toObject();
      entries.append(summarizeEntry(k, entry));
      total += 1;
    }
  }

  if (!kindFilter.isEmpty() && kindList.isEmpty()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Unknown script kind: %1. Valid kinds: painter, "
                     "frame_parser_js, frame_parser_lua, transform_js, "
                     "transform_lua, output_widget_js")
        .arg(kindFilter));
  }

  QJsonObject result;
  result[QStringLiteral("kinds")]   = kindList;
  result[QStringLiteral("entries")] = entries;
  result[QStringLiteral("total")]   = total;
  result[QStringLiteral("hint")] =
    QStringLiteral("Each entry has {id, name, kind, datasetCount?}. Read the body with "
                   "scripts.get {kind, id} -- the body is the exact text you can pass to "
                   "the corresponding setCode endpoint after light edits.");
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Returns the body of one script (UTF-8 text).
 */
API::CommandResponse API::Handlers::ScriptsHandler::get(const QString& id,
                                                        const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("kind")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: kind"));

  if (!params.contains(QStringLiteral("id")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: id"));

  const auto kindStr = params.value(QStringLiteral("kind")).toString();
  const auto entryId = params.value(QStringLiteral("id")).toString();
  const auto* kind   = findKind(kindStr);
  if (!kind)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Unknown script kind: %1").arg(kindStr));

  const auto manifest = loadManifest(*kind);
  bool known          = false;
  for (const auto& v : manifest) {
    if (v.toObject().value(QStringLiteral("file")).toString() == entryId) {
      known = true;
      break;
    }
  }
  if (!known)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Unknown template id '%1' for kind '%2'. "
                     "Use scripts.list {kind} to see what's available.")
        .arg(entryId, kindStr));

  QFile f(kind->bodyDir + entryId + kind->extension);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("Failed to read template '%1' (%2)").arg(entryId, f.fileName()));

  const auto body = QString::fromUtf8(f.readAll());

  QJsonObject result;
  result[QStringLiteral("kind")]      = kind->id;
  result[QStringLiteral("id")]        = entryId;
  result[QStringLiteral("body")]      = body;
  result[QStringLiteral("byteCount")] = body.size();
  return CommandResponse::makeSuccess(id, result);
}
