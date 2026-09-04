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

#pragma once

#include <QHash>
#include <QMap>
#include <QSet>
#include <QString>
#include <QVector>
#include <vector>

#include "DataModel/Frame.h"
#include "SerialStudio.h"

class AppState;

namespace DataModel {
class ProjectModel;
}  // namespace DataModel

namespace UI {
class WidgetRegistry;
class WidgetExtensions;

/**
 * @brief Min/max hold state for one extreme-hold dataset (spec 0052): the lowest and highest
 *        finite values observed since the last data reset; survives layout rebuilds.
 */
struct DatasetExtremes {
  double min = 0;
  double max = 0;
  bool valid = false;
};

/**
 * @brief Pre-resolved value-propagation entry: one incoming dataset to its widget copies.
 *        stringTargets is the subset whose string value is observable (DataGrid rows and the
 *        API-serialized m_lastFrame); the rest only ever consume the numeric fields.
 */
struct ValuePush {
  std::vector<DataModel::Dataset*> targets;
  std::vector<DataModel::Dataset*> stringTargets;
  int uniqueId;
};

/**
 * @brief Pre-resolved extreme-hold fold for one flagged dataset: the store slot plus value and
 *        numeric-gate pointers into the m_datasets copy (QMap nodes, address-stable across
 *        frames; both maps rebuild with the push tables).
 */
struct ExtremePush {
  DatasetExtremes* slot;
  const double* value;
  const bool* numeric;
};

/**
 * @brief The widget-model state one dashboard rebuild fills in. Every entry stays owned by
 *        UI::Dashboard and is bound here by reference: m_datasetReferences holds Dataset* pointers
 *        into the group and dataset buckets, so handing back freshly built containers for the
 *        facade to adopt would move the elements those pointers address.
 */
struct WidgetModelBindings {
  int& widgetCount;
  DataModel::Frame& lastFrame;
  SerialStudio::WidgetMap& widgetMap;
  QVector<QString>& extensionGroupIds;
  QVector<QString>& extensionDatasetIds;
  QMap<int, DataModel::Dataset>& datasets;
  QMap<int, DataModel::Frame>& sourceRawFrames;
  QMap<int, DatasetExtremes>& datasetExtremes;
  QHash<int, std::vector<ValuePush>>& valuePushes;
  QHash<int, std::vector<ExtremePush>>& extremePushes;
  QHash<int, QVector<DataModel::Dataset*>>& datasetReferences;
  QMap<SerialStudio::DashboardWidget, QVector<DataModel::Group>>& widgetGroups;
  QMap<SerialStudio::DashboardWidget, QVector<DataModel::Dataset>>& widgetDatasets;
};

/**
 * @brief Builds the dashboard's widget model: the per-type group and dataset buckets, the widget
 *        map handed to the registry, the display-title overrides, the dataset reference table and
 *        the pre-resolved value/extreme push tables the ingest path walks. Everything here runs at
 *        reconfigure rate, never per block, and the tables it writes stay Dashboard members.
 */
class WidgetMapBuilder {
public:
  WidgetMapBuilder(const WidgetModelBindings& bindings,
                   AppState& appState,
                   DataModel::ProjectModel& projectModel,
                   WidgetRegistry& registry,
                   WidgetExtensions& extensions);
  WidgetMapBuilder(WidgetMapBuilder&&)                 = delete;
  WidgetMapBuilder(const WidgetMapBuilder&)            = delete;
  WidgetMapBuilder& operator=(WidgetMapBuilder&&)      = delete;
  WidgetMapBuilder& operator=(const WidgetMapBuilder&) = delete;

  [[nodiscard]] QString extensionIdAt(const bool group, const int bucketIndex) const;
  [[nodiscard]] int datasetBucketBase(const SerialStudio::DashboardWidget key) const noexcept;

  void registerWidgets();
  void applyDisplayTitles();
  void buildValuePushes();
  void buildDatasetReferences();
  void rebuildDatasetReferences();
  void buildWidgetGroups(const DataModel::Frame& frame, bool pro);

private:
  void buildExtremePushes();
  void appendExtremePush(int sourceId, const DataModel::Dataset& dataset);
  void relabelGroupAsMultiplotFallback(int groupId, const QString& newTitle);
  void addExtensionStringTargets(QSet<const DataModel::Dataset*>& targets) const;
  void processDatasetIntoWidgetMaps(const DataModel::Dataset& datasetIn,
                                    DataModel::Group& ledPanel);
  [[nodiscard]] ValuePush makeValuePush(const DataModel::Dataset& dataset,
                                        const QSet<const DataModel::Dataset*>& stringTargets) const;

private:
  AppState& m_appState;
  DataModel::ProjectModel& m_projectModel;
  WidgetRegistry& m_registry;
  WidgetExtensions& m_extensions;

  int& m_widgetCount;
  DataModel::Frame& m_lastFrame;
  SerialStudio::WidgetMap& m_widgetMap;
  QVector<QString>& m_extensionGroupIds;
  QVector<QString>& m_extensionDatasetIds;
  QMap<int, DataModel::Dataset>& m_datasets;
  QMap<int, DataModel::Frame>& m_sourceRawFrames;
  QMap<int, DatasetExtremes>& m_datasetExtremes;
  QHash<int, std::vector<ValuePush>>& m_valuePushes;
  QHash<int, std::vector<ExtremePush>>& m_extremePushes;
  QHash<int, QVector<DataModel::Dataset*>>& m_datasetReferences;
  QMap<SerialStudio::DashboardWidget, QVector<DataModel::Group>>& m_widgetGroups;
  QMap<SerialStudio::DashboardWidget, QVector<DataModel::Dataset>>& m_widgetDatasets;
};

}  // namespace UI
