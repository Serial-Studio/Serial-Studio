/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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

#include <cmath>
#include <memory>
#include <QDirIterator>
#include <QFileInfo>
#include <QHash>
#include <QJsonObject>
#include <QSet>
#include <QTimer>

#include "DataModel/FrameBuilder.h"
#include "DataModel/ProjectEditor.h"
#include "DataModel/ProjectModel.h"
#include "IO/Checksum.h"
#include "IO/ConnectionManager.h"
#include "Misc/IconEngine.h"
#include "Misc/Translator.h"
#include "Misc/Utilities.h"
#include "SerialStudio.h"
#include "UI/WidgetExtensions.h"

#ifdef BUILD_COMMERCIAL
#  include "MQTT/Publisher.h"
#  include "MQTT/PublisherScriptEditor.h"
#endif
#include "ProjectEditorItemIds.h"
#include "ProjectEditorShared.h"

//--------------------------------------------------------------------------------------------------
// Constructor / singleton
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wires the basic ProjectModel rebuild signals into the tree-rebuild scheduler.
 */
void DataModel::ProjectEditor::wireProjectModelRebuilds()
{
  auto& pm = m_projectModelRef;

  connect(&pm,
          &DataModel::ProjectModel::groupsChanged,
          this,
          &DataModel::ProjectEditor::scheduleTreeRebuild,
          Qt::QueuedConnection);
  connect(&pm,
          &DataModel::ProjectModel::groupsChanged,
          this,
          &DataModel::ProjectEditor::editableOptionsChanged,
          Qt::QueuedConnection);
  connect(&pm,
          &DataModel::ProjectModel::actionsChanged,
          this,
          &DataModel::ProjectEditor::scheduleTreeRebuild,
          Qt::QueuedConnection);
  connect(&pm,
          &DataModel::ProjectModel::tablesChanged,
          this,
          &DataModel::ProjectEditor::scheduleTreeRebuild,
          Qt::QueuedConnection);
  connect(&pm,
          &DataModel::ProjectModel::editorWorkspacesChanged,
          this,
          &DataModel::ProjectEditor::scheduleTreeRebuild,
          Qt::QueuedConnection);
  connect(
    &pm,
    &DataModel::ProjectModel::sourcesChanged,
    this,
    [this] {
      scheduleTreeRebuild();

      if (m_currentView == GroupView)
        buildGroupModel(m_selectedGroup);
      else if (m_currentView == DatasetView)
        buildDatasetModel(m_selectedDataset);
    },
    Qt::QueuedConnection);
  connect(&pm, &DataModel::ProjectModel::modifiedChanged, this, [this] {
    if (m_currentView != ProjectView || !m_projectModel)
      return;

    const auto title = m_projectModelRef.title();
    for (int i = 0; i < m_projectModel->rowCount(); ++i) {
      auto* row = m_projectModel->item(i);
      if (!row || row->data(ParameterType).toInt() != kProjectView_Title)
        continue;

      if (row->data(EditableValue).toString() != title)
        buildProjectModel();

      return;
    }

    buildProjectModel();
  });
  connect(&pm, &DataModel::ProjectModel::frameDetectionChanged, this, [this] {
    if (m_currentView == ProjectView)
      buildProjectModel();
  });
  connect(&pm, &DataModel::ProjectModel::jsonFileChanged, this, [this] {
    const auto& path = m_projectModelRef.jsonFilePath();
    if (path == m_lastJsonFilePath)
      return;

    m_lastJsonFilePath       = path;
    m_seedExpansionFromModel = true;
    clearNavHistory();
    if (m_selectionModel) {
      auto index = m_treeModel->index(0, 0);
      m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    }
  });
  connect(&pm, &DataModel::ProjectModel::titleChanged, this, [this] {
    if (!m_treeModel)
      return;

    const auto title = m_projectModelRef.title();
    for (auto it = m_rootItems.constBegin(); it != m_rootItems.constEnd(); ++it) {
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
void DataModel::ProjectEditor::wireGroupSignals()
{
  auto& pm = m_projectModelRef;

  connect(
    &pm,
    &DataModel::ProjectModel::groupDeleted,
    this,
    [this] {
      if (!m_selectionModel)
        return;

      if (m_groupsRootItem) {
        m_selectionModel->setCurrentIndex(m_groupsRootItem->index(),
                                          QItemSelectionModel::ClearAndSelect);
        return;
      }

      auto index = m_treeModel->index(0, 0);
      m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    },
    Qt::QueuedConnection);

  connect(
    &pm,
    &DataModel::ProjectModel::groupAdded,
    this,
    [this](int groupId) {
      if (!m_selectionModel)
        return;

      for (auto it = m_groupItems.begin(); it != m_groupItems.end(); ++it) {
        if (it.value().groupId != groupId)
          continue;

        m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
        return;
      }

      m_pendingSelectionKind    = PendingSelectionKind::Group;
      m_pendingSelectionGroupId = groupId;
      m_pendingSelectionItemId  = -1;
    },
    Qt::QueuedConnection);
}

/**
 * @brief Wires ProjectModel dataset add/delete signals into selection bookkeeping.
 */
void DataModel::ProjectEditor::wireDatasetSignals()
{
  auto& pm = m_projectModelRef;

  connect(
    &pm,
    &DataModel::ProjectModel::datasetAdded,
    this,
    [this](int groupId, int datasetId) {
      if (!m_selectionModel)
        return;

      for (auto it = m_datasetItems.begin(); it != m_datasetItems.end(); ++it) {
        if (it.value().groupId != groupId || it.value().datasetId != datasetId)
          continue;

        m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
        return;
      }

      m_pendingSelectionKind    = PendingSelectionKind::Dataset;
      m_pendingSelectionGroupId = groupId;
      m_pendingSelectionItemId  = datasetId;
    },
    Qt::QueuedConnection);

  connect(
    &pm,
    &DataModel::ProjectModel::datasetDeleted,
    this,
    [this](int survivingGroupId) {
      if (!m_selectionModel)
        return;

      if (survivingGroupId >= 0) {
        for (auto it = m_groupItems.begin(); it != m_groupItems.end(); ++it) {
          if (it.value().groupId != survivingGroupId)
            continue;

          m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
          return;
        }
      }

      if (m_groupsRootItem) {
        m_selectionModel->setCurrentIndex(m_groupsRootItem->index(),
                                          QItemSelectionModel::ClearAndSelect);
        return;
      }

      auto index = m_treeModel->index(0, 0);
      m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    },
    Qt::QueuedConnection);
}

/**
 * @brief Wires ProjectModel action add/delete signals into selection bookkeeping.
 */
void DataModel::ProjectEditor::wireActionSignals()
{
  auto& pm = m_projectModelRef;

  connect(
    &pm,
    &DataModel::ProjectModel::actionAdded,
    this,
    [this](int actionId) {
      if (!m_selectionModel)
        return;

      for (auto it = m_actionItems.begin(); it != m_actionItems.end(); ++it) {
        if (it.value().actionId != actionId)
          continue;

        m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
        break;
      }
    },
    Qt::QueuedConnection);

  connect(
    &pm,
    &DataModel::ProjectModel::actionDeleted,
    this,
    [this] {
      if (m_selectionModel) {
        auto index = m_treeModel->index(0, 0);
        m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
      }
    },
    Qt::QueuedConnection);
}

/**
 * @brief Wires ProjectModel output-widget add/delete signals into selection bookkeeping.
 */
void DataModel::ProjectEditor::wireOutputWidgetSignals()
{
  auto& pm = m_projectModelRef;

  connect(
    &pm,
    &DataModel::ProjectModel::outputWidgetAdded,
    this,
    [this](int groupId, int widgetId) {
      if (!m_selectionModel)
        return;

      for (auto it = m_outputWidgetItems.begin(); it != m_outputWidgetItems.end(); ++it) {
        if (it.value().groupId != groupId || it.value().widgetId != widgetId)
          continue;

        m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
        return;
      }

      m_pendingSelectionKind    = PendingSelectionKind::OutputWidget;
      m_pendingSelectionGroupId = groupId;
      m_pendingSelectionItemId  = widgetId;
    },
    Qt::QueuedConnection);

  connect(
    &pm,
    &DataModel::ProjectModel::outputWidgetDeleted,
    this,
    [this](int groupId) {
      if (!m_selectionModel)
        return;

      for (auto it = m_groupItems.begin(); it != m_groupItems.end(); ++it) {
        if (it.value().groupId != groupId)
          continue;

        m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
        return;
      }

      auto index = m_treeModel->index(0, 0);
      m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    },
    Qt::QueuedConnection);
}

/**
 * @brief Wires ProjectModel source add/delete signals into selection bookkeeping.
 */
void DataModel::ProjectEditor::wireSourceSignals()
{
  auto& pm = m_projectModelRef;

  connect(
    &pm,
    &DataModel::ProjectModel::sourceAdded,
    this,
    [this](int sourceId) {
      if (!m_selectionModel)
        return;

      for (auto it = m_sourceItems.begin(); it != m_sourceItems.end(); ++it) {
        if (it.value().sourceId != sourceId)
          continue;

        m_selectionModel->setCurrentIndex(it.key()->index(), QItemSelectionModel::ClearAndSelect);
        return;
      }

      m_pendingSelectionKind    = PendingSelectionKind::Source;
      m_pendingSelectionGroupId = -1;
      m_pendingSelectionItemId  = sourceId;
    },
    Qt::QueuedConnection);

  connect(
    &pm,
    &DataModel::ProjectModel::sourceDeleted,
    this,
    [this] {
      if (m_selectionModel) {
        auto index = m_treeModel->index(0, 0);
        m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
      }
    },
    Qt::QueuedConnection);
}

/**
 * @brief Wires editor self-signals that fan out form-model change notifications.
 */
void DataModel::ProjectEditor::wireEditorSelfSignals()
{
  connect(this,
          &DataModel::ProjectEditor::groupModelChanged,
          this,
          &DataModel::ProjectEditor::editableOptionsChanged);
  connect(this,
          &DataModel::ProjectEditor::datasetModelChanged,
          this,
          &DataModel::ProjectEditor::editableOptionsChanged);
  connect(this,
          &DataModel::ProjectEditor::datasetModelChanged,
          this,
          &DataModel::ProjectEditor::datasetOptionsChanged);
}

/**
 * @brief Wires translator and connection-manager signals into editor refresh hooks.
 */
void DataModel::ProjectEditor::wireExternalSignals()
{
  connect(&m_translator, &Misc::Translator::languageChanged, this, [this] {
    generateComboBoxModels();
    buildTreeModel();

    switch (m_currentView) {
      case ProjectView:
        buildProjectModel();
        break;
      case GroupView:
        buildGroupModel(m_selectedGroup);
        break;
      case ActionView:
        buildActionModel(m_selectedAction);
        break;
      case DatasetView:
        buildDatasetModel(m_selectedDataset);
        break;
      case SourceView:
        buildSourceModel(m_selectedSource);
        break;
      case MqttPublisherView:
        buildMqttPublisherModel();
        break;
      default:
        break;
    }
  });

  static auto& widgetCatalog = UI::WidgetExtensions::instance();
  connect(&widgetCatalog, &UI::WidgetExtensions::catalogChanged, this, [this] {
    generateComboBoxModels();
    if (m_currentView == GroupView)
      buildGroupModel(m_selectedGroup);
  });

  connect(&m_connectionManager, &IO::ConnectionManager::driverChanged, this, [this] {
    if (m_currentView != SourceView)
      return;

    const auto& sources = m_projectModelRef.sources();
    for (const auto& src : sources) {
      if (src.sourceId == m_selectedSource.sourceId) {
        m_selectedSource = src;
        break;
      }
    }

    buildSourceModel(m_selectedSource);
  });

  connect(
    &m_projectModelRef,
    &DataModel::ProjectModel::sourceChanged,
    this,
    [this](int sourceId) {
      if (m_currentView != SourceView || sourceId != m_selectedSource.sourceId)
        return;

      const auto& sources = m_projectModelRef.sources();
      for (const auto& src : sources) {
        if (src.sourceId != sourceId)
          continue;

        if (DataModel::serialize(src) == DataModel::serialize(m_selectedSource))
          return;

        m_selectedSource = src;
        buildSourceModel(m_selectedSource);
        return;
      }
    },
    Qt::QueuedConnection);
}
