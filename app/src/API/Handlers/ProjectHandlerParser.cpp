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

#include <algorithm>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <QFile>
#include <QHash>
#include <QJSEngine>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

#include "API/CommandRegistry.h"
#include "API/EnumLabels.h"
#include "API/Handlers/ProjectHandler.h"
#include "API/PathPolicy.h"
#include "API/SchemaBuilder.h"
#include "AppState.h"
#include "DataModel/Frame.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/CFrameParser.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/FrameParserPipeline.h"
#include "DataModel/Scripting/JsScriptEngine.h"
#include "DataModel/Scripting/LuaScriptEngine.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "IO/ConnectionManager.h"
#include "Misc/BackupManager.h"
#include "SerialStudio.h"
#include "SSAssert.h"
#ifdef BUILD_COMMERCIAL
#  include "UI/Widgets/Output/Base.h"
#endif
#include "ProjectApiSupport.h"

namespace API::Handlers {

/**
 * @brief Builds the likely-cause suffix for a frame-parser compile failure (legacy two-parameter
 *        parse signature or a language/syntax mismatch); returns an empty string when neither
 *        heuristic fires.
 */
[[nodiscard]] static QString frameParserCompileHint(const QString& code, int language)
{
  static const QRegularExpression kTwoArgParse(QStringLiteral(
    R"(\bparse\b\s*(?:=\s*)?(?:function)?\s*\(\s*[a-zA-Z_$][\w$]*\s*,\s*[a-zA-Z_$][\w$]*\s*\))"));
  if (kTwoArgParse.match(code).hasMatch())
    return QStringLiteral(" Likely cause: the code defines the legacy two-parameter "
                          "parse(frame, separator) signature, which is rejected. Define "
                          "parse(frame) with a single parameter and split the frame yourself.");

  const auto mismatch = detectLanguageMismatch(code, language);
  if (!mismatch.isEmpty())
    return QStringLiteral(" Likely cause: ") + mismatch;

  return QString();
}

/**
 * @brief Validates a native template + params pair against the registry.
 */
static bool validateNativeTemplate(const QString& templateId,
                                   const QJsonObject& templateParams,
                                   QString& error)
{
  const auto* tmpl = DataModel::nativeTemplateById(templateId);
  if (!tmpl) {
    error = QStringLiteral("Unknown native parser template: \"%1\" (see "
                           "project.frameParser.listTemplates)")
              .arg(templateId);
    return false;
  }

  const auto parser = tmpl->makeParser(templateParams, error);
  return parser != nullptr;
}

/**
 * @brief Persists a validated native template config and flips the source to Native.
 */
static void applyNativeTemplate(int sourceId,
                                const QString& templateId,
                                const QJsonObject& templateParams)
{
  SS_ASSERT(sourceId >= 0, return);
  SS_ASSERT_LOG(!templateId.isEmpty());

  const auto* tmpl = DataModel::nativeTemplateById(templateId);
  if (!tmpl)
    return;

  const auto params =
    templateParams.isEmpty() ? DataModel::nativeTemplateDefaults(*tmpl) : templateParams;

  static auto& model = DataModel::ProjectModel::instance();
  model.updateSourceFrameParserLanguage(sourceId, SerialStudio::Native);
  model.updateSourceFrameParserParams(sourceId, params);
  model.updateSourceFrameParserTemplate(sourceId, templateId);
}

/**
 * @brief Handles parserSetCode for the Native language: code carries the JSON descriptor.
 */
static API::CommandResponse setNativeParserFromDescriptor(const QString& id,
                                                          int sourceId,
                                                          const QString& code)
{
  const auto doc = QJsonDocument::fromJson(code.toUtf8());
  if (!doc.isObject())
    return API::CommandResponse::makeError(
      id,
      API::ErrorCode::InvalidParam,
      QStringLiteral("Built-In parser code must be the JSON descriptor "
                     "{\"template\": id, \"params\": {...}}"));

  const auto descriptor      = doc.object();
  const QString template_id  = descriptor.value(QStringLiteral("template")).toString();
  const auto template_params = descriptor.value(QStringLiteral("params")).toObject();

  QString error;
  if (!validateNativeTemplate(template_id, template_params, error))
    return API::CommandResponse::makeError(id, API::ErrorCode::InvalidParam, error);

  applyNativeTemplate(sourceId, template_id, template_params);

  QJsonObject result;
  result[Keys::SourceId]             = sourceId;
  result[QStringLiteral("language")] = static_cast<int>(SerialStudio::Native);
  result[QStringLiteral("template")] = template_id;
  static auto& model                 = DataModel::ProjectModel::instance();
  result[QStringLiteral("params")]   = model.frameParserParams(sourceId);
  return API::CommandResponse::makeSuccess(id, result);
}

}  // namespace API::Handlers

/**
 * @brief Set frame parser code for a source.
 */
API::CommandResponse API::Handlers::ProjectHandler::parserSetCode(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("code"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));
  }

  const QString code = params.value(QStringLiteral("code")).toString();
  const int sourceId = params.contains(Keys::SourceId) ? params.value(Keys::SourceId).toInt() : 0;
  static auto& model = DataModel::ProjectModel::instance();
  const int srcCount = static_cast<int>(model.sources().size());

  if (sourceId < 0 || sourceId >= srcCount)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid sourceId"));

  const bool hasLanguage = params.contains(QStringLiteral("language"));
  int savedLanguage      = 0;
  if (hasLanguage) {
    const int language = params.value(QStringLiteral("language")).toInt();
    if (language != SerialStudio::JavaScript && language != SerialStudio::Lua
        && language != SerialStudio::Native)
      return CommandResponse::makeError(
        id,
        ErrorCode::InvalidParam,
        QStringLiteral("Invalid language: must be 0 (JavaScript), 1 (Lua) or 2 (Built-In)"));

    if (language == SerialStudio::Native)
      return setNativeParserFromDescriptor(id, sourceId, code);

    savedLanguage = model.frameParserLanguage(sourceId);

    model.updateSourceFrameParserLanguage(sourceId, language);

    static auto& parser     = DataModel::FrameParser::instance();
    const bool prevSuppress = model.suppressMessageBoxes();
    model.setSuppressMessageBoxes(true);
    parser.setSuppressMessageBoxes(true);

    const bool ok = parser.loadScript(sourceId, code, false);

    parser.setSuppressMessageBoxes(prevSuppress);
    model.setSuppressMessageBoxes(prevSuppress);

    if (!ok) {
      model.updateSourceFrameParserLanguage(sourceId, savedLanguage);
      return CommandResponse::makeError(
        id,
        ErrorCode::InvalidParam,
        QStringLiteral("Script engine rejected the parser code (check logs)"));
    }
  }

  if (sourceId == 0)
    model.setFrameParserCode(code);
  else
    model.updateSourceFrameParser(sourceId, code);

  const int effectiveLanguage = model.frameParserLanguage(sourceId);

  QJsonObject result;
  result[Keys::SourceId]               = sourceId;
  result[QStringLiteral("codeLength")] = code.length();
  result[QStringLiteral("language")]   = effectiveLanguage;

  if (!code.isEmpty()) {
    const auto warning = detectLanguageMismatch(code, effectiveLanguage);
    if (!warning.isEmpty())
      result[QStringLiteral("warning")] = warning;
  }

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get frame parser code for a source.
 */
API::CommandResponse API::Handlers::ProjectHandler::parserGetCode(const QString& id,
                                                                  const QJsonObject& params)
{
  const int sourceId = params.contains(Keys::SourceId) ? params.value(Keys::SourceId).toInt() : 0;
  static auto& model = DataModel::ProjectModel::instance();
  const int srcCount = static_cast<int>(model.sources().size());

  if (sourceId < 0 || sourceId >= srcCount)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid sourceId"));

  QString code =
    sourceId == 0 ? model.frameParserCode() : model.sources()[sourceId].frameParserCode;

  QJsonObject result;
  result[Keys::SourceId]             = sourceId;
  result[QStringLiteral("language")] = model.frameParserLanguage(sourceId);

  if (model.frameParserLanguage(sourceId) == SerialStudio::Native) {
    QString template_id = model.frameParserTemplate(sourceId);
    if (template_id.isEmpty())
      template_id = DataModel::defaultNativeTemplateId();

    QJsonObject template_params = model.frameParserParams(sourceId);
    if (template_params.isEmpty()) {
      if (const auto* tmpl = DataModel::nativeTemplateById(template_id))
        template_params = DataModel::nativeTemplateDefaults(*tmpl);
    }

    code = DataModel::CFrameParser::buildDescriptor(template_id, template_params);
    result[QStringLiteral("template")] = template_id;
    result[QStringLiteral("params")]   = template_params;
  }

  result[QStringLiteral("code")]       = code;
  result[QStringLiteral("codeLength")] = code.length();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the scripting language for a frame parser source.
 */
API::CommandResponse API::Handlers::ProjectHandler::parserSetLanguage(const QString& id,
                                                                      const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("language")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: language"));

  const int sourceId = params.contains(Keys::SourceId) ? params.value(Keys::SourceId).toInt() : 0;

  const int language = params.value(QStringLiteral("language")).toInt();
  if (language != SerialStudio::JavaScript && language != SerialStudio::Lua
      && language != SerialStudio::Native)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid language: must be 0 (JavaScript), 1 (Lua) or 2 (Built-In)"));

  static auto& model  = DataModel::ProjectModel::instance();
  const auto& sources = model.sources();
  const auto it =
    std::find_if(sources.begin(), sources.end(), [sourceId](const DataModel::Source& s) {
      return s.sourceId == sourceId;
    });

  if (it == sources.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Unknown sourceId"));

  model.updateSourceFrameParserLanguage(sourceId, language);

  if (language != SerialStudio::Native || model.frameParserTemplate(sourceId).isEmpty()) {
    static auto& parser = DataModel::FrameParser::instance();
    parser.loadDefaultTemplate(sourceId, true);
  } else {
    static auto& parser = DataModel::FrameParser::instance();
    parser.readCode();
  }

  QJsonObject result;
  result[Keys::SourceId]             = sourceId;
  result[QStringLiteral("language")] = language;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get the scripting language for a frame parser source.
 */
API::CommandResponse API::Handlers::ProjectHandler::parserGetLanguage(const QString& id,
                                                                      const QJsonObject& params)
{
  const int sourceId = params.contains(Keys::SourceId) ? params.value(Keys::SourceId).toInt() : 0;

  static auto& model  = DataModel::ProjectModel::instance();
  const auto& sources = model.sources();
  const auto it =
    std::find_if(sources.begin(), sources.end(), [sourceId](const DataModel::Source& s) {
      return s.sourceId == sourceId;
    });

  if (it == sources.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Unknown sourceId"));

  QJsonObject result;
  result[Keys::SourceId]             = sourceId;
  result[QStringLiteral("language")] = it->frameParserLanguage;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief List the available Native frame parser templates.
 */
API::CommandResponse API::Handlers::ProjectHandler::parserListTemplates(const QString& id,
                                                                        const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto catalog = DataModel::CFrameParser::templateCatalog();

  QJsonObject result;
  result[QStringLiteral("templates")] = catalog;
  result[QStringLiteral("count")]     = catalog.size();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get the parameter schema of a Native frame parser template.
 */
API::CommandResponse API::Handlers::ProjectHandler::parserGetTemplateSchema(
  const QString& id, const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("template")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: template"));

  const QString template_id = params.value(QStringLiteral("template")).toString();
  const auto schema         = DataModel::CFrameParser::templateSchema(template_id);
  if (schema.isEmpty())
    return CommandResponse::makeError(id,
                                      ErrorCode::InvalidParam,
                                      QStringLiteral("Unknown native parser template: \"%1\" (see "
                                                     "project.frameParser.listTemplates)")
                                        .arg(template_id));

  return CommandResponse::makeSuccess(id, schema);
}

/**
 * @brief Get the Native frame parser configuration for a source.
 */
API::CommandResponse API::Handlers::ProjectHandler::parserGetTemplate(const QString& id,
                                                                      const QJsonObject& params)
{
  const int sourceId  = params.contains(Keys::SourceId) ? params.value(Keys::SourceId).toInt() : 0;
  static auto& model  = DataModel::ProjectModel::instance();
  const auto& sources = model.sources();
  const auto it =
    std::find_if(sources.begin(), sources.end(), [sourceId](const DataModel::Source& s) {
      return s.sourceId == sourceId;
    });

  if (it == sources.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Unknown sourceId"));

  QJsonObject result;
  result[Keys::SourceId]             = sourceId;
  result[QStringLiteral("language")] = it->frameParserLanguage;
  result[QStringLiteral("template")] = it->frameParserTemplate;
  result[QStringLiteral("params")]   = it->frameParserParams;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Select a Native frame parser template (and flip the source to Native).
 */
API::CommandResponse API::Handlers::ProjectHandler::parserSetTemplate(const QString& id,
                                                                      const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("template")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: template"));

  const int sourceId  = params.contains(Keys::SourceId) ? params.value(Keys::SourceId).toInt() : 0;
  static auto& model  = DataModel::ProjectModel::instance();
  const auto& sources = model.sources();
  const auto it =
    std::find_if(sources.begin(), sources.end(), [sourceId](const DataModel::Source& s) {
      return s.sourceId == sourceId;
    });

  if (it == sources.end())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Unknown sourceId"));

  const QString template_id  = params.value(QStringLiteral("template")).toString();
  const auto template_params = params.value(QStringLiteral("params")).toObject();

  QString error;
  if (!validateNativeTemplate(template_id, template_params, error))
    return CommandResponse::makeError(id, ErrorCode::InvalidParam, error);

  applyNativeTemplate(sourceId, template_id, template_params);

  QJsonObject result;
  result[Keys::SourceId]             = sourceId;
  result[QStringLiteral("language")] = static_cast<int>(SerialStudio::Native);
  result[QStringLiteral("template")] = template_id;
  result[QStringLiteral("params")]   = model.frameParserParams(sourceId);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Applies frame extraction params to the primary source: live manager + model in
 *        lockstep so the running reader and the persisted project stay consistent.
 */
static bool configurePrimarySourceFrame(const QJsonObject& params)
{
  static auto& model   = DataModel::ProjectModel::instance();
  static auto& manager = IO::ConnectionManager::instance();
  bool updated         = false;

  if (params.contains(QStringLiteral("startSequence"))) {
    const QString start = params.value(QStringLiteral("startSequence")).toString();
    manager.setStartSequence(start.toUtf8());
    model.setFrameStartSequence(start);
    updated = true;
  }

  if (params.contains(QStringLiteral("endSequence"))) {
    const QString end = params.value(QStringLiteral("endSequence")).toString();
    manager.setFinishSequence(end.toUtf8());
    model.setFrameEndSequence(end);
    updated = true;
  }

  if (params.contains(Keys::ChecksumAlgorithm)) {
    const QString checksumName = params.value(Keys::ChecksumAlgorithm).toString();
    manager.setChecksumAlgorithm(checksumName);
    model.setChecksumAlgorithm(checksumName);
    updated = true;
  }

  if (params.contains(Keys::FrameDetection)) {
    const int detectionIdx = params.value(Keys::FrameDetection).toInt();
    if (detectionIdx >= 0 && detectionIdx <= 3) {
      model.setFrameDetection(static_cast<SerialStudio::FrameDetection>(detectionIdx));
      updated = true;
    }
  }

  if (params.contains(Keys::DecoderMethod)) {
    const int decoderIdx = params.value(Keys::DecoderMethod).toInt();
    if (decoderIdx >= 0 && decoderIdx <= 3) {
      model.setDecoderMethod(static_cast<SerialStudio::DecoderMethod>(decoderIdx));
      updated = true;
    }
  }

  if (params.contains(Keys::HexadecimalDelimiters)) {
    model.setHexadecimalDelimiters(params.value(Keys::HexadecimalDelimiters).toBool());
    updated = true;
  }

  return updated;
}

/**
 * @brief Applies frame extraction params to a secondary source by patching its Source row;
 *        commits via updateSource only when something actually changed.
 */
static bool configureSecondarySourceFrame(const QJsonObject& params, int sourceId)
{
  static auto& model    = DataModel::ProjectModel::instance();
  DataModel::Source src = model.sources()[sourceId];
  bool updated          = false;

  if (params.contains(QStringLiteral("startSequence"))) {
    src.frameStart = params.value(QStringLiteral("startSequence")).toString();
    updated        = true;
  }

  if (params.contains(QStringLiteral("endSequence"))) {
    src.frameEnd = params.value(QStringLiteral("endSequence")).toString();
    updated      = true;
  }

  if (params.contains(Keys::ChecksumAlgorithm)) {
    src.checksumAlgorithm = params.value(Keys::ChecksumAlgorithm).toString();
    updated               = true;
  }

  if (params.contains(Keys::FrameDetection)) {
    const int detectionIdx = params.value(Keys::FrameDetection).toInt();
    if (detectionIdx >= 0 && detectionIdx <= 3) {
      src.frameDetection = detectionIdx;
      updated            = true;
    }
  }

  if (params.contains(Keys::DecoderMethod)) {
    const int decoderIdx = params.value(Keys::DecoderMethod).toInt();
    if (decoderIdx >= 0 && decoderIdx <= 3) {
      src.decoderMethod = decoderIdx;
      updated           = true;
    }
  }

  if (params.contains(Keys::HexadecimalDelimiters)) {
    src.hexadecimalDelimiters = params.value(Keys::HexadecimalDelimiters).toBool();
    updated                   = true;
  }

  if (updated)
    model.updateSource(sourceId, src);

  return updated;
}

/**
 * @brief Configure frame parser settings for a specific source.
 */
API::CommandResponse API::Handlers::ProjectHandler::frameParserConfigure(const QString& id,
                                                                         const QJsonObject& params)
{
  static auto& model   = DataModel::ProjectModel::instance();
  static auto& manager = IO::ConnectionManager::instance();
  bool updated         = false;

  const int sourceId = params.contains(Keys::SourceId) ? params.value(Keys::SourceId).toInt() : 0;
  const int srcCount = static_cast<int>(model.sources().size());

  if (sourceId < 0 || (!model.sources().empty() && sourceId >= srcCount))
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid sourceId"));

  if (params.contains(QStringLiteral("operationMode"))) {
    const int modeIdx = params.value(QStringLiteral("operationMode")).toInt();
    if (modeIdx >= 0 && modeIdx <= 2) {
      static auto& appState = AppState::instance();
      appState.setOperationMode(static_cast<SerialStudio::OperationMode>(modeIdx));
      updated = true;
    }
  }

  if (sourceId == 0)
    updated = configurePrimarySourceFrame(params) || updated;
  else
    updated = configureSecondarySourceFrame(params, sourceId) || updated;

  if (updated && sourceId == 0)
    manager.resetFrameReader();

  QJsonObject result;
  result[QStringLiteral("updated")] = updated;
  result[Keys::SourceId]            = sourceId;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get current frame parser configuration
 */
API::CommandResponse API::Handlers::ProjectHandler::frameParserGetConfig(const QString& id,
                                                                         const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& appState = AppState::instance();
  const auto& cfg       = appState.frameConfig();

  QJsonArray startArr, endArr;
  for (const auto& s : cfg.startSequences)
    startArr.append(QString::fromUtf8(s));

  for (const auto& f : cfg.finishSequences)
    endArr.append(QString::fromUtf8(f));

  const QString primaryStart =
    cfg.startSequences.isEmpty() ? QString() : QString::fromUtf8(cfg.startSequences.first());
  const QString primaryEnd =
    cfg.finishSequences.isEmpty() ? QString() : QString::fromUtf8(cfg.finishSequences.first());

  QJsonObject result;
  result[QStringLiteral("startSequence")]  = primaryStart;
  result[QStringLiteral("endSequence")]    = primaryEnd;
  result[QStringLiteral("startSequences")] = startArr;
  result[QStringLiteral("endSequences")]   = endArr;
  result[Keys::ChecksumAlgorithm]          = cfg.checksumAlgorithm;
  result[QStringLiteral("operationMode")]  = static_cast<int>(cfg.operationMode);
  result[Keys::FrameDetection]             = static_cast<int>(cfg.frameDetection);

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set painter code for a specific group by id.
 */
API::CommandResponse API::Handlers::ProjectHandler::painterSetCode(const QString& id,
                                                                   const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  if (!params.contains(QStringLiteral("code")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));

  const int groupId    = params.value(QStringLiteral("groupId")).toInt();
  const QString code   = params.value(QStringLiteral("code")).toString();
  static auto& project = DataModel::ProjectModel::instance();
  const auto& groups   = project.groups();

  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  DataModel::Group g = groups[groupId];
  g.painterCode      = code;
  project.updateGroup(groupId, g, true);

  QJsonObject result;
  result[QStringLiteral("groupId")]    = groupId;
  result[QStringLiteral("codeLength")] = code.size();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Read the painter code for a group.
 */
API::CommandResponse API::Handlers::ProjectHandler::painterGetCode(const QString& id,
                                                                   const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));

  const int groupId  = params.value(QStringLiteral("groupId")).toInt();
  static auto& model = DataModel::ProjectModel::instance();
  const auto& groups = model.groups();
  if (groupId < 0 || static_cast<size_t>(groupId) >= groups.size())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Group id not found: %1").arg(groupId));

  QJsonObject result;
  result[QStringLiteral("groupId")] = groupId;
  result[QStringLiteral("code")]    = groups[groupId].painterCode;
  return CommandResponse::makeSuccess(id, result);
}

namespace API::Handlers {

//--------------------------------------------------------------------------------------------------
// Script dry-run helpers (compile + run in throwaway engines, never touch project)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the right script engine for a language tag (0=JS, 1=Lua, 2=Native).
 */
static std::unique_ptr<DataModel::IScriptEngine> makeScriptEngine(int language)
{
  if (language == SerialStudio::Native)
    return std::make_unique<DataModel::CFrameParser>();

  if (language == SerialStudio::Lua)
    return std::make_unique<DataModel::LuaScriptEngine>();

  return std::make_unique<DataModel::JsScriptEngine>();
}

/**
 * @brief Parses a delimiter field (frameStart / frameEnd) honoring the hexadecimalDelimiters flag.
 */
static QByteArray dryRunDelimiter(const QJsonObject& params, const QString& key, bool hex)
{
  const auto raw = params.value(key).toString();
  if (raw.isEmpty())
    return {};

  if (hex) {
    const auto resolved = SerialStudio::resolveEscapeSequences(raw);
    return QByteArray::fromHex(QString(resolved).remove(' ').toUtf8());
  }

  return SerialStudio::resolveEscapeSequences(raw).toUtf8();
}

/**
 * @brief Decodes the caller-supplied raw stream bytes; non-empty inputBytesHex wins, an empty
 *        string in either field counts as absent so the other field can still supply the data.
 */
static QByteArray dryRunInputBytes(const QJsonObject& params)
{
  const auto hex = params.value(QStringLiteral("inputBytesHex")).toString();
  if (!hex.trimmed().isEmpty())
    return SerialStudio::hexToBytes(hex);

  return params.value(QStringLiteral("inputBytes")).toString().toUtf8();
}

/**
 * @brief Serializes a single pipeline frame into the dryRun response shape.
 */
static QJsonObject dryRunSerializeFrame(const DataModel::PipelineFrame& frame)
{
  QJsonArray rows;
  for (const auto& row : frame.rows) {
    QJsonArray cells;
    for (const auto& cell : row)
      cells.append(cell);

    rows.append(cells);
  }

  QJsonObject obj;
  obj[QStringLiteral("rawHex")]          = QString::fromLatin1(frame.rawBytes.toHex(' '));
  obj[QStringLiteral("rawByteCount")]    = static_cast<int>(frame.rawBytes.size());
  obj[QStringLiteral("decoderOutput")]   = frame.decoderOutput;
  obj[QStringLiteral("decoderIsBinary")] = frame.decoderProducedBinary;
  obj[QStringLiteral("rows")]            = rows;
  obj[QStringLiteral("rowCount")]        = rows.size();
  return obj;
}

}  // namespace API::Handlers

/**
 * @brief Frame parser dry-run: drives extraction + decoder + parser against caller bytes.
 */
API::CommandResponse API::Handlers::ProjectHandler::frameParserDryRun(const QString& id,
                                                                      const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("code")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));

  if (!params.contains(QStringLiteral("language")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: language"));

  if (!params.contains(QStringLiteral("inputBytes"))
      && !params.contains(QStringLiteral("inputBytesHex")))
    return CommandResponse::makeError(
      id,
      ErrorCode::MissingParam,
      QStringLiteral("Missing required parameter: inputBytesHex (preferred, binary-safe) or "
                     "inputBytes (UTF-8 text)."));

  const auto bytes = dryRunInputBytes(params);
  if (bytes.isEmpty())
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("inputBytes / inputBytesHex were provided but decoded to zero bytes. "
                     "Pass the raw stream in exactly one of them: inputBytesHex as hex digits "
                     "(e.g. '61 2C 62') or inputBytes as UTF-8 text. Empty strings count as "
                     "absent."));

  const auto code     = params.value(QStringLiteral("code")).toString();
  const auto language = params.value(QStringLiteral("language")).toInt();

  DataModel::PipelineSpec spec;
  spec.operationMode = static_cast<SerialStudio::OperationMode>(
    params.value(QStringLiteral("operationMode")).toInt(int(SerialStudio::ProjectFile)));
  spec.frameDetection = static_cast<SerialStudio::FrameDetection>(
    params.value(Keys::FrameDetection).toInt(int(SerialStudio::EndDelimiterOnly)));
  spec.decoderMethod = static_cast<SerialStudio::DecoderMethod>(
    params.value(Keys::DecoderMethod).toInt(int(SerialStudio::PlainText)));
  spec.checksumAlgorithm = params.value(Keys::ChecksumAlgorithm).toString();

  const bool hexDelims = params.value(Keys::HexadecimalDelimiters).toBool(false);
  const auto start     = dryRunDelimiter(params, QString(Keys::FrameStart), hexDelims);
  const auto end       = dryRunDelimiter(params, QString(Keys::FrameEnd), hexDelims);
  if (!start.isEmpty())
    spec.startSequences.append(start);

  if (!end.isEmpty())
    spec.finishSequences.append(end);

  if (spec.operationMode == SerialStudio::QuickPlot && spec.finishSequences.isEmpty()) {
    spec.finishSequences = {QByteArray("\n"), QByteArray("\r\n"), QByteArray("\r")};
    spec.frameDetection  = SerialStudio::EndDelimiterOnly;
  }

  const auto run = DataModel::runFrameParserPipelineWithCode(bytes, spec, code, language);
  if (!run.stageError.isEmpty()) {
    QString message = run.stageError;
    if (run.stageWhere == QStringLiteral("compile"))
      message += frameParserCompileHint(code, language);

    return CommandResponse::makeError(id, ErrorCode::ExecutionError, message);
  }

  QJsonArray frameResults;
  int totalRows = 0;
  for (const auto& f : run.frames) {
    frameResults.append(dryRunSerializeFrame(f));
    totalRows += f.rows.size();
  }

  QJsonObject result;
  result[QStringLiteral("ok")]             = true;
  result[QStringLiteral("frames")]         = frameResults;
  result[QStringLiteral("frameCount")]     = frameResults.size();
  result[QStringLiteral("extractedCount")] = run.extractedCount;
  result[QStringLiteral("consumedBytes")]  = static_cast<int>(run.consumedBytes);
  result[QStringLiteral("remainingBytes")] = static_cast<int>(run.remainingBytes);
  result[QStringLiteral("droppedFrames")]  = static_cast<qint64>(run.droppedFrames);
  result[QStringLiteral("totalRows")]      = totalRows;
  result[QStringLiteral("hint")]           = QStringLiteral(
    "Bytes flow through extraction (delimiters / detection) -> decoder method -> parser, the "
              "same path the live FrameBuilder uses. Pick the Binary decoder for non-text streams "
              "(COBS, Modbus, custom binary) -- PlainText / Hex / Base64 route through "
              "QString::fromUtf8 and mojibake non-ASCII bytes.");

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Compile-only check for a frame parser; surfaces syntax errors without running.
 */
API::CommandResponse API::Handlers::ProjectHandler::frameParserDryCompile(const QString& id,
                                                                          const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("code")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));

  if (!params.contains(QStringLiteral("language")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: language"));

  const auto code     = params.value(QStringLiteral("code")).toString();
  const auto language = params.value(QStringLiteral("language")).toInt();

  if (language != SerialStudio::JavaScript && language != SerialStudio::Lua
      && language != SerialStudio::Native)
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid language: must be 0 (JavaScript), 1 (Lua) or 2 (Built-In)"));

  auto engine   = makeScriptEngine(language);
  const bool ok = engine->loadScript(code, 0, false);

  QJsonObject result;
  result[QStringLiteral("ok")] = ok;
  if (language == SerialStudio::Native)
    result[QStringLiteral("language")] = QStringLiteral("native");
  else
    result[QStringLiteral("language")] =
      (language == SerialStudio::Lua ? QStringLiteral("lua") : QStringLiteral("javascript"));

  if (!ok) {
    if (language == SerialStudio::Native)
      result[QStringLiteral("error")] = QStringLiteral(
        "Invalid native parser descriptor: expected {\"template\": id, \"params\": {...}} "
        "with a known template id and valid params.");
    else
      result[QStringLiteral("error")] =
        QStringLiteral("Compile failed or parse(frame) is not defined.");

    if (language != SerialStudio::Native) {
      const auto warning = detectLanguageMismatch(code, language);
      if (!warning.isEmpty())
        result[QStringLiteral("warning")] = warning;
    }
  }

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Transform dry-run: compile + apply transform() to a list of values.
 */
API::CommandResponse API::Handlers::ProjectHandler::transformDryRun(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("code")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));

  if (!params.contains(QStringLiteral("language")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: language"));

  if (!params.contains(QStringLiteral("values")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: values"));

  const auto code     = params.value(QStringLiteral("code")).toString();
  const auto language = params.value(QStringLiteral("language")).toInt();
  const auto values   = params.value(QStringLiteral("values")).toArray();

  QString wrapped;
  if (language == 1) {
    wrapped = code
            + QStringLiteral("\n\nfunction parse(frame)\n"
                             "  local v = tonumber(frame)\n"
                             "  if v == nil then v = frame end\n"
                             "  local out = transform(v)\n"
                             "  return { tostring(out) }\n"
                             "end\n");
  } else {
    wrapped = code
            + QStringLiteral("\n\nfunction parse(frame) {\n"
                             "  var v = parseFloat(frame);\n"
                             "  if (isNaN(v)) v = frame;\n"
                             "  return [String(transform(v))];\n"
                             "}\n");
  }

  auto engine = makeScriptEngine(language);
  if (!engine->loadScript(wrapped, 0, false))
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("Transform failed to compile or define transform(value)"));

  QJsonArray outputs;
  for (const auto& v : values) {
    QString sample;
    if (v.isDouble())
      sample = QString::number(SerialStudio::toDouble(v), 'g', 17);
    else
      sample = v.toString();

    const auto rows = engine->parseString(sample);
    if (rows.isEmpty() || rows.first().isEmpty()) {
      outputs.append(QJsonValue::Null);
      continue;
    }

    const auto cell = rows.first().first();
    bool isNum      = false;
    const auto num  = SerialStudio::toDouble(cell, &isNum);
    if (isNum)
      outputs.append(num);
    else
      outputs.append(cell);
  }

  QJsonObject result;
  result[QStringLiteral("ok")]      = true;
  result[QStringLiteral("outputs")] = outputs;
  result[QStringLiteral("hint")] =
    QStringLiteral("outputs[i] is the result of transform(values[i]). null means transform "
                   "returned a non-finite value -- the live runtime falls back to the raw "
                   "value in that case.");
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Painter dry-run: verify the script compiles and exposes paint().
 */
API::CommandResponse API::Handlers::ProjectHandler::painterDryRun(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("code")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));

  const auto code = params.value(QStringLiteral("code")).toString();

  QJSEngine engine;
  engine.installExtensions(QJSEngine::ConsoleExtension | QJSEngine::GarbageCollectionExtension);

  auto stub = engine.evaluate(
    QStringLiteral("var datasets = []; datasets.length = 0;"
                   "var group = { id: 0, title: '', columns: 0, sourceId: 0 };"
                   "var frame = { number: 0, timestampMs: 0 };"
                   "var theme = new Proxy({}, { get: function() { return '#000000'; } });"
                   "function tableGet() { return 0; }"
                   "function tableSet() {}"
                   "function datasetGetRaw() { return 0; }"
                   "function datasetGetFinal() { return 0; }"));
  if (stub.isError())
    return CommandResponse::makeError(id,
                                      ErrorCode::ExecutionError,
                                      QStringLiteral("Painter dry-run bootstrap failed: %1")
                                        .arg(stub.property(QStringLiteral("message")).toString()));

  const auto compiled = engine.evaluate(code, QStringLiteral("painter_dryrun.js"));
  if (compiled.isError()) {
    QJsonObject result;
    result[QStringLiteral("ok")] = false;
    result[QStringLiteral("compileError")] =
      compiled.property(QStringLiteral("message")).toString();
    result[QStringLiteral("line")] = compiled.property(QStringLiteral("lineNumber")).toInt();
    return CommandResponse::makeSuccess(id, result);
  }

  const auto paintFn = engine.globalObject().property(QStringLiteral("paint"));
  if (!paintFn.isCallable()) {
    QJsonObject result;
    result[QStringLiteral("ok")] = false;
    result[QStringLiteral("compileError")] =
      QStringLiteral("Script compiled but did not define paint(ctx, w, h). Canvas scripts "
                     "MUST define `function paint(ctx, w, h)`. The function is named "
                     "`paint`, not `draw` or `render`.");
    return CommandResponse::makeSuccess(id, result);
  }

  const auto onFrameFn = engine.globalObject().property(QStringLiteral("onFrame"));
  QJsonObject result;
  result[QStringLiteral("ok")]         = true;
  result[QStringLiteral("hasPaint")]   = true;
  result[QStringLiteral("hasOnFrame")] = onFrameFn.isCallable();
  result[QStringLiteral("hint")] =
    QStringLiteral("Compile + paint() lookup succeeded. Note: dry-run does NOT actually "
                   "render -- runtime errors inside paint() (out-of-bounds reads, missing "
                   "moveTo before arc, etc.) only surface when the live widget mounts.");
  return CommandResponse::makeSuccess(id, result);
}

namespace API::Handlers {

/**
 * @brief Runs a compiled transmit() against one sample value, reporting bytes or a runtime error.
 */
static QJsonObject runOutputWidgetSample(QJSEngine& engine,
                                         QJSValue& transmitFn,
                                         const QJsonValue& inputValue,
                                         bool hex)
{
  QJSValue jsValue;
  if (hex)
    jsValue =
      engine.toScriptValue(QString::fromLatin1(SerialStudio::hexToBytes(inputValue.toString())));
  else if (inputValue.isDouble())
    jsValue = engine.toScriptValue(SerialStudio::toDouble(inputValue));
  else {
    const auto text = inputValue.toString();
    bool numeric    = false;
    const auto num  = SerialStudio::toDouble(text, &numeric);
    jsValue         = numeric ? engine.toScriptValue(num) : engine.toScriptValue(text);
  }

  const auto called = transmitFn.call(QJSValueList{jsValue});
  QJsonObject out;
  if (called.isError()) {
    out[QStringLiteral("ok")]           = false;
    out[QStringLiteral("runtimeError")] = called.toString();
    out[QStringLiteral("line")]         = called.property(QStringLiteral("lineNumber")).toInt();
    return out;
  }

  const QByteArray payload =
    called.isString() ? called.toString().toLatin1() : called.toVariant().toByteArray();
  out[QStringLiteral("ok")]        = true;
  out[QStringLiteral("byteCount")] = payload.size();
  out[QStringLiteral("outputHex")] = QString::fromLatin1(payload.toHex(' ')).toUpper();
  return out;
}

}  // namespace API::Handlers

/**
 * @brief Compiles an output-widget transmit() in the live helper environment without applying.
 */
API::CommandResponse API::Handlers::ProjectHandler::outputWidgetDryRun(const QString& id,
                                                                       const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("code")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: code"));

  const auto code           = params.value(QStringLiteral("code")).toString();
  static auto& frameBuilder = DataModel::FrameBuilder::instance();
  frameBuilder.refreshTableStoreFromProjectModel();

  QJSEngine engine;
  engine.installExtensions(QJSEngine::ConsoleExtension | QJSEngine::GarbageCollectionExtension);
#ifdef BUILD_COMMERCIAL
  Widgets::Output::Base::installProtocolHelpers(engine);
#endif
  frameBuilder.injectTableApiJS(&engine);

  const auto wrapped =
    QStringLiteral("(function() { %1\n"
                   "return typeof transmit === 'function' ? transmit : undefined; })()")
      .arg(code);
  auto transmitFn = engine.evaluate(wrapped, QStringLiteral("output_widget_dryrun.js"));
  if (transmitFn.isError()) {
    QJsonObject result;
    result[QStringLiteral("ok")] = false;
    result[QStringLiteral("compileError")] =
      transmitFn.property(QStringLiteral("message")).toString();
    result[QStringLiteral("line")] = transmitFn.property(QStringLiteral("lineNumber")).toInt();
    return CommandResponse::makeSuccess(id, result);
  }

  if (!transmitFn.isCallable()) {
    QJsonObject result;
    result[QStringLiteral("ok")] = false;
    result[QStringLiteral("compileError")] =
      QStringLiteral("Script compiled but did not define transmit(value). Output-widget "
                     "transmit scripts MUST define `function transmit(value)` returning the "
                     "bytes to send (a Uint8Array, a byte array, or a string). The function "
                     "is named `transmit`, not `output` or `send`.");
    return CommandResponse::makeSuccess(id, result);
  }

  QJsonObject result;
  result[QStringLiteral("ok")]          = true;
  result[QStringLiteral("hasTransmit")] = true;
  if (params.contains(QStringLiteral("inputValue")))
    result[QStringLiteral("sampleRun")] =
      runOutputWidgetSample(engine,
                            transmitFn,
                            params.value(QStringLiteral("inputValue")),
                            params.value(QStringLiteral("hex")).toBool());
  else
    result[QStringLiteral("hint")] =
      QStringLiteral("Compiled and transmit(value) is defined. Pass inputValue (and hex:true "
                     "for hex byte input) to also execute it and see the produced bytes.");

  return CommandResponse::makeSuccess(id, result);
}

namespace API::Handlers {

/**
 * @brief Wraps a transform() script so it can be driven via IScriptEngine::parseString.
 */
static QString wrapTransformForParser(const QString& code, int language)
{
  if (language == 1)
    return code
         + QStringLiteral("\n\nfunction parse(frame)\n"
                          "  local v = tonumber(frame)\n"
                          "  if v == nil then v = frame end\n"
                          "  local out = transform(v)\n"
                          "  return { tostring(out) }\n"
                          "end\n");

  return code
       + QStringLiteral("\n\nfunction parse(frame) {\n"
                        "  var v = parseFloat(frame);\n"
                        "  if (isNaN(v)) v = frame;\n"
                        "  return [String(transform(v))];\n"
                        "}\n");
}

/**
 * @brief Applies a single dataset's transform to a raw cell value via a cached engine.
 */
static QJsonValue applyTransformForDryRun(
  const DataModel::Dataset& dataset,
  int defaultLanguage,
  const QString& rawCell,
  std::map<int, std::unique_ptr<DataModel::IScriptEngine>>& engines,
  std::map<int, bool>& engineOk)
{
  const int datasetKey = dataset.uniqueId;
  const int language =
    (dataset.transformLanguage == -1) ? defaultLanguage : dataset.transformLanguage;

  auto it = engines.find(datasetKey);
  if (it == engines.end()) {
    auto engine    = makeScriptEngine(language);
    const auto src = wrapTransformForParser(dataset.transformCode, language);
    const bool ok  = engine->loadScript(src, dataset.sourceId, false);

    engineOk[datasetKey] = ok;
    engines[datasetKey]  = std::move(engine);
    it                   = engines.find(datasetKey);
  }

  if (!engineOk[datasetKey])
    return QJsonValue::Null;

  const auto rows = it->second->parseString(rawCell);
  if (rows.isEmpty() || rows.first().isEmpty())
    return QJsonValue::Null;

  const auto cell = rows.first().first();
  bool isNum      = false;
  const auto num  = SerialStudio::toDouble(cell, &isNum);
  if (isNum)
    return num;

  return cell;
}

/**
 * @brief Build a single dataset entry for an endToEndDryRun row.
 */
static QJsonObject buildDryRunDatasetEntry(
  const DataModel::Dataset& dataset,
  int groupId,
  const QStringList& row,
  int language,
  bool verbose,
  std::map<int, std::unique_ptr<DataModel::IScriptEngine>>& transformEngines,
  std::map<int, bool>& transformEngineOk)
{
  const int idx = dataset.index;
  QString rawCell;
  if (idx >= 1 && idx <= row.size())
    rawCell = row.at(idx - 1);

  QJsonObject entry;
  entry[Keys::UniqueId]              = dataset.uniqueId;
  entry[Keys::Title]                 = dataset.title;
  entry[Keys::GroupId]               = groupId;
  entry[Keys::DatasetId]             = dataset.datasetId;
  entry[Keys::Index]                 = idx;
  entry[QStringLiteral("isVirtual")] = dataset.virtual_;

  if (verbose)
    entry[QStringLiteral("raw")] = rawCell;

  if (!dataset.transformCode.isEmpty()) {
    entry[QStringLiteral("final")] =
      applyTransformForDryRun(dataset, language, rawCell, transformEngines, transformEngineOk);
    entry[QStringLiteral("transformApplied")] = true;
    return entry;
  }

  bool isNum                                = false;
  const auto num                            = SerialStudio::toDouble(rawCell, &isNum);
  entry[QStringLiteral("final")]            = isNum ? QJsonValue(num) : QJsonValue(rawCell);
  entry[QStringLiteral("transformApplied")] = false;
  return entry;
}

/**
 * @brief Build a single parsed-row payload for an endToEndDryRun frame.
 */
static QJsonObject buildDryRunRow(
  const QStringList& row,
  int sourceId,
  const std::vector<DataModel::Group>& groups,
  int language,
  bool verbose,
  std::map<int, std::unique_ptr<DataModel::IScriptEngine>>& transformEngines,
  std::map<int, bool>& transformEngineOk)
{
  QJsonArray datasetResults;
  for (const auto& group : groups) {
    for (const auto& dataset : group.datasets) {
      if (dataset.sourceId != sourceId)
        continue;

      datasetResults.append(buildDryRunDatasetEntry(
        dataset, group.groupId, row, language, verbose, transformEngines, transformEngineOk));
    }
  }

  QJsonArray rawCells;
  for (const auto& cell : row)
    rawCells.append(cell);

  QJsonObject rowOut;
  rowOut[QStringLiteral("rawCells")] = rawCells;
  rowOut[QStringLiteral("datasets")] = datasetResults;
  return rowOut;
}

}  // namespace API::Handlers

/**
 * @brief End-to-end dry-run: parser + all dataset transforms applied to a sample frame.
 */
API::CommandResponse API::Handlers::ProjectHandler::endToEndDryRun(const QString& id,
                                                                   const QJsonObject& params)
{
  static auto& pm    = DataModel::ProjectModel::instance();
  const auto sources = pm.sources();
  const int sourceId = params.value(Keys::SourceId).toInt(0);
  if (sources.empty())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Project has no sources to dry-run against"));

  if (sourceId < 0 || sourceId >= static_cast<int>(sources.size()))
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Source id out of range: %1").arg(sourceId));

  const auto& source = sources[sourceId];

  const bool hasFrame  = params.contains(QStringLiteral("sampleFrame"));
  const bool hasFrames = params.contains(QStringLiteral("sampleFrames"));
  if (!hasFrame && !hasFrames)
    return CommandResponse::makeError(
      id,
      ErrorCode::MissingParam,
      QStringLiteral("Missing required parameter: sampleFrame (string) or sampleFrames (array)"));

  QStringList frames;
  if (hasFrames)
    for (const auto& v : params.value(QStringLiteral("sampleFrames")).toArray())
      frames.append(v.toString());
  else
    frames.append(params.value(QStringLiteral("sampleFrame")).toString());

  const bool verbose = params.value(QStringLiteral("verbose")).toBool(false);
  const QString code = params.contains(QStringLiteral("code"))
                       ? params.value(QStringLiteral("code")).toString()
                       : source.frameParserCode;
  const int language = params.contains(QStringLiteral("language"))
                       ? params.value(QStringLiteral("language")).toInt()
                       : source.frameParserLanguage;

  auto parser = makeScriptEngine(language);
  if (!parser->loadScript(code, sourceId, false))
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("Frame parser failed to compile or define parse(frame).")
        + frameParserCompileHint(code, language));

  std::map<int, std::unique_ptr<DataModel::IScriptEngine>> transformEngines;
  std::map<int, bool> transformEngineOk;
  const auto& groups = pm.groups();

  QJsonArray frameResults;
  for (const auto& sample : frames) {
    const auto parsed = parser->parseString(sample);

    QJsonArray rowResults;
    for (const auto& row : parsed)
      rowResults.append(buildDryRunRow(
        row, sourceId, groups, language, verbose, transformEngines, transformEngineOk));

    QJsonObject perFrame;
    perFrame[QStringLiteral("rows")]     = rowResults;
    perFrame[QStringLiteral("rowCount")] = rowResults.size();
    frameResults.append(perFrame);
  }

  QJsonArray failedTransforms;
  for (const auto& [uid, ok] : transformEngineOk)
    if (!ok)
      failedTransforms.append(uid);

  QJsonObject result;
  result[QStringLiteral("ok")]         = true;
  result[Keys::SourceId]               = sourceId;
  result[QStringLiteral("frames")]     = frameResults;
  result[QStringLiteral("frameCount")] = frameResults.size();

  if (!failedTransforms.isEmpty()) {
    result[QStringLiteral("transformCompileFailures")] = failedTransforms;
    result[QStringLiteral("warning")] =
      QStringLiteral("One or more dataset transforms failed to compile. Their `final` "
                     "values are null. Iterate the failing transforms via "
                     "project.dataset.transform.dryRun, then setTransformCode.");
  }

  result[QStringLiteral("hint")] =
    QStringLiteral("rawCells[i] maps to dataset.index = i+1. The table API "
                   "(tableGet/tableSet/datasetGetRaw/datasetGetFinal) is NOT injected in "
                   "this dry-run -- transforms that read other datasets will see 0/null. "
                   "Computed datasets show their transform applied to the raw cell at "
                   "their index (which is normally 0/unset for computed entries).");
  return CommandResponse::makeSuccess(id, result);
}
