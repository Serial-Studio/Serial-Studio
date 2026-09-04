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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "DataModel/Project/ProjectSources.h"

#include <algorithm>
#include <QInputDialog>
#include <QMessageBox>

#include "AppInfo.h"
#include "DataModel/Project/ProjectHistory.h"
#include "DataModel/Project/ProjectNaming.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "IO/ConnectionManager.h"
#include "Misc/Utilities.h"

//--------------------------------------------------------------------------------------------------
// Construction & defaults
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the source editor to @p model.
 */
DataModel::ProjectSources::ProjectSources(ProjectModel& model) : m_model(model) {}

/**
 * @brief Seeds a source with the default parser: the Native CSV (delimited/comma) template.
 *        Switching to a scripting language converts the template via the equivalence mapping.
 */
void DataModel::ProjectSources::seedDefaultFrameParser(DataModel::Source& source)
{
  source.frameParserLanguage = static_cast<int>(SerialStudio::Native);
  source.frameParserTemplate = DataModel::defaultNativeTemplateId();

  const auto* tmpl = DataModel::nativeTemplateById(source.frameParserTemplate);
  if (tmpl)
    source.frameParserParams = DataModel::nativeTemplateDefaults(*tmpl);
}

//--------------------------------------------------------------------------------------------------
// Source CRUD
//--------------------------------------------------------------------------------------------------

/**
 * @brief Adds a new source to the project (GPL: capped to one source).
 */
void DataModel::ProjectSources::addSource()
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Add Device")};
#ifndef BUILD_COMMERCIAL
  if (!m_model.m_sources.empty()) {
    if (!m_model.m_suppressMessageBoxes)
      Misc::Utilities::showMessageBox(
        ProjectModel::tr("Multiple data sources require a Pro license"),
        ProjectModel::tr("Serial Studio Pro allows connecting to multiple devices simultaneously. "
                         "Please upgrade to unlock this feature."),
        QMessageBox::Information);

    return;
  }
#endif

  const int newId = static_cast<int>(m_model.m_sources.size());

  DataModel::Source source;
  source.sourceId              = newId;
  source.title                 = ProjectModel::tr("Device %1").arg(QChar('A' + newId));
  source.busType               = static_cast<int>(SerialStudio::BusType::UART);
  source.frameStart            = m_model.m_frameStartSequence;
  source.frameEnd              = m_model.m_frameEndSequence;
  source.checksumAlgorithm     = m_model.m_checksumAlgorithm;
  source.frameDetection        = static_cast<int>(m_model.m_frameDetection);
  source.decoderMethod         = static_cast<int>(m_model.m_frameDecoder);
  source.hexadecimalDelimiters = m_model.m_hexadecimalDelimiters;
  seedDefaultFrameParser(source);

  m_model.m_sources.push_back(source);
  m_model.setModified(true);
  Q_EMIT m_model.sourcesChanged();
  Q_EMIT m_model.sourceStructureChanged();
  Q_EMIT m_model.sourceAdded(newId);
}

/**
 * @brief Deletes the source and reassigns dependent groups to source 0.
 */
void DataModel::ProjectSources::deleteSource(int sourceId, bool confirm)
{
#ifndef BUILD_COMMERCIAL
  (void)sourceId;
  (void)confirm;
  return;
#else
  auto& sources = m_model.m_sources;
  if (sourceId <= 0 || sourceId >= static_cast<int>(sources.size()))
    return;

  if (confirm && !m_model.m_suppressMessageBoxes) {
    const auto ret = Misc::Utilities::showMessageBox(
      ProjectModel::tr("Do you want to delete data source \"%1\"?").arg(sources[sourceId].title),
      ProjectModel::tr("Groups using this source will move to the default source. "
                       "This action cannot be undone."),
      QMessageBox::Question,
      APP_NAME,
      QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes)
      return;
  }

  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Delete Device")};
  sources.erase(sources.begin() + sourceId);

  const auto remapSourceId = [sourceId](int& id) {
    if (id == sourceId)
      id = 0;
    else if (id > sourceId)
      --id;
  };

  for (auto& group : m_model.m_groups) {
    remapSourceId(group.sourceId);
    for (auto& dataset : group.datasets)
      dataset.sourceId = group.sourceId;

    for (auto& widget : group.outputWidgets)
      remapSourceId(widget.sourceId);
  }

  for (auto& action : m_model.m_actions)
    remapSourceId(action.sourceId);

  for (size_t i = 0; i < sources.size(); ++i)
    sources[i].sourceId = static_cast<int>(i);

  m_model.setModified(true);
  Q_EMIT m_model.groupsChanged();
  Q_EMIT m_model.sourcesChanged();
  Q_EMIT m_model.sourceStructureChanged();
  Q_EMIT m_model.sourceDeleted();
#endif
}

/**
 * @brief Duplicates the source with the given @p sourceId.
 */
void DataModel::ProjectSources::duplicateSource(int sourceId)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Duplicate Device")};
#ifndef BUILD_COMMERCIAL
  (void)sourceId;
  return;
#else
  auto& sources = m_model.m_sources;
  if (sourceId < 0 || sourceId >= static_cast<int>(sources.size()))
    return;

  DataModel::Source copy  = sources[sourceId];
  copy.sourceId           = static_cast<int>(sources.size());
  copy.connectionSettings = QJsonObject();

  QStringList existingTitles;
  existingTitles.reserve(static_cast<int>(sources.size()));
  for (const auto& s : sources)
    existingTitles.append(s.title);

  copy.title = nextDuplicateTitle(sources[sourceId].title, existingTitles);

  sources.push_back(copy);
  m_model.setModified(true);
  Q_EMIT m_model.sourcesChanged();
  Q_EMIT m_model.sourceStructureChanged();
  Q_EMIT m_model.sourceAdded(copy.sourceId);
#endif
}

/**
 * @brief Updates the source with the given @p sourceId.
 */
void DataModel::ProjectSources::updateSource(int sourceId,
                                             const DataModel::Source& source,
                                             const bool rebuildTree)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Edit Device")};
  auto& sources = m_model.m_sources;
  if (sourceId < 0 || sourceId >= static_cast<int>(sources.size()))
    return;

  sources[sourceId]          = source;
  sources[sourceId].sourceId = sourceId;

  if (sourceId == 0) {
    m_model.m_frameStartSequence    = source.frameStart;
    m_model.m_frameEndSequence      = source.frameEnd;
    m_model.m_checksumAlgorithm     = source.checksumAlgorithm;
    m_model.m_hexadecimalDelimiters = source.hexadecimalDelimiters;
    m_model.m_frameDetection = static_cast<SerialStudio::FrameDetection>(source.frameDetection);
    m_model.m_frameDecoder   = static_cast<SerialStudio::DecoderMethod>(source.decoderMethod);
    Q_EMIT m_model.frameDetectionChanged();
  }

  m_model.setModified(true);
  if (rebuildTree)
    Q_EMIT m_model.sourcesChanged();

  Q_EMIT m_model.sourceChanged(sourceId);
}

/**
 * @brief Updates the title of the source with the given @p sourceId.
 */
void DataModel::ProjectSources::updateSourceTitle(int sourceId,
                                                  const QString& title,
                                                  const bool rebuildTree)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Rename Device")};
  auto& sources = m_model.m_sources;
  if (sourceId < 0 || sourceId >= static_cast<int>(sources.size()))
    return;

  sources[sourceId].title = title.simplified();
  m_model.setModified(true);
  if (rebuildTree)
    Q_EMIT m_model.sourcesChanged();
}

/**
 * @brief Updates the bus type of the source with the given @p sourceId.
 */
void DataModel::ProjectSources::updateSourceBusType(int sourceId, int busType)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Change Bus Type")};
  auto& sources = m_model.m_sources;
  if (sourceId < 0 || sourceId >= static_cast<int>(sources.size()))
    return;

  sources[sourceId].busType = busType;
  m_model.setModified(true);
  Q_EMIT m_model.sourcesChanged();
  Q_EMIT m_model.sourceStructureChanged();
}

/**
 * @brief Prompts for a new title and applies it to the source at @p sourceId.
 */
void DataModel::ProjectSources::promptRenameSource(int sourceId)
{
  const Source* src = nullptr;
  for (const auto& s : m_model.m_sources)
    if (s.sourceId == sourceId) {
      src = &s;
      break;
    }
  if (!src)
    return;

  bool ok          = false;
  const auto old   = src->title;
  const auto fresh = QInputDialog::getText(nullptr,
                                           ProjectModel::tr("Rename Data Source"),
                                           ProjectModel::tr("Name:"),
                                           QLineEdit::Normal,
                                           old,
                                           &ok)
                       .trimmed();
  if (!ok || fresh.isEmpty() || fresh == old)
    return;

  updateSourceTitle(sourceId, fresh, true);
}

//--------------------------------------------------------------------------------------------------
// Driver connection settings
//--------------------------------------------------------------------------------------------------

/**
 * @brief Snapshots the current driver settings for source @p sourceId into
 *        Source::connectionSettings. Password-typed properties are skipped: the project file is
 *        shared and version-controlled, so secrets stay in the driver's own credential vault.
 */
void DataModel::ProjectSources::captureSourceSettings(int sourceId)
{
  auto& sources = m_model.m_sources;
  if (sourceId < 0 || sourceId >= static_cast<int>(sources.size()))
    return;

  const auto busType     = static_cast<SerialStudio::BusType>(sources[sourceId].busType);
  static auto& ioManager = IO::ConnectionManager::instance();
  IO::HAL_Driver* driver = ioManager.uiDriverForBusType(busType);
  if (!driver)
    return;

  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Edit Device")};
  QJsonObject settings;
  for (const auto& prop : driver->driverProperties()) {
    if (prop.type == IO::DriverProperty::Password)
      continue;

    settings.insert(prop.key, QJsonValue::fromVariant(prop.value));
  }

  const auto deviceId = driver->deviceIdentifier();
  if (!deviceId.isEmpty())
    settings.insert(QStringLiteral("deviceId"), deviceId);

  sources[sourceId].connectionSettings = settings;
  m_model.setModified(true);
}

/**
 * @brief Applies the source's saved connectionSettings to its live driver. Reads the document and
 *        writes only the driver, so it opens no ProjectUndoScope: capturing a whole-document
 *        snapshot per call cost a serialization the history could never commit.
 */
void DataModel::ProjectSources::restoreSourceSettings(int sourceId)
{
  const auto& sources = m_model.m_sources;
  if (sourceId < 0 || sourceId >= static_cast<int>(sources.size()))
    return;

  const auto& source = sources[sourceId];
  if (source.connectionSettings.isEmpty())
    return;

  static auto& ioManager = IO::ConnectionManager::instance();
  IO::HAL_Driver* driver = ioManager.driverForEditing(sourceId);
  if (!driver)
    return;

  driver->applyConnectionSettings(source.connectionSettings);
}

/**
 * @brief Overwrites source[0].connectionSettings from the Setup pane mirror. Emits only
 *        sourceConnectionChanged: sourcesChanged would rebuild the editor tree per keystroke and
 *        sourceStructureChanged would rebuild the devices the mirror is reading from.
 */
void DataModel::ProjectSources::setSource0ConnectionSettings(const QJsonObject& settings)
{
  auto& sources = m_model.m_sources;
  if (sources.empty() || sources[0].connectionSettings == settings)
    return;

  const ProjectUndoScope undo_scope{
    m_model, ProjectModel::tr("Edit Device"), QStringLiteral("connection-settings:0")};
  sources[0].connectionSettings = settings;
  m_model.setModified(true);
  Q_EMIT m_model.sourceConnectionChanged(0);
}

/**
 * @brief Sets source[0].busType from the Setup pane mirror. Emits only sourceConnectionChanged
 *        (see setSource0ConnectionSettings); sourceStructureChanged would rebuild the devices.
 */
void DataModel::ProjectSources::setSource0BusType(int busType)
{
  auto& sources = m_model.m_sources;
  if (sources.empty() || sources[0].busType == busType)
    return;

  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Change Bus Type")};
  sources[0].busType = busType;
  m_model.setModified(true);
  Q_EMIT m_model.sourceConnectionChanged(0);
}

//--------------------------------------------------------------------------------------------------
// Frame parser settings
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets source[0].frameParserCode and emits frameParserCodeChanged.
 */
void DataModel::ProjectSources::setFrameParserCode(const QString& code)
{
  auto& sources = m_model.m_sources;
  if (sources.empty() || code == sources[0].frameParserCode)
    return;

  const ProjectUndoScope undo_scope{
    m_model, ProjectModel::tr("Edit Frame Parser"), QStringLiteral("parser-code:0")};
  sources[0].frameParserCode = code;
  m_model.setModified(true);
  Q_EMIT m_model.frameParserCodeChanged();
  Q_EMIT m_model.sourceFrameParserCodeChanged(0);
}

/**
 * @brief Sets the scripting language for the global frame parser (source 0).
 */
void DataModel::ProjectSources::setFrameParserLanguage(int language)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Change Parser Language")};
  auto& sources = m_model.m_sources;
  if (sources.empty() || language == sources[0].frameParserLanguage)
    return;

  sources[0].frameParserLanguage = language;
  m_model.setModified(true);
  Q_EMIT m_model.frameParserLanguageChanged();
  Q_EMIT m_model.sourceFrameParserLanguageChanged(0);
}

/**
 * @brief Sets the native parser template id for the global frame parser (source 0).
 */
void DataModel::ProjectSources::setFrameParserTemplate(const QString& templateId)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Change Parser Template")};
  if (m_model.m_sources.empty())
    return;

  updateSourceFrameParserTemplate(m_model.m_sources[0].sourceId, templateId);
}

/**
 * @brief Sets the native parser template params for the global frame parser (source 0).
 */
void DataModel::ProjectSources::setFrameParserParams(const QJsonObject& params)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Change Parser Parameters")};
  if (m_model.m_sources.empty())
    return;

  updateSourceFrameParserParams(m_model.m_sources[0].sourceId, params);
}

/**
 * @brief Updates the per-source JavaScript frame parser code.
 */
void DataModel::ProjectSources::updateSourceFrameParser(int sourceId, const QString& code)
{
  auto& sources = m_model.m_sources;
  if (sourceId < 0 || sourceId >= static_cast<int>(sources.size()))
    return;

  if (sources[sourceId].frameParserCode == code)
    return;

  const ProjectUndoScope undo_scope{
    m_model, ProjectModel::tr("Edit Frame Parser"), QStringLiteral("parser-code:%1").arg(sourceId)};
  sources[sourceId].frameParserCode = code;
  static auto& parser               = DataModel::FrameParser::instance();
  parser.setSourceCode(sourceId, code);
  m_model.setModified(true);

  Q_EMIT m_model.sourceFrameParserCodeChanged(sourceId);
}

/**
 * @brief Sets the scripting language for the source with the given sourceId.
 */
void DataModel::ProjectSources::updateSourceFrameParserLanguage(int sourceId, int language)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Change Parser Language")};
  auto& sources = m_model.m_sources;
  auto it = std::find_if(sources.begin(), sources.end(), [sourceId](const DataModel::Source& src) {
    return src.sourceId == sourceId;
  });

  if (it == sources.end())
    return;

  if (it->frameParserLanguage == language)
    return;

  it->frameParserLanguage = language;
  m_model.setModified(true);

  if (sourceId == 0)
    Q_EMIT m_model.frameParserLanguageChanged();

  Q_EMIT m_model.sourceFrameParserLanguageChanged(sourceId);
}

/**
 * @brief Sets the native parser template id for the source with the given sourceId.
 */
void DataModel::ProjectSources::updateSourceFrameParserTemplate(int sourceId,
                                                                const QString& templateId)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Change Parser Template")};
  auto& sources = m_model.m_sources;
  auto it = std::find_if(sources.begin(), sources.end(), [sourceId](const DataModel::Source& src) {
    return src.sourceId == sourceId;
  });

  if (it == sources.end() || it->frameParserTemplate == templateId)
    return;

  it->frameParserTemplate = templateId;
  m_model.setModified(true);

  if (sourceId == 0)
    Q_EMIT m_model.frameParserTemplateChanged();

  Q_EMIT m_model.sourceFrameParserTemplateChanged(sourceId);
}

/**
 * @brief Sets the native parser template params for the source with the given sourceId.
 */
void DataModel::ProjectSources::updateSourceFrameParserParams(int sourceId,
                                                              const QJsonObject& params)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Change Parser Parameters")};
  auto& sources = m_model.m_sources;
  auto it = std::find_if(sources.begin(), sources.end(), [sourceId](const DataModel::Source& src) {
    return src.sourceId == sourceId;
  });

  if (it == sources.end() || it->frameParserParams == params)
    return;

  it->frameParserParams = params;
  m_model.setModified(true);

  if (sourceId == 0)
    Q_EMIT m_model.frameParserParamsChanged();

  Q_EMIT m_model.sourceFrameParserParamsChanged(sourceId);
}

/**
 * @brief Applies a native template and its parameter defaults as ONE document mutation. Picking a
 *        template writes both fields; two scopes recorded two undo steps, so a single Ctrl+Z left
 *        the new parameters sitting on the previous template id and the native parser rebuilt
 *        against a pair that never existed.
 */
void DataModel::ProjectSources::setSourceFrameParserTemplateAndParams(int sourceId,
                                                                      const QString& templateId,
                                                                      const QJsonObject& params)
{
  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Change Parser Template")};
  updateSourceFrameParserParams(sourceId, params);
  updateSourceFrameParserTemplate(sourceId, templateId);
}

/**
 * @brief Sets the stream-lane override for the source with the given sourceId: "" or "auto"
 *        (stored as absent -- the driver decides), "on", or "off" (spec 0051 R6). Unknown
 *        values are rejected so a typo can never silently kill a source's data path.
 */
void DataModel::ProjectSources::updateSourceStreamLane(int sourceId, const QString& lane)
{
  const QString effective = (lane == QLatin1String("auto")) ? QString() : lane;
  if (!effective.isEmpty() && effective != QLatin1String("on") && effective != QLatin1String("off"))
    return;

  const ProjectUndoScope undo_scope{m_model, ProjectModel::tr("Change Stream Lane")};
  auto& sources = m_model.m_sources;
  auto it = std::find_if(sources.begin(), sources.end(), [sourceId](const DataModel::Source& src) {
    return src.sourceId == sourceId;
  });

  if (it == sources.end() || it->streamLane == effective)
    return;

  it->streamLane = effective;
  m_model.setModified(true);

  Q_EMIT m_model.sourceStreamLaneChanged(sourceId);
  Q_EMIT m_model.sourceChanged(sourceId);
}

/**
 * @brief Stores frame parser code without emitting signals or reloading the JS engine.
 */
void DataModel::ProjectSources::storeFrameParserCode(int sourceId, const QString& code)
{
  auto& sources = m_model.m_sources;
  if (sourceId < 0 || sourceId >= static_cast<int>(sources.size()))
    return;

  if (sources[sourceId].frameParserCode == code)
    return;

  const ProjectUndoScope undo_scope{
    m_model, ProjectModel::tr("Edit Frame Parser"), QStringLiteral("parser-code:%1").arg(sourceId)};
  sources[sourceId].frameParserCode = code;
  m_model.setModified(true);
}
