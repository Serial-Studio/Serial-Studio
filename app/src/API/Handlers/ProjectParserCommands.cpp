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

#include "API/Handlers/ProjectParserCommands.h"

#include <algorithm>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "API/Handlers/ProjectApiSupport.h"
#include "API/SchemaBuilder.h"
#include "AppState.h"
#include "DataModel/Frame.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/CFrameParser.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "IO/ConnectionManager.h"
#include "SerialStudio.h"
#include "SSAssert.h"

using namespace API::Handlers::ProjectApiSupport;

namespace API::Handlers {

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

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the command class to the registry it publishes into.
 */
API::Handlers::ProjectParserCommands::ProjectParserCommands(CommandRegistry& registry)
  : m_registry(registry)
{}

/**
 * @brief Register frame-parser commands (code/language/configuration).
 */
void API::Handlers::ProjectParserCommands::registerCommands()
{
  registerCodeCommands();
  registerTemplateCommands();
  registerConfigCommands();
}

/**
 * @brief Register frame-parser code and language commands (set/get code, set/get language).
 */
void API::Handlers::ProjectParserCommands::registerCodeCommands()
{
  auto& registry = m_registry;

  const auto setCodeSchema = makeSchema(
    {
      {QStringLiteral("code"),
       QStringLiteral("string"),
       QStringLiteral("Frame parser script code (JS or Lua), or for the Built-In language the "
                      "JSON descriptor {\"template\": id, \"params\": {...}}")}
  },
    {{QString(Keys::SourceId),
      QStringLiteral("integer"),
      QStringLiteral("Source index (default 0)")},
     {QStringLiteral("language"),
      QStringLiteral("integer"),
      QStringLiteral("Optional: 0 = JavaScript, 1 = Lua, 2 = Built-In (parametrized C++ "
                     "template). When supplied, the source language is flipped before the "
                     "code is validated and script errors are returned as API errors.")}});

  registry.registerCommand(QStringLiteral("project.frameParser.setCode"),
                           QStringLiteral("Set frame parser code (params: code, "
                                          "optional sourceId, optional language). "
                                          "Always pass `language` when authoring code "
                                          "to lock in the runtime engine -- mismatch = "
                                          "silent compile failure. Lua (1) is the "
                                          "recommended default; it's faster than "
                                          "JavaScript on the hotpath at typical "
                                          "telemetry rates. Use JavaScript only when "
                                          "you need a JS-specific library or feature. "
                                          "Validate with project.frameParser.dryRun (or "
                                          "dryCompile for a syntax-only check) before "
                                          "setCode. **Call meta.fetchScriptingDocs{kind: "
                                          "'frame_parser_lua' | 'frame_parser_js'} first** "
                                          "for the parse() signature, return-shape rules, "
                                          "and the tableGet/tableSet API. For Built-In (2), "
                                          "prefer project.frameParser.setTemplate; passing "
                                          "the JSON descriptor as `code` also works."),
                           setCodeSchema,
                           &parserSetCode);

  registry.registerCommand(QStringLiteral("project.frameParser.getCode"),
                           QStringLiteral("Read the current frame parser source for a "
                                          "given data source. Returns {code, language}; "
                                          "Built-In sources also return {template, params} "
                                          "and `code` carries the JSON descriptor. "
                                          "Always read BEFORE rewriting -- preserve the "
                                          "user's existing structure where reasonable."),
                           makeSchema(
                             {
  },
                             {{QString(Keys::SourceId),
                               QStringLiteral("integer"),
                               QStringLiteral("Source index (default 0)")}}),
                           &parserGetCode);

  registry.registerCommand(QStringLiteral("project.frameParser.setLanguage"),
                           QStringLiteral("Switch a source between JavaScript, Lua and Built-In "
                                          "frame parsers. WARNING: for JS/Lua this WIPES any "
                                          "existing frameParser code for that source -- the "
                                          "loaded default template for the new language "
                                          "replaces it. If you want to preserve+translate, "
                                          "frameParser.getCode first, switch, then "
                                          "frameParser.setCode with the translated source. "
                                          "Switching to Built-In (2) seeds the default "
                                          "'delimited' template and leaves JS/Lua code "
                                          "intact for round-trips."),
                           makeSchema(
                             {
                               {QStringLiteral("language"),
                                QStringLiteral("integer"),
                                QStringLiteral("Script language: 0 = JavaScript, 1 = Lua, "
                                               "2 = Built-In (parametrized C++ template)")}
  },
                             {{QString(Keys::SourceId),
                               QStringLiteral("integer"),
                               QStringLiteral("Source identifier (default 0)")}}),
                           &parserSetLanguage);

  registry.registerCommand(
    QStringLiteral("project.frameParser.getLanguage"),
    QStringLiteral("Get the script language used by the frame parser for a given source"),
    makeSchema(
      {
  },
      {{QString(Keys::SourceId),
        QStringLiteral("integer"),
        QStringLiteral("Source identifier (default 0)")}}),
    &parserGetLanguage);
}

/**
 * @brief Register Native frame-parser template discovery / configuration commands.
 */
void API::Handlers::ProjectParserCommands::registerTemplateCommands()
{
  auto& registry = m_registry;

  registry.registerCommand(
    QStringLiteral("project.frameParser.listTemplates"),
    QStringLiteral("List the Built-In (C++) frame parser templates. Returns {templates: "
                   "[{id, name, description}], count}. Built-In templates parse without "
                   "user code and are the fastest option; configure one with "
                   "project.frameParser.setTemplate after inspecting its parameters via "
                   "project.frameParser.getTemplateSchema."),
    emptySchema(),
    &parserListTemplates);

  registry.registerCommand(
    QStringLiteral("project.frameParser.getTemplateSchema"),
    QStringLiteral("Get the parameter schema of a Built-In frame parser template. Returns "
                   "{id, name, description, params: [{key, type, label, description, "
                   "default, options?, min?, max?}]}. Param types: string, char, int, "
                   "float, bool, enum (enum values come from options[].value)."),
    makeSchema({
      {QStringLiteral("template"),
       QStringLiteral("string"),
       QStringLiteral("Template id from project.frameParser.listTemplates")}
  }),
    &parserGetTemplateSchema);

  registry.registerCommand(
    QStringLiteral("project.frameParser.getTemplate"),
    QStringLiteral("Get the Built-In frame parser configuration for a source. Returns "
                   "{sourceId, language, template, params}; template is empty when the "
                   "source never used the Built-In language."),
    makeSchema(
      {
  },
      {{QString(Keys::SourceId),
        QStringLiteral("integer"),
        QStringLiteral("Source index (default 0)")}}),
    &parserGetTemplate);

  registry.registerCommand(
    QStringLiteral("project.frameParser.setTemplate"),
    QStringLiteral("Select a Built-In frame parser template for a source and switch the "
                   "source to the Built-In language. Params are validated against the "
                   "template schema; omitted params use the schema defaults. Use "
                   "project.frameParser.dryRun (language 2, descriptor as code) to "
                   "preview the output before or after applying."),
    makeSchema(
      {
        {QStringLiteral("template"),
         QStringLiteral("string"),
         QStringLiteral("Template id from project.frameParser.listTemplates")}
  },
      {{QString(Keys::SourceId),
        QStringLiteral("integer"),
        QStringLiteral("Source index (default 0)")},
       {QStringLiteral("params"),
        QStringLiteral("object"),
        QStringLiteral("Template parameters (see getTemplateSchema); omitted keys use "
                       "schema defaults")}}),
    &parserSetTemplate);
}

/**
 * @brief Register frame-parser configuration commands (delimiters, checksum, mode, getConfig).
 */
void API::Handlers::ProjectParserCommands::registerConfigCommands()
{
  auto& registry = m_registry;

  registry.registerCommand(
    QStringLiteral("project.frameParser.update"),
    QStringLiteral("Configure frame extraction settings on a source (params: startSequence, "
                   "endSequence, checksumAlgorithm, frameDetection, decoderMethod, "
                   "hexadecimalDelimiters, operationMode). This is the persist path for "
                   "delimiters -- parser code alone (setCode) never changes them."),
    makeSchema(
      {
  },
      {{QString(Keys::SourceId),
        QStringLiteral("integer"),
        QStringLiteral("Source index (default 0)")},
       {QStringLiteral("startSequence"),
        QStringLiteral("string"),
        QStringLiteral("Frame start delimiter")},
       {QStringLiteral("endSequence"),
        QStringLiteral("string"),
        QStringLiteral("Frame end delimiter")},
       {QString(Keys::ChecksumAlgorithm),
        QStringLiteral("string"),
        QStringLiteral("Checksum algorithm name")},
       {QString(Keys::FrameDetection),
        QStringLiteral("integer"),
        QStringLiteral("Frame detection mode (0-3)")},
       {QString(Keys::DecoderMethod),
        QStringLiteral("integer"),
        QStringLiteral("Decoder method (0=PlainText, 1=Hex, 2=Base64, 3=Binary)")},
       {QString(Keys::HexadecimalDelimiters),
        QStringLiteral("boolean"),
        QStringLiteral("Treat startSequence/endSequence as hex byte sequences")},
       {QStringLiteral("operationMode"),
        QStringLiteral("integer"),
        QStringLiteral("Operation mode (0-2)")}}),
    &frameParserConfigure);

  registry.registerCommand(QStringLiteral("project.frameParser.getConfig"),
                           QStringLiteral("Get frame parser configuration"),
                           emptySchema(),
                           &frameParserGetConfig);
}

/**
 * @brief Set frame parser code for a source.
 */
API::CommandResponse API::Handlers::ProjectParserCommands::parserSetCode(const QString& id,
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
API::CommandResponse API::Handlers::ProjectParserCommands::parserGetCode(const QString& id,
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
API::CommandResponse API::Handlers::ProjectParserCommands::parserSetLanguage(
  const QString& id, const QJsonObject& params)
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
API::CommandResponse API::Handlers::ProjectParserCommands::parserGetLanguage(
  const QString& id, const QJsonObject& params)
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
API::CommandResponse API::Handlers::ProjectParserCommands::parserListTemplates(
  const QString& id, const QJsonObject& params)
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
API::CommandResponse API::Handlers::ProjectParserCommands::parserGetTemplateSchema(
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
API::CommandResponse API::Handlers::ProjectParserCommands::parserGetTemplate(
  const QString& id, const QJsonObject& params)
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
API::CommandResponse API::Handlers::ProjectParserCommands::parserSetTemplate(
  const QString& id, const QJsonObject& params)
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
API::CommandResponse API::Handlers::ProjectParserCommands::frameParserConfigure(
  const QString& id, const QJsonObject& params)
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
API::CommandResponse API::Handlers::ProjectParserCommands::frameParserGetConfig(
  const QString& id, const QJsonObject& params)
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
