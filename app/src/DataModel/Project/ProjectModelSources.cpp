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
#include <QApplication>
#include <QCryptographicHash>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QHash>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJSValue>
#include <QMessageBox>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QTimer>

#include "AppInfo.h"
#include "AppState.h"
#include "DataModel/Editors/OutputCodeEditor.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "DataModel/Scripting/ControlScript.h"
#include "DataModel/Scripting/FrameParser.h"
#include "DataModel/Scripting/NativeTemplates/NativeTemplate.h"
#include "DataModel/Scripting/ScriptApiCall.h"
#include "IO/Checksum.h"
#include "IO/ConnectionManager.h"
#include "Misc/IconEngine.h"
#include "Misc/JsonValidator.h"
#include "Misc/PasswordHash.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "Misc/WorkspaceManager.h"
#include "ProjectModelShared.h"
#include "UI/Dashboard.h"

/**
 * @brief Adds a new source to the project (GPL: capped to one source).
 */
void DataModel::ProjectModel::addSource()
{
  const ProjectUndoScope undo_scope{*this, tr("Add Device")};
#ifndef BUILD_COMMERCIAL
  if (!m_sources.empty()) {
    if (!m_suppressMessageBoxes)
      Misc::Utilities::showMessageBox(
        tr("Multiple data sources require a Pro license"),
        tr("Serial Studio Pro allows connecting to multiple devices simultaneously. "
           "Please upgrade to unlock this feature."),
        QMessageBox::Information);

    return;
  }
#endif

  const int newId = static_cast<int>(m_sources.size());

  DataModel::Source source;
  source.sourceId              = newId;
  source.title                 = tr("Device %1").arg(QChar('A' + newId));
  source.busType               = static_cast<int>(SerialStudio::BusType::UART);
  source.frameStart            = m_frameStartSequence;
  source.frameEnd              = m_frameEndSequence;
  source.checksumAlgorithm     = m_checksumAlgorithm;
  source.frameDetection        = static_cast<int>(m_frameDetection);
  source.decoderMethod         = static_cast<int>(m_frameDecoder);
  source.hexadecimalDelimiters = m_hexadecimalDelimiters;
  seedDefaultFrameParser(source);

  m_sources.push_back(source);
  setModified(true);
  Q_EMIT sourcesChanged();
  Q_EMIT sourceStructureChanged();
  Q_EMIT sourceAdded(newId);
}

/**
 * @brief Deletes the source and reassigns dependent groups to source 0.
 */
void DataModel::ProjectModel::deleteSource(int sourceId, bool confirm)
{
#ifndef BUILD_COMMERCIAL
  (void)sourceId;
  (void)confirm;
  return;
#else
  if (sourceId <= 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  if (confirm && !m_suppressMessageBoxes) {
    const auto ret = Misc::Utilities::showMessageBox(
      tr("Do you want to delete data source \"%1\"?").arg(m_sources[sourceId].title),
      tr("Groups using this source will move to the default source. "
         "This action cannot be undone."),
      QMessageBox::Question,
      APP_NAME,
      QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes)
      return;
  }

  const ProjectUndoScope undo_scope{*this, tr("Delete Device")};
  m_sources.erase(m_sources.begin() + sourceId);

  const auto remapSourceId = [sourceId](int& id) {
    if (id == sourceId)
      id = 0;
    else if (id > sourceId)
      --id;
  };

  for (auto& group : m_groups) {
    remapSourceId(group.sourceId);
    for (auto& widget : group.outputWidgets)
      remapSourceId(widget.sourceId);
  }

  for (auto& action : m_actions)
    remapSourceId(action.sourceId);

  for (size_t i = 0; i < m_sources.size(); ++i)
    m_sources[i].sourceId = static_cast<int>(i);

  setModified(true);
  Q_EMIT groupsChanged();
  Q_EMIT sourcesChanged();
  Q_EMIT sourceStructureChanged();
  Q_EMIT sourceDeleted();
#endif
}

/**
 * @brief Duplicates the source with the given @p sourceId.
 */
void DataModel::ProjectModel::duplicateSource(int sourceId)
{
  const ProjectUndoScope undo_scope{*this, tr("Duplicate Device")};
#ifndef BUILD_COMMERCIAL
  (void)sourceId;
  return;
#else
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  DataModel::Source copy  = m_sources[sourceId];
  copy.sourceId           = static_cast<int>(m_sources.size());
  copy.connectionSettings = QJsonObject();

  QStringList existingTitles;
  existingTitles.reserve(static_cast<int>(m_sources.size()));
  for (const auto& s : m_sources)
    existingTitles.append(s.title);

  copy.title = nextDuplicateTitle(m_sources[sourceId].title, existingTitles);

  m_sources.push_back(copy);
  setModified(true);
  Q_EMIT sourcesChanged();
  Q_EMIT sourceStructureChanged();
  Q_EMIT sourceAdded(copy.sourceId);
#endif
}

/**
 * @brief Updates the source with the given @p sourceId.
 */
void DataModel::ProjectModel::updateSource(int sourceId,
                                           const DataModel::Source& source,
                                           const bool rebuildTree)
{
  const ProjectUndoScope undo_scope{*this, tr("Edit Device")};
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  m_sources[sourceId]          = source;
  m_sources[sourceId].sourceId = sourceId;

  if (sourceId == 0) {
    m_frameStartSequence    = source.frameStart;
    m_frameEndSequence      = source.frameEnd;
    m_checksumAlgorithm     = source.checksumAlgorithm;
    m_hexadecimalDelimiters = source.hexadecimalDelimiters;
    m_frameDetection        = static_cast<SerialStudio::FrameDetection>(source.frameDetection);
    m_frameDecoder          = static_cast<SerialStudio::DecoderMethod>(source.decoderMethod);
    Q_EMIT frameDetectionChanged();
  }

  setModified(true);
  if (rebuildTree)
    Q_EMIT sourcesChanged();

  Q_EMIT sourceChanged(sourceId);
}

/**
 * @brief Updates the title of the source with the given @p sourceId.
 */
void DataModel::ProjectModel::updateSourceTitle(int sourceId,
                                                const QString& title,
                                                const bool rebuildTree)
{
  const ProjectUndoScope undo_scope{*this, tr("Rename Device")};
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  m_sources[sourceId].title = title.simplified();
  setModified(true);
  if (rebuildTree)
    Q_EMIT sourcesChanged();
}

/**
 * @brief Updates the bus type of the source with the given @p sourceId.
 */
void DataModel::ProjectModel::updateSourceBusType(int sourceId, int busType)
{
  const ProjectUndoScope undo_scope{*this, tr("Change Bus Type")};
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  m_sources[sourceId].busType = busType;
  setModified(true);
  Q_EMIT sourcesChanged();
  Q_EMIT sourceStructureChanged();
}

/**
 * @brief Updates the per-source JavaScript frame parser code.
 */
void DataModel::ProjectModel::updateSourceFrameParser(int sourceId, const QString& code)
{
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  if (m_sources[sourceId].frameParserCode == code)
    return;

  const ProjectUndoScope undo_scope{
    *this, tr("Edit Frame Parser"), QStringLiteral("parser-code:%1").arg(sourceId)};
  m_sources[sourceId].frameParserCode = code;
  static auto& parser                 = DataModel::FrameParser::instance();
  parser.setSourceCode(sourceId, code);
  setModified(true);

  Q_EMIT sourceFrameParserCodeChanged(sourceId);
}

/**
 * @brief Snapshots the current driver settings for source @p sourceId into
 *        Source::connectionSettings. Password-typed properties are skipped: the project file is
 *        shared and version-controlled, so secrets stay in the driver's own credential vault.
 */
void DataModel::ProjectModel::captureSourceSettings(int sourceId)
{
  const ProjectUndoScope undo_scope{*this, tr("Edit Device")};
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  const auto busType     = static_cast<SerialStudio::BusType>(m_sources[sourceId].busType);
  static auto& ioManager = IO::ConnectionManager::instance();
  IO::HAL_Driver* driver = ioManager.uiDriverForBusType(busType);
  if (!driver)
    return;

  QJsonObject settings;
  for (const auto& prop : driver->driverProperties()) {
    if (prop.type == IO::DriverProperty::Password)
      continue;

    settings.insert(prop.key, QJsonValue::fromVariant(prop.value));
  }

  const auto deviceId = driver->deviceIdentifier();
  if (!deviceId.isEmpty())
    settings.insert(QStringLiteral("deviceId"), deviceId);

  m_sources[sourceId].connectionSettings = settings;
  setModified(true);
}

/**
 * @brief Applies the source's saved connectionSettings to its live driver.
 */
void DataModel::ProjectModel::restoreSourceSettings(int sourceId)
{
  const ProjectUndoScope undo_scope{*this, tr("Edit Device")};
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  const auto& source = m_sources[sourceId];
  if (source.connectionSettings.isEmpty())
    return;

  static auto& ioManager = IO::ConnectionManager::instance();
  IO::HAL_Driver* driver = ioManager.driverForEditing(sourceId);
  if (!driver)
    return;

  driver->applyConnectionSettings(source.connectionSettings);
}

/**
 * @brief Overwrites source[0].connectionSettings without emitting sourcesChanged.
 */
void DataModel::ProjectModel::setSource0ConnectionSettings(const QJsonObject& settings)
{
  if (m_sources.empty() || m_sources[0].connectionSettings == settings)
    return;

  const ProjectUndoScope undo_scope{
    *this, tr("Edit Device"), QStringLiteral("connection-settings:0")};
  m_sources[0].connectionSettings = settings;
  setModified(true);
}

/**
 * @brief Sets source[0].busType without emitting sourceStructureChanged.
 */
void DataModel::ProjectModel::setSource0BusType(int busType)
{
  if (m_sources.empty() || m_sources[0].busType == busType)
    return;

  const ProjectUndoScope undo_scope{*this, tr("Change Bus Type")};
  m_sources[0].busType = busType;
  setModified(true);
}

/**
 * @brief Sets source[0].frameParserCode and emits frameParserCodeChanged.
 */
void DataModel::ProjectModel::setFrameParserCode(const QString& code)
{
  if (m_sources.empty() || code == m_sources[0].frameParserCode)
    return;

  const ProjectUndoScope undo_scope{
    *this, tr("Edit Frame Parser"), QStringLiteral("parser-code:0")};
  m_sources[0].frameParserCode = code;
  setModified(true);
  Q_EMIT frameParserCodeChanged();
  Q_EMIT sourceFrameParserCodeChanged(0);
}

/**
 * @brief Sets the scripting language for the global frame parser (source 0).
 */
void DataModel::ProjectModel::setFrameParserLanguage(int language)
{
  const ProjectUndoScope undo_scope{*this, tr("Change Parser Language")};
  if (m_sources.empty() || language == m_sources[0].frameParserLanguage)
    return;

  m_sources[0].frameParserLanguage = language;
  setModified(true);
  Q_EMIT frameParserLanguageChanged();
  Q_EMIT sourceFrameParserLanguageChanged(0);
}

/**
 * @brief Sets the scripting language for the source with the given sourceId.
 */
void DataModel::ProjectModel::updateSourceFrameParserLanguage(int sourceId, int language)
{
  const ProjectUndoScope undo_scope{*this, tr("Change Parser Language")};
  auto it =
    std::find_if(m_sources.begin(), m_sources.end(), [sourceId](const DataModel::Source& src) {
      return src.sourceId == sourceId;
    });

  if (it == m_sources.end())
    return;

  if (it->frameParserLanguage == language)
    return;

  it->frameParserLanguage = language;
  setModified(true);

  if (sourceId == 0)
    Q_EMIT frameParserLanguageChanged();

  Q_EMIT sourceFrameParserLanguageChanged(sourceId);
}

/**
 * @brief Sets the native parser template id for the global frame parser (source 0).
 */
void DataModel::ProjectModel::setFrameParserTemplate(const QString& templateId)
{
  const ProjectUndoScope undo_scope{*this, tr("Change Parser Template")};
  if (m_sources.empty())
    return;

  updateSourceFrameParserTemplate(m_sources[0].sourceId, templateId);
}

/**
 * @brief Sets the native parser template params for the global frame parser (source 0).
 */
void DataModel::ProjectModel::setFrameParserParams(const QJsonObject& params)
{
  const ProjectUndoScope undo_scope{*this, tr("Change Parser Parameters")};
  if (m_sources.empty())
    return;

  updateSourceFrameParserParams(m_sources[0].sourceId, params);
}

/**
 * @brief Sets the native parser template id for the source with the given sourceId.
 */
void DataModel::ProjectModel::updateSourceFrameParserTemplate(int sourceId,
                                                              const QString& templateId)
{
  const ProjectUndoScope undo_scope{*this, tr("Change Parser Template")};
  auto it =
    std::find_if(m_sources.begin(), m_sources.end(), [sourceId](const DataModel::Source& src) {
      return src.sourceId == sourceId;
    });

  if (it == m_sources.end() || it->frameParserTemplate == templateId)
    return;

  it->frameParserTemplate = templateId;
  setModified(true);

  if (sourceId == 0)
    Q_EMIT frameParserTemplateChanged();

  Q_EMIT sourceFrameParserTemplateChanged(sourceId);
}

/**
 * @brief Sets the native parser template params for the source with the given sourceId.
 */
void DataModel::ProjectModel::updateSourceFrameParserParams(int sourceId, const QJsonObject& params)
{
  const ProjectUndoScope undo_scope{*this, tr("Change Parser Parameters")};
  auto it =
    std::find_if(m_sources.begin(), m_sources.end(), [sourceId](const DataModel::Source& src) {
      return src.sourceId == sourceId;
    });

  if (it == m_sources.end() || it->frameParserParams == params)
    return;

  it->frameParserParams = params;
  setModified(true);

  if (sourceId == 0)
    Q_EMIT frameParserParamsChanged();

  Q_EMIT sourceFrameParserParamsChanged(sourceId);
}

/**
 * @brief Sets the stream-lane override for the source with the given sourceId: "" or "auto"
 *        (stored as absent -- the driver decides), "on", or "off" (spec 0051 R6). Unknown
 *        values are rejected so a typo can never silently kill a source's data path.
 */
void DataModel::ProjectModel::updateSourceStreamLane(int sourceId, const QString& lane)
{
  const QString effective = (lane == QLatin1String("auto")) ? QString() : lane;
  if (!effective.isEmpty() && effective != QLatin1String("on") && effective != QLatin1String("off"))
    return;

  const ProjectUndoScope undo_scope{*this, tr("Change Stream Lane")};
  auto it =
    std::find_if(m_sources.begin(), m_sources.end(), [sourceId](const DataModel::Source& src) {
      return src.sourceId == sourceId;
    });

  if (it == m_sources.end() || it->streamLane == effective)
    return;

  it->streamLane = effective;
  setModified(true);

  Q_EMIT sourceStreamLaneChanged(sourceId);
  Q_EMIT sourceChanged(sourceId);
}

/**
 * @brief Stores frame parser code without emitting signals or reloading the JS engine.
 */
void DataModel::ProjectModel::storeFrameParserCode(int sourceId, const QString& code)
{
  if (sourceId < 0 || sourceId >= static_cast<int>(m_sources.size()))
    return;

  if (m_sources[sourceId].frameParserCode == code)
    return;

  const ProjectUndoScope undo_scope{
    *this, tr("Edit Frame Parser"), QStringLiteral("parser-code:%1").arg(sourceId)};
  m_sources[sourceId].frameParserCode = code;
  setModified(true);
}
