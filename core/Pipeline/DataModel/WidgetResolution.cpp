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

#include "DataModel/WidgetResolution.h"

#include <QHash>

#include "Core/Bus/MessageBus.h"
#include "Core/Bus/Messages.h"
#include "Core/Services.h"

//--------------------------------------------------------------------------------------------------
// Extension lookup
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether @p id names an installed package attaching to @p scope. Consulted only
 *        after every built-in comparison misses, and it can only ever answer DashboardExtension:
 *        the manifest validator refuses a package id equal to a built-in widget string, so no
 *        catalog data resolves to a Pro widget type on any build.
 */
/**
 * @brief The UI::WidgetExtensions::Scope ordinals the catalog topic carries.
 */
constexpr int kGroupScope   = 0;
constexpr int kDatasetScope = 1;

/**
 * @brief Whether @p id names an installed extension package of @p scope, per the catalog the Ui
 *        layer retained on the bus; nothing resolves before the first rescan publishes it.
 */
[[nodiscard]] static bool isWidgetExtension(const QString& id, const int scope)
{
  if (id.isEmpty())
    return false;

  const auto* bus    = &Core::services().bus;
  const auto catalog = bus ? bus->latest<Core::Bus::WidgetExtensionCatalog>() : nullptr;
  if (!catalog)
    return false;

  for (const auto& entry : catalog->entries)
    if (entry.id == id)
      return entry.scope == scope;

  return false;
}

//--------------------------------------------------------------------------------------------------
// Resolution
//--------------------------------------------------------------------------------------------------

/**
 * @brief Determines the dashboard widget type from a given JSON group.
 */
SerialStudio::DashboardWidget SerialStudio::getDashboardWidget(const DataModel::Group& group)
{
#ifdef BUILD_COMMERCIAL
  if (group.groupType == DataModel::GroupType::Output)
    return DashboardOutputPanel;
#else
  if (group.groupType == DataModel::GroupType::Output)
    return DashboardNoWidget;
#endif

  const auto& widget = group.widget;

  if (widget == QStringLiteral("accelerometer"))
    return DashboardAccelerometer;

  if (widget == QStringLiteral("datagrid"))
    return DashboardDataGrid;

  if (widget == QStringLiteral("gyro") || widget == QStringLiteral("gyroscope"))
    return DashboardGyroscope;

  if (widget == QStringLiteral("gps") || widget == QStringLiteral("map"))
    return DashboardGPS;

  if (widget == QStringLiteral("multiplot"))
    return DashboardMultiPlot;

  if (widget == QStringLiteral("plot3d"))
    return DashboardPlot3D;

  if (widget == QStringLiteral("terminal"))
    return DashboardTerminal;

  if (widget == QStringLiteral("clock"))
    return DashboardClock;

  if (widget == QStringLiteral("stopwatch"))
    return DashboardStopwatch;

  if (widget == QStringLiteral("webview"))
    return DashboardWebView;

  if (widget == QStringLiteral("barpanel"))
    return DashboardBarPanel;

#ifdef BUILD_COMMERCIAL
  if (widget == QStringLiteral("image"))
    return DashboardImageView;

  if (widget == QStringLiteral("notification-log"))
    return DashboardNotificationLog;

  if (widget == QStringLiteral("painter"))
    return DashboardPainter;
#else
  if (widget == QStringLiteral("painter"))
    return DashboardDataGrid;
#endif

  if (isWidgetExtension(widget, kGroupScope))
    return DashboardExtension;

  return DashboardNoWidget;
}

/**
 * @brief Retrieves a list of dashboard widgets for a specified JSON dataset.
 */
QList<SerialStudio::DashboardWidget> SerialStudio::getDashboardWidgets(
  const DataModel::Dataset& dataset)
{
  QList<DashboardWidget> list;

  static const QHash<QString, DashboardWidget> kDatasetWidgetMap = {
    {QStringLiteral("compass"), DashboardCompass},
    {    QStringLiteral("bar"),     DashboardBar},
    {  QStringLiteral("gauge"),   DashboardGauge},
    {  QStringLiteral("meter"),   DashboardMeter},
  };
  const auto it = kDatasetWidgetMap.constFind(dataset.widget);
  if (it != kDatasetWidgetMap.constEnd())
    list.append(it.value());

  else if (isWidgetExtension(dataset.widget, kDatasetScope))
    list.append(DashboardExtension);

  if (dataset.plt)
    list.append(DashboardPlot);

  if (dataset.fft)
    list.append(DashboardFFT);

  if (dataset.led)
    list.append(DashboardLED);

#ifdef BUILD_COMMERCIAL
  if (dataset.waterfall)
    list.append(DashboardWaterfall);
#endif

  return list;
}

/**
 * @brief Counts the group-scope extension widgets a project contributes. Both extension scopes
 *        share one enum value, so the dashboard bucket lists the group-scope widgets first and
 *        offsets the dataset-scope ones by this count; every site that mirrors that numbering for
 *        workspace references needs the same offset.
 */
int SerialStudio::extensionGroupWidgetCount(const std::vector<DataModel::Group>& groups)
{
  int count = 0;
  for (const auto& group : groups)
    if (getDashboardWidget(group) == DashboardExtension)
      ++count;

  return count;
}

/**
 * @brief Whether a band-drawing widget is configured for @p dataset: its group renders a bar
 *        panel, or the dataset is not hidden and resolves to a bar, gauge, meter or LED. The one
 *        alarm-band applicability rule (spec 0093), built on the two resolvers above so it cannot
 *        drift from the dashboard. Rebuild and command rate only: the dataset resolver allocates.
 */
bool SerialStudio::datasetRendersAlarmBands(const DataModel::Dataset& dataset,
                                            const DataModel::Group& group)
{
  if (getDashboardWidget(group) == DashboardBarPanel)
    return true;

  if (dataset.hideOnDashboard)
    return false;

  const auto widgets = getDashboardWidgets(dataset);
  return widgets.contains(DashboardBar) || widgets.contains(DashboardGauge)
      || widgets.contains(DashboardMeter) || widgets.contains(DashboardLED);
}

/**
 * @brief Whether @p dataset's bands notify, raise alarm points and sound: it has bands, a widget
 *        draws them, and Suppress Alarms is off. The alarm side asks this; the editor's band gates
 *        ask datasetRendersAlarmBands, since suppressed bands stay drawn and editable (spec 0093).
 */
bool SerialStudio::datasetRaisesBandAlarms(const DataModel::Dataset& dataset,
                                           const DataModel::Group& group)
{
  if (dataset.suppressAlarms || dataset.alarmBands.empty())
    return false;

  return datasetRendersAlarmBands(dataset, group);
}
