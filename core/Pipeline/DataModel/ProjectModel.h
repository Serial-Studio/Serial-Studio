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
 *        property/slot surface. Every other concern is a composed sub-object; a member that only
 *        forwards to the sub-object owning the state is inline, so the .cpp holds facade behaviour.
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
  void sourceConnectionChanged(int sourceId);
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
  void actionDataChanged(int actionId);
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

  [[nodiscard]] bool modified() const noexcept { return m_modified; }

  [[nodiscard]] SerialStudio::DecoderMethod decoderMethod() const noexcept
  {
    return m_frameDecoder;
  }

  [[nodiscard]] SerialStudio::FrameDetection frameDetection() const noexcept
  {
    return m_frameDetection;
  }

  [[nodiscard]] QString jsonFileName() const;
  [[nodiscard]] QString jsonProjectsPath() const;

  [[nodiscard]] QStringList xDataSources() const { return m_presentation.xDataSources(); }

  [[nodiscard]] QList<int> xDataSourceUniqueIds() const
  {
    return m_presentation.xDataSourceUniqueIds();
  }

  [[nodiscard]] QStringList yWaterfallSources() const { return m_presentation.yWaterfallSources(); }

  [[nodiscard]] QList<int> yWaterfallSourceUniqueIds() const
  {
    return m_presentation.yWaterfallSourceUniqueIds();
  }

  [[nodiscard]] int groupIdForUniqueId(int uniqueId) const;
  [[nodiscard]] int groupUniqueIdForGroupId(int groupId) const;

  [[nodiscard]] const QString& title() const noexcept { return m_title; }

  [[nodiscard]] QString controlScriptCode() const { return m_controlScriptCode; }

  [[nodiscard]] const QString& jsonFilePath() const noexcept { return m_filePath; }

  [[nodiscard]] int frameParserLanguage() const;
  [[nodiscard]] int frameParserLanguage(int sourceId) const;
  [[nodiscard]] QString frameParserCode() const;
  [[nodiscard]] QString frameParserTemplate() const;
  [[nodiscard]] QString frameParserTemplate(int sourceId) const;
  [[nodiscard]] QJsonObject frameParserParams() const;
  [[nodiscard]] QJsonObject frameParserParams(int sourceId) const;

  [[nodiscard]] bool suppressMessageBoxes() const noexcept { return m_suppressMessageBoxes; }

  [[nodiscard]] int activeGroupId() const { return m_presentation.activeGroupId(); }

  [[nodiscard]] QJsonObject groupLayout(int groupId) const
  {
    return m_presentation.groupLayout(groupId);
  }

  [[nodiscard]] QJsonObject groupLayout(const QString& scope, int groupId) const
  {
    return m_presentation.groupLayout(scope, groupId);
  }

  [[nodiscard]] QJsonObject layoutChoice(const QString& scope, int groupId) const
  {
    return m_presentation.layoutChoice(scope, groupId);
  }

  void setLayoutChoice(const QString& scope, int groupId, const QString& pattern, int ratio)
  {
    m_presentation.setLayoutChoice(scope, groupId, pattern, ratio);
  }

  [[nodiscard]] bool containsCommercialFeatures() const
  {
    return SerialStudio::commercialCfg(m_groups);
  }

  [[nodiscard]] int pointCount() const noexcept { return m_pointCount; }

  [[nodiscard]] double plotTimeRange() const noexcept { return m_plotTimeRange; }

  [[nodiscard]] bool frozen() const noexcept { return m_frozen; }

  [[nodiscard]] bool changeDrivenTransforms() const noexcept { return m_changeDrivenTransforms; }

  [[nodiscard]] bool luaFastMode() const noexcept { return m_luaFastMode; }

  [[nodiscard]] int groupCount() const noexcept { return static_cast<int>(m_groups.size()); }

  [[nodiscard]] int datasetCount() const;

  [[nodiscard]] int sourceCount() const noexcept { return static_cast<int>(m_sources.size()); }

  [[nodiscard]] const std::vector<Group>& groups() const noexcept { return m_groups; }

  [[nodiscard]] const std::vector<Action>& actions() const noexcept { return m_actions; }

  [[nodiscard]] const std::vector<Source>& sources() const noexcept { return m_sources; }

  [[nodiscard]] const std::vector<Workspace>& editorWorkspaces() const noexcept
  {
    return m_workspaces.list();
  }

  [[nodiscard]] const std::vector<WorkspaceFolder>& editorWorkspaceFolders() const noexcept
  {
    return m_folders.workspaceFolders();
  }

  [[nodiscard]] const std::vector<GroupFolder>& editorGroupFolders() const noexcept
  {
    return m_folders.groupFolders();
  }

  [[nodiscard]] const std::vector<TableFolder>& editorTableFolders() const noexcept
  {
    return m_folders.tableFolders();
  }

  [[nodiscard]] const std::vector<Workspace>& activeWorkspaces() const
  {
    return m_workspaces.activeList();
  }

  [[nodiscard]] const QSet<int>& hiddenGroupIds() const noexcept
  {
    return m_workspaces.hiddenGroupIds();
  }

  [[nodiscard]] int workspaceCount() const noexcept { return m_workspaces.count(); }

  [[nodiscard]] bool isGroupHidden(int groupId) const
  {
    return m_workspaces.isGroupHidden(groupId);
  }

  [[nodiscard]] int tableCount() const noexcept { return m_tables.count(); }

  [[nodiscard]] bool customizeWorkspaces() const noexcept
  {
    return m_workspaces.customizeWorkspaces();
  }

  [[nodiscard]] const std::vector<TableDef>& tables() const noexcept { return m_tables.list(); }

  [[nodiscard]] const QJsonObject& mqttPublisher() const noexcept { return m_mqttPublisher; }

  [[nodiscard]] const QJsonObject& influxSink() const noexcept { return m_influxSink; }

  [[nodiscard]] qint64 mutationEpoch() const noexcept { return m_mutationEpoch; }

  [[nodiscard]] bool canUndo() const noexcept { return m_history.canUndo(); }

  [[nodiscard]] bool canRedo() const noexcept { return m_history.canRedo(); }

  [[nodiscard]] QString undoText() const { return m_history.undoText(); }

  [[nodiscard]] QString redoText() const { return m_history.redoText(); }

  [[nodiscard]] ProjectHistory& history() noexcept { return m_history; }

  [[nodiscard]] bool locked() const noexcept { return m_locked; }

  [[nodiscard]] bool validateProject(const bool silent);

  [[nodiscard]] bool canSave() const { return saveBlockerCode() == SaveBlocker::None; }

  [[nodiscard]] QString saveBlockerTitle() const;
  [[nodiscard]] QString saveBlockerDetail() const;

  Q_INVOKABLE [[nodiscard]] bool askSave() { return m_persistence.askSave(); }

  Q_INVOKABLE [[nodiscard]] QVariantList sourcesForDiagram() const
  {
    return m_presentation.sourcesForDiagram();
  }

  Q_INVOKABLE [[nodiscard]] QVariantList groupsForDiagram() const
  {
    return m_presentation.groupsForDiagram();
  }

  Q_INVOKABLE [[nodiscard]] QVariantList actionsForDiagram() const
  {
    return m_presentation.actionsForDiagram();
  }

  Q_INVOKABLE [[nodiscard]] QVariantList tablesForDiagram() const
  {
    return m_presentation.tablesForDiagram();
  }

  Q_INVOKABLE [[nodiscard]] QJsonObject serializeToJson() const
  {
    return m_persistence.serializeToJson();
  }

  Q_INVOKABLE [[nodiscard]] bool saveJsonFile(const bool askPath = false)
  {
    return m_persistence.saveJsonFile(askPath);
  }

  Q_INVOKABLE [[nodiscard]] bool apiSaveJsonFile(const QString& path)
  {
    return m_persistence.apiSaveJsonFile(path);
  }

  Q_INVOKABLE [[nodiscard]] QJsonObject widgetSettings(const QString& widgetId) const
  {
    return m_presentation.widgetSettings(widgetId);
  }

  Q_INVOKABLE [[nodiscard]] QJsonObject pluginState(const QString& pluginId) const
  {
    return m_presentation.pluginState(pluginId);
  }

  Q_INVOKABLE [[nodiscard]] QJsonArray externalWindows() const
  {
    return m_presentation.externalWindows();
  }

  Q_INVOKABLE [[nodiscard]] QString displayTitle(int uniqueId) const
  {
    return m_presentation.displayTitle(uniqueId);
  }

  Q_INVOKABLE [[nodiscard]] QString widgetDisplayTitle(int widgetType, int uniqueId) const
  {
    return m_presentation.widgetDisplayTitle(widgetType, uniqueId);
  }

  Q_INVOKABLE [[nodiscard]] QString freezeTitleMode(int widgetType, int uniqueId) const
  {
    return m_presentation.freezeTitleMode(widgetType, uniqueId);
  }

  [[nodiscard]] QJsonObject displayTitles() const { return m_presentation.displayTitles(); }

  [[nodiscard]] const QJsonObject& treeExpansion() const noexcept
  {
    return m_presentation.treeExpansion();
  }

  [[nodiscard]] const QJsonObject& diagramCollapse() const noexcept
  {
    return m_presentation.diagramCollapse();
  }

public slots:
  bool undo();
  bool redo();

  void setNextUndoHint(const QString& label, const QString& coalesceKey)
  {
    m_history.setNextHint(label, coalesceKey);
  }

  void lockProject();
  void unlockProject();

  void savePluginState(const QString& pluginId, const QJsonObject& state)
  {
    m_presentation.savePluginState(pluginId, state);
  }

  void saveWidgetSetting(const QString& widgetId, const QString& key, const QVariant& value)
  {
    m_presentation.saveWidgetSetting(widgetId, key, value);
  }

  void setDisplayTitle(int uniqueId, const QString& title)
  {
    m_presentation.setDisplayTitle(uniqueId, title);
  }

  void setWidgetDisplayTitle(int widgetType, int uniqueId, const QString& title)
  {
    m_presentation.setWidgetDisplayTitle(widgetType, uniqueId, title);
  }

  void setFreezeTitleMode(int widgetType, int uniqueId, const QString& mode)
  {
    m_presentation.setFreezeTitleMode(widgetType, uniqueId, mode);
  }

  void promptRenameWidget(int widgetType, int uniqueId, const QString& currentTitle)
  {
    m_presentation.promptRenameWidget(widgetType, uniqueId, currentTitle);
  }

  void setExternalWindows(const QJsonArray& windows) { m_presentation.setExternalWindows(windows); }

  void setTreeExpansion(const QJsonObject& expansion)
  {
    m_presentation.setTreeExpansion(expansion);
  }

  void storeTreeExpansion(const QJsonObject& expansion)
  {
    m_presentation.storeTreeExpansion(expansion);
  }

  void setDiagramCollapse(const QJsonObject& state) { m_presentation.setDiagramCollapse(state); }

  void setupExternalConnections();

  void setSuppressMessageBoxes(const bool suppress) { m_suppressMessageBoxes = suppress; }

  void newJsonFile();

  void openJsonFile() { m_loader.openJsonFile(); }

  bool openJsonFile(const QString& path) { return m_loader.openJsonFile(path); }

  [[nodiscard]] bool lastOpenReloaded() const noexcept { return m_loader.lastOpenReloaded(); }

  bool loadFromJsonDocument(const QJsonDocument& document, const QString& sourcePath = {})
  {
    return m_loader.loadFromJsonDocument(document, sourcePath);
  }

  void importProjectFromJson(const QJsonObject& project, const QString& suggestedFileName)
  {
    m_loader.importProjectFromJson(project, suggestedFileName);
  }

  Q_INVOKABLE [[nodiscard]] int seedDatasetAliases() { return m_entities.seedDatasetAliases(); }

  void setTitle(const QString& title);
  void setControlScriptCode(const QString& code);
  void setPointCount(const int points);
  void setPlotTimeRange(const double seconds);
  void setFrozen(const bool frozen);
  void setChangeDrivenTransforms(const bool enabled);
  void setLuaFastMode(const bool enabled);
  void requestLuaFastMode(const bool enabled);
  void clearJsonFilePath();

  Q_INVOKABLE QString addTable(const QString& name, int parentFolderId = -1)
  {
    return m_tables.addTable(name, parentFolderId);
  }

  Q_INVOKABLE [[nodiscard]] QVariantList registersForTable(const QString& table) const
  {
    return m_tables.registersForTable(table);
  }

  void deleteTable(const QString& name) { m_tables.deleteTable(name); }

  void renameTable(const QString& oldName, const QString& newName)
  {
    m_tables.renameTable(oldName, newName);
  }

  void addRegister(const QString& table,
                   const QString& registerName,
                   bool computed,
                   const QVariant& defaultValue)
  {
    m_tables.addRegister(table, registerName, computed, defaultValue);
  }

  void deleteRegister(const QString& table, const QString& registerName)
  {
    m_tables.deleteRegister(table, registerName);
  }

  [[nodiscard]] bool updateRegister(const QString& table,
                                    const QString& registerName,
                                    const QString& newName,
                                    bool computed,
                                    const QVariant& defaultValue)
  {
    return m_tables.updateRegister(table, registerName, newName, computed, defaultValue);
  }

  void promptAddTable() { m_tables.promptAddTable(); }

  void promptRenameTable(const QString& oldName) { m_tables.promptRenameTable(oldName); }

  void promptRenameGroup(int groupId) { m_entities.promptRenameGroup(groupId); }

  void promptRenameDataset(int groupId, int datasetId)
  {
    m_entities.promptRenameDataset(groupId, datasetId);
  }

  void promptRenameSource(int sourceId) { m_sourceOps.promptRenameSource(sourceId); }

  void promptRenameAction(int actionId) { m_entities.promptRenameAction(actionId); }

  void promptAddRegister(const QString& table) { m_tables.promptAddRegister(table); }

  void promptRenameRegister(const QString& table, const QString& registerName)
  {
    m_tables.promptRenameRegister(table, registerName);
  }

  void confirmDeleteTable(const QString& name) { m_tables.confirmDeleteTable(name); }

  void confirmDeleteRegister(const QString& table, const QString& registerName)
  {
    m_tables.confirmDeleteRegister(table, registerName);
  }

  void importTableFromCsv(const QString& tableName) { m_tables.importTableFromCsv(tableName); }

  void exportTableToCsv(const QString& tableName) { m_tables.exportTableToCsv(tableName); }

  void setFrameStartSequence(const QString& sequence);
  void setFrameEndSequence(const QString& sequence);
  void setChecksumAlgorithm(const QString& algorithm);
  void setFrameDetection(const SerialStudio::FrameDetection detection);

  void deleteCurrentGroup() { m_entities.deleteCurrentGroup(); }

  void deleteCurrentAction() { m_entities.deleteCurrentAction(); }

  void deleteCurrentDataset() { m_entities.deleteCurrentDataset(); }

  void duplicateCurrentGroup() { m_entities.duplicateCurrentGroup(); }

  void duplicateCurrentAction() { m_entities.duplicateCurrentAction(); }

  void duplicateCurrentDataset() { m_entities.duplicateCurrentDataset(); }

  void deleteGroup(int groupId, bool confirm = false) { m_entities.deleteGroup(groupId, confirm); }

  void duplicateGroup(int groupId) { m_entities.duplicateGroup(groupId); }

  void deleteDataset(int groupId, int datasetId, bool confirm = false)
  {
    m_entities.deleteDataset(groupId, datasetId, confirm);
  }

  void duplicateDataset(int groupId, int datasetId)
  {
    m_entities.duplicateDataset(groupId, datasetId);
  }

  void setGroupEnabled(int groupId, bool enabled) { m_entities.setGroupEnabled(groupId, enabled); }

  void setDatasetEnabled(int groupId, int datasetId, bool enabled)
  {
    m_entities.setDatasetEnabled(groupId, datasetId, enabled);
  }

  void deleteAction(int actionId, bool confirm = false)
  {
    m_entities.deleteAction(actionId, confirm);
  }

  void duplicateAction(int actionId) { m_entities.duplicateAction(actionId); }

  void moveGroup(int fromGroupId, int toGroupId) { m_entities.moveGroup(fromGroupId, toGroupId); }

  void moveDataset(int groupId, int fromDatasetId, int toDatasetId)
  {
    m_entities.moveDataset(groupId, fromDatasetId, toDatasetId);
  }

  void moveWorkspace(int workspaceId, int targetIndex)
  {
    m_workspaces.moveWorkspace(workspaceId, targetIndex);
  }

  void moveAction(int fromActionId, int toActionId)
  {
    m_entities.moveAction(fromActionId, toActionId);
  }

  void moveOutputWidget(int groupId, int fromWidgetId, int toWidgetId)
  {
    m_outputWidgets.moveOutputWidget(groupId, fromWidgetId, toWidgetId);
  }

  void ensureValidGroup(int sourceId = -1) { m_entities.ensureValidGroup(sourceId); }

  void addDataset(const SerialStudio::DatasetOption options, int sourceId = -1)
  {
    m_entities.addDataset(options, sourceId);
  }

  void ensurePainterDatasets(int groupId, const QVariantList& specs)
  {
    m_entities.ensurePainterDatasets(groupId, specs);
  }

  void changeDatasetOption(const SerialStudio::DatasetOption option, const bool checked)
  {
    m_entities.changeDatasetOption(option, checked);
  }

  void addAction(int sourceId = -1) { m_entities.addAction(sourceId); }

  void addSource() { m_sourceOps.addSource(); }

  void deleteSource(int sourceId, bool confirm = false)
  {
    m_sourceOps.deleteSource(sourceId, confirm);
  }

  void duplicateSource(int sourceId) { m_sourceOps.duplicateSource(sourceId); }

  void updateSource(int sourceId, const DataModel::Source& source, bool rebuildTree = true)
  {
    m_sourceOps.updateSource(sourceId, source, rebuildTree);
  }

  void updateSourceTitle(int sourceId, const QString& title, bool rebuildTree = true)
  {
    m_sourceOps.updateSourceTitle(sourceId, title, rebuildTree);
  }

  void updateSourceBusType(int sourceId, int busType)
  {
    m_sourceOps.updateSourceBusType(sourceId, busType);
  }

  void updateSourceFrameParser(int sourceId, const QString& code)
  {
    m_sourceOps.updateSourceFrameParser(sourceId, code);
  }

  void captureSourceSettings(int sourceId) { m_sourceOps.captureSourceSettings(sourceId); }

  void restoreSourceSettings(int sourceId) { m_sourceOps.restoreSourceSettings(sourceId); }

  void setSource0BusType(int busType) { m_sourceOps.setSource0BusType(busType); }

  void setSource0ConnectionSettings(const QJsonObject& settings)
  {
    m_sourceOps.setSource0ConnectionSettings(settings);
  }

  void addGroup(const QString& title,
                const SerialStudio::GroupWidget widget,
                int sourceId       = -1,
                int parentFolderId = -1)
  {
    m_entities.addGroup(title, widget, sourceId, parentFolderId);
  }

  bool setGroupWidget(const int group, const SerialStudio::GroupWidget widget)
  {
    return m_entities.setGroupWidget(group, widget);
  }

  void setModified(const bool modified);

  void setFrameParserCode(const QString& code) { m_sourceOps.setFrameParserCode(code); }

  void setFrameParserLanguage(int language) { m_sourceOps.setFrameParserLanguage(language); }

  void updateSourceFrameParserLanguage(int sourceId, int language)
  {
    m_sourceOps.updateSourceFrameParserLanguage(sourceId, language);
  }

  void storeFrameParserCode(int sourceId, const QString& code)
  {
    m_sourceOps.storeFrameParserCode(sourceId, code);
  }

  void setFrameParserTemplate(const QString& templateId)
  {
    m_sourceOps.setFrameParserTemplate(templateId);
  }

  void setFrameParserParams(const QJsonObject& params) { m_sourceOps.setFrameParserParams(params); }

  void updateSourceFrameParserTemplate(int sourceId, const QString& templateId)
  {
    m_sourceOps.updateSourceFrameParserTemplate(sourceId, templateId);
  }

  void updateSourceFrameParserParams(int sourceId, const QJsonObject& params)
  {
    m_sourceOps.updateSourceFrameParserParams(sourceId, params);
  }

  void updateSourceStreamLane(int sourceId, const QString& lane)
  {
    m_sourceOps.updateSourceStreamLane(sourceId, lane);
  }

  void setSourceFrameParserTemplateAndParams(int sourceId,
                                             const QString& templateId,
                                             const QJsonObject& params)
  {
    m_sourceOps.setSourceFrameParserTemplateAndParams(sourceId, templateId, params);
  }

  void setActiveGroupId(const int groupId) { m_presentation.setActiveGroupId(groupId); }

  void setGroupLayout(const int groupId, const QJsonObject& layout)
  {
    m_presentation.setGroupLayout(groupId, layout);
  }

  void setDecoderMethod(const SerialStudio::DecoderMethod method);
  void setHexadecimalDelimiters(const bool hexadecimal);

  void updateGroup(const int groupId, const DataModel::Group& group, const bool rebuildTree = true)
  {
    m_entities.updateGroup(groupId, group, rebuildTree);
  }

  void updateAction(const int actionId,
                    const DataModel::Action& action,
                    const bool rebuildTree = true)
  {
    m_entities.updateAction(actionId, action, rebuildTree);
  }

  void updateDataset(const int groupId,
                     const int datasetId,
                     const DataModel::Dataset& dataset,
                     const bool rebuildTree = false)
  {
    m_entities.updateDataset(groupId, datasetId, dataset, rebuildTree);
  }

  void setSelectedGroup(const DataModel::Group& group) { m_selectedGroup = group; }

  void setSelectedAction(const DataModel::Action& action) { m_selectedAction = action; }

  void setSelectedDataset(const DataModel::Dataset& dataset) { m_selectedDataset = dataset; }

  void setSelectedOutputWidget(const DataModel::OutputWidget& widget)
  {
    m_selectedOutputWidget = widget;
  }

  void setCustomizeWorkspaces(const bool enabled) { m_workspaces.setCustomizeWorkspaces(enabled); }

  void setMqttPublisher(const QJsonObject& config);
  void setInfluxSink(const QJsonObject& config);

  Q_INVOKABLE int addWorkspace(const QString& title) { return m_workspaces.addWorkspace(title); }

  Q_INVOKABLE int autoGenerateWorkspaces() { return m_workspaces.autoGenerateWorkspaces(); }

  Q_INVOKABLE [[nodiscard]] QString workspaceTitle(int workspaceId) const
  {
    return m_workspaces.workspaceTitle(workspaceId);
  }

  Q_INVOKABLE [[nodiscard]] QString workspaceIcon(int workspaceId) const
  {
    return m_workspaces.workspaceIcon(workspaceId);
  }

  Q_INVOKABLE [[nodiscard]] QVariantList hiddenGroupsSummary() const
  {
    return m_workspaces.hiddenGroupsSummary();
  }

  void resetWorkspacesToAuto() { m_workspaces.resetWorkspacesToAuto(); }

  void confirmResetWorkspacesToAuto() { m_workspaces.confirmResetWorkspacesToAuto(); }

  void showAllHiddenGroups() { m_workspaces.showAllHiddenGroups(); }

  void deleteWorkspace(int workspaceId) { m_workspaces.deleteWorkspace(workspaceId); }

  void clearAllWorkspaces() { m_workspaces.clearAllWorkspaces(); }

  void renameWorkspace(int workspaceId, const QString& title)
  {
    m_workspaces.renameWorkspace(workspaceId, title);
  }

  void updateWorkspace(int workspaceId,
                       const QString& title,
                       const QString& icon,
                       const QString& description,
                       bool setTitle,
                       bool setIcon,
                       bool setDescription)
  {
    m_workspaces.updateWorkspace(
      workspaceId, title, icon, description, setTitle, setIcon, setDescription);
  }

  void setWorkspaceIcon(int workspaceId, const QString& icon)
  {
    m_workspaces.setWorkspaceIcon(workspaceId, icon);
  }

  void reorderWorkspaces(const QList<int>& userWorkspaceIds)
  {
    m_workspaces.reorderWorkspaces(userWorkspaceIds);
  }

  void addWidgetToWorkspace(int workspaceId, int widgetType, int groupUniqueId, int relativeIndex)
  {
    m_workspaces.addWidgetToWorkspace(workspaceId, widgetType, groupUniqueId, relativeIndex);
  }

  void removeWidgetFromWorkspace(int workspaceId,
                                 int widgetType,
                                 int groupUniqueId,
                                 int relativeIndex)
  {
    m_workspaces.removeWidgetFromWorkspace(workspaceId, widgetType, groupUniqueId, relativeIndex);
  }

  int cleanupWorkspaceWidgetRefs(const QSet<qint64>& validKeys)
  {
    return m_workspaces.cleanupWorkspaceWidgetRefs(validKeys);
  }

  void promptAddWorkspace() { m_workspaces.promptAddWorkspace(); }

  void promptRenameWorkspace(int workspaceId) { m_workspaces.promptRenameWorkspace(workspaceId); }

  void confirmDeleteWorkspace(int workspaceId) { m_workspaces.confirmDeleteWorkspace(workspaceId); }

  Q_INVOKABLE int addWorkspaceFolder(int parentFolderId, const QString& title)
  {
    return m_folders.addWorkspaceFolder(parentFolderId, title);
  }

  Q_INVOKABLE [[nodiscard]] QString workspaceFolderTitle(int folderId) const
  {
    return m_folders.workspaceFolderTitle(folderId);
  }

  void renameWorkspaceFolder(int folderId, const QString& title)
  {
    m_folders.renameWorkspaceFolder(folderId, title);
  }

  void deleteWorkspaceFolder(int folderId) { m_folders.deleteWorkspaceFolder(folderId); }

  void moveWorkspaceToFolder(int workspaceId, int parentFolderId)
  {
    m_folders.moveWorkspaceToFolder(workspaceId, parentFolderId);
  }

  void moveFolderToFolder(int folderId, int parentFolderId)
  {
    m_folders.moveFolderToFolder(folderId, parentFolderId);
  }

  void moveWorkspaceInFolder(int workspaceId, int direction)
  {
    m_folders.moveWorkspaceInFolder(workspaceId, direction);
  }

  void moveWorkspaceFolderInParent(int folderId, int direction)
  {
    m_folders.moveWorkspaceFolderInParent(folderId, direction);
  }

  void promptAddWorkspaceFolder(int parentFolderId)
  {
    m_folders.promptAddWorkspaceFolder(parentFolderId);
  }

  void promptAddWorkspaceInFolder(int parentFolderId)
  {
    m_folders.promptAddWorkspaceInFolder(parentFolderId);
  }

  void promptRenameWorkspaceFolder(int folderId)
  {
    m_folders.promptRenameWorkspaceFolder(folderId);
  }

  void confirmDeleteWorkspaceFolder(int folderId)
  {
    m_folders.confirmDeleteWorkspaceFolder(folderId);
  }

  Q_INVOKABLE int addGroupFolder(int parentFolderId, const QString& title)
  {
    return m_folders.addGroupFolder(parentFolderId, title);
  }

  Q_INVOKABLE [[nodiscard]] QString groupFolderTitle(int folderId) const
  {
    return m_folders.groupFolderTitle(folderId);
  }

  void renameGroupFolder(int folderId, const QString& title)
  {
    m_folders.renameGroupFolder(folderId, title);
  }

  void deleteGroupFolder(int folderId) { m_folders.deleteGroupFolder(folderId); }

  void moveGroupToFolder(int groupId, int parentFolderId)
  {
    m_folders.moveGroupToFolder(groupId, parentFolderId);
  }

  void moveGroupFolderToFolder(int folderId, int parentFolderId)
  {
    m_folders.moveGroupFolderToFolder(folderId, parentFolderId);
  }

  void moveGroupFolderInParent(int folderId, int direction)
  {
    m_folders.moveGroupFolderInParent(folderId, direction);
  }

  void promptAddGroupFolder(int parentFolderId) { m_folders.promptAddGroupFolder(parentFolderId); }

  void promptRenameGroupFolder(int folderId) { m_folders.promptRenameGroupFolder(folderId); }

  void confirmDeleteGroupFolder(int folderId) { m_folders.confirmDeleteGroupFolder(folderId); }

  Q_INVOKABLE int addTableFolder(int parentFolderId, const QString& title)
  {
    return m_folders.addTableFolder(parentFolderId, title);
  }

  Q_INVOKABLE [[nodiscard]] QString tableFolderTitle(int folderId) const
  {
    return m_folders.tableFolderTitle(folderId);
  }

  void renameTableFolder(int folderId, const QString& title)
  {
    m_folders.renameTableFolder(folderId, title);
  }

  void deleteTableFolder(int folderId) { m_folders.deleteTableFolder(folderId); }

  void moveTableToFolder(const QString& tablePath, int parentFolderId)
  {
    m_folders.moveTableToFolder(tablePath, parentFolderId);
  }

  void moveTableFolderToFolder(int folderId, int parentFolderId)
  {
    m_folders.moveTableFolderToFolder(folderId, parentFolderId);
  }

  void moveTableFolderInParent(int folderId, int direction)
  {
    m_folders.moveTableFolderInParent(folderId, direction);
  }

  void promptAddTableFolder(int parentFolderId) { m_folders.promptAddTableFolder(parentFolderId); }

  void promptAddTableInFolder(int parentFolderId)
  {
    m_folders.promptAddTableInFolder(parentFolderId);
  }

  void promptRenameTableFolder(int folderId) { m_folders.promptRenameTableFolder(folderId); }

  void confirmDeleteTableFolder(int folderId) { m_folders.confirmDeleteTableFolder(folderId); }

  void removeWidgetFromWorkspace(int workspaceId, int index)
  {
    m_workspaces.removeWidgetFromWorkspace(workspaceId, index);
  }

  void hideGroup(int groupId) { m_workspaces.hideGroup(groupId); }

  void showGroup(int groupId) { m_workspaces.showGroup(groupId); }

  void addOutputControl(const SerialStudio::OutputWidgetType type, int sourceId = -1)
  {
    m_outputWidgets.addOutputControl(type, sourceId);
  }

  void addOutputPanel(int sourceId = -1) { m_outputWidgets.addOutputPanel(sourceId); }

  void setOutputWidgetType(int type) { m_outputWidgets.setOutputWidgetType(type); }

  void setOutputWidgetIcon(const QString& icon) { m_outputWidgets.setOutputWidgetIcon(icon); }

  void deleteCurrentOutputWidget() { m_outputWidgets.deleteCurrentOutputWidget(); }

  void duplicateCurrentOutputWidget() { m_outputWidgets.duplicateCurrentOutputWidget(); }

  void deleteOutputWidget(int groupId, int widgetId, bool confirm = false)
  {
    m_outputWidgets.deleteOutputWidget(groupId, widgetId, confirm);
  }

  void duplicateOutputWidget(int groupId, int widgetId)
  {
    m_outputWidgets.duplicateOutputWidget(groupId, widgetId);
  }

  void updateOutputWidget(int groupId,
                          int widgetId,
                          const DataModel::OutputWidget& widget,
                          bool rebuildTree = false)
  {
    m_outputWidgets.updateOutputWidget(groupId, widgetId, widget, rebuildTree);
  }

  void duplicateSelectedItems(const QVariantList& items) { m_bulk.duplicateSelectedItems(items); }

  void deleteSelectedItems(const QVariantList& items) { m_bulk.deleteSelectedItems(items); }

  void confirmDeleteSelectedItems(const QVariantList& items)
  {
    m_bulk.confirmDeleteSelectedItems(items);
  }

  void moveSelectedItemsToFolder(const QVariantList& items, int folderId)
  {
    m_bulk.moveSelectedItemsToFolder(items, folderId);
  }

  void setItemsEnabled(const QVariantList& items, bool enabled)
  {
    m_bulk.setItemsEnabled(items, enabled);
  }

public:
  void flushAutoSave() { m_persistence.flushAutoSave(); }

  void scheduleAutoSave() { m_persistence.scheduleAutoSave(); }

  void setAutoSaveSuspended(bool suspend) { m_persistence.setAutoSaveSuspended(suspend); }

private:
  void clearTransientState();
  void scheduleWorkspaceRegen();
  void flushWorkspaceRegen();
  void emitSinkConfigResets(bool hadMqttPublisher, bool hadInfluxSink);

  [[nodiscard]] int nextDatasetIndex();

  [[nodiscard]] int allocateUniqueId() { return m_nextUniqueId++; }

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
  bool m_workspaceRegenPending;
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
