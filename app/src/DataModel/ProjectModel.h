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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QObject>
#include <QSet>
#include <QVariantList>

#include "DataModel/Frame.h"
#include "DataModel/Project/ProjectBulkOps.h"
#include "DataModel/Project/ProjectEntities.h"
#include "DataModel/Project/ProjectFolders.h"
#include "DataModel/Project/ProjectHistory.h"
#include "DataModel/Project/ProjectLoader.h"
#include "DataModel/Project/ProjectOutputWidgets.h"
#include "DataModel/Project/ProjectPersistence.h"
#include "DataModel/Project/ProjectPresentation.h"
#include "DataModel/Project/ProjectSources.h"
#include "DataModel/Project/ProjectTables.h"
#include "DataModel/Project/ProjectWorkspaces.h"
#include "SerialStudio.h"

class SessionContext;

namespace DataModel {

/**
 * @brief Facade for the Serial Studio project configuration: the document core (title, scalars,
 *        groups, actions, sources, selection, presentation blobs, undo history) plus the QML
 *        property/slot surface. Every other concern is a composed sub-object, and the slots below
 *        forward to whichever one owns the state.
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
  Q_PROPERTY(QString controlScriptCode
             READ  controlScriptCode
             WRITE setControlScriptCode
             NOTIFY controlScriptChanged)
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
             NOTIFY editorWorkspacesChanged)
  Q_PROPERTY(int pointCount
             READ pointCount
             NOTIFY pointCountChanged)
  Q_PROPERTY(double plotTimeRange
             READ plotTimeRange
             NOTIFY plotTimeRangeChanged)
  Q_PROPERTY(bool frozen
             READ frozen
             NOTIFY frozenChanged)
  Q_PROPERTY(bool changeDrivenTransforms
             READ changeDrivenTransforms
             WRITE setChangeDrivenTransforms
             NOTIFY changeDrivenTransformsChanged)
  Q_PROPERTY(bool luaFastMode
             READ luaFastMode
             WRITE setLuaFastMode
             NOTIFY luaFastModeChanged)
  Q_PROPERTY(int frameParserLanguage
             READ  frameParserLanguage
             WRITE setFrameParserLanguage
             NOTIFY frameParserLanguageChanged)
  Q_PROPERTY(QString frameParserTemplate
             READ  frameParserTemplate
             WRITE setFrameParserTemplate
             NOTIFY frameParserTemplateChanged)
  Q_PROPERTY(QJsonObject frameParserParams
             READ  frameParserParams
             WRITE setFrameParserParams
             NOTIFY frameParserParamsChanged)
  Q_PROPERTY(int tableCount
             READ tableCount
             NOTIFY tablesChanged)
  Q_PROPERTY(bool customizeWorkspaces
             READ  customizeWorkspaces
             WRITE setCustomizeWorkspaces
             NOTIFY customizeWorkspacesChanged)
  Q_PROPERTY(bool locked
             READ locked
             NOTIFY lockedChanged)
  Q_PROPERTY(bool canSave
             READ canSave
             NOTIFY saveStatusChanged)
  Q_PROPERTY(QString saveBlockerTitle
             READ saveBlockerTitle
             NOTIFY saveStatusChanged)
  Q_PROPERTY(QString saveBlockerDetail
             READ saveBlockerDetail
             NOTIFY saveStatusChanged)
  Q_PROPERTY(QJsonObject mqttPublisher
             READ mqttPublisher
             WRITE setMqttPublisher
             NOTIFY mqttPublisherChanged)
  Q_PROPERTY(QJsonObject influxSink
             READ influxSink
             WRITE setInfluxSink
             NOTIFY influxSinkChanged)
  Q_PROPERTY(QJsonObject diagramCollapse
             READ diagramCollapse
             NOTIFY diagramCollapseChanged)
  Q_PROPERTY(bool canUndo
             READ canUndo
             NOTIFY projectHistoryChanged)
  Q_PROPERTY(bool canRedo
             READ canRedo
             NOTIFY projectHistoryChanged)
  Q_PROPERTY(QString undoText
             READ undoText
             NOTIFY projectHistoryChanged)
  Q_PROPERTY(QString redoText
             READ redoText
             NOTIFY projectHistoryChanged)
  // clang-format on

signals:
  void titleChanged();
  void controlScriptChanged();
  void saveStatusChanged();
  void pointCountChanged();
  void plotTimeRangeChanged();
  void frozenChanged();
  void changeDrivenTransformsChanged();
  void luaFastModeChanged();
  void jsonFileChanged();
  void projectFileChangedOnDisk();
  void modifiedChanged();
  void contentTouched();
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
  void frameParserTemplateChanged();
  void frameParserParamsChanged();
  void sourceFrameParserTemplateChanged(int sourceId);
  void sourceFrameParserParamsChanged(int sourceId);
  void sourceStreamLaneChanged(int sourceId);
  void activeGroupIdChanged();
  void widgetSettingsChanged();
  void widgetDisplayChanged();
  void editorWorkspacesChanged();
  void activeWorkspacesChanged();
  void tablesChanged();
  void customizeWorkspacesChanged();
  void lockedChanged();
  void mqttPublisherChanged();
  void influxSinkChanged();
  void diagramCollapseChanged();
  void saveDialogCompleted(bool accepted);
  void importCompleted(bool accepted, const QString& savedPath);
  void projectHistoryChanged();
  void historySnapshotApplied();

  void groupAdded(int groupId);
  void groupDeleted();
  void datasetAdded(int groupId, int datasetId);
  void datasetDeleted(int survivingGroupId);
  void actionAdded(int actionId);
  void actionDeleted();
  void sourceAdded(int sourceId);
  void sourceChanged(int sourceId);
  void sourceDeleted();
  void outputWidgetAdded(int groupId, int widgetId);
  void outputWidgetDeleted(int groupId);

private:
  /**
   * @brief Reason the current project state cannot be saved (or None when valid).
   */
  enum class SaveBlocker {
    None = 0,
    MissingTitle,
    MissingGroup,
    MissingDataset,
  };

  [[nodiscard]] SaveBlocker saveBlockerCode() const;

  friend class ::SessionContext;
  friend class ProjectBulkOps;
  friend class ProjectEntities;
  friend class ProjectFolders;
  friend class ProjectLoader;
  friend class ProjectOutputWidgets;
  friend class ProjectPersistence;
  friend class ProjectPresentation;
  friend class ProjectSources;
  friend class ProjectTables;
  friend class ProjectWorkspaces;

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
  [[nodiscard]] QList<int> xDataSourceUniqueIds() const;
  [[nodiscard]] QStringList yWaterfallSources() const;
  [[nodiscard]] QList<int> yWaterfallSourceUniqueIds() const;

  [[nodiscard]] int groupIdForUniqueId(int uniqueId) const;
  [[nodiscard]] int groupUniqueIdForGroupId(int groupId) const;

  [[nodiscard]] const QString& title() const noexcept;
  [[nodiscard]] QString controlScriptCode() const;
  [[nodiscard]] const QString& jsonFilePath() const noexcept;
  [[nodiscard]] int frameParserLanguage() const;
  [[nodiscard]] int frameParserLanguage(int sourceId) const;
  [[nodiscard]] QString frameParserCode() const;
  [[nodiscard]] QString frameParserTemplate() const;
  [[nodiscard]] QString frameParserTemplate(int sourceId) const;
  [[nodiscard]] QJsonObject frameParserParams() const;
  [[nodiscard]] QJsonObject frameParserParams(int sourceId) const;

  [[nodiscard]] bool suppressMessageBoxes() const noexcept;

  [[nodiscard]] int activeGroupId() const;
  [[nodiscard]] QJsonObject groupLayout(int groupId) const;
  [[nodiscard]] QJsonObject groupLayout(const QString& scope, int groupId) const;
  [[nodiscard]] QJsonObject layoutChoice(const QString& scope, int groupId) const;
  void setLayoutChoice(const QString& scope, int groupId, const QString& pattern, int ratio);

  [[nodiscard]] bool containsCommercialFeatures() const;

  [[nodiscard]] int pointCount() const noexcept;
  [[nodiscard]] double plotTimeRange() const noexcept;
  [[nodiscard]] bool frozen() const noexcept;
  [[nodiscard]] bool changeDrivenTransforms() const noexcept;
  [[nodiscard]] bool luaFastMode() const noexcept;
  [[nodiscard]] int groupCount() const noexcept;
  [[nodiscard]] int datasetCount() const;
  [[nodiscard]] int sourceCount() const noexcept;
  [[nodiscard]] const std::vector<Group>& groups() const noexcept;
  [[nodiscard]] const std::vector<Action>& actions() const noexcept;
  [[nodiscard]] const std::vector<Source>& sources() const noexcept;
  [[nodiscard]] const std::vector<Workspace>& editorWorkspaces() const noexcept;
  [[nodiscard]] const std::vector<WorkspaceFolder>& editorWorkspaceFolders() const noexcept;
  [[nodiscard]] const std::vector<GroupFolder>& editorGroupFolders() const noexcept;
  [[nodiscard]] const std::vector<TableFolder>& editorTableFolders() const noexcept;
  [[nodiscard]] const std::vector<Workspace>& activeWorkspaces() const;
  [[nodiscard]] const QSet<int>& hiddenGroupIds() const noexcept;
  [[nodiscard]] int workspaceCount() const noexcept;
  [[nodiscard]] bool isGroupHidden(int groupId) const;

  [[nodiscard]] int tableCount() const noexcept;
  [[nodiscard]] bool customizeWorkspaces() const noexcept;
  [[nodiscard]] const std::vector<TableDef>& tables() const noexcept;
  [[nodiscard]] const QJsonObject& mqttPublisher() const noexcept;
  [[nodiscard]] const QJsonObject& influxSink() const noexcept;

  [[nodiscard]] qint64 mutationEpoch() const noexcept;

  [[nodiscard]] bool canUndo() const noexcept;
  [[nodiscard]] bool canRedo() const noexcept;
  [[nodiscard]] QString undoText() const;
  [[nodiscard]] QString redoText() const;
  [[nodiscard]] ProjectHistory& history() noexcept;

  [[nodiscard]] bool locked() const noexcept;
  [[nodiscard]] bool validateProject(const bool silent);

  [[nodiscard]] bool canSave() const;
  [[nodiscard]] QString saveBlockerTitle() const;
  [[nodiscard]] QString saveBlockerDetail() const;

  Q_INVOKABLE [[nodiscard]] bool askSave();
  Q_INVOKABLE [[nodiscard]] QVariantList sourcesForDiagram() const;
  Q_INVOKABLE [[nodiscard]] QVariantList groupsForDiagram() const;
  Q_INVOKABLE [[nodiscard]] QVariantList actionsForDiagram() const;
  Q_INVOKABLE [[nodiscard]] QVariantList tablesForDiagram() const;
  Q_INVOKABLE [[nodiscard]] QJsonObject serializeToJson() const;
  Q_INVOKABLE [[nodiscard]] bool saveJsonFile(const bool askPath = false);
  Q_INVOKABLE [[nodiscard]] bool apiSaveJsonFile(const QString& path);
  Q_INVOKABLE [[nodiscard]] QJsonObject widgetSettings(const QString& widgetId) const;
  Q_INVOKABLE [[nodiscard]] QJsonObject pluginState(const QString& pluginId) const;
  Q_INVOKABLE [[nodiscard]] QJsonArray externalWindows() const;
  Q_INVOKABLE [[nodiscard]] QString displayTitle(int uniqueId) const;
  Q_INVOKABLE [[nodiscard]] QString widgetDisplayTitle(int widgetType, int uniqueId) const;
  Q_INVOKABLE [[nodiscard]] QString freezeTitleMode(int widgetType, int uniqueId) const;

  [[nodiscard]] QJsonObject displayTitles() const;

  [[nodiscard]] const QJsonObject& treeExpansion() const noexcept;
  [[nodiscard]] const QJsonObject& diagramCollapse() const noexcept;

public slots:
  bool undo();
  bool redo();
  void setNextUndoHint(const QString& label, const QString& coalesceKey);

  void lockProject();
  void unlockProject();

  void savePluginState(const QString& pluginId, const QJsonObject& state);
  void saveWidgetSetting(const QString& widgetId, const QString& key, const QVariant& value);
  void setDisplayTitle(int uniqueId, const QString& title);
  void setWidgetDisplayTitle(int widgetType, int uniqueId, const QString& title);
  void setFreezeTitleMode(int widgetType, int uniqueId, const QString& mode);
  void promptRenameWidget(int widgetType, int uniqueId, const QString& currentTitle);
  void setExternalWindows(const QJsonArray& windows);
  void setTreeExpansion(const QJsonObject& expansion);
  void setDiagramCollapse(const QJsonObject& state);

  void setupExternalConnections();
  void setSuppressMessageBoxes(const bool suppress);

  void newJsonFile();
  void openJsonFile();
  bool openJsonFile(const QString& path);

  bool loadFromJsonDocument(const QJsonDocument& document, const QString& sourcePath = {});
  void importProjectFromJson(const QJsonObject& project, const QString& suggestedFileName);

  Q_INVOKABLE [[nodiscard]] int seedDatasetAliases();

  void setTitle(const QString& title);
  void setControlScriptCode(const QString& code);
  void setPointCount(const int points);
  void setPlotTimeRange(const double seconds);
  void setFrozen(const bool frozen);
  void setChangeDrivenTransforms(const bool enabled);
  void setLuaFastMode(const bool enabled);
  void requestLuaFastMode(const bool enabled);
  void clearJsonFilePath();

  Q_INVOKABLE QString addTable(const QString& name, int parentFolderId = -1);
  Q_INVOKABLE [[nodiscard]] QVariantList registersForTable(const QString& table) const;

  void deleteTable(const QString& name);
  void renameTable(const QString& oldName, const QString& newName);
  void addRegister(const QString& table,
                   const QString& registerName,
                   bool computed,
                   const QVariant& defaultValue);
  void deleteRegister(const QString& table, const QString& registerName);
  [[nodiscard]] bool updateRegister(const QString& table,
                                    const QString& registerName,
                                    const QString& newName,
                                    bool computed,
                                    const QVariant& defaultValue);

  void promptAddTable();
  void promptRenameTable(const QString& oldName);

  void promptRenameGroup(int groupId);
  void promptRenameDataset(int groupId, int datasetId);
  void promptRenameSource(int sourceId);
  void promptRenameAction(int actionId);
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

  void deleteGroup(int groupId, bool confirm = false);
  void duplicateGroup(int groupId);
  void deleteDataset(int groupId, int datasetId, bool confirm = false);
  void duplicateDataset(int groupId, int datasetId);
  void setGroupEnabled(int groupId, bool enabled);
  void setDatasetEnabled(int groupId, int datasetId, bool enabled);
  void deleteAction(int actionId, bool confirm = false);
  void duplicateAction(int actionId);

  void moveGroup(int fromGroupId, int toGroupId);
  void moveDataset(int groupId, int fromDatasetId, int toDatasetId);
  void moveWorkspace(int workspaceId, int targetIndex);
  void moveAction(int fromActionId, int toActionId);
  void moveOutputWidget(int groupId, int fromWidgetId, int toWidgetId);

  void ensureValidGroup(int sourceId = -1);
  void addDataset(const SerialStudio::DatasetOption options, int sourceId = -1);
  void ensurePainterDatasets(int groupId, const QVariantList& specs);
  void changeDatasetOption(const SerialStudio::DatasetOption option, const bool checked);

  void addAction(int sourceId = -1);
  void addSource();
  void deleteSource(int sourceId, bool confirm = false);
  void duplicateSource(int sourceId);
  void updateSource(int sourceId, const DataModel::Source& source, bool rebuildTree = true);
  void updateSourceTitle(int sourceId, const QString& title, bool rebuildTree = true);
  void updateSourceBusType(int sourceId, int busType);
  void updateSourceFrameParser(int sourceId, const QString& code);
  void captureSourceSettings(int sourceId);
  void restoreSourceSettings(int sourceId);
  void setSource0BusType(int busType);
  void setSource0ConnectionSettings(const QJsonObject& settings);
  void addGroup(const QString& title,
                const SerialStudio::GroupWidget widget,
                int sourceId       = -1,
                int parentFolderId = -1);
  bool setGroupWidget(const int group, const SerialStudio::GroupWidget widget);

  void setModified(const bool modified);
  void setFrameParserCode(const QString& code);
  void setFrameParserLanguage(int language);
  void updateSourceFrameParserLanguage(int sourceId, int language);
  void storeFrameParserCode(int sourceId, const QString& code);
  void setFrameParserTemplate(const QString& templateId);
  void setFrameParserParams(const QJsonObject& params);
  void updateSourceFrameParserTemplate(int sourceId, const QString& templateId);
  void updateSourceFrameParserParams(int sourceId, const QJsonObject& params);
  void updateSourceStreamLane(int sourceId, const QString& lane);
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
  void setMqttPublisher(const QJsonObject& config);
  void setInfluxSink(const QJsonObject& config);

  Q_INVOKABLE int addWorkspace(const QString& title);
  Q_INVOKABLE int autoGenerateWorkspaces();
  Q_INVOKABLE [[nodiscard]] QString workspaceTitle(int workspaceId) const;
  Q_INVOKABLE [[nodiscard]] QString workspaceIcon(int workspaceId) const;
  Q_INVOKABLE [[nodiscard]] QVariantList hiddenGroupsSummary() const;
  void resetWorkspacesToAuto();
  void confirmResetWorkspacesToAuto();
  void showAllHiddenGroups();

  void deleteWorkspace(int workspaceId);
  void clearAllWorkspaces();
  void renameWorkspace(int workspaceId, const QString& title);
  void updateWorkspace(int workspaceId,
                       const QString& title,
                       const QString& icon,
                       const QString& description,
                       bool setTitle,
                       bool setIcon,
                       bool setDescription);
  void setWorkspaceIcon(int workspaceId, const QString& icon);
  void reorderWorkspaces(const QList<int>& userWorkspaceIds);
  void addWidgetToWorkspace(int workspaceId, int widgetType, int groupUniqueId, int relativeIndex);
  void removeWidgetFromWorkspace(int workspaceId,
                                 int widgetType,
                                 int groupUniqueId,
                                 int relativeIndex);
  int cleanupWorkspaceWidgetRefs(const QSet<qint64>& validKeys);

  void promptAddWorkspace();
  void promptRenameWorkspace(int workspaceId);
  void confirmDeleteWorkspace(int workspaceId);

  Q_INVOKABLE int addWorkspaceFolder(int parentFolderId, const QString& title);
  Q_INVOKABLE [[nodiscard]] QString workspaceFolderTitle(int folderId) const;
  void renameWorkspaceFolder(int folderId, const QString& title);
  void deleteWorkspaceFolder(int folderId);
  void moveWorkspaceToFolder(int workspaceId, int parentFolderId);
  void moveFolderToFolder(int folderId, int parentFolderId);
  void moveWorkspaceInFolder(int workspaceId, int direction);
  void moveWorkspaceFolderInParent(int folderId, int direction);

  void promptAddWorkspaceFolder(int parentFolderId);
  void promptAddWorkspaceInFolder(int parentFolderId);
  void promptRenameWorkspaceFolder(int folderId);
  void confirmDeleteWorkspaceFolder(int folderId);

  Q_INVOKABLE int addGroupFolder(int parentFolderId, const QString& title);
  Q_INVOKABLE [[nodiscard]] QString groupFolderTitle(int folderId) const;
  void renameGroupFolder(int folderId, const QString& title);
  void deleteGroupFolder(int folderId);
  void moveGroupToFolder(int groupId, int parentFolderId);
  void moveGroupFolderToFolder(int folderId, int parentFolderId);
  void moveGroupFolderInParent(int folderId, int direction);

  void promptAddGroupFolder(int parentFolderId);
  void promptRenameGroupFolder(int folderId);
  void confirmDeleteGroupFolder(int folderId);

  Q_INVOKABLE int addTableFolder(int parentFolderId, const QString& title);
  Q_INVOKABLE [[nodiscard]] QString tableFolderTitle(int folderId) const;
  void renameTableFolder(int folderId, const QString& title);
  void deleteTableFolder(int folderId);
  void moveTableToFolder(const QString& tablePath, int parentFolderId);
  void moveTableFolderToFolder(int folderId, int parentFolderId);
  void moveTableFolderInParent(int folderId, int direction);

  void promptAddTableFolder(int parentFolderId);
  void promptAddTableInFolder(int parentFolderId);
  void promptRenameTableFolder(int folderId);
  void confirmDeleteTableFolder(int folderId);

  void removeWidgetFromWorkspace(int workspaceId, int index);
  void hideGroup(int groupId);
  void showGroup(int groupId);

  void addOutputControl(const SerialStudio::OutputWidgetType type, int sourceId = -1);
  void addOutputPanel(int sourceId = -1);
  void setOutputWidgetType(int type);
  void setOutputWidgetIcon(const QString& icon);
  void deleteCurrentOutputWidget();
  void duplicateCurrentOutputWidget();
  void deleteOutputWidget(int groupId, int widgetId, bool confirm = false);
  void duplicateOutputWidget(int groupId, int widgetId);
  void updateOutputWidget(int groupId,
                          int widgetId,
                          const DataModel::OutputWidget& widget,
                          bool rebuildTree = false);

  void duplicateSelectedItems(const QVariantList& items);
  void deleteSelectedItems(const QVariantList& items);
  void confirmDeleteSelectedItems(const QVariantList& items);
  void moveSelectedItemsToFolder(const QVariantList& items, int folderId);
  void setItemsEnabled(const QVariantList& items, bool enabled);

public:
  void flushAutoSave();
  void scheduleAutoSave();
  void setAutoSaveSuspended(bool suspend);

private:
  void emitSinkConfigResets(bool hadMqttPublisher, bool hadInfluxSink);
  void clearTransientState();

  [[nodiscard]] int nextDatasetIndex();
  [[nodiscard]] int allocateUniqueId();

private:
  QString m_title;
  QString m_frameEndSequence;
  QString m_checksumAlgorithm;
  QString m_frameStartSequence;
  QString m_writerVersionAtCreation;
  bool m_hexadecimalDelimiters;

  SerialStudio::DecoderMethod m_frameDecoder;
  SerialStudio::FrameDetection m_frameDetection;

  int m_pointCount;
  double m_plotTimeRange;
  bool m_frozen;
  bool m_changeDrivenTransforms;
  bool m_luaFastMode;
  int m_nextUniqueId;
  bool m_modified;
  bool m_initialized;
  bool m_silentReload;
  QString m_filePath;
  bool m_suppressMessageBoxes;
  QString m_controlScriptCode;

  std::vector<DataModel::Group> m_groups;
  std::vector<DataModel::Action> m_actions;
  std::vector<DataModel::Source> m_sources;

  QString m_passwordHash;
  bool m_locked;

  qint64 m_mutationEpoch;
  ProjectHistory m_history;

  DataModel::Group m_selectedGroup;
  DataModel::Action m_selectedAction;
  DataModel::Dataset m_selectedDataset;
  DataModel::OutputWidget m_selectedOutputWidget;

  QJsonObject m_mqttPublisher;
  QJsonObject m_influxSink;

  ProjectPresentation m_presentation;
  ProjectPersistence m_persistence;
  ProjectFolders m_folders;
  ProjectWorkspaces m_workspaces;
  ProjectTables m_tables;
  ProjectLoader m_loader;
  ProjectSources m_sourceOps;
  ProjectEntities m_entities;
  ProjectOutputWidgets m_outputWidgets;
  ProjectBulkOps m_bulk;
};
}  // namespace DataModel
