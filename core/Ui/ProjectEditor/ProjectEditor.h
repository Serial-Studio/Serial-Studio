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

#include <QHash>
#include <QItemSelectionModel>
#include <QList>
#include <QObject>
#include <QStandardItemModel>
#include <QVector>

#include "Core/DataModel/Frame.h"
#include "Core/SerialStudio.h"
#include "DataModel/Project/EntityKinds.h"
#include "ProjectEditor/EditorCommit.h"
#include "ProjectEditor/EditorForms.h"
#include "ProjectEditor/EditorInflux.h"
#include "ProjectEditor/EditorMqtt.h"
#include "ProjectEditor/EditorMultiSelect.h"
#include "ProjectEditor/EditorSelection.h"
#include "ProjectEditor/EditorSummaries.h"
#include "ProjectEditor/EditorTree.h"
#include "ProjectEditor/EditorWiring.h"

namespace IO {
class ConnectionManager;
}  // namespace IO

namespace DataModel {
class CustomModel;
class ProjectModel;

/**
 * @brief Facade for the Project Editor window: the QML-facing property, slot and signal surface
 *        plus the state every concern shares (current view, selected-entity snapshots, the
 *        tree-item maps, the form models and the combo vocabulary). Each concern is a composed
 *        sub-object reached through friendship; the facade forwards to it.
 */
class ProjectEditor : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(QAbstractItemModel* groupModel
             READ groupModel
             NOTIFY groupModelChanged)
  Q_PROPERTY(QAbstractItemModel* actionModel
             READ actionModel
             NOTIFY actionModelChanged)
  Q_PROPERTY(QAbstractItemModel* projectModel
             READ projectModel
             NOTIFY projectModelChanged)
  Q_PROPERTY(QAbstractItemModel* datasetModel
             READ datasetModel
             NOTIFY datasetModelChanged)
  Q_PROPERTY(QAbstractItemModel* sourceModel
             READ sourceModel
             NOTIFY sourceModelChanged)
  Q_PROPERTY(QAbstractItemModel* outputWidgetModel
             READ outputWidgetModel
             NOTIFY outputWidgetModelChanged)
  Q_PROPERTY(QAbstractItemModel* mqttPublisherModel
             READ mqttPublisherModel
             NOTIFY mqttPublisherModelChanged)
  Q_PROPERTY(QAbstractItemModel* influxSinkModel
             READ influxSinkModel
             NOTIFY influxSinkModelChanged)
  Q_PROPERTY(int selectedSourceId
             READ selectedSourceId
             NOTIFY sourceModelChanged)
  Q_PROPERTY(int selectedSourceBusType
             READ selectedSourceBusType
             NOTIFY sourceModelChanged)
  Q_PROPERTY(QAbstractItemModel* treeModel
             READ treeModel
             NOTIFY treeModelChanged)
  Q_PROPERTY(QItemSelectionModel* selectionModel
             READ selectionModel
             NOTIFY treeModelChanged)
  Q_PROPERTY(CurrentView currentView
             READ currentView
             NOTIFY currentViewChanged)
  Q_PROPERTY(bool canGoBack
             READ canGoBack
             NOTIFY navHistoryChanged)
  Q_PROPERTY(bool canGoForward
             READ canGoForward
             NOTIFY navHistoryChanged)
  Q_PROPERTY(int multiSelectionKind
             READ multiSelectionKind
             NOTIFY currentViewChanged)
  Q_PROPERTY(int multiSelectionCount
             READ multiSelectionCount
             NOTIFY currentViewChanged)
  Q_PROPERTY(QString selectedText
             READ selectedText
             NOTIFY selectedTextChanged)
  Q_PROPERTY(QString selectedIcon
             READ selectedIcon
             NOTIFY currentViewChanged)
  Q_PROPERTY(quint16 datasetOptions
             READ datasetOptions
             NOTIFY datasetOptionsChanged)
  Q_PROPERTY(bool canMoveCurrentUp
             READ canMoveCurrentUp
             NOTIFY editableOptionsChanged)
  Q_PROPERTY(bool canMoveCurrentDown
             READ canMoveCurrentDown
             NOTIFY editableOptionsChanged)
  Q_PROPERTY(bool currentGroupIsEditable
             READ currentGroupIsEditable
             NOTIFY editableOptionsChanged)
  Q_PROPERTY(bool currentDatasetIsEditable
             READ currentDatasetIsEditable
             NOTIFY editableOptionsChanged)
  Q_PROPERTY(bool selectedGroupEnabled
             READ selectedGroupEnabled
             NOTIFY editableOptionsChanged)
  Q_PROPERTY(bool selectedDatasetEnabled
             READ selectedDatasetEnabled
             NOTIFY editableOptionsChanged)
  Q_PROPERTY(bool currentGroupIsOutputPanel
             READ currentGroupIsOutputPanel
             NOTIFY editableOptionsChanged)
  Q_PROPERTY(int outputWidgetType
             READ outputWidgetType
             NOTIFY outputWidgetModelChanged)
  Q_PROPERTY(QString outputWidgetIcon
             READ outputWidgetIcon
             NOTIFY outputWidgetModelChanged)
  Q_PROPERTY(QString actionIcon
             READ actionIcon
             NOTIFY actionModelChanged)
  Q_PROPERTY(QStringList availableActionIcons
             READ availableActionIcons
             CONSTANT)
  Q_PROPERTY(QString selectedSourceFrameParserCode
             READ  selectedSourceFrameParserCode
             WRITE setSelectedSourceFrameParserCode
             NOTIFY selectedSourceFrameParserCodeChanged)
  Q_PROPERTY(QString currentGroupPainterCode
             READ  currentGroupPainterCode
             WRITE setCurrentGroupPainterCode
             NOTIFY currentGroupPainterCodeChanged)
  Q_PROPERTY(bool currentGroupIsPainter
             READ currentGroupIsPainter
             NOTIFY editableOptionsChanged)
  Q_PROPERTY(QString treeSearchQuery
             READ  treeSearchQuery
             WRITE setTreeSearchQuery
             NOTIFY treeSearchQueryChanged)
  Q_PROPERTY(QString selectedUserTable
             READ selectedUserTable
             NOTIFY selectedUserTableChanged)
  Q_PROPERTY(int selectedWorkspaceId
             READ selectedWorkspaceId
             NOTIFY selectedWorkspaceIdChanged)
  Q_PROPERTY(int selectedFolderId
             READ selectedFolderId
             NOTIFY selectedFolderIdChanged)
  Q_PROPERTY(int selectedGroupFolderId
             READ selectedGroupFolderId
             NOTIFY selectedGroupFolderIdChanged)
  Q_PROPERTY(int selectedTableFolderId
             READ selectedTableFolderId
             NOTIFY selectedTableFolderIdChanged)
  // clang-format on

signals:
  void treeModelChanged();
  void treeSearchQueryChanged();
  void selectedUserTableChanged();
  void selectedWorkspaceIdChanged();
  void selectedFolderIdChanged();
  void selectedGroupFolderIdChanged();
  void selectedTableFolderIdChanged();
  void selectedSourceFrameParserCodeChanged();
  void currentGroupPainterCodeChanged();
  void groupModelChanged();
  void sourceModelChanged();
  void currentViewChanged();
  void navHistoryChanged();
  void selectedTextChanged();
  void actionModelChanged();
  void projectModelChanged();
  void datasetModelChanged();
  void datasetOptionsChanged();
  void editableOptionsChanged();
  void outputWidgetModelChanged();
  void mqttPublisherModelChanged();
  void influxSinkModelChanged();
  void treeRebuildFinished(const QModelIndex& revealIndex);
  void openAlarmBandsEditor(
    int groupId, int datasetId, double rangeMin, double rangeMax, QVariantList currentBands);
  void openFrequencyMarkersEditor(int groupId,
                                  int datasetId,
                                  double nyquist,
                                  QVariantList currentMarkers);

private:
  friend class EditorCommit;
  friend class EditorForms;
  friend class EditorInflux;
  friend class EditorMqtt;
  friend class EditorMultiSelect;
  friend class EditorSelection;
  friend class EditorSummaries;
  friend class EditorTree;
  friend class EditorWiring;

  ~ProjectEditor();
  explicit ProjectEditor();
  ProjectEditor(ProjectEditor&&)                 = delete;
  ProjectEditor(const ProjectEditor&)            = delete;
  ProjectEditor& operator=(ProjectEditor&&)      = delete;
  ProjectEditor& operator=(const ProjectEditor&) = delete;

public:
  [[nodiscard]] static ProjectEditor& instance();

  enum CurrentView {
    ProjectView,
    GroupView,
    DatasetView,
    ActionView,
    SourceView,
    SourceFrameParserView,
    OutputWidgetView,
    DataTablesView,
    SystemDatasetsView,
    UserTableView,
    WorkspacesView,
    WorkspaceView,
    WorkspaceFolderView,
    GroupsView,
    GroupFolderView,
    TableFolderView,
    MqttPublisherView,
    InfluxSinkView,
    ControlScriptView,
    TransformLibraryView,
    JsLibraryView,
    ProjectScriptsView,
    DataExportView,
    MultiSelectionView,
  };
  Q_ENUM(CurrentView)

  enum EditorWidget {
    TextField     = DataModel::RowTextField,
    HexTextField  = DataModel::RowHexTextField,
    IntField      = DataModel::RowIntField,
    FloatField    = DataModel::RowFloatField,
    CheckBox      = DataModel::RowCheckBox,
    ComboBox      = DataModel::RowComboBox,
    IconPicker    = DataModel::RowIconPicker,
    SectionHeader = DataModel::RowSectionHeader,
    PasswordField = DataModel::RowPasswordField,
    AutoIntField  = DataModel::RowAutoIntField,
    Button        = DataModel::RowButton,
    NavRow        = DataModel::RowNavRow,
    ColorPicker   = DataModel::RowColorPicker,
  };
  Q_ENUM(EditorWidget)

  enum CustomRoles {
    Active = Qt::UserRole,

    TreeViewIcon       = Qt::UserRole + 1,
    TreeViewText       = Qt::UserRole + 2,
    TreeViewExpanded   = Qt::UserRole + 3,
    TreeViewFrameIndex = Qt::UserRole + 4,

    ParameterName        = Qt::UserRole + 5,
    EditableValue        = Qt::UserRole + 6,
    ParameterType        = Qt::UserRole + 7,
    PlaceholderValue     = Qt::UserRole + 8,
    ParameterDescription = Qt::UserRole + 9,
    ParameterIcon        = Qt::UserRole + 10,

    WidgetType   = Qt::UserRole + 11,
    ComboBoxData = Qt::UserRole + 12,
    ParameterKey = Qt::UserRole + 13,

    TreeViewSourceName = Qt::UserRole + 14,
    TreeViewSourceId   = Qt::UserRole + 15,
    TreeViewVirtual    = Qt::UserRole + 16,

    TreeItemKind     = Qt::UserRole + 17,
    TreeItemId       = Qt::UserRole + 18,
    TreeItemParentId = Qt::UserRole + 19,

    TreeViewWorkspaceStale = Qt::UserRole + 20,

    MinValue = Qt::UserRole + 21,
    MaxValue = Qt::UserRole + 22,

    TreeItemPath = Qt::UserRole + 23,

    TreeViewEnabled     = Qt::UserRole + 24,
    TreeViewSelfEnabled = Qt::UserRole + 25,
  };
  Q_ENUM(CustomRoles)

  enum ItemKind {
    KindNone             = DataModel::KindNone,
    KindGroup            = DataModel::KindGroup,
    KindDataset          = DataModel::KindDataset,
    KindWorkspace        = DataModel::KindWorkspace,
    KindWorkspaceFolder  = DataModel::KindWorkspaceFolder,
    KindAction           = DataModel::KindAction,
    KindOutputWidget     = DataModel::KindOutputWidget,
    KindMqttPublisher    = DataModel::KindMqttPublisher,
    KindControlScript    = DataModel::KindControlScript,
    KindGroupFolder      = DataModel::KindGroupFolder,
    KindUserTable        = DataModel::KindUserTable,
    KindTableFolder      = DataModel::KindTableFolder,
    KindSource           = DataModel::KindSource,
    KindProjectRoot      = DataModel::KindProjectRoot,
    KindFrameParser      = DataModel::KindFrameParser,
    KindGroupsRoot       = DataModel::KindGroupsRoot,
    KindTablesRoot       = DataModel::KindTablesRoot,
    KindSystemDatasets   = DataModel::KindSystemDatasets,
    KindWorkspacesRoot   = DataModel::KindWorkspacesRoot,
    KindInfluxSink       = DataModel::KindInfluxSink,
    KindTransformLibrary = DataModel::KindTransformLibrary,
    KindScriptsRoot      = DataModel::KindScriptsRoot,
    KindJsLibrary        = DataModel::KindJsLibrary,
    KindExportRoot       = DataModel::KindExportRoot,
  };
  Q_ENUM(ItemKind)

  [[nodiscard]] CurrentView currentView() const;
  [[nodiscard]] bool canGoBack() const noexcept;
  [[nodiscard]] bool canGoForward() const noexcept;
  [[nodiscard]] Q_INVOKABLE int navDirection() const noexcept;
  [[nodiscard]] Q_INVOKABLE bool treeIndexHasChildren(const QModelIndex& index) const;
  [[nodiscard]] Q_INVOKABLE bool treeIndexExpanded(const QModelIndex& index) const;
  [[nodiscard]] int multiSelectionKind() const noexcept;
  [[nodiscard]] int multiSelectionCount() const noexcept;
  [[nodiscard]] QString selectedText() const;
  [[nodiscard]] QString selectedIcon() const;
  [[nodiscard]] const QString actionIcon() const;
  [[nodiscard]] const QString outputWidgetIcon() const;
  [[nodiscard]] const QStringList& availableActionIcons() const;
  [[nodiscard]] bool canMoveCurrentUp() const;
  [[nodiscard]] bool canMoveCurrentDown() const;
  [[nodiscard]] bool currentGroupIsEditable() const;
  [[nodiscard]] bool currentDatasetIsEditable() const;
  [[nodiscard]] bool datasetWidgetEditable(const DataModel::Dataset& dataset) const;
  [[nodiscard]] bool selectedGroupEnabled() const;
  [[nodiscard]] bool selectedDatasetEnabled() const;
  [[nodiscard]] bool currentGroupIsOutputPanel() const;
  [[nodiscard]] int outputWidgetType() const noexcept;
  [[nodiscard]] quint16 datasetOptions() const;

  [[nodiscard]] int selectedSourceId() const noexcept;
  [[nodiscard]] int selectedSourceBusType() const noexcept;
  [[nodiscard]] QString selectedSourceFrameParserCode() const;
  [[nodiscard]] QString currentGroupPainterCode() const;
  [[nodiscard]] bool currentGroupIsPainter() const;
  [[nodiscard]] int currentGroupId() const;
  Q_INVOKABLE [[nodiscard]] QVariantList currentGroupDatasetsForPreview() const;

  [[nodiscard]] CustomModel* treeModel() const;
  [[nodiscard]] QItemSelectionModel* selectionModel() const;
  [[nodiscard]] CustomModel* groupModel() const;
  [[nodiscard]] CustomModel* sourceModel() const;
  [[nodiscard]] CustomModel* actionModel() const;
  [[nodiscard]] CustomModel* projectModel() const;
  [[nodiscard]] CustomModel* datasetModel() const;
  [[nodiscard]] CustomModel* outputWidgetModel() const;
  [[nodiscard]] CustomModel* mqttPublisherModel() const;
  [[nodiscard]] CustomModel* influxSinkModel() const;
  [[nodiscard]] const DataModel::OutputWidget& selectedOutputWidget() const noexcept;

  Q_INVOKABLE [[nodiscard]] QVariantList selectedTreeItems() const;

  Q_INVOKABLE [[nodiscard]] QVariantList tablesSummary() const;
  Q_INVOKABLE [[nodiscard]] QVariantList systemDatasetsSummary() const;
  [[nodiscard]] QString selectedUserTable() const;

  Q_INVOKABLE [[nodiscard]] QVariantList workspacesSummary() const;
  Q_INVOKABLE [[nodiscard]] QVariantList widgetsForWorkspace(int workspaceId) const;
  Q_INVOKABLE [[nodiscard]] QVariantList allWidgetsSummary() const;
  Q_INVOKABLE [[nodiscard]] bool workspaceHasUnresolvedRefs(int workspaceId) const;
  Q_INVOKABLE [[nodiscard]] int unresolvedWorkspaceWidgetCount() const;
  Q_INVOKABLE int cleanupUnresolvedWorkspaceWidgets();
  Q_INVOKABLE [[nodiscard]] QVariantList workspaceFolderTree() const;
  Q_INVOKABLE [[nodiscard]] QVariantList workspaceFolderContents(int parentFolderId) const;
  [[nodiscard]] int selectedWorkspaceId() const noexcept;
  [[nodiscard]] int selectedFolderId() const noexcept;

  Q_INVOKABLE [[nodiscard]] QVariantList groupFolderTree() const;
  Q_INVOKABLE [[nodiscard]] QVariantList groupFolderContents(int parentFolderId) const;
  Q_INVOKABLE [[nodiscard]] QVariantMap groupFolderPaths() const;
  Q_INVOKABLE [[nodiscard]] QVariantList tableFolderTree() const;
  Q_INVOKABLE [[nodiscard]] QVariantList tableFolderContents(int parentFolderId) const;
  [[nodiscard]] int selectedGroupFolderId() const noexcept;
  [[nodiscard]] int selectedTableFolderId() const noexcept;

  [[nodiscard]] const QString& treeSearchQuery() const noexcept;

public slots:
  void navigateBack();
  void navigateForward();
  void selectUserTable(const QString& tableName);
  void selectWorkspace(int workspaceId);
  void selectWorkspaceFolder(int folderId);
  void selectGroupFolder(int folderId);
  void selectTableFolder(int folderId);
  void selectMqttPublisher();
  void selectInfluxSink();
  void selectControlScript();
  void selectTransformLibrary();
  void selectJsLibrary();
  void selectProjectScripts();
  void selectDataExport();
  void openMqttScriptEditor();
  void setTreeSearchQuery(const QString& query);
  void confirmCleanupUnresolvedWorkspaceWidgets();
  void persistTreeExpansion();
  void expandAllTreeItems();
  void collapseTreeToOverview();
  void expandTreeToIndex(const QModelIndex& index);
  void setTreeIndexExpanded(const QModelIndex& index, bool expanded);

  void buildTreeModel();
  void buildProjectModel();
  void buildGroupModel(const DataModel::Group& group);
  void buildSourceModel(const DataModel::Source& source);
  void buildActionModel(const DataModel::Action& action);
  void buildDatasetModel(const DataModel::Dataset& dataset);
  void buildOutputWidgetModel(const DataModel::OutputWidget& widget);
  void buildMqttPublisherModel();
  void displayFrameParserView(int sourceId);
  void selectSource(int sourceId);
  void selectGroup(int groupId);
  void selectDataset(int groupId, int datasetId);
  void selectAction(int actionId);
  void selectOutputWidget(int groupId, int widgetId);
  void selectFrameParser(int sourceId);
  void setSelectedSourceFrameParserCode(const QString& code);
  void setSelectedOutputWidgetTransmitFunction(const QString& code);
  void setCurrentGroupPainterCode(const QString& code);
  void openTransformEditor();
  void openTransformEditorFor(int groupId, int datasetId);
  void setSuppressViewChange(bool suppress) noexcept;
  void openAlarmBandsEditorForSelection();
  void commitAlarmBands(const QVariantList& bands);
  void openFrequencyMarkersEditorForSelection();
  void commitFrequencyMarkers(const QVariantList& markers);
  void changeDatasetOptionForSelection(int option, bool checked);

  Q_INVOKABLE [[nodiscard]] bool moveCurrentGroup(int direction);
  Q_INVOKABLE [[nodiscard]] bool moveCurrentDataset(int direction);
  Q_INVOKABLE [[nodiscard]] bool moveCurrentAction(int direction);
  Q_INVOKABLE [[nodiscard]] bool moveCurrentOutputWidget(int direction);
  Q_INVOKABLE [[nodiscard]] bool moveWorkspace(int workspaceId, int direction);

private slots:
  void appendExtensionWidgets();
  void generateComboBoxModels();
  void setCurrentView(const DataModel::ProjectEditor::CurrentView view);

private:
  void addGeneralSection(CustomModel* model, const DataModel::Dataset& dataset);
  void addGeneralColorRow(CustomModel* model, const DataModel::Dataset& dataset);
  void addDatasetAliasRow(CustomModel* model, const DataModel::Dataset& dataset);
  void addDatasetRangeRows(CustomModel* model, const DataModel::Dataset& dataset);
  void addPlotSection(CustomModel* model, const DataModel::Dataset& dataset);
  void addFFTSection(CustomModel* model, const DataModel::Dataset& dataset);
  void addWidgetSection(CustomModel* model, const DataModel::Dataset& dataset);
  void addLEDSection(CustomModel* model, const DataModel::Dataset& dataset);

  void buildFftGeneralRows(CustomModel* model, const DataModel::Dataset& dataset);
  void buildFftRangeRows(CustomModel* model, const DataModel::Dataset& dataset);

  void buildWidgetRangeRows(CustomModel* model,
                            const DataModel::Dataset& dataset,
                            bool rangeEnabled);
  void buildWidgetFormatRows(CustomModel* model,
                             const DataModel::Dataset& dataset,
                             bool rangeEnabled);

private:
  DataModel::ProjectModel& m_projectModelRef;
  IO::ConnectionManager& m_connectionManager;

  CurrentView m_currentView;
  bool m_suppressViewChange;

  ItemKind m_batchKind;
  bool m_batchApplying;
  QVector<QPair<int, int>> m_batchItems;

  QMap<QStandardItem*, int> m_rootItems;
  QMap<QStandardItem*, DataModel::Group> m_groupItems;
  QMap<QStandardItem*, DataModel::Source> m_sourceItems;
  QMap<QStandardItem*, DataModel::Action> m_actionItems;
  QMap<QStandardItem*, DataModel::Dataset> m_datasetItems;
  QMap<QStandardItem*, DataModel::OutputWidget> m_outputWidgetItems;
  QMap<QStandardItem*, DataModel::Source> m_sourceParserItems;

  QStandardItem* m_groupsRootItem;
  QMap<QStandardItem*, int> m_groupFolderItems;
  int m_selectedGroupFolderId;

  QStandardItem* m_tablesRootItem;
  QStandardItem* m_systemDatasetsItem;
  QMap<QStandardItem*, QString> m_userTableItems;
  QMap<QStandardItem*, int> m_tableFolderItems;
  QString m_selectedUserTable;
  int m_selectedTableFolderId;

  QStandardItem* m_workspacesRootItem;
  QMap<QStandardItem*, int> m_workspaceItems;
  QMap<QStandardItem*, int> m_workspaceFolderItems;
  int m_selectedWorkspaceId;
  int m_selectedFolderId;

  QStandardItem* m_mqttPublisherItem;
  QStandardItem* m_influxSinkItem;
  QStandardItem* m_controlScriptItem;
  QStandardItem* m_transformLibraryItem;
  QStandardItem* m_jsLibraryItem;
  QStandardItem* m_scriptsRootItem;
  QStandardItem* m_exportRootItem;

  QString m_treeSearchQuery;
  bool m_seedExpansionFromModel;

  DataModel::Group m_selectedGroup;
  DataModel::Action m_selectedAction;
  DataModel::Source m_selectedSource;
  DataModel::Dataset m_selectedDataset;
  DataModel::OutputWidget m_selectedOutputWidget;

  CustomModel* m_treeModel;
  QItemSelectionModel* m_selectionModel;

  CustomModel* m_groupModel;
  CustomModel* m_sourceModel;
  CustomModel* m_actionModel;
  CustomModel* m_projectModel;
  CustomModel* m_datasetModel;
  CustomModel* m_outputWidgetModel;
  CustomModel* m_mqttPublisherModel;
  CustomModel* m_influxSinkModel;

  QStringList m_fftSamples;
  QStringList m_fftWindows;
  QList<SerialStudio::FFTWindow> m_fftWindowValues;
  QStringList m_timerModes;
  QStringList m_decoderOptions;
  QStringList m_checksumMethods;
  QStringList m_imgDetectionModes;
  QStringList m_outputWidgetTypes;
  QStringList m_frameDetectionMethods;
  QList<SerialStudio::FrameDetection> m_frameDetectionMethodsValues;

  /**
   * @brief Kind of node queued for selection after the next tree rebuild.
   */
  enum class PendingSelectionKind {
    None,
    Source,
    Group,
    Dataset,
    OutputWidget
  };
  PendingSelectionKind m_pendingSelectionKind;
  int m_pendingSelectionGroupId;
  int m_pendingSelectionItemId;

  QMap<QString, QString> m_eolSequences;
  QMap<QString, QString> m_groupWidgets;
  QMap<QString, QString> m_datasetWidgets;
  QMap<QString, QString> m_displayFormats;
  QMap<QPair<bool, bool>, QString> m_plotOptions;

  EditorWiring m_wiring;
  EditorSelection m_selection;
  EditorTree m_tree;
  EditorForms m_forms;
  EditorCommit m_commit;
  EditorSummaries m_summaries;
  EditorMultiSelect m_multiSelect;
  EditorMqtt m_mqtt;
  EditorInflux m_influx;
};

/**
 * @brief QStandardItemModel subclass exposing Project Editor custom roles to QML.
 */
class CustomModel : public QStandardItemModel {
  // clang-format off
  Q_OBJECT
  // clang-format on

public:
  explicit CustomModel(QObject* parent = nullptr) : QStandardItemModel(parent) {}

  QHash<int, QByteArray> roleNames() const override
  {
    // clang-format off
#define BAL(x) QByteArrayLiteral(x)
    static const QHash<int, QByteArray> kNames = {
      {ProjectEditor::Active,                 BAL("active")},
      {ProjectEditor::WidgetType,             BAL("widgetType")},
      {ProjectEditor::TreeViewIcon,           BAL("treeViewIcon")},
      {ProjectEditor::TreeViewText,           BAL("treeViewText")},
      {ProjectEditor::ComboBoxData,           BAL("comboBoxData")},
      {ProjectEditor::ParameterIcon,          BAL("parameterIcon")},
      {ProjectEditor::ParameterName,          BAL("parameterName")},
      {ProjectEditor::EditableValue,          BAL("editableValue")},
      {ProjectEditor::ParameterType,          BAL("parameterType")},
      {ProjectEditor::PlaceholderValue,       BAL("placeholderValue")},
      {ProjectEditor::TreeViewExpanded,       BAL("treeViewExpanded")},
      {ProjectEditor::TreeViewFrameIndex,     BAL("treeViewFrameIndex")},
      {ProjectEditor::ParameterDescription,   BAL("parameterDescription")},
      {ProjectEditor::ParameterKey,           BAL("parameterKey")},
      {ProjectEditor::TreeViewSourceName,     BAL("treeViewSourceName")},
      {ProjectEditor::TreeViewSourceId,       BAL("treeViewSourceId")},
      {ProjectEditor::TreeViewVirtual,        BAL("treeViewVirtual")},
      {ProjectEditor::TreeItemKind,           BAL("treeItemKind")},
      {ProjectEditor::TreeItemId,             BAL("treeItemId")},
      {ProjectEditor::TreeItemParentId,       BAL("treeItemParentId")},
      {ProjectEditor::TreeItemPath,           BAL("treeItemPath")},
      {ProjectEditor::TreeViewWorkspaceStale, BAL("treeViewWorkspaceStale")},
      {ProjectEditor::TreeViewEnabled,        BAL("treeViewEnabled")},
      {ProjectEditor::TreeViewSelfEnabled,    BAL("treeViewSelfEnabled")},
      {ProjectEditor::MinValue,               BAL("minValue")},
      {ProjectEditor::MaxValue,               BAL("maxValue")},
    };
#undef BAL
    // clang-format on

    return kNames;
  }
};
}  // namespace DataModel
