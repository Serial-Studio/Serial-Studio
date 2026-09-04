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

#include "UI/Widgets/ExtensionData.h"

#include <algorithm>

#include "Core/SSAssert.h"
#include "DataModel/ProjectModel.h"
#include "UI/Dashboard.h"
#include "UI/WidgetExtensions.h"
#include "UI/WidgetRegistry.h"

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the severity of the alarm band holding the dataset's current value, or -1.
 */
[[nodiscard]] static int activeAlarmSeverity(const DataModel::Dataset& dataset)
{
  if (dataset.alarmBands.empty() || !dataset.isNumeric)
    return -1;

  const double low  = std::min(dataset.wgtMin, dataset.wgtMax);
  const double high = std::max(dataset.wgtMin, dataset.wgtMax);
  const double v = low < high ? std::clamp(dataset.numericValue, low, high) : dataset.numericValue;

  for (const auto& band : dataset.alarmBands)
    if (v >= band.min && v <= band.max)
      return static_cast<int>(band.severity);

  return -1;
}

/**
 * @brief Returns whether two rows differ in any value the dashboard tick can move.
 */
[[nodiscard]] static bool volatileFieldsDiffer(const Widgets::ExtensionRow& a,
                                               const Widgets::ExtensionRow& b)
{
  return a.title != b.title || a.text != b.text || a.value != b.value || a.isNumeric != b.isNumeric
      || a.alarmSeverity != b.alarmSeverity || DSP::notEqual(a.numericValue, b.numericValue);
}

/**
 * @brief Formats a dataset value for display; empty until the first sample arrives.
 */
[[nodiscard]] static QString formatDatasetValue(const DataModel::Dataset& dataset)
{
  if (dataset.value.isEmpty())
    return QString();

  if (!dataset.isNumeric)
    return dataset.value;

  QString value = (dataset.decimalPoints < 0)
                  ? FMT_VAL(dataset.numericValue, dataset)
                  : QString::number(dataset.numericValue, 'f', dataset.decimalPoints);
  if (!dataset.units.isEmpty())
    value += QStringLiteral(" ") + dataset.units;

  return value;
}

//--------------------------------------------------------------------------------------------------
// ExtensionRowsModel: backing list model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs an empty list model.
 */
Widgets::ExtensionRowsModel::ExtensionRowsModel(QObject* parent) : QAbstractListModel(parent) {}

/**
 * @brief Returns the number of rows currently held by the model.
 */
int Widgets::ExtensionRowsModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid())
    return 0;

  return m_rows.size();
}

/**
 * @brief Returns the value of @p role for the row at @p index.
 */
QVariant Widgets::ExtensionRowsModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size())
    return {};

  const auto& row = m_rows.at(index.row());
  switch (role) {
    case TitleRole:
      return row.title;
    case TextRole:
      return row.text;
    case ValueRole:
      return row.value;
    case UnitsRole:
      return row.units;
    case IndexRole:
      return row.index;
    case MinimumRole:
      return row.minValue;
    case MaximumRole:
      return row.maxValue;
    case UniqueIdRole:
      return row.uniqueId;
    case IsNumericRole:
      return row.isNumeric;
    case NumericValueRole:
      return row.numericValue;
    case DecimalPointsRole:
      return row.decimalPoints;
    case DisplayFormatRole:
      return row.displayFormat;
    case AlarmsDefinedRole:
      return row.alarmsDefined;
    case AlarmSeverityRole:
      return row.alarmSeverity;
    case WidgetsRole:
      return row.widgets;
    default:
      return {};
  }
}

/**
 * @brief Maps the role enum to the names extension QML binds to.
 */
QHash<int, QByteArray> Widgets::ExtensionRowsModel::roleNames() const
{
  static const QHash<int, QByteArray> kNames = {
    {        TitleRole,         "title"},
    {         TextRole,          "text"},
    {        ValueRole,         "value"},
    {        UnitsRole,         "units"},
    {        IndexRole,         "index"},
    {      MinimumRole,      "minValue"},
    {      MaximumRole,      "maxValue"},
    {     UniqueIdRole,      "uniqueId"},
    {    IsNumericRole,     "isNumeric"},
    { NumericValueRole,  "numericValue"},
    {DecimalPointsRole, "decimalPoints"},
    {DisplayFormatRole, "displayFormat"},
    {AlarmsDefinedRole, "alarmsDefined"},
    {AlarmSeverityRole, "alarmSeverity"},
    {      WidgetsRole,       "widgets"},
  };
  return kNames;
}

/**
 * @brief Replaces every row. Use only when the dataset set itself changes -- it tears down every
 *        delegate the package built.
 */
void Widgets::ExtensionRowsModel::reset(const QVector<ExtensionRow>& rows)
{
  beginResetModel();
  m_rows = rows;
  endResetModel();
}

/**
 * @brief Updates one row in place, emitting dataChanged() only for the roles that moved.
 */
bool Widgets::ExtensionRowsModel::updateRow(int row, const ExtensionRow& fresh)
{
  if (row < 0 || row >= m_rows.size())
    return false;

  auto& entry = m_rows[row];
  if (!volatileFieldsDiffer(entry, fresh))
    return false;

  static const QVector<int> kRoles = {
    TitleRole, TextRole, ValueRole, IsNumericRole, NumericValueRole, AlarmSeverityRole};

  entry.text          = fresh.text;
  entry.title         = fresh.title;
  entry.value         = fresh.value;
  entry.isNumeric     = fresh.isNumeric;
  entry.numericValue  = fresh.numericValue;
  entry.alarmSeverity = fresh.alarmSeverity;

  const auto idx = index(row);
  Q_EMIT dataChanged(idx, idx, kRoles);
  return true;
}

//--------------------------------------------------------------------------------------------------
// Constructor & initialization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the model for one extension widget. @p type and @p index address the dashboard
 *        bucket the widget was registered in, which is the same bucket a built-in of that type
 *        uses -- that is what lets a bundled package replace a built-in with no identity change.
 */
Widgets::ExtensionData::ExtensionData(const QString& extensionId,
                                      const SerialStudio::DashboardWidget type,
                                      const int index,
                                      QQuickItem* parent)
  : QQuickItem(parent)
  , m_paused(false)
  , m_groupScope(false)
  , m_index(index)
  , m_bucketIndex(index)
  , m_lastRowCount(-1)
  , m_type(type)
  , m_extensionId(extensionId)
  , m_rowsModel(new ExtensionRowsModel(this))
  , m_dashboard(UI::Dashboard::instance())
{
  const auto slot = m_dashboard.widgetSlot(m_type, m_index);
  m_groupScope    = slot.group;
  m_bucketIndex   = slot.bucketIndex;
  if (m_extensionId.isEmpty())
    m_extensionId = slot.extensionId;

  if (!valid())
    return;

  reloadConfig();
  rebuildRows();

  static auto& projectModel = DataModel::ProjectModel::instance();
  connect(&m_dashboard, &UI::Dashboard::updated, this, &Widgets::ExtensionData::updateData);
  connect(
    &m_dashboard, &UI::Dashboard::widgetCountChanged, this, &Widgets::ExtensionData::rebuildRows);
  connect(&projectModel,
          &DataModel::ProjectModel::widgetSettingsChanged,
          this,
          &Widgets::ExtensionData::reloadConfig);
}

//--------------------------------------------------------------------------------------------------
// State queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether the per-tick refresh is suspended.
 */
bool Widgets::ExtensionData::paused() const noexcept
{
  return m_paused;
}

/**
 * @brief Returns whether the lead dataset's last sample parsed as a number.
 */
bool Widgets::ExtensionData::isNumeric() const noexcept
{
  return m_lead.isNumeric;
}

/**
 * @brief Returns whether the package attaches to a group rather than to a single dataset.
 */
bool Widgets::ExtensionData::groupScope() const noexcept
{
  return m_groupScope;
}

/**
 * @brief Returns whether the lead dataset declares any alarm band.
 */
bool Widgets::ExtensionData::alarmsDefined() const noexcept
{
  return m_lead.alarmsDefined;
}

/**
 * @brief Returns whether the lead dataset's value currently sits inside an alarm band.
 */
bool Widgets::ExtensionData::alarmTriggered() const noexcept
{
  return m_lead.alarmSeverity >= 0;
}

/**
 * @brief Returns the id of the group backing this widget, or of the dataset's parent group.
 */
int Widgets::ExtensionData::groupId() const
{
  if (!valid())
    return -1;

  if (m_groupScope)
    return GET_GROUP(m_type, m_bucketIndex).groupId;

  return GET_DATASET(m_type, m_bucketIndex).groupId;
}

/**
 * @brief Returns the source (device) index the widget's entity belongs to.
 */
int Widgets::ExtensionData::sourceId() const
{
  if (!valid())
    return 0;

  if (m_groupScope)
    return GET_GROUP(m_type, m_bucketIndex).sourceId;

  return GET_DATASET(m_type, m_bucketIndex).sourceId;
}

/**
 * @brief Returns the persisted unique id of the group or dataset backing this widget.
 */
int Widgets::ExtensionData::uniqueId() const
{
  if (!valid())
    return -1;

  if (m_groupScope)
    return GET_GROUP(m_type, m_bucketIndex).uniqueId;

  return GET_DATASET(m_type, m_bucketIndex).uniqueId;
}

/**
 * @brief Returns the number of datasets the widget currently exposes.
 */
int Widgets::ExtensionData::datasetCount() const noexcept
{
  return m_rowsModel->rowCount();
}

/**
 * @brief Returns the lead dataset's fixed decimal count, or -1 for range-driven auto.
 */
int Widgets::ExtensionData::decimalPoints() const noexcept
{
  return m_lead.decimalPoints;
}

/**
 * @brief Returns the severity of the alarm band holding the lead value, or -1 when none does.
 */
int Widgets::ExtensionData::alarmSeverity() const noexcept
{
  return m_lead.alarmSeverity;
}

/**
 * @brief Returns the lead dataset's numeric value after transforms.
 */
double Widgets::ExtensionData::value() const noexcept
{
  return m_lead.numericValue;
}

/**
 * @brief Returns the lead dataset's widget-range minimum.
 */
double Widgets::ExtensionData::minValue() const noexcept
{
  return m_lead.minValue;
}

/**
 * @brief Returns the lead dataset's widget-range maximum.
 */
double Widgets::ExtensionData::maxValue() const noexcept
{
  return m_lead.maxValue;
}

/**
 * @brief Returns the lead dataset's value formatted for display, units included.
 */
const QString& Widgets::ExtensionData::text() const noexcept
{
  return m_lead.text;
}

/**
 * @brief Returns the widget title, with the project's display-title override already applied.
 */
const QString& Widgets::ExtensionData::title() const noexcept
{
  return m_title;
}

/**
 * @brief Returns the lead dataset's measurement units (empty = no suffix).
 */
const QString& Widgets::ExtensionData::units() const noexcept
{
  return m_lead.units;
}

/**
 * @brief Returns the lead dataset's raw string value after transforms.
 */
const QString& Widgets::ExtensionData::stringValue() const noexcept
{
  return m_lead.value;
}

/**
 * @brief Returns the lead dataset's tick/value label format.
 */
const QString& Widgets::ExtensionData::displayFormat() const noexcept
{
  return m_lead.displayFormat;
}

/**
 * @brief Returns the id of the package rendering this widget.
 */
const QString& Widgets::ExtensionData::extensionId() const noexcept
{
  return m_extensionId;
}

/**
 * @brief Returns the declared configuration: package defaults overlaid with the project's
 *        per-widget settings.
 */
const QVariantMap& Widgets::ExtensionData::config() const noexcept
{
  return m_config;
}

/**
 * @brief Returns the per-dataset row model.
 */
Widgets::ExtensionRowsModel* Widgets::ExtensionData::datasets() const noexcept
{
  return m_rowsModel;
}

//--------------------------------------------------------------------------------------------------
// Configuration setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Suspends or resumes the per-tick refresh; resuming pulls a fresh snapshot immediately.
 */
void Widgets::ExtensionData::setPaused(const bool paused)
{
  if (m_paused == paused)
    return;

  m_paused = paused;

  if (!m_paused)
    updateData();

  Q_EMIT pausedChanged();
}

/**
 * @brief Writes one declared configuration value into the project's per-widget settings, under
 *        the same widget id the generic settings form and the built-in widgets already use.
 */
void Widgets::ExtensionData::setConfigValue(const QString& key, const QVariant& value)
{
  if (key.isEmpty() || !valid())
    return;

  static auto& projectModel = DataModel::ProjectModel::instance();
  projectModel.saveWidgetSetting(widgetId(), key, value);
  reloadConfig();
}

//--------------------------------------------------------------------------------------------------
// Data updates
//--------------------------------------------------------------------------------------------------

/**
 * @brief Republishes the dashboard's current values on the UI tick; rows update in place and the
 *        change signal fires only when something actually moved.
 */
void Widgets::ExtensionData::updateData()
{
  if (m_paused || !valid())
    return;

  const int count = sourceDatasetCount();
  if (count != m_lastRowCount) [[unlikely]] {
    rebuildRows();
    return;
  }

  bool changed = false;
  ExtensionRow lead;
  for (int i = 0; i < count; ++i) {
    const ExtensionRow row  = buildVolatileRow(datasetAt(i));
    changed                |= m_rowsModel->updateRow(i, row);
    if (i == 0)
      lead = row;
  }

  const auto title = currentTitle();
  if (m_title != title) {
    m_title = title;
    changed = true;
  }

  changed |= refreshLead(lead);

  if (changed)
    Q_EMIT updated();
}

/**
 * @brief Re-reads the package defaults and the project's stored settings for this widget.
 */
void Widgets::ExtensionData::reloadConfig()
{
  static auto& catalog      = UI::WidgetExtensions::instance();
  static auto& projectModel = DataModel::ProjectModel::instance();

  QVariantMap merged;
  const auto& descriptor = catalog.descriptor(m_extensionId);
  for (const auto& property : descriptor.config)
    merged.insert(property.id, property.defaultValue);

  const auto stored = projectModel.widgetSettings(widgetId());
  for (auto it = stored.constBegin(); it != stored.constEnd(); ++it)
    merged.insert(it.key(), it.value().toVariant());

  if (merged == m_config)
    return;

  m_config = merged;
  Q_EMIT configChanged();
}

//--------------------------------------------------------------------------------------------------
// Utility functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether the widget still addresses a live dashboard entry.
 */
bool Widgets::ExtensionData::valid() const
{
  return VALIDATE_WIDGET(m_type, m_index);
}

/**
 * @brief Returns the "type:groupId:datasetIndex" key this widget's settings are stored under.
 */
QString Widgets::ExtensionData::widgetId() const
{
  static auto& registry = UI::WidgetRegistry::instance();

  const auto id   = registry.widgetIdByTypeAndIndex(m_type, m_index);
  const auto info = registry.widgetInfo(id);
  return QStringLiteral("%1:%2:%3")
    .arg(static_cast<int>(m_type))
    .arg(info.groupId)
    .arg(info.datasetIndex);
}

/**
 * @brief Returns the widget's current title from the dashboard's display-title-resolved copy.
 */
QString Widgets::ExtensionData::currentTitle() const
{
  if (!valid())
    return {};

  if (m_groupScope)
    return GET_GROUP(m_type, m_bucketIndex).title;

  return GET_DATASET(m_type, m_bucketIndex).title;
}

/**
 * @brief Snapshots every dataset the widget exposes: the whole group for a group-scope package,
 *        the single dataset for a dataset-scope one.
 */
QVector<Widgets::ExtensionRow> Widgets::ExtensionData::collectRows() const
{
  QVector<ExtensionRow> rows;
  if (!valid())
    return rows;

  if (!m_groupScope) {
    rows.append(buildRow(GET_DATASET(m_type, m_bucketIndex)));
    return rows;
  }

  const auto& group = GET_GROUP(m_type, m_bucketIndex);
  rows.reserve(static_cast<int>(group.datasets.size()));
  for (const auto& dataset : group.datasets)
    rows.append(buildRow(dataset));

  return rows;
}

/**
 * @brief Number of datasets the widget currently exposes, read from the dashboard rather than
 *        from the model, so a structure change is visible before the model is reseeded.
 */
int Widgets::ExtensionData::sourceDatasetCount() const
{
  if (!valid())
    return 0;

  if (!m_groupScope)
    return 1;

  return static_cast<int>(GET_GROUP(m_type, m_bucketIndex).datasets.size());
}

/**
 * @brief The dashboard's copy of dataset @p index of this widget.
 */
const DataModel::Dataset& Widgets::ExtensionData::datasetAt(const int index) const
{
  static const DataModel::Dataset empty;
  if (!valid() || index < 0)
    return empty;

  if (!m_groupScope)
    return index == 0 ? GET_DATASET(m_type, m_bucketIndex) : empty;

  const auto& group = GET_GROUP(m_type, m_bucketIndex);
  if (static_cast<std::size_t>(index) >= group.datasets.size())
    return empty;

  return group.datasets[static_cast<std::size_t>(index)];
}

/**
 * @brief Builds the fields a tick can change. The jump-button list is deliberately absent: it
 *        walks the whole widget map per dataset, and only a structure change can move it, so a
 *        rebuild owns it and the per-tick pass never pays for it (F6).
 */
Widgets::ExtensionRow Widgets::ExtensionData::buildVolatileRow(
  const DataModel::Dataset& dataset) const
{
  ExtensionRow row;
  row.index         = dataset.index;
  row.title         = dataset.title;
  row.units         = dataset.units;
  row.value         = dataset.value;
  row.uniqueId      = dataset.uniqueId;
  row.minValue      = dataset.wgtMin;
  row.maxValue      = dataset.wgtMax;
  row.isNumeric     = dataset.isNumeric;
  row.numericValue  = dataset.numericValue;
  row.decimalPoints = dataset.decimalPoints;
  row.displayFormat = dataset.displayFormat;
  row.alarmsDefined = !dataset.alarmBands.empty();
  row.alarmSeverity = activeAlarmSeverity(dataset);
  row.text          = formatDatasetValue(dataset);
  return row;
}

/**
 * @brief Builds one row from a dashboard dataset copy.
 */
Widgets::ExtensionRow Widgets::ExtensionData::buildRow(const DataModel::Dataset& dataset) const
{
  ExtensionRow row = buildVolatileRow(dataset);
  row.widgets      = datasetWidgets(dataset);
  return row;
}

/**
 * @brief Builds the {windowId, icon, title} entry of every dashboard widget that also displays
 *        @p dataset, in dashboard order, so a package can offer the same jump buttons the data
 *        grid does without reaching into the dashboard itself.
 */
QVariantList Widgets::ExtensionData::datasetWidgets(const DataModel::Dataset& dataset) const
{
  QVariantList widgets;
  QVariantList plots;

  const auto& map = m_dashboard.widgetMap();
  for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
    const auto type = it.value().first;
    if (!SerialStudio::isDatasetWidget(type))
      continue;

    const auto& shown = GET_DATASET(type, it.value().second);
    if (shown.sourceId != dataset.sourceId || shown.groupId != dataset.groupId
        || shown.datasetId != dataset.datasetId)
      continue;

    QVariantMap entry;
    entry.insert(QStringLiteral("windowId"), it.key());
    entry.insert(QStringLiteral("icon"), SerialStudio::dashboardWidgetIcon(type));
    entry.insert(QStringLiteral("title"), SerialStudio::dashboardWidgetTitle(type));

    if (type == SerialStudio::DashboardPlot)
      plots.append(entry);

    else
      widgets.append(entry);
  }

  widgets += plots;
  return widgets;
}

/**
 * @brief Replaces the lead scalars from @p row, reporting whether anything moved.
 */
bool Widgets::ExtensionData::refreshLead(const ExtensionRow& row)
{
  if (!volatileFieldsDiffer(m_lead, row) && m_lead.uniqueId == row.uniqueId)
    return false;

  m_lead = row;
  return true;
}

/**
 * @brief Reseeds the row model from scratch when the dataset set changes.
 */
void Widgets::ExtensionData::rebuildRows()
{
  const auto rows = collectRows();

  m_rowsModel->reset(rows);
  m_lastRowCount = rows.count();
  m_title        = currentTitle();
  m_lead         = rows.isEmpty() ? ExtensionRow() : rows.first();

  Q_EMIT updated();
}
