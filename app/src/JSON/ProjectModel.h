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

#include <QObject>
#include <QStandardItemModel>
#include <QItemSelectionModel>

#include "JSON/Frame.h"
#include "SerialStudio.h"

namespace JSON
{
class CustomModel;

/**
 * @brief The core class for managing the project structure and data models in
 *        the application.
 *
 * The `Model` class handles the loading, saving, and modification of project
 * data from JSON files.
 *
 * It provides access to project properties such as groups, datasets, and
 * settings. The class supports operations for modifying the project structure,
 * including adding, deleting, and duplicating groups and datasets.
 *
 * It also builds and manages various models that represent the project's
 * hierarchical data, such as the tree model, group model, and dataset model.
 *
 * Key functionalities include:
 * - Loading and saving projects from/to JSON files.
 * - Managing and organizing groups and datasets.
 * - Handling user interactions such as changing the view and editing
 *   project properties.
 * - Emitting signals to notify the user interface of changes.
 *
 * This class follows a singleton pattern, ensuring there is only one instance
 * managing the project.
 *
 * It integrates closely with other components of the application, including
 * the tree view for project navigation and data parsing mechanisms for frame
 * analysis.
 */
class ProjectModel : public QObject
{
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
  Q_PROPERTY(QAbstractItemModel* treeModel
             READ treeModel
             NOTIFY treeModelChanged)
  Q_PROPERTY(QItemSelectionModel* selectionModel
             READ selectionModel
             NOTIFY treeModelChanged)
  Q_PROPERTY(CurrentView currentView
             READ currentView
             NOTIFY currentViewChanged)
  Q_PROPERTY(QString selectedText
             READ selectedText
             NOTIFY currentViewChanged)
  Q_PROPERTY(QString selectedIcon
             READ selectedIcon
             NOTIFY currentViewChanged)
  Q_PROPERTY(int groupCount
             READ groupCount
             NOTIFY modifiedChanged)
  Q_PROPERTY(int datasetCount
             READ datasetCount
             NOTIFY modifiedChanged)
  Q_PROPERTY(bool containsCommercialFeatures
             READ containsCommercialFeatures
             NOTIFY modifiedChanged)
  Q_PROPERTY(quint8 datasetOptions
             READ datasetOptions
             NOTIFY datasetOptionsChanged)
  Q_PROPERTY(bool currentGroupIsEditable
             READ currentGroupIsEditable
             NOTIFY editableOptionsChanged)
  Q_PROPERTY(bool currentDatasetIsEditable
             READ currentDatasetIsEditable
             NOTIFY editableOptionsChanged)
  Q_PROPERTY(QString actionIcon
             READ actionIcon
             NOTIFY actionModelChanged)
  Q_PROPERTY(QStringList availableActionIcons
             READ availableActionIcons
             CONSTANT)
  // clang-format on

signals:
  void titleChanged();
  void jsonFileChanged();
  void modifiedChanged();
  void treeModelChanged();
  void groupModelChanged();
  void currentViewChanged();
  void actionModelChanged();
  void projectModelChanged();
  void datasetModelChanged();
  void datasetOptionsChanged();
  void frameDetectionChanged();
  void editableOptionsChanged();
  void frameParserCodeChanged();

private:
  explicit ProjectModel();
  ProjectModel(ProjectModel &&) = delete;
  ProjectModel(const ProjectModel &) = delete;
  ProjectModel &operator=(ProjectModel &&) = delete;
  ProjectModel &operator=(const ProjectModel &) = delete;

public:
  static ProjectModel &instance();

  /**
   * @brief Enum representing the different views available in the project
   *        editor.
   */
  enum CurrentView
  {
    ProjectView,
    GroupView,
    DatasetView,
    FrameParserView,
    ActionView
  };
  Q_ENUM(CurrentView)

  /**
   * @brief Enum representing the different types of editor widgets available
   *        for editing parameters.
   */
  enum EditorWidget
  {
    TextField,
    HexTextField,
    IntField,
    FloatField,
    CheckBox,
    ComboBox,
    IconPicker,
    SectionHeader,
  };
  Q_ENUM(EditorWidget)

  /**
   * @brief Enum representing custom roles used in the tree view for data
   *        access.
   */
  enum CustomRoles
  {
    Active = Qt::UserRole,

    TreeViewIcon = Qt::UserRole + 1,
    TreeViewText = Qt::UserRole + 2,
    TreeViewExpanded = Qt::UserRole + 3,
    TreeViewFrameIndex = Qt::UserRole + 4,

    ParameterName = Qt::UserRole + 5,
    EditableValue = Qt::UserRole + 6,
    ParameterType = Qt::UserRole + 7,
    PlaceholderValue = Qt::UserRole + 8,
    ParameterDescription = Qt::UserRole + 9,
    ParameterIcon = Qt::UserRole + 10,

    WidgetType = Qt::UserRole + 11,
    ComboBoxData = Qt::UserRole + 12,
  };
  Q_ENUM(CustomRoles)

  [[nodiscard]] bool modified() const;
  [[nodiscard]] CurrentView currentView() const;
  [[nodiscard]] SerialStudio::DecoderMethod decoderMethod() const;
  [[nodiscard]] SerialStudio::FrameDetection frameDetection() const;

  [[nodiscard]] QString jsonFileName() const;
  [[nodiscard]] QString jsonProjectsPath() const;

  [[nodiscard]] QString selectedText() const;
  [[nodiscard]] QString selectedIcon() const;

  [[nodiscard]] QStringList xDataSources() const;

  [[nodiscard]] const QString actionIcon() const;
  [[nodiscard]] const QStringList &availableActionIcons() const;

  [[nodiscard]] const QString &title() const;
  [[nodiscard]] const QString &jsonFilePath() const;
  [[nodiscard]] const QString &frameParserCode() const;

  [[nodiscard]] bool currentGroupIsEditable() const;
  [[nodiscard]] bool currentDatasetIsEditable() const;
  [[nodiscard]] bool containsCommercialFeatures() const;

  [[nodiscard]] int groupCount() const;
  [[nodiscard]] int datasetCount() const;
  [[nodiscard]] quint8 datasetOptions() const;
  [[nodiscard]] const std::vector<Group> &groups() const;

  [[nodiscard]] CustomModel *treeModel() const;
  [[nodiscard]] QItemSelectionModel *selectionModel() const;

  [[nodiscard]] CustomModel *groupModel() const;
  [[nodiscard]] CustomModel *actionModel() const;
  [[nodiscard]] CustomModel *projectModel() const;
  [[nodiscard]] CustomModel *datasetModel() const;

  Q_INVOKABLE bool askSave();
  Q_INVOKABLE bool saveJsonFile(const bool askPath = false);

public slots:
  void setupExternalConnections();

  void newJsonFile();
  void openJsonFile();
  void openJsonFile(const QString &path);

  void enableProjectMode();

  void deleteCurrentGroup();
  void deleteCurrentAction();
  void deleteCurrentDataset();

  void duplicateCurrentGroup();
  void duplicateCurrentAction();
  void duplicateCurrentDataset();

  void ensureValidGroup();
  void addDataset(const SerialStudio::DatasetOption options);
  void changeDatasetOption(const SerialStudio::DatasetOption option,
                           const bool checked);

  void addAction();
  void addGroup(const QString &title, const SerialStudio::GroupWidget widget);
  bool setGroupWidget(const int group, const SerialStudio::GroupWidget widget);

  void setModified(const bool modified);
  void setFrameParserCode(const QString &code);

  void displayFrameParserView();

  void buildTreeModel();
  void buildProjectModel();
  void buildGroupModel(const JSON::Group &group);
  void buildActionModel(const JSON::Action &action);
  void buildDatasetModel(const JSON::Dataset &dataset);

private slots:
  void onJsonLoaded();
  void generateComboBoxModels();
  void setCurrentView(const CurrentView view);
  void onGroupItemChanged(QStandardItem *item);
  void onActionItemChanged(QStandardItem *item);
  void onProjectItemChanged(QStandardItem *item);
  void onDatasetItemChanged(QStandardItem *item);
  void onCurrentSelectionChanged(const QModelIndex &current,
                                 const QModelIndex &previous);

private:
  int nextDatasetIndex();
  bool finalizeProjectSave();

  void saveExpandedStateMap(QStandardItem *item, QHash<QString, bool> &map,
                            const QString &title);
  void restoreExpandedStateMap(QStandardItem *item, QHash<QString, bool> &map,
                               const QString &title);

private:
  QString m_title;
  QString m_frameParserCode;
  QString m_frameEndSequence;
  QString m_checksumAlgorithm;
  QString m_frameStartSequence;
  bool m_hexadecimalDelimiters;

  CurrentView m_currentView;
  SerialStudio::DecoderMethod m_frameDecoder;
  SerialStudio::FrameDetection m_frameDetection;

  bool m_modified;
  QString m_filePath;

  QMap<QStandardItem *, int> m_rootItems;
  QMap<QStandardItem *, JSON::Group> m_groupItems;
  QMap<QStandardItem *, JSON::Action> m_actionItems;
  QMap<QStandardItem *, JSON::Dataset> m_datasetItems;

  std::vector<JSON::Group> m_groups;
  std::vector<JSON::Action> m_actions;

  CustomModel *m_treeModel;
  QItemSelectionModel *m_selectionModel;

  CustomModel *m_groupModel;
  CustomModel *m_actionModel;
  CustomModel *m_projectModel;
  CustomModel *m_datasetModel;

  QStringList m_fftSamples;
  QStringList m_timerModes;
  QStringList m_decoderOptions;
  QStringList m_checksumMethods;
  QStringList m_frameDetectionMethods;
  QList<SerialStudio::FrameDetection> m_frameDetectionMethodsValues;

  QMap<QString, QString> m_eolSequences;
  QMap<QString, QString> m_groupWidgets;
  QMap<QString, QString> m_datasetWidgets;
  QMap<QPair<bool, bool>, QString> m_plotOptions;

  JSON::Group m_selectedGroup;
  JSON::Action m_selectedAction;
  JSON::Dataset m_selectedDataset;
};

/**
 * @brief A custom data model extending QStandardItemModel for managing
 *        project-specific data in the UI.
 *
 * The `CustomModel` class is a specialized model used for representing
 * different types of project data (such as groups, datasets, and settings) in
 * the user interface. It provides the ability to define custom roles for
 * accessing item properties in QML, enabling dynamic interaction with the
 * project data in a tree view or other UI components.
 *
 * The class overrides the `roleNames()` function to return a custom set of
 * roles, which are essential for mapping data between the C++ backend and the
 * QML frontend. It supports common Qt item model functionalities, allowing data
 * to be set, modified, and accessed within the project structure.
 */
class CustomModel : public QStandardItemModel
{
public:
  explicit CustomModel(QObject *parent = 0)
    : QStandardItemModel(parent)
  {
  }

  QHash<int, QByteArray> roleNames() const override
  {
    QHash<int, QByteArray> names;

    // clang-format off
#define BAL(x) QByteArrayLiteral(x)
    names.insert(ProjectModel::Active, BAL("active"));
    names.insert(ProjectModel::WidgetType, BAL("widgetType"));
    names.insert(ProjectModel::TreeViewIcon, BAL("treeViewIcon"));
    names.insert(ProjectModel::TreeViewText, BAL("treeViewText"));
    names.insert(ProjectModel::ComboBoxData, BAL("comboBoxData"));
    names.insert(ProjectModel::ParameterIcon, BAL("parameterIcon"));
    names.insert(ProjectModel::ParameterName, BAL("parameterName"));
    names.insert(ProjectModel::EditableValue, BAL("editableValue"));
    names.insert(ProjectModel::ParameterType, BAL("parameterType"));
    names.insert(ProjectModel::PlaceholderValue, BAL("placeholderValue"));
    names.insert(ProjectModel::TreeViewExpanded, BAL("treeViewExpanded"));
    names.insert(ProjectModel::TreeViewFrameIndex, BAL("treeViewFrameIndex"));
    names.insert(ProjectModel::ParameterDescription, BAL("parameterDescription"));
#undef BAL
    // clang-format on

    return names;
  }
};
} // namespace JSON
