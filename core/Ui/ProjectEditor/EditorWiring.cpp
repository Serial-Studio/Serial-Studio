/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "ProjectEditor/EditorWiring.h"

#include <cmath>
#include <memory>
#include <QDirIterator>
#include <QFileInfo>
#include <QHash>
#include <QJsonObject>
#include <QSet>
#include <QTimer>

#include "Core/Checksum.h"
#include "Core/SerialStudio.h"
#include "Core/Services.h"
#include "Core/Translator.h"
#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "ProjectEditor/ProjectEditor.h"
#include "ProjectEditorItemIds.h"
#include "UI/WidgetExtensions.h"

namespace DataModel {

using enum ProjectEditor::CustomRoles;
using enum ProjectEditor::CurrentView;

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the facade and the model; the translator is captured here because a language
 *        change is the one refresh trigger this class listens to outside the model.
 */
EditorWiring::EditorWiring(ProjectEditor& editor, ProjectModel& model)
  : m_editor(editor), m_model(model), m_translator(Core::services().translator)
{}

//--------------------------------------------------------------------------------------------------
// Project model signals
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires the basic ProjectModel rebuild signals into the tree-rebuild scheduler.
 */
void EditorWiring::wireProjectModelRebuilds()
{
  auto& pm = m_model;

  QObject::connect(
    &pm,
    &DataModel::ProjectModel::groupsChanged,
    &m_editor,
    [this] { m_editor.m_tree.scheduleTreeRebuild(); },
    Qt::QueuedConnection);
  QObject::connect(&pm,
                   &DataModel::ProjectModel::groupsChanged,
                   &m_editor,
                   &ProjectEditor::editableOptionsChanged,
                   Qt::QueuedConnection);
  QObject::connect(
    &pm,
    &DataModel::ProjectModel::actionsChanged,
    &m_editor,
    [this] { m_editor.m_tree.scheduleTreeRebuild(); },
    Qt::QueuedConnection);
  QObject::connect(
    &pm,
    &DataModel::ProjectModel::tablesChanged,
    &m_editor,
    [this] { m_editor.m_tree.scheduleTreeRebuild(); },
    Qt::QueuedConnection);
  QObject::connect(
    &pm,
    &DataModel::ProjectModel::editorWorkspacesChanged,
    &m_editor,
    [this] { m_editor.m_tree.scheduleTreeRebuild(); },
    Qt::QueuedConnection);
  QObject::connect(
    &pm,
    &DataModel::ProjectModel::sourcesChanged,
    &m_editor,
    [this] {
      m_editor.m_tree.scheduleTreeRebuild();

      if (m_editor.m_currentView == GroupView)
        m_editor.m_forms.buildGroupModel(m_editor.m_selectedGroup);
      else if (m_editor.m_currentView == DatasetView)
        m_editor.m_forms.buildDatasetModel(m_editor.m_selectedDataset);
    },
    Qt::QueuedConnection);
  QObject::connect(&pm, &DataModel::ProjectModel::modifiedChanged, &m_editor, [this] {
    if (m_editor.m_currentView != ProjectView || !m_editor.m_projectModel)
      return;

    const auto title = m_model.title();
    for (int i = 0; i < m_editor.m_projectModel->rowCount(); ++i) {
      auto* row = m_editor.m_projectModel->item(i);
      if (!row || row->data(ParameterType).toInt() != kProjectView_Title)
        continue;

      if (row->data(EditableValue).toString() != title)
        m_editor.m_forms.buildProjectModel();

      return;
    }

    m_editor.m_forms.buildProjectModel();
  });
  QObject::connect(&pm, &DataModel::ProjectModel::frameDetectionChanged, &m_editor, [this] {
    if (m_editor.m_currentView == ProjectView)
      m_editor.m_forms.buildProjectModel();
  });
  wireProjectFileSignals();
}

/**
 * @brief Wires the project-file signals (path, history snapshot, title) that reseed the tree
 *        expansion and the root item.
 */
void EditorWiring::wireProjectFileSignals()
{
  auto& pm = m_model;

  QObject::connect(&pm, &DataModel::ProjectModel::jsonFileChanged, &m_editor, [this] {
    const auto& path = m_model.jsonFilePath();
    if (path == m_lastJsonFilePath)
      return;

    m_lastJsonFilePath                = path;
    m_editor.m_seedExpansionFromModel = true;
    m_editor.m_selection.clearNavHistory();
    if (m_editor.m_selectionModel) {
      auto index = m_editor.m_treeModel->index(0, 0);
      m_editor.m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    }
  });
  QObject::connect(&pm, &DataModel::ProjectModel::historySnapshotApplied, &m_editor, [this] {
    m_editor.m_seedExpansionFromModel = true;
  });
  QObject::connect(&pm, &DataModel::ProjectModel::titleChanged, &m_editor, [this] {
    if (!m_editor.m_treeModel)
      return;

    const auto title = m_model.title();
    for (auto it = m_editor.m_rootItems.constBegin(); it != m_editor.m_rootItems.constEnd(); ++it) {
      if (it.value() != kRootItem)
        continue;

      auto* root = it.key();
      if (root->text() == title)
        return;

      root->setText(title);
      root->setData(title, TreeViewText);
      return;
    }
  });
}

/**
 * @brief Wires ProjectModel group add/delete signals into selection bookkeeping.
 */
void EditorWiring::wireGroupSignals()
{
  auto& pm = m_model;

  QObject::connect(
    &pm,
    &DataModel::ProjectModel::groupDeleted,
    &m_editor,
    [this] {
      if (!m_editor.m_selectionModel)
        return;

      if (m_editor.m_groupsRootItem) {
        m_editor.m_selectionModel->setCurrentIndex(m_editor.m_groupsRootItem->index(),
                                                   QItemSelectionModel::ClearAndSelect);
        return;
      }

      auto index = m_editor.m_treeModel->index(0, 0);
      m_editor.m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    },
    Qt::QueuedConnection);

  QObject::connect(
    &pm,
    &DataModel::ProjectModel::groupAdded,
    &m_editor,
    [this](int groupId) {
      if (!m_editor.m_selectionModel)
        return;

      for (auto it = m_editor.m_groupItems.begin(); it != m_editor.m_groupItems.end(); ++it) {
        if (it.value().groupId != groupId)
          continue;

        m_editor.m_selectionModel->setCurrentIndex(it.key()->index(),
                                                   QItemSelectionModel::ClearAndSelect);
        return;
      }

      m_editor.m_pendingSelectionKind    = ProjectEditor::PendingSelectionKind::Group;
      m_editor.m_pendingSelectionGroupId = groupId;
      m_editor.m_pendingSelectionItemId  = -1;
    },
    Qt::QueuedConnection);
}

/**
 * @brief Wires ProjectModel dataset add/delete signals into selection bookkeeping.
 */
void EditorWiring::wireDatasetSignals()
{
  auto& pm = m_model;

  QObject::connect(
    &pm,
    &DataModel::ProjectModel::datasetAdded,
    &m_editor,
    [this](int groupId, int datasetId) {
      if (!m_editor.m_selectionModel)
        return;

      for (auto it = m_editor.m_datasetItems.begin(); it != m_editor.m_datasetItems.end(); ++it) {
        if (it.value().groupId != groupId || it.value().datasetId != datasetId)
          continue;

        m_editor.m_selectionModel->setCurrentIndex(it.key()->index(),
                                                   QItemSelectionModel::ClearAndSelect);
        return;
      }

      m_editor.m_pendingSelectionKind    = ProjectEditor::PendingSelectionKind::Dataset;
      m_editor.m_pendingSelectionGroupId = groupId;
      m_editor.m_pendingSelectionItemId  = datasetId;
    },
    Qt::QueuedConnection);

  QObject::connect(
    &pm,
    &DataModel::ProjectModel::datasetDeleted,
    &m_editor,
    [this](int survivingGroupId) {
      if (!m_editor.m_selectionModel)
        return;

      if (survivingGroupId >= 0) {
        for (auto it = m_editor.m_groupItems.begin(); it != m_editor.m_groupItems.end(); ++it) {
          if (it.value().groupId != survivingGroupId)
            continue;

          m_editor.m_selectionModel->setCurrentIndex(it.key()->index(),
                                                     QItemSelectionModel::ClearAndSelect);
          return;
        }
      }

      if (m_editor.m_groupsRootItem) {
        m_editor.m_selectionModel->setCurrentIndex(m_editor.m_groupsRootItem->index(),
                                                   QItemSelectionModel::ClearAndSelect);
        return;
      }

      auto index = m_editor.m_treeModel->index(0, 0);
      m_editor.m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    },
    Qt::QueuedConnection);
}

/**
 * @brief Wires ProjectModel action add/delete signals into selection bookkeeping.
 */
void EditorWiring::wireActionSignals()
{
  auto& pm = m_model;

  QObject::connect(
    &pm,
    &DataModel::ProjectModel::actionAdded,
    &m_editor,
    [this](int actionId) {
      if (!m_editor.m_selectionModel)
        return;

      for (auto it = m_editor.m_actionItems.begin(); it != m_editor.m_actionItems.end(); ++it) {
        if (it.value().actionId != actionId)
          continue;

        m_editor.m_selectionModel->setCurrentIndex(it.key()->index(),
                                                   QItemSelectionModel::ClearAndSelect);
        break;
      }
    },
    Qt::QueuedConnection);

  QObject::connect(
    &pm,
    &DataModel::ProjectModel::actionDeleted,
    &m_editor,
    [this] {
      if (m_editor.m_selectionModel) {
        auto index = m_editor.m_treeModel->index(0, 0);
        m_editor.m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
      }
    },
    Qt::QueuedConnection);
}

/**
 * @brief Wires ProjectModel output-widget add/delete signals into selection bookkeeping.
 */
void EditorWiring::wireOutputWidgetSignals()
{
  auto& pm = m_model;

  QObject::connect(
    &pm,
    &DataModel::ProjectModel::outputWidgetAdded,
    &m_editor,
    [this](int groupId, int widgetId) {
      if (!m_editor.m_selectionModel)
        return;

      for (auto it = m_editor.m_outputWidgetItems.begin(); it != m_editor.m_outputWidgetItems.end();
           ++it) {
        if (it.value().groupId != groupId || it.value().widgetId != widgetId)
          continue;

        m_editor.m_selectionModel->setCurrentIndex(it.key()->index(),
                                                   QItemSelectionModel::ClearAndSelect);
        return;
      }

      m_editor.m_pendingSelectionKind    = ProjectEditor::PendingSelectionKind::OutputWidget;
      m_editor.m_pendingSelectionGroupId = groupId;
      m_editor.m_pendingSelectionItemId  = widgetId;
    },
    Qt::QueuedConnection);

  QObject::connect(
    &pm,
    &DataModel::ProjectModel::outputWidgetDeleted,
    &m_editor,
    [this](int groupId) {
      if (!m_editor.m_selectionModel)
        return;

      for (auto it = m_editor.m_groupItems.begin(); it != m_editor.m_groupItems.end(); ++it) {
        if (it.value().groupId != groupId)
          continue;

        m_editor.m_selectionModel->setCurrentIndex(it.key()->index(),
                                                   QItemSelectionModel::ClearAndSelect);
        return;
      }

      auto index = m_editor.m_treeModel->index(0, 0);
      m_editor.m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    },
    Qt::QueuedConnection);
}

/**
 * @brief Wires ProjectModel source add/delete signals into selection bookkeeping.
 */
void EditorWiring::wireSourceSignals()
{
  auto& pm = m_model;

  QObject::connect(
    &pm,
    &DataModel::ProjectModel::sourceAdded,
    &m_editor,
    [this](int sourceId) {
      if (!m_editor.m_selectionModel)
        return;

      for (auto it = m_editor.m_sourceItems.begin(); it != m_editor.m_sourceItems.end(); ++it) {
        if (it.value().sourceId != sourceId)
          continue;

        m_editor.m_selectionModel->setCurrentIndex(it.key()->index(),
                                                   QItemSelectionModel::ClearAndSelect);
        return;
      }

      m_editor.m_pendingSelectionKind    = ProjectEditor::PendingSelectionKind::Source;
      m_editor.m_pendingSelectionGroupId = -1;
      m_editor.m_pendingSelectionItemId  = sourceId;
    },
    Qt::QueuedConnection);

  QObject::connect(
    &pm,
    &DataModel::ProjectModel::sourceDeleted,
    &m_editor,
    [this] {
      if (m_editor.m_selectionModel) {
        auto index = m_editor.m_treeModel->index(0, 0);
        m_editor.m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
      }
    },
    Qt::QueuedConnection);

  QObject::connect(
    &pm,
    &DataModel::ProjectModel::sourceConnectionChanged,
    &m_editor,
    [this](int sourceId) {
      if (m_editor.m_currentView != SourceView || m_editor.m_selectedSource.sourceId != sourceId)
        return;

      for (const auto& source : m_model.sources()) {
        if (source.sourceId != sourceId)
          continue;

        m_editor.m_selectedSource = source;
        m_editor.m_forms.buildSourceModel(m_editor.m_selectedSource);
        return;
      }
    },
    Qt::QueuedConnection);
}

/**
 * @brief Wires the model's select-after-create requests to the tree selection slots. Direct, like
 *        the calls it replaces: the model already defers each request past the queued rebuild.
 */
void EditorWiring::wireSelectionRequests()
{
  QObject::connect(
    &m_model,
    &DataModel::ProjectModel::editorSelectionRequested,
    &m_editor,
    [this](int kind, int id, const QString& path) {
      switch (kind) {
        case ProjectEditor::KindWorkspace:
          m_editor.m_summaries.selectWorkspace(id);
          break;
        case ProjectEditor::KindWorkspaceFolder:
          m_editor.m_summaries.selectWorkspaceFolder(id);
          break;
        case ProjectEditor::KindGroupFolder:
          m_editor.m_summaries.selectGroupFolder(id);
          break;
        case ProjectEditor::KindTableFolder:
          m_editor.m_summaries.selectTableFolder(id);
          break;
        case ProjectEditor::KindUserTable:
          m_editor.m_summaries.selectUserTable(path);
          break;
        default:
          break;
      }
    },
    Qt::DirectConnection);
}

/**
 * @brief Wires editor self-signals that fan out form-model change notifications.
 */
void EditorWiring::wireEditorSelfSignals()
{
  QObject::connect(&m_editor,
                   &ProjectEditor::groupModelChanged,
                   &m_editor,
                   &ProjectEditor::editableOptionsChanged);
  QObject::connect(&m_editor,
                   &ProjectEditor::datasetModelChanged,
                   &m_editor,
                   &ProjectEditor::editableOptionsChanged);
  QObject::connect(&m_editor,
                   &ProjectEditor::datasetModelChanged,
                   &m_editor,
                   &ProjectEditor::datasetOptionsChanged);
}

/**
 * @brief Wires translator and connection-manager signals into editor refresh hooks.
 */
void EditorWiring::wireExternalSignals()
{
  QObject::connect(&m_translator, &Misc::Translator::languageChanged, &m_editor, [this] {
    m_editor.generateComboBoxModels();
    m_editor.m_tree.buildTreeModel();

    switch (m_editor.m_currentView) {
      case ProjectView:
        m_editor.m_forms.buildProjectModel();
        break;
      case GroupView:
        m_editor.m_forms.buildGroupModel(m_editor.m_selectedGroup);
        break;
      case ActionView:
        m_editor.m_forms.buildActionModel(m_editor.m_selectedAction);
        break;
      case DatasetView:
        m_editor.m_forms.buildDatasetModel(m_editor.m_selectedDataset);
        break;
      case SourceView:
        m_editor.m_forms.buildSourceModel(m_editor.m_selectedSource);
        break;
      case MqttPublisherView:
        m_editor.m_mqtt.buildMqttPublisherModel();
        break;
      case InfluxSinkView:
        m_editor.m_influx.buildInfluxSinkModel();
        break;
      default:
        break;
    }
  });

  static auto& widgetCatalog = UI::WidgetExtensions::instance();
  QObject::connect(&widgetCatalog, &UI::WidgetExtensions::catalogChanged, &m_editor, [this] {
    m_editor.generateComboBoxModels();
    if (m_editor.m_currentView == GroupView)
      m_editor.m_forms.buildGroupModel(m_editor.m_selectedGroup);
  });

  QObject::connect(
    &m_editor.m_connectionManager, &IO::ConnectionManager::driverChanged, &m_editor, [this] {
      if (m_editor.m_currentView != SourceView)
        return;

      const auto& sources = m_model.sources();
      for (const auto& src : sources) {
        if (src.sourceId == m_editor.m_selectedSource.sourceId) {
          m_editor.m_selectedSource = src;
          break;
        }
      }

      m_editor.m_forms.buildSourceModel(m_editor.m_selectedSource);
    });

  QObject::connect(
    &m_model,
    &DataModel::ProjectModel::sourceChanged,
    &m_editor,
    [this](int sourceId) {
      if (m_editor.m_currentView != SourceView || sourceId != m_editor.m_selectedSource.sourceId)
        return;

      const auto& sources = m_model.sources();
      for (const auto& src : sources) {
        if (src.sourceId != sourceId)
          continue;

        if (DataModel::serialize(src) == DataModel::serialize(m_editor.m_selectedSource))
          return;

        m_editor.m_selectedSource = src;
        m_editor.m_forms.buildSourceModel(m_editor.m_selectedSource);
        return;
      }
    },
    Qt::QueuedConnection);
}

}  // namespace DataModel
