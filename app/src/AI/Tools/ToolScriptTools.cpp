/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "AI/Tools/ToolScriptTools.h"

#include <QJsonArray>
#include <QString>

#include "AI/Tools/ToolResolve.h"
#include "AI/Tools/ToolSupport.h"
#include "DataModel/Frame.h"

namespace AI::ToolDetail {

//--------------------------------------------------------------------------------------------------
// Dry-run routing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps a script kind + language pair to the matching scripting-docs key.
 */
static QString scriptingDocKindForScriptKind(const QString& kind, int language)
{
  if (kind == QStringLiteral("frame_parser"))
    return language == 0 ? QStringLiteral("frame_parser_js") : QStringLiteral("frame_parser_lua");

  if (kind == QStringLiteral("transform"))
    return language == 0 ? QStringLiteral("transform_js") : QStringLiteral("transform_lua");

  if (kind == QStringLiteral("painter"))
    return QStringLiteral("painter_js");

  if (kind == QStringLiteral("output_widget"))
    return QStringLiteral("output_widget_js");

  return {};
}

/**
 * @brief Returns true when a dry-run reply succeeded at both envelope and inner-result level.
 */
static bool dryRunResultOk(const QJsonObject& reply)
{
  if (!reply.value(QStringLiteral("ok")).toBool())
    return false;

  const auto result = reply.value(QStringLiteral("result")).toObject();
  return result.isEmpty() || result.value(QStringLiteral("ok")).toBool(true);
}

/**
 * @brief Resolves the dataset that a transform-apply call should target.
 */
static QJsonObject scriptTargetDataset(const QJsonObject& args)
{
  if (args.contains(QStringLiteral("groupId")) && args.contains(Keys::DatasetId)) {
    QJsonObject dataset;
    dataset[QStringLiteral("groupId")] = args.value(QStringLiteral("groupId")).toInt();
    dataset[Keys::DatasetId]           = args.value(Keys::DatasetId).toInt();
    QJsonObject out;
    out[QStringLiteral("ok")]      = true;
    out[QStringLiteral("dataset")] = dataset;
    return out;
  }

  QJsonObject dsArgs;
  if (args.contains(Keys::UniqueId))
    dsArgs[Keys::UniqueId] = args.value(Keys::UniqueId);

  if (!args.value(QStringLiteral("dataset")).toString().isEmpty()) {
    const auto datasetName = args.value(QStringLiteral("dataset")).toString();
    if (datasetName.contains(QLatin1Char('/')))
      dsArgs[QStringLiteral("path")] = datasetName;
    else
      dsArgs[QStringLiteral("title")] = datasetName;
  }
  if (args.contains(QStringLiteral("groupId")))
    dsArgs[QStringLiteral("groupId")] = args.value(QStringLiteral("groupId")).toInt();

  if (dsArgs.isEmpty()) {
    QJsonObject out;
    out[QStringLiteral("ok")]    = false;
    out[QStringLiteral("error")] = QStringLiteral("missing_dataset_target");
    out[QStringLiteral("hint")] =
      QStringLiteral("Pass groupId+datasetId, dataset path/title, or uniqueId.");
    return out;
  }

  return resolveDataset(dsArgs);
}

/**
 * @brief Picks the frame-parser dry-run command and seeds its inputs.
 */
static QString frameParserDryRunCommand(const QJsonObject& args, QJsonObject& dryArgs, int language)
{
  dryArgs[QStringLiteral("language")] = language;

  if (args.contains(QStringLiteral("inputBytes"))
      || args.contains(QStringLiteral("inputBytesHex"))) {
    static const Keys::KeyView keys[] = {
      Keys::KeyView("inputBytes"),
      Keys::KeyView("inputBytesHex"),
      Keys::DecoderMethod,
      Keys::FrameDetection,
      Keys::FrameStart,
      Keys::FrameEnd,
      Keys::HexadecimalDelimiters,
      Keys::ChecksumAlgorithm,
      Keys::KeyView("operationMode"),
    };
    for (const auto& k : keys)
      if (args.contains(k))
        dryArgs[k] = args.value(k);

    return QStringLiteral("project.frameParser.dryRun");
  }

  return QStringLiteral("project.frameParser.dryCompile");
}

/**
 * @brief Populates transform dry-run inputs with language and sample values.
 */
static void seedTransformDryArgs(const QJsonObject& args, QJsonObject& dryArgs, int language)
{
  dryArgs[QStringLiteral("language")] = language;
  dryArgs[QStringLiteral("values")]   = args.contains(QStringLiteral("values"))
                                        ? args.value(QStringLiteral("values")).toArray()
                                        : QJsonArray{0};
}

/**
 * @brief Populates end-to-end dry-run inputs, copying optional source / sample / verbose keys.
 */
static void seedEndToEndDryArgs(const QJsonObject& args, QJsonObject& dryArgs, int language)
{
  if (args.contains(Keys::SourceId))
    dryArgs[Keys::SourceId] = args.value(Keys::SourceId).toInt();

  if (args.contains(QStringLiteral("language")))
    dryArgs[QStringLiteral("language")] = language;

  if (args.contains(QStringLiteral("sampleFrames")))
    dryArgs[QStringLiteral("sampleFrames")] = args.value(QStringLiteral("sampleFrames")).toArray();

  if (args.contains(QStringLiteral("sampleFrame")))
    dryArgs[QStringLiteral("sampleFrame")] = args.value(QStringLiteral("sampleFrame")).toString();

  if (args.contains(QStringLiteral("verbose")))
    dryArgs[QStringLiteral("verbose")] = args.value(QStringLiteral("verbose")).toBool();
}

/**
 * @brief Picks the dry-run command for a script kind and seeds its argument map.
 */
static QString dryRunCommandForKind(const QString& kind,
                                    const QJsonObject& args,
                                    QJsonObject& dryArgs,
                                    int language)
{
  if (kind == QStringLiteral("frame_parser"))
    return frameParserDryRunCommand(args, dryArgs, language);

  if (kind == QStringLiteral("transform")) {
    seedTransformDryArgs(args, dryArgs, language);
    return QStringLiteral("project.dataset.transform.dryRun");
  }

  if (kind == QStringLiteral("painter"))
    return QStringLiteral("project.painter.dryRun");

  if (kind == QStringLiteral("output_widget")) {
    if (args.contains(QStringLiteral("inputValue")))
      dryArgs[QStringLiteral("inputValue")] = args.value(QStringLiteral("inputValue")).toString();

    if (args.contains(QStringLiteral("hex")))
      dryArgs[QStringLiteral("hex")] = args.value(QStringLiteral("hex")).toBool();

    return QStringLiteral("project.outputWidget.dryRun");
  }

  if (kind == QStringLiteral("end_to_end")) {
    seedEndToEndDryArgs(args, dryArgs, language);
    return QStringLiteral("project.dryRun.endToEnd");
  }

  return {};
}

/**
 * @brief Dispatches a dry-run call to the matching scripting endpoint and attaches references.
 */
QJsonObject executeScriptDryRun(const QJsonObject& args)
{
  const auto kind = args.value(QStringLiteral("kind")).toString();
  const auto code = args.value(QStringLiteral("code")).toString();
  const int language =
    args.contains(QStringLiteral("language")) ? args.value(QStringLiteral("language")).toInt() : 1;

  QJsonObject dryArgs;
  if (!code.isEmpty())
    dryArgs[QStringLiteral("code")] = code;

  const QString command = dryRunCommandForKind(kind, args, dryArgs, language);
  if (command.isEmpty()) {
    QJsonObject out;
    out[QStringLiteral("ok")]    = false;
    out[QStringLiteral("error")] = QStringLiteral("unknown_script_kind");
    return out;
  }

  auto reply                         = runCommand(command, dryArgs);
  reply[QStringLiteral("command")]   = command;
  reply[QStringLiteral("arguments")] = dryArgs;
  const auto docsKind                = scriptingDocKindForScriptKind(kind, language);
  if (!docsKind.isEmpty())
    reply[QStringLiteral("reference")] =
      QStringLiteral("meta.fetchScriptingDocs{kind:'%1'}").arg(docsKind);

  return attachRepairHint(reply, command);
}

//--------------------------------------------------------------------------------------------------
// Apply routing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Toggles the target dataset to virtual when the apply request asks for it.
 */
static QJsonObject maybeMarkDatasetVirtual(const QJsonObject& args,
                                           int groupId,
                                           int datasetId,
                                           QJsonArray& steps)
{
  if (!args.value(QStringLiteral("virtual")).toBool(false))
    return {};

  const QJsonObject virtualArgs{
    {QStringLiteral("groupId"),   groupId},
    {          Keys::DatasetId, datasetId},
    {QStringLiteral("virtual"),      true}
  };
  const auto virtualReply = runCommand(QStringLiteral("project.dataset.setVirtual"), virtualArgs);
  steps.append(QJsonObject{
    {QStringLiteral("command"),      QStringLiteral("project.dataset.setVirtual")},
    {     QStringLiteral("ok"), virtualReply.value(QStringLiteral("ok")).toBool()}
  });
  return virtualReply;
}

/**
 * @brief Applies a transform script to its target dataset, optionally promoting it to virtual.
 */
static QJsonObject applyTransformScript(const QJsonObject& args,
                                        const QJsonObject& dryRun,
                                        int language)
{
  const auto target = scriptTargetDataset(args);
  if (!target.value(QStringLiteral("ok")).toBool())
    return target;

  const auto dataset  = target.value(QStringLiteral("dataset")).toObject();
  const int groupId   = dataset.value(QStringLiteral("groupId")).toInt();
  const int datasetId = dataset.value(Keys::DatasetId).toInt();

  QJsonArray steps;
  const auto virtualReply = maybeMarkDatasetVirtual(args, groupId, datasetId, steps);
  if (!virtualReply.isEmpty() && !virtualReply.value(QStringLiteral("ok")).toBool())
    return attachRepairHint(virtualReply, QStringLiteral("project.dataset.setVirtual"));

  QJsonObject applyArgs;
  applyArgs[QStringLiteral("code")]     = args.value(QStringLiteral("code")).toString();
  applyArgs[QStringLiteral("groupId")]  = groupId;
  applyArgs[Keys::DatasetId]            = datasetId;
  applyArgs[QStringLiteral("language")] = language;

  const QString command                = QStringLiteral("project.dataset.setTransformCode");
  auto applyReply                      = runCommand(command, applyArgs);
  applyReply[QStringLiteral("dryRun")] = dryRun;
  if (!steps.isEmpty())
    applyReply[QStringLiteral("steps")] = steps;

  return attachRepairHint(applyReply, command);
}

/**
 * @brief Maps frame-detection args from assistant.script.apply onto project.frameParser.update
 *        params; returns an empty object when the request carried no frame config.
 */
static QJsonObject frameConfigArgsFor(const QJsonObject& args)
{
  QJsonObject config;
  if (args.contains(Keys::FrameStart))
    config[QStringLiteral("startSequence")] = args.value(Keys::FrameStart).toString();

  if (args.contains(Keys::FrameEnd))
    config[QStringLiteral("endSequence")] = args.value(Keys::FrameEnd).toString();

  if (args.contains(Keys::FrameDetection))
    config[Keys::FrameDetection] = args.value(Keys::FrameDetection).toInt();

  if (args.contains(Keys::DecoderMethod))
    config[Keys::DecoderMethod] = args.value(Keys::DecoderMethod).toInt();

  if (args.contains(Keys::HexadecimalDelimiters))
    config[Keys::HexadecimalDelimiters] = args.value(Keys::HexadecimalDelimiters).toBool();

  if (args.contains(Keys::ChecksumAlgorithm))
    config[Keys::ChecksumAlgorithm] = args.value(Keys::ChecksumAlgorithm).toString();

  if (!config.isEmpty() && args.contains(Keys::SourceId))
    config[Keys::SourceId] = args.value(Keys::SourceId).toInt();

  return config;
}

/**
 * @brief Applies a frame-parser script, forwarding optional sourceId to the API. Frame
 *        config args (delimiters, detection, decoder, checksum) are persisted to the source
 *        via project.frameParser.update -- the dry run alone never changes the project, and
 *        silently dropping them here shipped a model-visible trap once (2026-07-14).
 */
static QJsonObject applyFrameParserScript(const QJsonObject& args,
                                          const QJsonObject& dryRun,
                                          int language)
{
  QJsonObject applyArgs;
  applyArgs[QStringLiteral("code")]     = args.value(QStringLiteral("code")).toString();
  applyArgs[QStringLiteral("language")] = language;
  if (args.contains(Keys::SourceId))
    applyArgs[Keys::SourceId] = args.value(Keys::SourceId).toInt();

  const QString command                = QStringLiteral("project.frameParser.setCode");
  auto applyReply                      = runCommand(command, applyArgs);
  applyReply[QStringLiteral("dryRun")] = dryRun;

  const auto configArgs = frameConfigArgsFor(args);
  if (applyReply.value(QStringLiteral("ok")).toBool() && !configArgs.isEmpty()) {
    const auto configReply = runCommand(QStringLiteral("project.frameParser.update"), configArgs);
    applyReply[QStringLiteral("frameConfig")] = configReply;
    if (!configReply.value(QStringLiteral("ok")).toBool()) {
      applyReply[QStringLiteral("ok")]    = false;
      applyReply[QStringLiteral("error")] = QJsonObject{
        {   QStringLiteral("code"),QStringLiteral("frame_config_failed")                    },
        {QStringLiteral("message"),
         QStringLiteral("Parser code applied, but persisting the frame detection config "
         "failed; see frameConfig for the underlying error.")}
      };
    }
  }

  return attachRepairHint(applyReply, command);
}

/**
 * @brief Applies a painter script, resolving the target group from the request.
 */
static QJsonObject applyPainterScript(const QJsonObject& args, const QJsonObject& dryRun)
{
  auto groupReply = resolveGroup(args);
  if (!groupReply.value(QStringLiteral("ok")).toBool())
    return groupReply;

  QJsonObject applyArgs;
  applyArgs[QStringLiteral("code")] = args.value(QStringLiteral("code")).toString();
  applyArgs[QStringLiteral("groupId")] =
    groupReply.value(QStringLiteral("group")).toObject().value(QStringLiteral("groupId")).toInt();

  const QString command                = QStringLiteral("project.painter.setCode");
  auto applyReply                      = runCommand(command, applyArgs);
  applyReply[QStringLiteral("dryRun")] = dryRun;
  return attachRepairHint(applyReply, command);
}

/**
 * @brief Validates a script with dryRun, then routes the apply to the right project mutation.
 */
QJsonObject executeScriptApply(const QJsonObject& args)
{
  const auto kind = args.value(QStringLiteral("kind")).toString();
  const int language =
    args.contains(QStringLiteral("language")) ? args.value(QStringLiteral("language")).toInt() : 1;

  QJsonObject dryRun = executeScriptDryRun(args);
  if (!dryRunResultOk(dryRun)) {
    QJsonObject out;
    out[QStringLiteral("ok")]     = false;
    out[QStringLiteral("error")]  = QStringLiteral("dry_run_failed");
    out[QStringLiteral("dryRun")] = dryRun;
    out[QStringLiteral("repair")] =
      QStringLiteral("Fix the script using the returned dry-run error before applying.");
    return out;
  }

  if (kind == QStringLiteral("frame_parser"))
    return applyFrameParserScript(args, dryRun, language);

  if (kind == QStringLiteral("transform"))
    return applyTransformScript(args, dryRun, language);

  if (kind == QStringLiteral("painter"))
    return applyPainterScript(args, dryRun);

  QJsonObject out;
  out[QStringLiteral("ok")]    = false;
  out[QStringLiteral("error")] = QStringLiteral("unsupported_apply_kind");
  out[QStringLiteral("hint")] =
    kind == QStringLiteral("output_widget")
      ? QStringLiteral("output_widget is dry-run only here: validate with "
                       "assistant.script.dryRun{kind:'output_widget'}, then apply via "
                       "project.outputWidget.update{groupId, widgetId, transmitFunction}.")
      : QStringLiteral("assistant.script.apply supports frame_parser, transform, and painter.");
  return out;
}

}  // namespace AI::ToolDetail
