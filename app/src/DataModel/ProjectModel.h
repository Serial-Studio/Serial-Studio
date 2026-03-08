/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#pragma once

#include <QJsonObject>
#include <QObject>

#include "DataModel/Frame.h"
#include "SerialStudio.h"

namespace DataModel {

/**
 * @brief Pure data model for Serial Studio project configuration.
 *
 * Owns the project title, frame detection settings, frame parser code,
 * groups, actions, widget settings, and all file I/O. Does not own any
 * Qt item models or UI state — those live in ProjectEditor.
 */
class ProjectModel : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool modified
             READ modified
             NOTIFY modifiedChanged)
  Q_PROPERTY(QString title
             READ title
             NOTIFY titleChanged)
  Q_PROPERTY(QString jsonFilePath
             READ jsonFilePath
             NOTIFY jsonFileChanged)
  Q_PROPERTY(QString jsonFileName
             READ jsonFileName
             NOTIFY jsonFileChanged)
  Q_PROPERTY(int groupCount
             READ groupCount
             NOTIFY groupsChanged)
  Q_PROPERTY(int datasetCount
             READ datasetCount
             NOTIFY groupsChanged)
  Q_PROPERTY(bool containsCommercialFeatures
             READ containsCommercialFeatures
             NOTIFY groupsChanged)
  // clang-format on

signals:
  void titleChanged();
  void jsonFileChanged();
  void modifiedChanged();
  void groupsChanged();
  void actionsChanged();
  void frameDetectionChanged();
  void frameParserCodeChanged();
  void activeGroupIdChanged();
  void widgetSettingsChanged();

  void groupAdded(int groupId);
  void groupDeleted();
  void datasetAdded(int groupId, int datasetId);
  void datasetDeleted(int survivingGroupId);
  void actionAdded(int actionId);
  void actionDeleted();

private:
  explicit ProjectModel();
  ProjectModel(ProjectModel&&)                 = delete;
  ProjectModel(const ProjectModel&)            = delete;
  ProjectModel& operator=(ProjectModel&&)      = delete;
  ProjectModel& operator=(const ProjectModel&) = delete;

public:
  static ProjectModel& instance();

  [[nodiscard]] bool modified() const;
  [[nodiscard]] SerialStudio::DecoderMethod decoderMethod() const;
  [[nodiscard]] SerialStudio::FrameDetection frameDetection() const;

  [[nodiscard]] QString jsonFileName() const;
  [[nodiscard]] QString jsonProjectsPath() const;

  [[nodiscard]] QStringList xDataSources() const;

  [[nodiscard]] const QString& title() const;
  [[nodiscard]] const QString& jsonFilePath() const;
  [[nodiscard]] const QString& frameParserCode() const;

  [[nodiscard]] bool suppressMessageBoxes() const;

  [[nodiscard]] int activeGroupId() const;
  [[nodiscard]] QJsonObject groupLayout(int groupId) const;

  [[nodiscard]] bool containsCommercialFeatures() const;

  [[nodiscard]] int groupCount() const;
  [[nodiscard]] int datasetCount() const;
  [[nodiscard]] const std::vector<Group>& groups() const;
  [[nodiscard]] const std::vector<Action>& actions() const;

  Q_INVOKABLE bool askSave();
  Q_INVOKABLE QJsonObject serializeToJson() const;
  Q_INVOKABLE bool saveJsonFile(const bool askPath = false);
  Q_INVOKABLE QJsonObject widgetSettings(const QString& widgetId) const;
  Q_INVOKABLE void saveWidgetSetting(const QString& widgetId,
                                     const QString& key,
                                     const QVariant& value);

public slots:
  void setupExternalConnections();
  void setSuppressMessageBoxes(const bool suppress);

  void newJsonFile();
  void openJsonFile();
  void openJsonFile(const QString& path);

  void setTitle(const QString& title);
  void clearJsonFilePath();

  void setFrameStartSequence(const QString& sequence);
  void setFrameEndSequence(const QString& sequence);
  void setChecksumAlgorithm(const QString& algorithm);
  void setFrameDetection(const SerialStudio::FrameDetection detection);

  void enableProjectMode();

  void deleteCurrentGroup();
  void deleteCurrentAction();
  void deleteCurrentDataset();

  void duplicateCurrentGroup();
  void duplicateCurrentAction();
  void duplicateCurrentDataset();

  void ensureValidGroup();
  void addDataset(const SerialStudio::DatasetOption options);
  void changeDatasetOption(const SerialStudio::DatasetOption option, const bool checked);

  void addAction();
  void addGroup(const QString& title, const SerialStudio::GroupWidget widget);
  bool setGroupWidget(const int group, const SerialStudio::GroupWidget widget);

  void setModified(const bool modified);
  void setFrameParserCode(const QString& code);
  void setActiveGroupId(const int groupId);
  void setGroupLayout(const int groupId, const QJsonObject& layout);
  void setDecoderMethod(const SerialStudio::DecoderMethod method);
  void setHexadecimalDelimiters(const bool hexadecimal);

  void updateGroup(const int groupId, const DataModel::Group& group, const bool rebuildTree = true);
  void updateAction(const int actionId, const DataModel::Action& action);
  void updateDataset(const int groupId,
                     const int datasetId,
                     const DataModel::Dataset& dataset,
                     const bool rebuildTree = false);

  void setSelectedGroup(const DataModel::Group& group);
  void setSelectedAction(const DataModel::Action& action);
  void setSelectedDataset(const DataModel::Dataset& dataset);

private slots:
  void onJsonLoaded();

private:
  int nextDatasetIndex();
  bool finalizeProjectSave();

private:
  QString m_title;
  QString m_frameParserCode;
  QString m_frameEndSequence;
  QString m_checksumAlgorithm;
  QString m_frameStartSequence;
  bool m_hexadecimalDelimiters;

  SerialStudio::DecoderMethod m_frameDecoder;
  SerialStudio::FrameDetection m_frameDetection;

  bool m_modified;
  QString m_filePath;
  bool m_suppressMessageBoxes;

  std::vector<DataModel::Group> m_groups;
  std::vector<DataModel::Action> m_actions;

  DataModel::Group m_selectedGroup;
  DataModel::Action m_selectedAction;
  DataModel::Dataset m_selectedDataset;

  QJsonObject m_widgetSettings;
};
}  // namespace DataModel
