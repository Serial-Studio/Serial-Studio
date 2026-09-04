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

#include "UI/Dashboard/WidgetMapBuilder.h"

#include <limits>
#include <utility>

#include "AppState.h"
#include "Core/SSAssert.h"
#include "DataModel/ProjectModel.h"
#include "DSP.h"
#include "UI/Dashboard.h"
#include "UI/WidgetExtensions.h"
#include "UI/WidgetRegistry.h"

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the builder to the widget-model state the Dashboard owns. The references outlive
 *        every call: they name members of the facade that constructs this object.
 */
UI::WidgetMapBuilder::WidgetMapBuilder(const WidgetModelBindings& bindings,
                                       AppState& appState,
                                       DataModel::ProjectModel& projectModel,
                                       WidgetRegistry& registry,
                                       WidgetExtensions& extensions)
  : m_appState(appState)
  , m_projectModel(projectModel)
  , m_registry(registry)
  , m_extensions(extensions)
  , m_widgetCount(bindings.widgetCount)
  , m_lastFrame(bindings.lastFrame)
  , m_widgetMap(bindings.widgetMap)
  , m_extensionGroupIds(bindings.extensionGroupIds)
  , m_extensionDatasetIds(bindings.extensionDatasetIds)
  , m_datasets(bindings.datasets)
  , m_sourceRawFrames(bindings.sourceRawFrames)
  , m_datasetExtremes(bindings.datasetExtremes)
  , m_valuePushes(bindings.valuePushes)
  , m_extremePushes(bindings.extremePushes)
  , m_datasetReferences(bindings.datasetReferences)
  , m_widgetGroups(bindings.widgetGroups)
  , m_widgetDatasets(bindings.widgetDatasets)
{}

//--------------------------------------------------------------------------------------------------
// Extension bucket addressing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the package id owning one entry of the extension bucket, empty when out of range.
 */
QString UI::WidgetMapBuilder::extensionIdAt(const bool group, const int bucketIndex) const
{
  const auto& ids = group ? m_extensionGroupIds : m_extensionDatasetIds;
  if (bucketIndex < 0 || bucketIndex >= ids.count())
    return {};

  return ids.at(bucketIndex);
}

/**
 * @brief Returns the relative-index offset of one dataset bucket. Extension widgets share a single
 *        enum value with the group-scope packages that occupy the bucket's first slots, so their
 *        dataset copies start after them; every built-in type owns its bucket alone.
 */
int UI::WidgetMapBuilder::datasetBucketBase(const SerialStudio::DashboardWidget key) const noexcept
{
  return key == SerialStudio::DashboardExtension ? m_extensionGroupIds.count() : 0;
}

//--------------------------------------------------------------------------------------------------
// Widget map population
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers a dataset's index and per-widget-key mappings.
 */
void UI::WidgetMapBuilder::processDatasetIntoWidgetMaps(const DataModel::Dataset& datasetIn,
                                                        DataModel::Group& ledPanel)
{
  SS_ASSERT(datasetIn.index >= 0, return);
  SS_ASSERT(datasetIn.uniqueId >= 0, return);

  DataModel::Dataset dataset = datasetIn;
  if (DSP::almostEqual(dataset.wgtMin, dataset.wgtMax)) {
    dataset.wgtMin = dataset.pltMin;
    dataset.wgtMax = dataset.pltMax;
  }

  if (DSP::almostEqual(dataset.fftMin, dataset.fftMax)) {
    dataset.fftMin = dataset.pltMin;
    dataset.fftMax = dataset.pltMax;
  }

  if (!m_datasets.contains(dataset.uniqueId)) {
    m_datasets.insert(dataset.uniqueId, dataset);
  } else {
    auto prev     = m_datasets.value(dataset.uniqueId);
    double newMin = qMin(prev.pltMin, dataset.pltMin);
    double newMax = qMax(prev.pltMax, dataset.pltMax);

    auto d   = dataset;
    d.pltMin = newMin;
    d.pltMax = newMax;
    m_datasets.insert(dataset.uniqueId, d);
  }

  if (dataset.hideOnDashboard)
    return;

  auto keys = SerialStudio::getDashboardWidgets(dataset);
  for (const auto& widgetKey : std::as_const(keys)) {
    if (widgetKey == SerialStudio::DashboardLED) {
      ledPanel.datasets.push_back(dataset);
      continue;
    }
    if (widgetKey == SerialStudio::DashboardExtension)
      m_extensionDatasetIds.append(dataset.widget);

    if (widgetKey != SerialStudio::DashboardNoWidget)
      m_widgetDatasets[widgetKey].append(dataset);
  }
}

/**
 * @brief Populates m_widgetGroups and m_widgetDatasets from the current frame. A datasetless data
 *        grid materialises as an empty table on purpose: dropping it would shift every later
 *        widget's relativeIndex and orphan saved workspace references. Extension entries bucket
 *        under DashboardExtension in walk order, ids index-aligned in m_extensionGroupIds.
 */
void UI::WidgetMapBuilder::buildWidgetGroups(const DataModel::Frame& frame, bool pro)
{
  SS_ASSERT(!m_lastFrame.groups.empty(), return);
  SS_ASSERT(!frame.groups.empty(), return);
  (void)frame;

  for (const auto& group : m_lastFrame.groups) {
    const auto key = SerialStudio::getDashboardWidget(group);

    if (key == SerialStudio::DashboardExtension)
      m_extensionGroupIds.append(group.widget);

    if (key != SerialStudio::DashboardNoWidget)
      m_widgetGroups[key].append(group);

    if (key == SerialStudio::DashboardPlot3D && !pro) {
      auto& bucket = m_widgetGroups[key];
      if (!bucket.isEmpty() && bucket.last().groupId == group.groupId)
        bucket.removeLast();

      if (bucket.isEmpty())
        m_widgetGroups.remove(key);

      auto copy  = group;
      copy.title = Dashboard::tr("%1 (Fallback)").arg(group.title);
      m_widgetGroups[SerialStudio::DashboardMultiPlot].append(copy);
      relabelGroupAsMultiplotFallback(group.groupId, copy.title);
    }

#ifdef BUILD_COMMERCIAL
    if (key == SerialStudio::DashboardPainter && !pro) {
      auto& bucket = m_widgetGroups[key];
      if (!bucket.isEmpty() && bucket.last().groupId == group.groupId)
        bucket.removeLast();

      if (bucket.isEmpty())
        m_widgetGroups.remove(key);

      auto copy  = group;
      copy.title = Dashboard::tr("%1 (Fallback)").arg(group.title);
      m_widgetGroups[SerialStudio::DashboardDataGrid].append(copy);
    }
#endif

    if (key == SerialStudio::DashboardAccelerometer) {
      m_widgetGroups[SerialStudio::DashboardMultiPlot].append(group);
      if (pro)
        m_widgetGroups[SerialStudio::DashboardPlot3D].append(group);
    }

    if (key == SerialStudio::DashboardGyroscope)
      m_widgetGroups[SerialStudio::DashboardMultiPlot].append(group);

    DataModel::Group ledPanel;
    for (const auto& dataset : group.datasets)
      processDatasetIntoWidgetMaps(dataset, ledPanel);

    if (ledPanel.datasets.size() > 0) {
      ledPanel.widget   = "led-panel";
      ledPanel.groupId  = group.groupId;
      ledPanel.uniqueId = group.uniqueId;
      ledPanel.title    = Dashboard::tr("LED Panel (%1)").arg(group.title);
      m_widgetGroups[SerialStudio::DashboardLED].append(ledPanel);
    }
  }
}

/**
 * @brief Rewrites the matching group entry in m_lastFrame as a multiplot fallback.
 */
void UI::WidgetMapBuilder::relabelGroupAsMultiplotFallback(int groupId, const QString& newTitle)
{
  for (size_t i = 0; i < m_lastFrame.groups.size(); ++i) {
    if (m_lastFrame.groups[i].groupId != groupId)
      continue;

    m_lastFrame.groups[i].title  = newTitle;
    m_lastFrame.groups[i].widget = "multiplot";
    return;
  }
}

/**
 * @brief Applies display-title overrides to the widget copies in m_widgetGroups and
 *        m_widgetDatasets: widget-level entries ("type:uid") beat entity-level ones ("uid"),
 *        canonical titles resolve from m_lastFrame so a removed override restores the original
 *        text; extension widgets key off "ext:&lt;id&gt;" instead of the numeric type.
 */
void UI::WidgetMapBuilder::applyDisplayTitles()
{
  if (m_appState.operationMode() != SerialStudio::ProjectFile)
    return;

  SS_ASSERT(!m_lastFrame.groups.empty(), return);
  const auto overrides = m_projectModel.displayTitles();

  QHash<int, QString> canonical;
  for (const auto& group : m_lastFrame.groups) {
    canonical.insert(group.uniqueId, group.title);
    for (const auto& dataset : group.datasets)
      canonical.insert(dataset.uniqueId, dataset.title);
  }

  const auto widgetOverride = [&](const QString& token, int uniqueId) {
    return overrides.value(token + QLatin1Char(':') + QString::number(uniqueId)).toString();
  };

  const auto entityResolve = [&](int uniqueId, const QString& current) {
    const auto over = overrides.value(QString::number(uniqueId)).toString();
    if (!over.isEmpty())
      return over;

    return canonical.value(uniqueId, current);
  };

  const auto resolve = [&](const QString& token, int uniqueId, const QString& current) {
    const auto scoped = widgetOverride(token, uniqueId);
    return scoped.isEmpty() ? entityResolve(uniqueId, current) : scoped;
  };

  const auto typeToken = [this](SerialStudio::DashboardWidget key, bool group, int index) {
    if (key != SerialStudio::DashboardExtension)
      return QString::number(static_cast<int>(key));

    return UI::WidgetExtensions::persistedTypeToken(extensionIdAt(group, index));
  };

  for (auto i = m_widgetGroups.begin(); i != m_widgetGroups.end(); ++i) {
    for (int j = 0; j < i.value().count(); ++j) {
      auto& group      = i.value()[j];
      const auto token = typeToken(i.key(), true, j);
      if (group.widget != QLatin1String("led-panel")) {
        group.title = resolve(token, group.uniqueId, group.title);
        continue;
      }

      const auto scoped = widgetOverride(token, group.uniqueId);
      group.title =
        scoped.isEmpty()
          ? Dashboard::tr("LED Panel (%1)").arg(entityResolve(group.uniqueId, group.title))
          : scoped;
    }
  }

  for (auto i = m_widgetDatasets.begin(); i != m_widgetDatasets.end(); ++i) {
    for (int j = 0; j < i.value().count(); ++j) {
      auto& dataset    = i.value()[j];
      const auto token = typeToken(i.key(), false, j);
      dataset.title    = resolve(token, dataset.uniqueId, dataset.title);
    }
  }
}

/**
 * @brief Registers all group and dataset widgets with the WidgetRegistry. Registry ids are handed
 *        out in creation order per type, so the dataset pass offsets its relative indices for the
 *        extension bucket (whose first slots belong to the group-scope packages registered above).
 */
void UI::WidgetMapBuilder::registerWidgets()
{
  SS_ASSERT(!m_widgetGroups.isEmpty() || !m_widgetDatasets.isEmpty(), return);
  SS_ASSERT(m_widgetCount == 0, m_widgetCount = 0);

  m_registry.beginBatchUpdate();

  for (auto i = m_widgetGroups.begin(); i != m_widgetGroups.end(); ++i) {
    const auto key   = i.key();
    const auto count = i.value().count();
    for (int j = 0; j < count; ++j) {
      const auto& group = i.value().at(j);
      (void)m_registry.createWidget(key, group.title, group.groupId, -1, true);
      m_widgetMap.insert(m_widgetCount++, qMakePair(key, j));
    }
  }

  for (auto i = m_widgetDatasets.begin(); i != m_widgetDatasets.end(); ++i) {
    const auto key   = i.key();
    const int base   = datasetBucketBase(key);
    const auto count = i.value().count();
    for (int j = 0; j < count; ++j) {
      const auto& dataset = i.value().at(j);
      (void)m_registry.createWidget(key, dataset.title, dataset.groupId, dataset.index, false);
      m_widgetMap.insert(m_widgetCount++, qMakePair(key, base + j));
    }
  }

  m_registry.endBatchUpdate();
}

//--------------------------------------------------------------------------------------------------
// Dataset references & push tables
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the m_datasetReferences map from all widget and frame sources.
 */
void UI::WidgetMapBuilder::buildDatasetReferences()
{
  SS_ASSERT(!m_lastFrame.groups.empty(), return);
  SS_ASSERT(!m_widgetGroups.isEmpty() || !m_widgetDatasets.isEmpty(), return);

  for (auto& groupList : m_widgetGroups) {
    for (auto& group : groupList)
      for (auto& dataset : group.datasets)
        m_datasetReferences[dataset.uniqueId].append(&dataset);
  }

  for (auto& datasetList : m_widgetDatasets)
    for (auto& dataset : datasetList)
      m_datasetReferences[dataset.uniqueId].append(&dataset);

  for (auto& dataset : m_datasets)
    m_datasetReferences[dataset.uniqueId].append(&dataset);

  for (auto& group : m_lastFrame.groups) {
    for (auto& dataset : group.datasets) {
      auto& list = m_datasetReferences[dataset.uniqueId];
      if (!list.contains(&dataset))
        list.append(&dataset);
    }
  }
}

/**
 * @brief Rebuilds the dataset reference map after the frame layout has changed.
 *        Any push_back/erase on m_lastFrame.groups shifts elements and dangles the
 *        &dataset pointers stored here, so every such mutation must call this; the
 *        early-out guards buildDatasetReferences(), which asserts on an empty frame.
 */
void UI::WidgetMapBuilder::rebuildDatasetReferences()
{
  m_datasetReferences.clear();
  m_valuePushes.clear();

  if (m_lastFrame.groups.empty())
    return;

  buildDatasetReferences();
  buildValuePushes();
}

/**
 * @brief Resolves one dataset's propagation targets from m_datasetReferences.
 */
UI::ValuePush UI::WidgetMapBuilder::makeValuePush(
  const DataModel::Dataset& dataset, const QSet<const DataModel::Dataset*>& stringTargets) const
{
  SS_ASSERT_LOG(!m_datasetReferences.isEmpty());

  ValuePush push;
  push.uniqueId = dataset.uniqueId;

  const auto ref_it = m_datasetReferences.constFind(dataset.uniqueId);
  if (ref_it == m_datasetReferences.cend()) {
    push.uniqueId = std::numeric_limits<int>::min();
    return push;
  }

  for (auto* target : ref_it.value()) {
    push.targets.push_back(target);
    if (stringTargets.contains(target))
      push.stringTargets.push_back(target);
  }

  return push;
}

/**
 * @brief Pre-resolves the per-source value-propagation tables from m_datasetReferences. A
 *        zero-dataset layout (image/painter-only) still registers an empty table per source:
 *        a legitimate datasetless frame must find its table instead of tripping the
 *        missing-dataset quarantine on every frame.
 */
void UI::WidgetMapBuilder::buildValuePushes()
{
  SS_ASSERT(!m_lastFrame.groups.empty(), return);
  SS_ASSERT_LOG(!m_widgetGroups.isEmpty() || !m_widgetDatasets.isEmpty());

  m_valuePushes.clear();

  QSet<const DataModel::Dataset*> string_targets;
  for (auto& group : m_lastFrame.groups)
    for (auto& dataset : group.datasets)
      string_targets.insert(&dataset);

  const auto grid_it = m_widgetGroups.constFind(SerialStudio::DashboardDataGrid);
  if (grid_it != m_widgetGroups.cend()) {
    for (const auto& group : grid_it.value())
      for (const auto& dataset : group.datasets)
        string_targets.insert(&dataset);
  }

  const auto panel_it = m_widgetGroups.constFind(SerialStudio::DashboardBarPanel);
  if (panel_it != m_widgetGroups.cend()) {
    for (const auto& group : panel_it.value())
      for (const auto& dataset : group.datasets)
        string_targets.insert(&dataset);
  }

  addExtensionStringTargets(string_targets);

  for (auto it = m_sourceRawFrames.cbegin(); it != m_sourceRawFrames.cend(); ++it) {
    auto& table = m_valuePushes[it.key()];
    for (const auto& group : it.value().groups)
      for (const auto& dataset : group.datasets)
        table.push_back(makeValuePush(dataset, string_targets));
  }

  buildExtremePushes();
}

/**
 * @brief Pre-resolves the per-source extreme-hold fold tables (spec 0052): one entry per opted-in
 *        dataset, pointing at the address-stable m_datasets copy and its m_datasetExtremes slot.
 *        Datasets that never opt in contribute no entry, so the per-frame fold walks nothing.
 */
void UI::WidgetMapBuilder::buildExtremePushes()
{
  m_extremePushes.clear();

  for (auto it = m_sourceRawFrames.cbegin(); it != m_sourceRawFrames.cend(); ++it)
    for (const auto& group : it.value().groups)
      for (const auto& dataset : group.datasets)
        appendExtremePush(it.key(), dataset);
}

/**
 * @brief Appends one extreme-hold fold entry when @a dataset opted in and resolves in m_datasets.
 */
void UI::WidgetMapBuilder::appendExtremePush(int sourceId, const DataModel::Dataset& dataset)
{
  if (!dataset.extremeHold)
    return;

  const auto ds_it = m_datasets.constFind(dataset.uniqueId);
  if (ds_it == m_datasets.cend())
    return;

  ExtremePush push;
  push.slot    = &m_datasetExtremes[dataset.uniqueId];
  push.value   = &ds_it.value().numericValue;
  push.numeric = &ds_it.value().isNumeric;
  m_extremePushes[sourceId].push_back(push);
}

/**
 * @brief Adds the widget copies of every extension package that declared readsStringValues to the
 *        string-target set, which is what keeps a package that renders Dataset::value from reading
 *        a stale string. Reconfigure-time only: the per-frame walk is untouched, and a package
 *        that never declares the flag contributes no target and therefore no work.
 */
void UI::WidgetMapBuilder::addExtensionStringTargets(QSet<const DataModel::Dataset*>& targets) const
{
  const auto groups = m_widgetGroups.constFind(SerialStudio::DashboardExtension);
  if (groups != m_widgetGroups.cend()) {
    for (int i = 0; i < groups->count(); ++i) {
      if (!m_extensions.descriptor(extensionIdAt(true, i)).readsStringValues)
        continue;

      for (const auto& dataset : groups->at(i).datasets)
        targets.insert(&dataset);
    }
  }

  const auto datasets = m_widgetDatasets.constFind(SerialStudio::DashboardExtension);
  if (datasets != m_widgetDatasets.cend()) {
    for (int i = 0; i < datasets->count(); ++i)
      if (m_extensions.descriptor(extensionIdAt(false, i)).readsStringValues)
        targets.insert(&datasets->at(i));
  }
}
