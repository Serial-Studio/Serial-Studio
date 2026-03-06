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

#include <QItemSelectionModel>
#include <QObject>
#include <QStandardItemModel>

#include "DataModel/Frame.h"
#include "SerialStudio.h"

namespace DataModel {
class CustomModel;

/**
 * @brief Editor controller for the project structure UI.
 *
 * Owns the tree model, form models, selection state, combobox data and view
 * state for the Project Editor window. Observes ProjectModel signals and
 * rebuilds models when the project data changes.
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
  void treeModelChanged();
  void groupModelChanged();
  void currentViewChanged();
  void actionModelChanged();
  void projectModelChanged();
  void datasetModelChanged();
  void datasetOptionsChanged();
  void editableOptionsChanged();

private:
  explicit ProjectEditor();
  ProjectEditor(ProjectEditor&&)                 = delete;
  ProjectEditor(const ProjectEditor&)            = delete;
  ProjectEditor& operator=(ProjectEditor&&)      = delete;
  ProjectEditor& operator=(const ProjectEditor&) = delete;

public:
  static ProjectEditor& instance();

  /**
   * @brief Enum representing the different views available in the project
   *        editor.
   */
  enum CurrentView {
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
  enum EditorWidget {
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
  };
  Q_ENUM(CustomRoles)

  [[nodiscard]] CurrentView currentView() const;
  [[nodiscard]] QString selectedText() const;
  [[nodiscard]] QString selectedIcon() const;
  [[nodiscard]] const QString actionIcon() const;
  [[nodiscard]] const QStringList& availableActionIcons() const;
  [[nodiscard]] bool currentGroupIsEditable() const;
  [[nodiscard]] bool currentDatasetIsEditable() const;
  [[nodiscard]] quint8 datasetOptions() const;

  [[nodiscard]] CustomModel* treeModel() const;
  [[nodiscard]] QItemSelectionModel* selectionModel() const;
  [[nodiscard]] CustomModel* groupModel() const;
  [[nodiscard]] CustomModel* actionModel() const;
  [[nodiscard]] CustomModel* projectModel() const;
  [[nodiscard]] CustomModel* datasetModel() const;

public slots:
  void buildTreeModel();
  void buildProjectModel();
  void buildGroupModel(const DataModel::Group& group);
  void buildActionModel(const DataModel::Action& action);
  void buildDatasetModel(const DataModel::Dataset& dataset);
  void displayFrameParserView();

private slots:
  void generateComboBoxModels();
  void onGroupItemChanged(QStandardItem* item);
  void onActionItemChanged(QStandardItem* item);
  void onProjectItemChanged(QStandardItem* item);
  void onDatasetItemChanged(QStandardItem* item);
  void setCurrentView(const DataModel::ProjectEditor::CurrentView view);
  void onCurrentSelectionChanged(const QModelIndex& current, const QModelIndex& previous);

private:
  void addGeneralSection(CustomModel* model, const DataModel::Dataset& dataset);
  void addPlotSection(CustomModel* model, const DataModel::Dataset& dataset);
  void addFFTSection(CustomModel* model, const DataModel::Dataset& dataset);
  void addWidgetSection(CustomModel* model, const DataModel::Dataset& dataset);
  void addAlarmSection(CustomModel* model, const DataModel::Dataset& dataset);
  void addLEDSection(CustomModel* model, const DataModel::Dataset& dataset);

  void saveExpandedStateMap(QStandardItem* item, QHash<QString, bool>& map, const QString& title);
  void restoreExpandedStateMap(QStandardItem* item,
                               QHash<QString, bool>& map,
                               const QString& title);

private:
  CurrentView m_currentView;

  QMap<QStandardItem*, int> m_rootItems;
  QMap<QStandardItem*, DataModel::Group> m_groupItems;
  QMap<QStandardItem*, DataModel::Action> m_actionItems;
  QMap<QStandardItem*, DataModel::Dataset> m_datasetItems;

  DataModel::Group m_selectedGroup;
  DataModel::Action m_selectedAction;
  DataModel::Dataset m_selectedDataset;

  CustomModel* m_treeModel;
  QItemSelectionModel* m_selectionModel;

  CustomModel* m_groupModel;
  CustomModel* m_actionModel;
  CustomModel* m_projectModel;
  CustomModel* m_datasetModel;

  QStringList m_fftSamples;
  QStringList m_timerModes;
  QStringList m_decoderOptions;
  QStringList m_checksumMethods;
  QStringList m_frameDetectionMethods;
  QList<SerialStudio::FrameDetection> m_frameDetectionMethodsValues;
  QStringList m_imgDetectionModes;

  QMap<QString, QString> m_eolSequences;
  QMap<QString, QString> m_groupWidgets;
  QMap<QString, QString> m_datasetWidgets;
  QMap<QPair<bool, bool>, QString> m_plotOptions;
};

/**
 * @brief A custom data model extending QStandardItemModel for managing
 *        project-specific data in the UI.
 */
class CustomModel : public QStandardItemModel {
public:
  explicit CustomModel(QObject* parent = nullptr) : QStandardItemModel(parent) {}

  QHash<int, QByteArray> roleNames() const override
  {
    QHash<int, QByteArray> names;

    // clang-format off
#define BAL(x) QByteArrayLiteral(x)
    names.insert(ProjectEditor::Active,               BAL("active"));
    names.insert(ProjectEditor::WidgetType,           BAL("widgetType"));
    names.insert(ProjectEditor::TreeViewIcon,         BAL("treeViewIcon"));
    names.insert(ProjectEditor::TreeViewText,         BAL("treeViewText"));
    names.insert(ProjectEditor::ComboBoxData,         BAL("comboBoxData"));
    names.insert(ProjectEditor::ParameterIcon,        BAL("parameterIcon"));
    names.insert(ProjectEditor::ParameterName,        BAL("parameterName"));
    names.insert(ProjectEditor::EditableValue,        BAL("editableValue"));
    names.insert(ProjectEditor::ParameterType,        BAL("parameterType"));
    names.insert(ProjectEditor::PlaceholderValue,     BAL("placeholderValue"));
    names.insert(ProjectEditor::TreeViewExpanded,     BAL("treeViewExpanded"));
    names.insert(ProjectEditor::TreeViewFrameIndex,   BAL("treeViewFrameIndex"));
    names.insert(ProjectEditor::ParameterDescription, BAL("parameterDescription"));
#undef BAL
    // clang-format on

    return names;
  }
};
}  // namespace DataModel
