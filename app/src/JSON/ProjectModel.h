/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#pragma once

#include <QObject>
#include <JSON/Group.h>
#include <JSON/Action.h>
#include <JSON/Dataset.h>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <Misc/OsmTemplateServer.h>

#include <SerialStudio.h>

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
  Q_PROPERTY(CustomModel* groupModel
             READ groupModel
             NOTIFY groupModelChanged)
  Q_PROPERTY(CustomModel* actionModel
             READ actionModel
             NOTIFY actionModelChanged)
  Q_PROPERTY(CustomModel* projectModel
             READ projectModel
             NOTIFY projectModelChanged)
  Q_PROPERTY(CustomModel* datasetModel
             READ datasetModel
             NOTIFY datasetModelChanged)
  Q_PROPERTY(CustomModel* treeModel
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
  Q_PROPERTY(QString osmAddress
             READ osmAddress
             CONSTANT)
  // clang-format on

signals:
  void titleChanged();
  void jsonFileChanged();
  void modifiedChanged();
  void treeModelChanged();
  void groupModelChanged();
  void gpsApiKeysChanged();
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
    IntField,
    FloatField,
    CheckBox,
    ComboBox,
    IconPicker
  };
  Q_ENUM(EditorWidget)

  /**
   * @brief Enum representing custom roles used in the tree view for data
   *        access.
   */
  enum CustomRoles
  {
    TreeViewIcon = 0x01,
    TreeViewText = 0x02,
    TreeViewExpanded = 0x03,
    TreeViewFrameIndex = 0x04,

    ParameterName = 0x10,
    EditableValue = 0x11,
    ParameterType = 0x12,
    PlaceholderValue = 0x13,
    ParameterDescription = 0x14,

    WidgetType = 0x20,
    ComboBoxData = 0x21,
  };
  Q_ENUM(CustomRoles)

  [[nodiscard]] bool modified() const;
  [[nodiscard]] CurrentView currentView() const;
  [[nodiscard]] SerialStudio::DecoderMethod decoderMethod() const;
  [[nodiscard]] SerialStudio::FrameDetection frameDetection() const;

  [[nodiscard]] QString jsonFileName() const;
  [[nodiscard]] QString jsonProjectsPath() const;

  [[nodiscard]] QString osmAddress() const;
  [[nodiscard]] QString selectedText() const;
  [[nodiscard]] QString selectedIcon() const;

  [[nodiscard]] const QString actionIcon() const;
  [[nodiscard]] const QStringList &availableActionIcons() const;

  [[nodiscard]] const QString &title() const;
  [[nodiscard]] const QString &jsonFilePath() const;
  [[nodiscard]] const QString &frameParserCode() const;
  [[nodiscard]] const QString &mapTilerApiKey() const;
  [[nodiscard]] const QString &thunderforestApiKey() const;

  [[nodiscard]] bool currentGroupIsEditable() const;
  [[nodiscard]] bool currentDatasetIsEditable() const;

  [[nodiscard]] int groupCount() const;
  [[nodiscard]] int datasetCount() const;
  [[nodiscard]] quint8 datasetOptions() const;
  [[nodiscard]] const QVector<JSON::Group> &groups() const;

  [[nodiscard]] CustomModel *treeModel() const;
  [[nodiscard]] QItemSelectionModel *selectionModel() const;

  [[nodiscard]] CustomModel *groupModel() const;
  [[nodiscard]] CustomModel *actionModel() const;
  [[nodiscard]] CustomModel *projectModel() const;
  [[nodiscard]] CustomModel *datasetModel() const;

  Q_INVOKABLE bool askSave();
  Q_INVOKABLE bool saveJsonFile();

public slots:
  void setupExternalConnections();

  void newJsonFile();
  void openJsonFile();
  void openJsonFile(const QString &path);

  void deleteCurrentGroup();
  void deleteCurrentAction();
  void deleteCurrentDataset();
  void duplicateCurrentGroup();
  void duplicateCurrentAction();
  void duplicateCurrentDataset();
  void addDataset(const SerialStudio::DatasetOption options);
  void changeDatasetOption(const SerialStudio::DatasetOption option,
                           const bool checked);

  void addAction();
  void addGroup(const QString &title, const SerialStudio::GroupWidget widget);
  bool setGroupWidget(const int group, const SerialStudio::GroupWidget widget);

  void setFrameParserCode(const QString &code);

  void buildTreeModel();
  void buildProjectModel();
  void buildGroupModel(const JSON::Group &group);
  void buildActionModel(const JSON::Action &action);
  void buildDatasetModel(const JSON::Dataset &dataset);

private slots:
  void onJsonLoaded();
  void onGpsApiKeysChanged();
  void generateComboBoxModels();
  void setModified(const bool modified);
  void setCurrentView(const CurrentView view);
  void onGroupItemChanged(QStandardItem *item);
  void onActionItemChanged(QStandardItem *item);
  void onProjectItemChanged(QStandardItem *item);
  void onDatasetItemChanged(QStandardItem *item);
  void onCurrentSelectionChanged(const QModelIndex &current,
                                 const QModelIndex &previous);

private:
  int nextDatasetIndex();
  void saveExpandedStateMap(QStandardItem *item, QHash<QString, bool> &map,
                            const QString &title);
  void restoreExpandedStateMap(QStandardItem *item, QHash<QString, bool> &map,
                               const QString &title);

private:
  QString m_title;
  QString m_separator;
  QString m_frameParserCode;
  QString m_frameEndSequence;
  QString m_frameStartSequence;

  QString m_mapTilerApiKey;
  QString m_thunderforestApiKey;

  CurrentView m_currentView;
  SerialStudio::DecoderMethod m_frameDecoder;
  SerialStudio::FrameDetection m_frameDetection;

  bool m_modified;
  QString m_filePath;

  QMap<QStandardItem *, int> m_rootItems;
  QMap<QStandardItem *, JSON::Group> m_groupItems;
  QMap<QStandardItem *, JSON::Action> m_actionItems;
  QMap<QStandardItem *, JSON::Dataset> m_datasetItems;

  QVector<JSON::Group> m_groups;
  QVector<JSON::Action> m_actions;

  CustomModel *m_treeModel;
  QItemSelectionModel *m_selectionModel;

  CustomModel *m_groupModel;
  CustomModel *m_actionModel;
  CustomModel *m_projectModel;
  CustomModel *m_datasetModel;

  QStringList m_fftSamples;
  QStringList m_decoderOptions;
  QStringList m_frameDetectionMethods;
  QMap<QString, QString> m_eolSequences;
  QMap<QString, QString> m_groupWidgets;
  QMap<QString, QString> m_datasetWidgets;
  QMap<QPair<bool, bool>, QString> m_plotOptions;

  JSON::Group m_selectedGroup;
  JSON::Action m_selectedAction;
  JSON::Dataset m_selectedDataset;

  OsmTemplateServer m_server;
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
#define BAL(x) QByteArrayLiteral(x)
    names.insert(ProjectModel::TreeViewIcon, BAL("treeViewIcon"));
    names.insert(ProjectModel::TreeViewText, BAL("treeViewText"));
    names.insert(ProjectModel::TreeViewExpanded, BAL("treeViewExpanded"));
    names.insert(ProjectModel::TreeViewFrameIndex, BAL("treeViewFrameIndex"));
    names.insert(ProjectModel::ParameterName, BAL("parameterName"));
    names.insert(ProjectModel::EditableValue, BAL("editableValue"));
    names.insert(ProjectModel::ParameterType, BAL("parameterType"));
    names.insert(ProjectModel::PlaceholderValue, BAL("placeholderValue"));
    names.insert(ProjectModel::ParameterDescription,
                 BAL("parameterDescription"));
    names.insert(ProjectModel::WidgetType, BAL("widgetType"));
    names.insert(ProjectModel::ComboBoxData, BAL("comboBoxData"));
#undef BAL
    return names;
  }
};
} // namespace JSON
