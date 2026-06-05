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

#include "DataModel/Editors/FrameParserModel.h"

#include <QFile>
#include <QInputDialog>
#include <QStandardItem>

#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/FrameParserPipeline.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "IO/Checksum.h"
#include "Misc/Translator.h"
#include "SerialStudio.h"

//--------------------------------------------------------------------------------------------------
// Internal helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Frame detection values in combobox order.
 */
static const QList<SerialStudio::FrameDetection> kDetectionValues = {
  SerialStudio::EndDelimiterOnly,
  SerialStudio::StartAndEndDelimiter,
  SerialStudio::StartDelimiterOnly,
  SerialStudio::NoDelimiters,
};

/**
 * @brief Truncates a printable hex preview to 64 bytes so the tree stays readable.
 */
static QString hexPreview(const QByteArray& bytes, qsizetype maxBytes = 64)
{
  if (bytes.size() <= maxBytes)
    return QString::fromLatin1(bytes.toHex(' ')).toUpper();

  return QString::fromLatin1(bytes.left(maxBytes).toHex(' ')).toUpper()
       + QStringLiteral(" ... (+%1 bytes)").arg(bytes.size() - maxBytes);
}

/**
 * @brief Returns the bytes encoded by a delimiter field, honoring hex mode.
 */
static QByteArray delimiterBytes(const QString& text, bool hex)
{
  if (text.isEmpty())
    return {};

  if (hex) {
    const auto resolved = SerialStudio::resolveEscapeSequences(text);
    return QByteArray::fromHex(QString(resolved).remove(' ').toUtf8());
  }

  return SerialStudio::resolveEscapeSequences(text).toUtf8();
}

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the bridge and wires project-model + translation subscriptions.
 */
DataModel::FrameParserModel::FrameParserModel(QObject* parent)
  : QObject(parent), m_sourceId(0), m_applying(false), m_parameterModel(nullptr)
{
  connect(&ProjectModel::instance(),
          &ProjectModel::sourceFrameParserTemplateChanged,
          this,
          &FrameParserModel::onSourceTemplateChanged);

  connect(&ProjectModel::instance(),
          &ProjectModel::sourceFrameParserParamsChanged,
          this,
          &FrameParserModel::onSourceParamsChanged);

  connect(&ProjectModel::instance(), &ProjectModel::sourceChanged, this, [this](int sourceId) {
    if (sourceId == m_sourceId)
      Q_EMIT pipelineChanged();
  });

  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &FrameParserModel::onLanguageChanged);

  rebuildTemplateNames();
  rebuildParameterModel();
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the source ID this editor is bound to.
 */
int DataModel::FrameParserModel::sourceId() const noexcept
{
  return m_sourceId;
}

/**
 * @brief Returns the registry index of the source's template (default template if unset).
 */
int DataModel::FrameParserModel::templateIndex() const
{
  const QString id      = ProjectModel::instance().frameParserTemplate(m_sourceId);
  const auto& templates = nativeTemplates();
  for (int i = 0; i < templates.size(); ++i)
    if (templates.at(i)->id() == id)
      return i;

  return 0;
}

/**
 * @brief Returns the current parameter validation error, or an empty string.
 */
QString DataModel::FrameParserModel::paramError() const
{
  return m_paramError;
}

/**
 * @brief Returns the translated description of the selected template.
 */
QString DataModel::FrameParserModel::templateDescription() const
{
  const auto* tmpl = currentTemplate();
  if (!tmpl)
    return QString();

  return tmpl->description();
}

/**
 * @brief Returns the Markdown documentation page bundled for the selected template.
 */
QString DataModel::FrameParserModel::templateDocumentation() const
{
  const auto* tmpl = currentTemplate();
  if (!tmpl)
    return QString();

  QFile file(QStringLiteral(":/scripts/native/%1.md").arg(tmpl->id()));
  if (file.open(QFile::ReadOnly | QFile::Text))
    return QString::fromUtf8(file.readAll());

  return tmpl->description();
}

/**
 * @brief Returns the translated display names of every native template.
 */
const QStringList& DataModel::FrameParserModel::templateNames() const
{
  return m_templateNames;
}

/**
 * @brief Returns the dynamic parameter form model for the selected template.
 */
QAbstractItemModel* DataModel::FrameParserModel::parameterModel() const
{
  return m_parameterModel;
}

/**
 * @brief Returns the number of parameter rows for the selected template.
 */
int DataModel::FrameParserModel::parameterCount() const
{
  return m_parameterModel ? m_parameterModel->rowCount() : 0;
}

/**
 * @brief Returns the source's decoder method as a combobox index.
 */
int DataModel::FrameParserModel::decoderIndex() const
{
  return currentSource().decoderMethod;
}

/**
 * @brief Returns the source's frame detection mode as a combobox index.
 */
int DataModel::FrameParserModel::detectionIndex() const
{
  const auto detection = static_cast<SerialStudio::FrameDetection>(currentSource().frameDetection);
  const auto idx       = kDetectionValues.indexOf(detection);
  return idx >= 0 ? static_cast<int>(idx) : 0;
}

/**
 * @brief Returns the source's checksum algorithm as a combobox index.
 */
int DataModel::FrameParserModel::checksumIndex() const
{
  const auto idx = IO::availableChecksums().indexOf(currentSource().checksumAlgorithm);
  return idx >= 0 ? static_cast<int>(idx) : 0;
}

/**
 * @brief Returns the source's frame start delimiter.
 */
QString DataModel::FrameParserModel::frameStart() const
{
  return currentSource().frameStart;
}

/**
 * @brief Returns the source's frame end delimiter.
 */
QString DataModel::FrameParserModel::frameEnd() const
{
  return currentSource().frameEnd;
}

/**
 * @brief Returns true when the source's delimiters are hex-encoded.
 */
bool DataModel::FrameParserModel::hexDelimiters() const
{
  return currentSource().hexadecimalDelimiters;
}

/**
 * @brief Returns the translated decoder option labels (enum order, dialog wording).
 */
QStringList DataModel::FrameParserModel::decoderOptions() const
{
  return {tr("Plain text (UTF-8)"), tr("Hexadecimal"), tr("Base64"), tr("Binary (raw bytes)")};
}

/**
 * @brief Returns the translated frame detection option labels (dialog wording).
 */
QStringList DataModel::FrameParserModel::detectionOptions() const
{
  return {tr("End delimiter only"),
          tr("Start + end delimiters"),
          tr("Start delimiter only"),
          tr("No delimiters (whole chunk)")};
}

/**
 * @brief Returns the checksum algorithm labels, mapping the empty algorithm to "No Checksum".
 */
QStringList DataModel::FrameParserModel::checksumOptions() const
{
  auto options = IO::availableChecksums();
  for (auto& option : options)
    if (option.isEmpty())
      option = tr("No Checksum");

  return options;
}

//--------------------------------------------------------------------------------------------------
// Property setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the editor to a source and reloads template + params from the project.
 */
void DataModel::FrameParserModel::setSourceId(int sourceId)
{
  if (m_sourceId == sourceId)
    return;

  m_sourceId = sourceId;
  Q_EMIT sourceIdChanged();
  Q_EMIT pipelineChanged();
  Q_EMIT templateIndexChanged();
  rebuildParameterModel();
}

/**
 * @brief Shows a native-template picker dialog and applies the chosen template.
 */
void DataModel::FrameParserModel::selectTemplate()
{
  bool ok = false;
  const auto name = QInputDialog::getItem(nullptr,
                                          tr("Select Frame Parser Template"),
                                          tr("Choose a template to load:"),
                                          m_templateNames,
                                          templateIndex(),
                                          false,
                                          &ok);
  if (!ok)
    return;

  const int idx = m_templateNames.indexOf(name);
  if (idx >= 0)
    setTemplateIndex(idx);
}

/**
 * @brief Selects the template at the registry index and seeds its default parameters.
 */
void DataModel::FrameParserModel::setTemplateIndex(int index)
{
  const auto& templates = nativeTemplates();
  if (index < 0 || index >= templates.size() || index == templateIndex())
    return;

  const auto* tmpl = templates.at(index);
  Q_ASSERT(tmpl != nullptr);

  m_applying  = true;
  auto& model = ProjectModel::instance();
  model.updateSourceFrameParserParams(m_sourceId, nativeTemplateDefaults(*tmpl));
  model.updateSourceFrameParserTemplate(m_sourceId, tmpl->id());
  m_applying = false;

  setParamError(QString());
  Q_EMIT templateIndexChanged();
  rebuildParameterModel();
}

/**
 * @brief Persists the decoder method to the source.
 */
void DataModel::FrameParserModel::setDecoderIndex(int index)
{
  if (index < 0 || index == decoderIndex())
    return;

  auto src          = currentSource();
  src.decoderMethod = index;
  ProjectModel::instance().updateSource(m_sourceId, src);
}

/**
 * @brief Persists the frame detection mode to the source.
 */
void DataModel::FrameParserModel::setDetectionIndex(int index)
{
  if (index < 0 || index >= kDetectionValues.size() || index == detectionIndex())
    return;

  auto src           = currentSource();
  src.frameDetection = static_cast<int>(kDetectionValues.at(index));
  ProjectModel::instance().updateSource(m_sourceId, src);
}

/**
 * @brief Persists the checksum algorithm to the source.
 */
void DataModel::FrameParserModel::setChecksumIndex(int index)
{
  const auto algorithms = IO::availableChecksums();
  if (index < 0 || index >= algorithms.size() || index == checksumIndex())
    return;

  auto src              = currentSource();
  src.checksumAlgorithm = algorithms.at(index);
  ProjectModel::instance().updateSource(m_sourceId, src);
}

/**
 * @brief Persists the frame start delimiter to the source.
 */
void DataModel::FrameParserModel::setFrameStart(const QString& sequence)
{
  auto src = currentSource();
  if (src.frameStart == sequence)
    return;

  src.frameStart = sequence;
  ProjectModel::instance().updateSource(m_sourceId, src);
}

/**
 * @brief Persists the frame end delimiter to the source.
 */
void DataModel::FrameParserModel::setFrameEnd(const QString& sequence)
{
  auto src = currentSource();
  if (src.frameEnd == sequence)
    return;

  src.frameEnd = sequence;
  ProjectModel::instance().updateSource(m_sourceId, src);
}

/**
 * @brief Persists the hex-delimiters toggle to the source.
 */
void DataModel::FrameParserModel::setHexDelimiters(bool hexadecimal)
{
  auto src = currentSource();
  if (src.hexadecimalDelimiters == hexadecimal)
    return;

  src.hexadecimalDelimiters = hexadecimal;
  ProjectModel::instance().updateSource(m_sourceId, src);
}

//--------------------------------------------------------------------------------------------------
// Actions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Restores every parameter of the selected template to its schema default.
 */
void DataModel::FrameParserModel::resetToDefaults()
{
  const auto* tmpl = currentTemplate();
  if (!tmpl)
    return;

  m_applying = true;
  ProjectModel::instance().updateSourceFrameParserParams(m_sourceId, nativeTemplateDefaults(*tmpl));
  m_applying = false;

  setParamError(QString());
  rebuildParameterModel();
}

/**
 * @brief Returns true when the sample input carries every delimiter the detection mode needs.
 */
bool DataModel::FrameParserModel::inputContainsDelimiters(const QString& input, bool hex) const
{
  if (input.isEmpty())
    return true;

  const auto src       = currentSource();
  const auto detection = static_cast<SerialStudio::FrameDetection>(src.frameDetection);
  if (detection == SerialStudio::NoDelimiters)
    return true;

  // Same byte conversion as dryRun, so hex mode and escape sequences match extraction
  const QByteArray bytes =
    hex ? SerialStudio::hexToBytes(input) : SerialStudio::resolveEscapeSequences(input).toUtf8();

  const bool needs_start = detection == SerialStudio::StartAndEndDelimiter
                        || detection == SerialStudio::StartDelimiterOnly;
  const bool needs_end =
    detection == SerialStudio::EndDelimiterOnly || detection == SerialStudio::StartAndEndDelimiter;
  Q_ASSERT(needs_start || needs_end);

  const auto start = delimiterBytes(src.frameStart, src.hexadecimalDelimiters);
  const auto end   = delimiterBytes(src.frameEnd, src.hexadecimalDelimiters);
  if (needs_start && (start.isEmpty() || !bytes.contains(start)))
    return false;

  if (needs_end && (end.isEmpty() || !bytes.contains(end)))
    return false;

  return true;
}

/**
 * @brief Runs extraction + decoder + the source's parser against sample input bytes. Native
 * sources run their template; JS/Lua sources run the live engine (like the old test dialog).
 */
void DataModel::FrameParserModel::dryRun(const QString& input, bool hex)
{
  if (input.isEmpty()) {
    Q_EMIT previewReady(QVariantList(), QString(), QString());
    return;
  }

  // Text input resolves escape sequences so a typed \n matches the newline end delimiter
  const QByteArray bytes =
    hex ? SerialStudio::hexToBytes(input) : SerialStudio::resolveEscapeSequences(input).toUtf8();
  if (bytes.isEmpty()) {
    Q_EMIT previewReady(QVariantList(), tr("Invalid hexadecimal input."), QString());
    return;
  }

  // Mirror the live FrameBuilder path: extraction spec comes from the source's settings
  const auto src = currentSource();
  PipelineSpec spec;
  spec.operationMode     = SerialStudio::ProjectFile;
  spec.frameDetection    = static_cast<SerialStudio::FrameDetection>(src.frameDetection);
  spec.decoderMethod     = static_cast<SerialStudio::DecoderMethod>(src.decoderMethod);
  spec.checksumAlgorithm = src.checksumAlgorithm;

  const auto start = delimiterBytes(src.frameStart, src.hexadecimalDelimiters);
  const auto end   = delimiterBytes(src.frameEnd, src.hexadecimalDelimiters);
  if (!start.isEmpty())
    spec.startSequences.append(start);

  if (!end.isEmpty())
    spec.finishSequences.append(end);

  if (src.frameParserLanguage != SerialStudio::Native) {
    // Pick up uncommitted edits to shared tables, then run the live engine
    DataModel::FrameBuilder::instance().refreshTableStoreFromProjectModel();
    emitPreview(runFrameParserPipeline(bytes, spec, m_sourceId), spec.decoderMethod);
    return;
  }

  const auto* tmpl = currentTemplate();
  if (!tmpl) {
    Q_EMIT previewReady(QVariantList(), tr("No template selected."), QString());
    return;
  }

  QString error;
  if (!validateParams(m_params, error)) {
    Q_EMIT previewReady(QVariantList(), error, QString());
    return;
  }

  emitPreview(runNativeTemplatePipeline(bytes, spec, tmpl->id(), m_params), spec.decoderMethod);
}

/**
 * @brief Serializes a pipeline result into the previewReady payload.
 */
void DataModel::FrameParserModel::emitPreview(const PipelineResult& result, int decoderMethod)
{
  if (!result.stageError.isEmpty()) {
    Q_EMIT previewReady(QVariantList(), result.stageError, QString());
    return;
  }

  // Uppercase hex-style decoder output (Hex / Binary modes), like the test dialog
  const bool decoder_is_hex =
    (decoderMethod == SerialStudio::Hexadecimal || decoderMethod == SerialStudio::Binary);

  QVariantList frames;
  for (const auto& frame : result.frames) {
    QVariantList rows;
    for (const auto& row : frame.rows)
      rows.append(QVariant(row));

    QVariantMap entry;
    entry.insert(QStringLiteral("rawHex"), hexPreview(frame.rawBytes));
    entry.insert(QStringLiteral("decoderOutput"),
                 decoder_is_hex ? frame.decoderOutput.toUpper() : frame.decoderOutput);
    entry.insert(QStringLiteral("rows"), rows);
    frames.append(entry);
  }

  const QString stats = tr("%1 frame(s) extracted | %2 byte(s) consumed | %3 byte(s) buffered | "
                           "%4 dropped")
                          .arg(result.extractedCount)
                          .arg(result.consumedBytes)
                          .arg(result.remainingBytes)
                          .arg(result.droppedFrames);

  Q_EMIT previewReady(frames, QString(), stats);
}

//--------------------------------------------------------------------------------------------------
// Model synchronization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies a single parameter edit from the form model to the project.
 */
void DataModel::FrameParserModel::onItemChanged(QStandardItem* item)
{
  if (m_applying || !item)
    return;

  const auto* tmpl  = currentTemplate();
  const QString key = item->data(ProjectEditor::ParameterKey).toString();
  if (!tmpl || key.isEmpty())
    return;

  // Locate the schema entry for the edited row
  NativeParamSpec spec;
  bool found       = false;
  const auto specs = tmpl->params();
  for (const auto& candidate : specs) {
    if (candidate.key == key) {
      spec  = candidate;
      found = true;
      break;
    }
  }

  if (!found)
    return;

  // Convert the widget payload to the parameter's JSON value
  const auto value = item->data(ProjectEditor::EditableValue);
  QJsonValue json;
  switch (spec.type) {
    case NativeParamType::Enum:
      json = spec.optionValues.value(value.toInt(), spec.defaultValue.toString());
      break;
    case NativeParamType::Bool:
      json = value.toBool();
      break;
    case NativeParamType::Int:
      json = value.toInt();
      break;
    case NativeParamType::Float:
      json = SerialStudio::toDouble(value);
      break;
    case NativeParamType::Char: {
      QString text = value.toString();
      if (text.length() > 1) {
        text.truncate(1);
        m_applying = true;
        item->setData(text, ProjectEditor::EditableValue);
        m_applying = false;
      }
      json = text;
      break;
    }
    case NativeParamType::String:
    default:
      json = value.toString();
      break;
  }

  m_params.insert(key, json);

  // Invalid configs stay local (banner only) so the live engine never loads them
  QString error;
  if (!validateParams(m_params, error)) {
    setParamError(error);
    return;
  }

  setParamError(QString());
  m_applying = true;
  ProjectModel::instance().updateSourceFrameParserParams(m_sourceId, m_params);
  m_applying = false;
}

/**
 * @brief Reacts to external template changes targeting this editor's source.
 */
void DataModel::FrameParserModel::onSourceTemplateChanged(int sourceId)
{
  if (m_applying || sourceId != m_sourceId)
    return;

  setParamError(QString());
  Q_EMIT templateIndexChanged();
  rebuildParameterModel();
}

/**
 * @brief Reacts to external parameter changes targeting this editor's source.
 */
void DataModel::FrameParserModel::onSourceParamsChanged(int sourceId)
{
  if (m_applying || sourceId != m_sourceId)
    return;

  setParamError(QString());
  rebuildParameterModel();
}

/**
 * @brief Refreshes translated template names and option labels on language change.
 */
void DataModel::FrameParserModel::onLanguageChanged()
{
  rebuildTemplateNames();
  rebuildParameterModel();
  Q_EMIT pipelineOptionsChanged();
  Q_EMIT templateIndexChanged();
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rebuilds the cached translated template name list.
 */
void DataModel::FrameParserModel::rebuildTemplateNames()
{
  m_templateNames.clear();
  const auto& templates = nativeTemplates();
  for (const auto* tmpl : templates)
    m_templateNames.append(tmpl->name());

  Q_ASSERT(!m_templateNames.isEmpty());
  Q_EMIT templatesChanged();
}

/**
 * @brief Rebuilds the parameter form model from the selected template's schema.
 */
void DataModel::FrameParserModel::rebuildParameterModel()
{
  const auto* tmpl = currentTemplate();

  // Replace the model wholesale so delegates never see rows mutate across templates
  auto* model = new CustomModel(this);
  connect(model, &CustomModel::itemChanged, this, &FrameParserModel::onItemChanged);

  m_params = ProjectModel::instance().frameParserParams(m_sourceId);
  if (tmpl && m_params.isEmpty())
    m_params = nativeTemplateDefaults(*tmpl);

  auto* header = new QStandardItem();
  header->setData(ProjectEditor::SectionHeader, ProjectEditor::WidgetType);
  header->setData(tr("Parameters"), ProjectEditor::PlaceholderValue);
  header->setData(QStringLiteral("qrc:/icons/project-editor/model/widget.svg"),
                  ProjectEditor::ParameterIcon);
  model->appendRow(header);

  const auto specs = tmpl ? tmpl->params() : QList<NativeParamSpec>();
  for (const auto& spec : specs) {
    // Structured params are importer/API-managed; the form never edits them
    if (spec.type == NativeParamType::Json)
      continue;

    auto* item = new QStandardItem();
    item->setEditable(true);
    item->setData(true, ProjectEditor::Active);
    item->setData(spec.key, ProjectEditor::ParameterKey);
    item->setData(spec.label, ProjectEditor::ParameterName);
    item->setData(spec.description, ProjectEditor::ParameterDescription);

    const auto stored = m_params.value(spec.key);
    switch (spec.type) {
      case NativeParamType::Enum: {
        item->setData(ProjectEditor::ComboBox, ProjectEditor::WidgetType);
        item->setData(spec.optionLabels, ProjectEditor::ComboBoxData);
        const auto idx = spec.optionValues.indexOf(stored.toString());
        const auto def = spec.optionValues.indexOf(spec.defaultValue.toString());
        item->setData(static_cast<int>(idx >= 0 ? idx : qMax<qsizetype>(0, def)),
                      ProjectEditor::EditableValue);
        break;
      }
      case NativeParamType::Bool:
        item->setData(ProjectEditor::CheckBox, ProjectEditor::WidgetType);
        item->setData(stored.toBool(spec.defaultValue.toBool()), ProjectEditor::EditableValue);
        break;
      case NativeParamType::Int:
        item->setData(ProjectEditor::IntField, ProjectEditor::WidgetType);
        item->setData(stored.toInt(spec.defaultValue.toInt()), ProjectEditor::EditableValue);
        if (spec.minValue < spec.maxValue) {
          item->setData(spec.minValue, ProjectEditor::MinValue);
          item->setData(spec.maxValue, ProjectEditor::MaxValue);
        }
        break;
      case NativeParamType::Float:
        item->setData(ProjectEditor::FloatField, ProjectEditor::WidgetType);
        item->setData(SerialStudio::toDouble(stored, SerialStudio::toDouble(spec.defaultValue)),
                      ProjectEditor::EditableValue);
        if (spec.minValue < spec.maxValue) {
          item->setData(spec.minValue, ProjectEditor::MinValue);
          item->setData(spec.maxValue, ProjectEditor::MaxValue);
        }
        break;
      case NativeParamType::Char:
      case NativeParamType::String:
      default:
        item->setData(ProjectEditor::TextField, ProjectEditor::WidgetType);
        item->setData(stored.toString(spec.defaultValue.toString()), ProjectEditor::EditableValue);
        item->setData(spec.defaultValue.toString(), ProjectEditor::PlaceholderValue);
        break;
    }

    model->appendRow(item);
  }

  auto* previous   = m_parameterModel;
  m_parameterModel = model;
  Q_EMIT parameterModelChanged();

  if (previous)
    previous->deleteLater();
}

/**
 * @brief Stores and publishes the parameter validation error.
 */
void DataModel::FrameParserModel::setParamError(const QString& error)
{
  if (m_paramError == error)
    return;

  m_paramError = error;
  Q_EMIT paramErrorChanged();
}

/**
 * @brief Returns a copy of the source this editor is bound to.
 */
DataModel::Source DataModel::FrameParserModel::currentSource() const
{
  const auto& sources = ProjectModel::instance().sources();
  for (const auto& src : sources)
    if (src.sourceId == m_sourceId)
      return src;

  return Source();
}

/**
 * @brief Returns the descriptor of the source's selected template (default when unset).
 */
const DataModel::INativeTemplate* DataModel::FrameParserModel::currentTemplate() const
{
  const QString id = ProjectModel::instance().frameParserTemplate(m_sourceId);
  if (const auto* tmpl = nativeTemplateById(id))
    return tmpl;

  return nativeTemplateById(defaultNativeTemplateId());
}

/**
 * @brief Validates params by building a throwaway parser instance.
 */
bool DataModel::FrameParserModel::validateParams(const QJsonObject& params, QString& error) const
{
  const auto* tmpl = currentTemplate();
  if (!tmpl)
    return false;

  const auto parser = tmpl->makeParser(params, error);
  return parser != nullptr;
}
