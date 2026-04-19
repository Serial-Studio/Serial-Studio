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
#include <QSet>
#include <QVariantList>

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
  Q_PROPERTY(int sourceCount
             READ sourceCount
             NOTIFY sourcesChanged)
  Q_PROPERTY(int workspaceCount
             READ workspaceCount
             NOTIFY workspacesChanged)
  Q_PROPERTY(int pointCount
             READ pointCount
             NOTIFY pointCountChanged)
  Q_PROPERTY(int frameParserLanguage
             READ  frameParserLanguage
             WRITE setFrameParserLanguage
             NOTIFY frameParserLanguageChanged)
  Q_PROPERTY(int tableCount
             READ tableCount
             NOTIFY tablesChanged)
  Q_PROPERTY(bool customizeWorkspaces
             READ  customizeWorkspaces
             WRITE setCustomizeWorkspaces
             NOTIFY customizeWorkspacesChanged)
  // clang-format on

signals:
  void titleChanged();
  void pointCountChanged();
  void jsonFileChanged();
  void modifiedChanged();
  void groupsChanged();
  void groupDataChanged();
  void actionsChanged();
  void sourcesChanged();
  void sourceStructureChanged();
  void frameDetectionChanged();
  void frameParserCodeChanged();
  void frameParserLanguageChanged();
  void sourceFrameParserCodeChanged(int sourceId);
  void sourceFrameParserLanguageChanged(int sourceId);
  void activeGroupIdChanged();
  void widgetSettingsChanged();
  void workspacesChanged();
  void tablesChanged();
  void customizeWorkspacesChanged();

  void groupAdded(int groupId);
  void groupDeleted();
  void datasetAdded(int groupId, int datasetId);
  void datasetDeleted(int survivingGroupId);
  void actionAdded(int actionId);
  void actionDeleted();
  void sourceAdded(int sourceId);
  void sourceDeleted();
  void outputWidgetAdded(int groupId, int widgetId);
  void outputWidgetDeleted(int groupId);

private:
  explicit ProjectModel();
  ProjectModel(ProjectModel&&)                 = delete;
  ProjectModel(const ProjectModel&)            = delete;
  ProjectModel& operator=(ProjectModel&&)      = delete;
  ProjectModel& operator=(const ProjectModel&) = delete;

public:
  [[nodiscard]] static ProjectModel& instance();

  [[nodiscard]] bool modified() const noexcept;
  [[nodiscard]] SerialStudio::DecoderMethod decoderMethod() const noexcept;
  [[nodiscard]] SerialStudio::FrameDetection frameDetection() const noexcept;

  [[nodiscard]] QString jsonFileName() const;
  [[nodiscard]] QString jsonProjectsPath() const;

  [[nodiscard]] QStringList xDataSources() const;

  [[nodiscard]] const QString& title() const noexcept;
  [[nodiscard]] const QString& jsonFilePath() const noexcept;
  [[nodiscard]] int frameParserLanguage() const;
  [[nodiscard]] int frameParserLanguage(int sourceId) const;
  [[nodiscard]] QString frameParserCode() const;

  [[nodiscard]] bool suppressMessageBoxes() const noexcept;

  [[nodiscard]] int activeGroupId() const;
  [[nodiscard]] QJsonObject groupLayout(int groupId) const;

  [[nodiscard]] bool containsCommercialFeatures() const;

  [[nodiscard]] int pointCount() const noexcept;
  [[nodiscard]] int groupCount() const noexcept;
  [[nodiscard]] int datasetCount() const;
  [[nodiscard]] int sourceCount() const noexcept;
  [[nodiscard]] const std::vector<Group>& groups() const noexcept;
  [[nodiscard]] const std::vector<Action>& actions() const noexcept;
  [[nodiscard]] const std::vector<Source>& sources() const noexcept;
  [[nodiscard]] const std::vector<Workspace>& workspaces() const;
  [[nodiscard]] const QSet<int>& hiddenGroupIds() const noexcept;
  [[nodiscard]] int workspaceCount() const noexcept;
  [[nodiscard]] bool isGroupHidden(int groupId) const;

  [[nodiscard]] int tableCount() const noexcept;
  [[nodiscard]] bool customizeWorkspaces() const noexcept;
  [[nodiscard]] const std::vector<TableDef>& tables() const noexcept;

  Q_INVOKABLE [[nodiscard]] bool askSave();
  Q_INVOKABLE [[nodiscard]] QVariantList sourcesForDiagram() const;
  Q_INVOKABLE [[nodiscard]] QVariantList groupsForDiagram() const;
  Q_INVOKABLE [[nodiscard]] QVariantList actionsForDiagram() const;
  Q_INVOKABLE [[nodiscard]] QJsonObject serializeToJson() const;
  Q_INVOKABLE [[nodiscard]] bool saveJsonFile(const bool askPath = false);
  Q_INVOKABLE [[nodiscard]] bool apiSaveJsonFile(const QString& path);
  Q_INVOKABLE [[nodiscard]] QJsonObject widgetSettings(const QString& widgetId) const;
  Q_INVOKABLE [[nodiscard]] QJsonObject pluginState(const QString& pluginId) const;

public slots:
  void savePluginState(const QString& pluginId, const QJsonObject& state);
  void saveWidgetSetting(const QString& widgetId, const QString& key, const QVariant& value);

  void setupExternalConnections();
  void setSuppressMessageBoxes(const bool suppress);

  void newJsonFile();
  void openJsonFile();
  bool openJsonFile(const QString& path);

  // Loads a project directly from an in-memory JSON document — used by the
  // session-replay flow and the project.loadFromJSON API command so neither
  // needs to round-trip through a temp file. sourcePath is recorded as
  // m_filePath when non-empty; callers without a real file on disk should
  // pass an empty string, and the project will load with no file association.
  bool loadFromJsonDocument(const QJsonDocument& document, const QString& sourcePath = {});

  void setTitle(const QString& title);
  void setPointCount(const int points);
  void clearJsonFilePath();

  // Data-table CRUD — user-defined shared-memory tables
  Q_INVOKABLE QString addTable(const QString& name);
  Q_INVOKABLE [[nodiscard]] QVariantList registersForTable(const QString& table) const;

  void deleteTable(const QString& name);
  void renameTable(const QString& oldName, const QString& newName);
  void addRegister(const QString& table,
                   const QString& registerName,
                   bool computed,
                   const QVariant& defaultValue);
  void deleteRegister(const QString& table, const QString& registerName);
  void updateRegister(const QString& table,
                      const QString& registerName,
                      const QString& newName,
                      bool computed,
                      const QVariant& defaultValue);

  // QInputDialog wrappers — native prompts invoked from QML buttons
  void promptAddTable();
  void promptRenameTable(const QString& oldName);
  void promptAddRegister(const QString& table);
  void promptRenameRegister(const QString& table, const QString& registerName);
  void confirmDeleteTable(const QString& name);
  void confirmDeleteRegister(const QString& table, const QString& registerName);
  void importTableFromCsv(const QString& tableName);
  void exportTableToCsv(const QString& tableName);

  void setFrameStartSequence(const QString& sequence);
  void setFrameEndSequence(const QString& sequence);
  void setChecksumAlgorithm(const QString& algorithm);
  void setFrameDetection(const SerialStudio::FrameDetection detection);

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
  void addSource();
  void deleteSource(int sourceId);
  void duplicateSource(int sourceId);
  void updateSource(int sourceId, const DataModel::Source& source);
  void updateSourceTitle(int sourceId, const QString& title);
  void updateSourceBusType(int sourceId, int busType);
  void updateSourceFrameParser(int sourceId, const QString& code);
  void captureSourceSettings(int sourceId);
  void restoreSourceSettings(int sourceId);
  void setSource0BusType(int busType);
  void setSource0ConnectionSettings(const QJsonObject& settings);
  void addGroup(const QString& title, const SerialStudio::GroupWidget widget);
  bool setGroupWidget(const int group, const SerialStudio::GroupWidget widget);

  void setModified(const bool modified);
  void setFrameParserCode(const QString& code);
  void setFrameParserLanguage(int language);
  void updateSourceFrameParserLanguage(int sourceId, int language);
  void storeFrameParserCode(int sourceId, const QString& code);
  void setActiveGroupId(const int groupId);
  void setGroupLayout(const int groupId, const QJsonObject& layout);
  void setDecoderMethod(const SerialStudio::DecoderMethod method);
  void setHexadecimalDelimiters(const bool hexadecimal);

  void updateGroup(const int groupId, const DataModel::Group& group, const bool rebuildTree = true);
  void updateAction(const int actionId,
                    const DataModel::Action& action,
                    const bool rebuildTree = true);
  void updateDataset(const int groupId,
                     const int datasetId,
                     const DataModel::Dataset& dataset,
                     const bool rebuildTree = false);

  void setSelectedGroup(const DataModel::Group& group);
  void setSelectedAction(const DataModel::Action& action);
  void setSelectedDataset(const DataModel::Dataset& dataset);
  void setSelectedOutputWidget(const DataModel::OutputWidget& widget);

  void setCustomizeWorkspaces(const bool enabled);

  Q_INVOKABLE int addWorkspace(const QString& title);
  Q_INVOKABLE int autoGenerateWorkspaces();
  Q_INVOKABLE [[nodiscard]] QString workspaceTitle(int workspaceId) const;

  void deleteWorkspace(int workspaceId);
  void renameWorkspace(int workspaceId, const QString& title);
  void addWidgetToWorkspace(int workspaceId, int widgetType, int groupId, int relativeIndex);
  void removeWidgetFromWorkspace(int workspaceId, int widgetType, int groupId, int relativeIndex);

  void promptAddWorkspace();
  void promptRenameWorkspace(int workspaceId);
  void confirmDeleteWorkspace(int workspaceId);

  void removeWidgetFromWorkspace(int workspaceId, int index);
  void hideGroup(int groupId);
  void showGroup(int groupId);

  void addOutputControl(const SerialStudio::OutputWidgetType type);
  void addOutputPanel();
  void setOutputWidgetType(int type);
  void setOutputWidgetIcon(const QString& icon);
  void deleteCurrentOutputWidget();
  void duplicateCurrentOutputWidget();
  void updateOutputWidget(int groupId,
                          int widgetId,
                          const DataModel::OutputWidget& widget,
                          bool rebuildTree = false);

private:
  int nextDatasetIndex();
  bool finalizeProjectSave();
  void clearTransientState();

  // Builds a fresh synthetic workspace list from the current groups/datasets.
  // Used by workspaces() when customizeWorkspaces is off, and as the initial
  // seed when the user flips the flag on.
  [[nodiscard]] std::vector<Workspace> buildAutoWorkspaces() const;

  // Per-widget-type counts that a group contributes to Dashboard's walker —
  // used when computing the ref shift after a group delete.
  [[nodiscard]] QMap<int, int> widgetTypeCountsForGroup(const Group& g) const;

  // After a group is removed and groupIds renumbered, shift user-customised
  // widgetRefs. deletedTypeCounts is a snapshot of the deleted group's
  // per-widget-type contribution, captured BEFORE the erase.
  void shiftWorkspaceRefsAfterGroupDelete(int deletedGid, const QMap<int, int>& deletedTypeCounts);

  // After a single dataset is removed from a surviving group, shift refs of
  // the same widget type that lived later in project order. datasetTypeCounts
  // is a snapshot of the deleted dataset's per-widget-type contribution.
  void shiftWorkspaceRefsAfterDatasetDelete(int groupId, const QMap<int, int>& datasetTypeCounts);

private:
  QString m_title;
  QString m_frameEndSequence;
  QString m_checksumAlgorithm;
  QString m_frameStartSequence;
  bool m_hexadecimalDelimiters;

  SerialStudio::DecoderMethod m_frameDecoder;
  SerialStudio::FrameDetection m_frameDetection;

  int m_pointCount;
  bool m_modified;
  bool m_silentReload;
  QString m_filePath;
  bool m_suppressMessageBoxes;

  std::vector<DataModel::Group> m_groups;
  std::vector<DataModel::Action> m_actions;
  std::vector<DataModel::Source> m_sources;
  QSet<int> m_hiddenGroupIds;
  std::vector<DataModel::Workspace> m_workspaces;
  std::vector<DataModel::TableDef> m_tables;

  // customizeWorkspaces == false: m_workspaces stays empty and workspaces()
  // returns a lazily-built synthetic list (m_autoWorkspaces). The synthetic
  // list is never serialised. When the user flips the flag on, the current
  // synthetic list is copied into m_workspaces and saved from then on.
  bool m_customizeWorkspaces;
  mutable std::vector<DataModel::Workspace> m_autoWorkspaces;
  mutable bool m_autoWorkspacesDirty;

  DataModel::Group m_selectedGroup;
  DataModel::Action m_selectedAction;
  DataModel::Dataset m_selectedDataset;
  DataModel::OutputWidget m_selectedOutputWidget;

  QJsonObject m_widgetSettings;
};
}  // namespace DataModel
