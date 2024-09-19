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
#include <JSON/Dataset.h>
#include <QStandardItemModel>
#include <QItemSelectionModel>

namespace Project
{
class CustomModel;
class Model : public QObject
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
  // clang-format on

signals:
  void titleChanged();
  void jsonFileChanged();
  void modifiedChanged();
  void treeModelChanged();
  void groupModelChanged();
  void currentViewChanged();
  void projectModelChanged();
  void datasetModelChanged();
  void frameParserCodeChanged();
  void thunderforestApyKeyChanged();
  void groupAdded(const QModelIndex &index);
  void datasetAdded(const QModelIndex &index);

private:
  explicit Model();
  Model(Model &&) = delete;
  Model(const Model &) = delete;
  Model &operator=(Model &&) = delete;
  Model &operator=(const Model &) = delete;

public:
  static Model &instance();

  enum CurrentView
  {
    ProjectView,
    GroupView,
    DatasetView,
    FrameParserView,
  };
  Q_ENUM(CurrentView)

  enum DecoderMethod
  {
    Normal,
    Hexadecimal,
    Base64
  };
  Q_ENUM(DecoderMethod)

  enum GroupWidget
  {
    CustomGroup,
    Accelerometer,
    Gyroscope,
    GPS,
    MultiPlot
  };
  Q_ENUM(GroupWidget)

  enum DatasetOption
  {
    DatasetGeneric,
    DatasetPlot,
    DatasetFFT,
    DatasetBar,
    DatasetGauge,
    DatasetCompass
  };
  Q_ENUM(DatasetOption)

  enum EditorWidget
  {
    TextField,
    IntField,
    FloatField,
    CheckBox,
    ComboBox
  };
  Q_ENUM(EditorWidget)

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

  [[nodiscard]] QString jsonFileName() const;
  [[nodiscard]] QString jsonProjectsPath() const;

  [[nodiscard]] QString selectedText() const;
  [[nodiscard]] QString selectedIcon() const;

  [[nodiscard]] const QString &title() const;
  [[nodiscard]] const QString &jsonFilePath() const;
  [[nodiscard]] const QString &frameParserCode() const;
  [[nodiscard]] const QString &thunderforestApiKey() const;

  [[nodiscard]] int groupCount() const;
  [[nodiscard]] const QVector<JSON::Group> &groups() const;

  [[nodiscard]] CustomModel *treeModel() const;
  [[nodiscard]] QItemSelectionModel *selectionModel() const;

  [[nodiscard]] CustomModel *groupModel() const;
  [[nodiscard]] CustomModel *projectModel() const;
  [[nodiscard]] CustomModel *datasetModel() const;

  Q_INVOKABLE bool askSave();
  Q_INVOKABLE bool saveJsonFile();

public slots:
  void newJsonFile();
  void openJsonFile();
  void openJsonFile(const QString &path);

  void deleteCurrentGroup();
  void deleteCurrentDataset();
  void duplicateCurrentGroup();
  void duplicateCurrentDataset();
  void changeDatasetParentGroup();
  void addDataset(const DatasetOption options);
  void changeDatasetOptions(const DatasetOption options);

  void addGroup(const QString &title, const GroupWidget widget);
  bool setGroupWidget(const int group, const GroupWidget widget);

  void setFrameParserCode(const QString &code);

  void buildTreeModel();
  void buildProjectModel();
  void buildGroupModel(const JSON::Group &group);
  void buildDatasetModel(const JSON::Dataset &dataset);

private slots:
  void onJsonLoaded();
  void generateComboBoxModels();
  void setModified(const bool modified);
  void setCurrentView(const CurrentView view);
  void onGroupItemChanged(QStandardItem *item);
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
  QString m_thunderforestApiKey;

  CurrentView m_currentView;
  DecoderMethod m_frameDecoder;

  bool m_modified;
  QString m_filePath;

  QMap<QStandardItem *, int> m_rootItems;
  QMap<QStandardItem *, JSON::Group> m_groupItems;
  QMap<QStandardItem *, JSON::Dataset> m_datasetItems;

  QVector<JSON::Group> m_groups;

  CustomModel *m_treeModel;
  QItemSelectionModel *m_selectionModel;

  CustomModel *m_groupModel;
  CustomModel *m_projectModel;
  CustomModel *m_datasetModel;

  QStringList m_fftSamples;
  QStringList m_decoderOptions;
  QMap<QString, QString> m_groupWidgets;
  QMap<QString, QString> m_datasetWidgets;
  QMap<QPair<bool, bool>, QString> m_plotOptions;

  JSON::Group m_selectedGroup;
  JSON::Dataset m_selectedDataset;
};

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
    names.insert(Model::TreeViewIcon, BAL("treeViewIcon"));
    names.insert(Model::TreeViewText, BAL("treeViewText"));
    names.insert(Model::TreeViewExpanded, BAL("treeViewExpanded"));
    names.insert(Model::TreeViewFrameIndex, BAL("treeViewFrameIndex"));
    names.insert(Model::ParameterName, BAL("parameterName"));
    names.insert(Model::EditableValue, BAL("editableValue"));
    names.insert(Model::ParameterType, BAL("parameterType"));
    names.insert(Model::PlaceholderValue, BAL("placeholderValue"));
    names.insert(Model::ParameterDescription, BAL("parameterDescription"));
    names.insert(Model::WidgetType, BAL("widgetType"));
    names.insert(Model::ComboBoxData, BAL("comboBoxData"));
#undef BAL
    return names;
  }
};
} // namespace Project
