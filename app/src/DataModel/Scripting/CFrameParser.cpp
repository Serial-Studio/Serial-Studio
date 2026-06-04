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

#include "DataModel/Scripting/CFrameParser.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QMessageBox>

#include "Misc/Utilities.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Descriptor keys & helpers
//--------------------------------------------------------------------------------------------------

static const QString kTemplateKey = QStringLiteral("template");
static const QString kParamsKey   = QStringLiteral("params");

/**
 * @brief Returns the schema string identifier for a parameter type.
 */
static QString paramTypeName(DataModel::NativeParamType type)
{
  switch (type) {
    case DataModel::NativeParamType::Char:
      return QStringLiteral("char");
    case DataModel::NativeParamType::Int:
      return QStringLiteral("int");
    case DataModel::NativeParamType::Float:
      return QStringLiteral("float");
    case DataModel::NativeParamType::Bool:
      return QStringLiteral("bool");
    case DataModel::NativeParamType::Enum:
      return QStringLiteral("enum");
    case DataModel::NativeParamType::String:
    default:
      return QStringLiteral("string");
  }
}

/**
 * @brief Serializes one parameter spec into its JSON schema entry.
 */
static QJsonObject paramSpecToJson(const DataModel::NativeParamSpec& spec)
{
  Q_ASSERT(!spec.key.isEmpty());

  QJsonObject entry;
  entry.insert(QStringLiteral("key"), spec.key);
  entry.insert(QStringLiteral("type"), paramTypeName(spec.type));
  entry.insert(QStringLiteral("label"), spec.label);
  entry.insert(QStringLiteral("description"), spec.description);
  entry.insert(QStringLiteral("default"), spec.defaultValue);

  if (spec.type == DataModel::NativeParamType::Enum) {
    Q_ASSERT(spec.optionValues.size() == spec.optionLabels.size());

    QJsonArray options;
    for (qsizetype i = 0; i < spec.optionValues.size(); ++i) {
      QJsonObject option;
      option.insert(QStringLiteral("value"), spec.optionValues.at(i));
      option.insert(QStringLiteral("label"), spec.optionLabels.at(i));
      options.append(option);
    }

    entry.insert(QStringLiteral("options"), options);
  }

  if (spec.type == DataModel::NativeParamType::Int
      || spec.type == DataModel::NativeParamType::Float) {
    entry.insert(QStringLiteral("min"), spec.minValue);
    entry.insert(QStringLiteral("max"), spec.maxValue);
  }

  return entry;
}

//--------------------------------------------------------------------------------------------------
// Static catalog API
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the catalog of native templates as [{id, name, description}, ...].
 */
QJsonArray DataModel::CFrameParser::templateCatalog()
{
  QJsonArray catalog;
  const auto& templates = nativeTemplates();
  for (const auto* tmpl : templates) {
    Q_ASSERT(tmpl != nullptr);

    QJsonObject entry;
    entry.insert(QStringLiteral("id"), tmpl->id());
    entry.insert(QStringLiteral("name"), tmpl->name());
    entry.insert(QStringLiteral("description"), tmpl->description());
    catalog.append(entry);
  }

  Q_ASSERT(!catalog.isEmpty());
  return catalog;
}

/**
 * @brief Returns the parameter schema for the template id, or an empty object when unknown.
 */
QJsonObject DataModel::CFrameParser::templateSchema(const QString& id)
{
  const auto* tmpl = nativeTemplateById(id);
  if (!tmpl)
    return QJsonObject();

  QJsonArray params;
  const auto specs = tmpl->params();
  for (const auto& spec : specs)
    params.append(paramSpecToJson(spec));

  QJsonObject schema;
  schema.insert(QStringLiteral("id"), tmpl->id());
  schema.insert(QStringLiteral("name"), tmpl->name());
  schema.insert(QStringLiteral("description"), tmpl->description());
  schema.insert(kParamsKey, params);

  Q_ASSERT(schema.value(QStringLiteral("id")).toString() == id);
  return schema;
}

/**
 * @brief Builds the canonical (compact, key-sorted) JSON descriptor for a template + params.
 */
QString DataModel::CFrameParser::buildDescriptor(const QString& templateId,
                                                 const QJsonObject& params)
{
  Q_ASSERT(!templateId.isEmpty());

  // QJsonObject stores keys sorted, so compact serialization is byte-stable
  QJsonObject descriptor;
  descriptor.insert(kTemplateKey, templateId);
  descriptor.insert(kParamsKey, params);

  const auto json = QJsonDocument(descriptor).toJson(QJsonDocument::Compact);
  Q_ASSERT(!json.isEmpty());
  return QString::fromUtf8(json);
}

//--------------------------------------------------------------------------------------------------
// Engine implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an unloaded native parser engine.
 */
DataModel::CFrameParser::CFrameParser() : m_parser(nullptr) {}

/**
 * @brief Parses the JSON descriptor and configures the selected native template.
 */
bool DataModel::CFrameParser::loadScript(const QString& script, int sourceId, bool showMessageBoxes)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(!script.isEmpty());

  QJsonParseError parse_error;
  const auto doc = QJsonDocument::fromJson(script.toUtf8(), &parse_error);
  if (parse_error.error != QJsonParseError::NoError || !doc.isObject()) {
    reportLoadError(QObject::tr("The native parser configuration is not a valid JSON object."),
                    sourceId,
                    showMessageBoxes);
    return false;
  }

  const auto descriptor     = doc.object();
  const QString template_id = descriptor.value(kTemplateKey).toString();
  const auto* tmpl          = nativeTemplateById(template_id);
  if (!tmpl) {
    reportLoadError(QObject::tr("Unknown native parser template: \"%1\".").arg(template_id),
                    sourceId,
                    showMessageBoxes);
    return false;
  }

  QString error;
  auto parser = tmpl->makeParser(descriptor.value(kParamsKey).toObject(), error);
  if (!parser) {
    reportLoadError(error, sourceId, showMessageBoxes);
    return false;
  }

  m_parser     = std::move(parser);
  m_templateId = template_id;
  m_lastError.clear();
  return true;
}

/**
 * @brief Runs the configured template over a text frame.
 */
QList<QStringList> DataModel::CFrameParser::parseString(const QString& frame)
{
  Q_ASSERT(!frame.isEmpty());

  if (!m_parser) [[unlikely]]
    return {};

  return m_parser->parseText(frame);
}

/**
 * @brief Runs the configured template over a UTF-8 text frame without a QString round-trip.
 */
QList<QStringList> DataModel::CFrameParser::parseUtf8(const QByteArray& frame)
{
  Q_ASSERT(!frame.isEmpty());

  if (!m_parser) [[unlikely]]
    return {};

  return m_parser->parseUtf8(frame);
}

/**
 * @brief Forwards the allocation-free span fast path to the configured template.
 */
qsizetype DataModel::CFrameParser::parseUtf8Spans(QByteArrayView frame,
                                                  QByteArrayView* out,
                                                  qsizetype maxSpans) noexcept
{
  Q_ASSERT(out != nullptr);
  Q_ASSERT(maxSpans > 0);

  if (!m_parser) [[unlikely]]
    return -1;

  return m_parser->parseSpans(frame, out, maxSpans);
}

/**
 * @brief Runs the configured template over a binary frame.
 */
QList<QStringList> DataModel::CFrameParser::parseBinary(const QByteArray& frame)
{
  Q_ASSERT(!frame.isEmpty());

  if (!m_parser) [[unlikely]]
    return {};

  return m_parser->parseBinary(frame);
}

/**
 * @brief Returns true once a template has been configured successfully.
 */
bool DataModel::CFrameParser::isLoaded() const noexcept
{
  return m_parser != nullptr;
}

/**
 * @brief Returns the ScriptLanguage implemented by this engine.
 */
int DataModel::CFrameParser::language() const noexcept
{
  return SerialStudio::Native;
}

/**
 * @brief Returns the most recent load error, or an empty string.
 */
QString DataModel::CFrameParser::lastError() const
{
  return m_lastError;
}

/**
 * @brief Returns the id of the currently loaded template, or an empty string.
 */
QString DataModel::CFrameParser::templateId() const
{
  return m_templateId;
}

/**
 * @brief No-op: native templates have no garbage-collected runtime.
 */
void DataModel::CFrameParser::collectGarbage() {}

/**
 * @brief Resets the engine to an unloaded state, dropping the configured template.
 */
void DataModel::CFrameParser::reset()
{
  m_parser.reset();
  m_templateId.clear();
  m_lastError.clear();
}

/**
 * @brief Stores the load error and reports it via message box or warning log.
 */
void DataModel::CFrameParser::reportLoadError(const QString& error,
                                              int sourceId,
                                              bool showMessageBoxes)
{
  Q_ASSERT(sourceId >= 0);
  Q_ASSERT(!error.isEmpty());

  m_lastError = error;
  if (showMessageBoxes) {
    Misc::Utilities::showMessageBox(
      QObject::tr("Built-In Parser Error"), error, QMessageBox::Critical);
  } else {
    qWarning() << "[CFrameParser] Source" << sourceId << "load failed:" << error;
  }
}
