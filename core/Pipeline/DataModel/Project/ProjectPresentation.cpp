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

#include "DataModel/Project/ProjectPresentation.h"

#include <QInputDialog>
#include <QJSValue>
#include <QMap>
#include <QSet>

#include "AppState.h"
#include "DataModel/Project/ProjectTables.h"
#include "DataModel/ProjectModel.h"
#include "Misc/IconEngine.h"
#include "UI/WidgetExtensions.h"

namespace DataModel {

/**
 * @brief True while per-widget settings are live. Only a loaded project owns them, and a widget id
 *        is just type:groupId:datasetIndex: outside ProjectFile a QuickPlot widget reusing an id
 *        would read settings the loaded project's widget saved and it can never save back.
 */
static bool widget_settings_active()
{
  static auto& appState = AppState::instance();
  return appState.operationMode() == SerialStudio::ProjectFile;
}

/**
 * @brief Builds the widget-scoped subkey ("<type>:<uid>") used by the titles and
 *        freezeTitle maps.
 */
static QString widget_scope_key(int widgetType, int uniqueId)
{
  return QString::number(widgetType) + QLatin1Char(':') + QString::number(uniqueId);
}

/**
 * @brief Builds the widget-scoped subkey for a widget-extension slot
 * ("ext:&lt;id&gt;:&lt;uid&gt;"). Extension widgets share one enum value, so the numeric key cannot
 * address one; the package id comes from the entity's own widget string (the dashboard's
 * title-resolution key), falling back to the numeric key when the entity names no package.
 */
static QString extension_scope_key(int widgetType,
                                   int uniqueId,
                                   const std::vector<DataModel::Group>& groups)
{
  if (widgetType != static_cast<int>(SerialStudio::DashboardExtension))
    return widget_scope_key(widgetType, uniqueId);

  QString package;
  for (const auto& group : groups) {
    if (group.uniqueId == uniqueId)
      package = group.widget;

    for (const auto& dataset : group.datasets)
      if (dataset.uniqueId == uniqueId)
        package = dataset.widget;
  }

  if (package.isEmpty())
    return widget_scope_key(widgetType, uniqueId);

  return UI::WidgetExtensions::persistedTypeToken(package) + QLatin1Char(':')
       + QString::number(uniqueId);
}

}  // namespace DataModel

//--------------------------------------------------------------------------------------------------
// Construction & blob access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Binds the presentation store to @p model.
 */
DataModel::ProjectPresentation::ProjectPresentation(ProjectModel& model) : m_model(model) {}

/**
 * @brief Returns the raw per-widget settings blob (serializer and migrations).
 */
const QJsonObject& DataModel::ProjectPresentation::widgetSettingsBlob() const noexcept
{
  return m_widgetSettings;
}

/**
 * @brief Returns the raw display-override blob (serializer).
 */
const QJsonObject& DataModel::ProjectPresentation::widgetDisplayBlob() const noexcept
{
  return m_widgetDisplay;
}

/**
 * @brief Path-keyed Project Editor tree node expansion map (persisted in the file).
 */
const QJsonObject& DataModel::ProjectPresentation::treeExpansion() const noexcept
{
  return m_treeExpansion;
}

/**
 * @brief Stable-id keyed Project Overview diagram node collapse map (persisted in the file).
 */
const QJsonObject& DataModel::ProjectPresentation::diagramCollapse() const noexcept
{
  return m_diagramCollapse;
}

/**
 * @brief Mutable settings blob for the group-renumbering and legacy-key migrations, which rewrite
 *        layout keys in place rather than through the per-widget setters.
 */
QJsonObject& DataModel::ProjectPresentation::mutableWidgetSettings() noexcept
{
  return m_widgetSettings;
}

/**
 * @brief Drops every blob (document reset).
 */
void DataModel::ProjectPresentation::resetDocument()
{
  m_widgetSettings  = QJsonObject();
  m_widgetDisplay   = QJsonObject();
  m_treeExpansion   = QJsonObject();
  m_diagramCollapse = QJsonObject();
}

/**
 * @brief Drops the per-widget settings blob only; returns whether anything was discarded.
 */
bool DataModel::ProjectPresentation::clearWidgetSettings()
{
  if (m_widgetSettings.isEmpty())
    return false;

  m_widgetSettings = QJsonObject();
  return true;
}

/**
 * @brief Installs the four blobs straight from a parsed project document.
 */
void DataModel::ProjectPresentation::loadBlobs(const QJsonObject& widgetSettings,
                                               const QJsonObject& widgetDisplay,
                                               const QJsonObject& treeExpansion,
                                               const QJsonObject& diagramCollapse)
{
  m_widgetSettings  = widgetSettings;
  m_widgetDisplay   = widgetDisplay;
  m_treeExpansion   = treeExpansion;
  m_diagramCollapse = diagramCollapse;
}

//--------------------------------------------------------------------------------------------------
// Dashboard tab and layouts
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the active group ID for the dashboard tab bar, or -1.
 */
int DataModel::ProjectPresentation::activeGroupId() const
{
  return m_widgetSettings.value(Keys::kActiveGroupSubKey).toInt(-1);
}

/**
 * @brief Stages the active dashboard tab group ID.
 */
void DataModel::ProjectPresentation::setActiveGroupId(const int groupId)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  const int current = m_widgetSettings.value(Keys::kActiveGroupSubKey).toInt(-1);
  if (current == groupId)
    return;

  if (groupId >= 0)
    m_widgetSettings.insert(Keys::kActiveGroupSubKey, groupId);
  else
    m_widgetSettings.remove(Keys::kActiveGroupSubKey);

  m_model.setModified(true);
  Q_EMIT m_model.activeGroupIdChanged();
  Q_EMIT m_model.widgetSettingsChanged();
}

/**
 * @brief Returns the persisted layout for the given group ID.
 */
QJsonObject DataModel::ProjectPresentation::groupLayout(int groupId) const
{
  return m_widgetSettings.value(Keys::layoutKey(groupId)).toObject().value("data").toObject();
}

/**
 * @brief Returns the persisted layout for the given group within a window scope.
 */
QJsonObject DataModel::ProjectPresentation::groupLayout(const QString& scope, int groupId) const
{
  const auto key = Keys::layoutKey(scope, groupId);
  return m_widgetSettings.value(key).toObject().value("data").toObject();
}

/**
 * @brief Returns the auto-layout pattern and split ratio stored for a group or workspace, as
 *        "pattern" and "ratio". These live beside the manual geometry under the same layout
 *        key, which is what keeps the choice out of the workspace list: no customize mode, and
 *        group tabs carry one just like user workspaces do.
 */
QJsonObject DataModel::ProjectPresentation::layoutChoice(const QString& scope, int groupId) const
{
  const auto entry = m_widgetSettings.value(Keys::layoutKey(scope, groupId)).toObject();

  QJsonObject out;
  out.insert(QStringLiteral("pattern"), entry.value(QStringLiteral("pattern")).toString());
  out.insert(QStringLiteral("ratio"), entry.value(QStringLiteral("ratio")).toInt(8));
  return out;
}

/**
 * @brief Stores the auto-layout pattern and split ratio for a group or workspace. Writes the
 *        two sub-keys separately so the manual geometry sitting under "data" is untouched.
 */
void DataModel::ProjectPresentation::setLayoutChoice(const QString& scope,
                                                     int groupId,
                                                     const QString& pattern,
                                                     int ratio)
{
  const auto key = Keys::layoutKey(scope, groupId);
  saveWidgetSetting(key, QStringLiteral("pattern"), pattern.trimmed().toLower());
  saveWidgetSetting(key, QStringLiteral("ratio"), qBound(1, ratio, 15));
}

/**
 * @brief Stages the widget layout for a specific group.
 */
void DataModel::ProjectPresentation::setGroupLayout(const int groupId, const QJsonObject& layout)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  QJsonObject entry;
  entry[QStringLiteral("data")] = layout;
  m_widgetSettings.insert(Keys::layoutKey(groupId), entry);

  m_model.setModified(true);
  Q_EMIT m_model.widgetSettingsChanged();
}

/**
 * @brief Returns the persisted external-window records (workspace, geometry, state).
 */
QJsonArray DataModel::ProjectPresentation::externalWindows() const
{
  return m_widgetSettings.value(Keys::kDashboardWindowsSubKey).toArray();
}

/**
 * @brief Persists the external dashboard windows and prunes layouts of closed windows.
 */
void DataModel::ProjectPresentation::setExternalWindows(const QJsonArray& windows)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  if (m_widgetSettings.value(Keys::kDashboardWindowsSubKey).toArray() == windows)
    return;

  QSet<QString> liveScopes;
  for (const auto& value : windows) {
    const auto id = value.toObject().value(QStringLiteral("id")).toString();
    if (!id.isEmpty())
      liveScopes.insert(id);
  }

  const auto keys = m_widgetSettings.keys();
  for (const auto& key : keys) {
    const auto parts = key.split(QLatin1Char(':'));
    if (parts.size() == 3 && parts.first() == QLatin1String("layout")
        && !liveScopes.contains(parts.at(1)))
      m_widgetSettings.remove(key);
  }

  if (windows.isEmpty())
    m_widgetSettings.remove(Keys::kDashboardWindowsSubKey);
  else
    m_widgetSettings.insert(Keys::kDashboardWindowsSubKey, windows);

  m_model.setModified(true);
  Q_EMIT m_model.widgetSettingsChanged();
}

//--------------------------------------------------------------------------------------------------
// Per-widget and per-plugin settings
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the persisted settings object for the given widget.
 */
QJsonObject DataModel::ProjectPresentation::widgetSettings(const QString& widgetId) const
{
  if (!widget_settings_active())
    return QJsonObject();

  return m_widgetSettings.value(widgetId).toObject();
}

/**
 * @brief Stages a single widget setting and marks the project dirty. QML callers pass JS
 *        arrays/objects as QJSValue-wrapped variants, which QJsonValue::fromVariant silently
 *        turns into null, so they are unwrapped first.
 */
void DataModel::ProjectPresentation::saveWidgetSetting(const QString& widgetId,
                                                       const QString& key,
                                                       const QVariant& value)
{
  if (!widget_settings_active())
    return;

  auto normalized = value;
  if (normalized.userType() == qMetaTypeId<QJSValue>())
    normalized = normalized.value<QJSValue>().toVariant();

  auto obj            = m_widgetSettings.value(widgetId).toObject();
  const auto newValue = QJsonValue::fromVariant(normalized);
  if (obj.value(key) == newValue)
    return;

  obj.insert(key, newValue);
  m_widgetSettings.insert(widgetId, obj);

  m_model.setModified(true);
  Q_EMIT m_model.widgetSettingsChanged();
}

/**
 * @brief Returns the persisted state object for the given plugin.
 */
QJsonObject DataModel::ProjectPresentation::pluginState(const QString& pluginId) const
{
  return m_widgetSettings.value(QStringLiteral("plugin:") + pluginId).toObject();
}

/**
 * @brief Stages a plugin's state in the project and marks it dirty.
 */
void DataModel::ProjectPresentation::savePluginState(const QString& pluginId,
                                                     const QJsonObject& state)
{
  if (!widget_settings_active())
    return;

  const auto key = QStringLiteral("plugin:") + pluginId;
  if (m_widgetSettings.value(key).toObject() == state)
    return;

  m_widgetSettings.insert(key, state);
  m_model.setModified(true);
  Q_EMIT m_model.widgetSettingsChanged();
}

//--------------------------------------------------------------------------------------------------
// Display-title and freeze-title overrides
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the entity-level display-title override for the given unique ID (empty =
 *        no entity-level override; widget-level entries may still exist).
 */
QString DataModel::ProjectPresentation::displayTitle(int uniqueId) const
{
  const auto titles = m_widgetDisplay.value(Keys::Titles).toObject();
  return titles.value(QString::number(uniqueId)).toString();
}

/**
 * @brief Returns the widget-level display-title override for (widgetType, uniqueId); this
 *        scope wins over the entity-level override during resolution.
 */
QString DataModel::ProjectPresentation::widgetDisplayTitle(int widgetType, int uniqueId) const
{
  const auto titles = m_widgetDisplay.value(Keys::Titles).toObject();
  return titles.value(extension_scope_key(widgetType, uniqueId, m_model.groups())).toString();
}

/**
 * @brief Returns the freeze-title mode ("bar", "painted" or "hidden") for the given widget;
 *        unset widgets resolve to "painted" for title-painting instruments, "bar" otherwise.
 */
QString DataModel::ProjectPresentation::freezeTitleMode(int widgetType, int uniqueId) const
{
  const auto modes = m_widgetDisplay.value(Keys::FreezeTitle).toObject();
  const auto mode =
    modes.value(extension_scope_key(widgetType, uniqueId, m_model.groups())).toString();
  if (!mode.isEmpty())
    return mode;

  const auto widget = static_cast<SerialStudio::DashboardWidget>(widgetType);
  return SerialStudio::dashboardWidgetPaintsTitle(widget) ? QStringLiteral("painted")
                                                          : QStringLiteral("bar");
}

/**
 * @brief Returns the full display-title override map (uniqueId -> title).
 */
QJsonObject DataModel::ProjectPresentation::displayTitles() const
{
  return m_widgetDisplay.value(Keys::Titles).toObject();
}

/**
 * @brief Stages one titles-map entry under @p key; an empty title removes the entry.
 *        Display-only: canonical dataset/group titles are never touched.
 */
void DataModel::ProjectPresentation::stageDisplayTitle(const QString& key, const QString& title)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  auto titles    = m_widgetDisplay.value(Keys::Titles).toObject();
  const auto old = titles.value(key).toString();
  if (old == title)
    return;

  if (title.isEmpty())
    titles.remove(key);
  else
    titles.insert(key, title);

  if (titles.isEmpty())
    m_widgetDisplay.remove(Keys::Titles);
  else
    m_widgetDisplay.insert(Keys::Titles, titles);

  m_model.setModified(true);
  Q_EMIT m_model.widgetDisplayChanged();
}

/**
 * @brief Stages an entity-level display-title override (every widget of the dataset/group
 *        with the given unique ID); an empty title removes the entry.
 */
void DataModel::ProjectPresentation::setDisplayTitle(int uniqueId, const QString& title)
{
  stageDisplayTitle(QString::number(uniqueId), title);
}

/**
 * @brief Stages a widget-level display-title override for (widgetType, uniqueId); wins over
 *        the entity-level entry, an empty title removes it.
 */
void DataModel::ProjectPresentation::setWidgetDisplayTitle(int widgetType,
                                                           int uniqueId,
                                                           const QString& title)
{
  stageDisplayTitle(extension_scope_key(widgetType, uniqueId, m_model.groups()), title);
}

/**
 * @brief Stages the freeze-title mode ("bar", "painted" or "hidden") for the given widget;
 *        "painted" is only valid for title-painting instruments, and writing a widget's
 *        per-type default removes the stored entry instead of recording it.
 */
void DataModel::ProjectPresentation::setFreezeTitleMode(int widgetType,
                                                        int uniqueId,
                                                        const QString& mode)
{
  static auto& appState = AppState::instance();
  if (appState.operationMode() != SerialStudio::ProjectFile)
    return;

  const auto widget = static_cast<SerialStudio::DashboardWidget>(widgetType);
  const bool paints = SerialStudio::dashboardWidgetPaintsTitle(widget);
  if (mode != QLatin1String("bar") && mode != QLatin1String("hidden")
      && !(mode == QLatin1String("painted") && paints))
    return;

  const auto fallback = paints ? QLatin1String("painted") : QLatin1String("bar");
  const auto key      = extension_scope_key(widgetType, uniqueId, m_model.groups());
  auto modes          = m_widgetDisplay.value(Keys::FreezeTitle).toObject();
  const auto old      = modes.value(key).toString();
  const auto now      = (mode == fallback) ? QString() : mode;
  if (old == now)
    return;

  if (now.isEmpty())
    modes.remove(key);
  else
    modes.insert(key, now);

  if (modes.isEmpty())
    m_widgetDisplay.remove(Keys::FreezeTitle);
  else
    m_widgetDisplay.insert(Keys::FreezeTitle, modes);

  m_model.setModified(true);
  Q_EMIT m_model.widgetDisplayChanged();
}

/**
 * @brief Prompts for a widget display title and stages it as a widget-level override;
 *        clearing the field restores the entity-level or canonical title.
 */
void DataModel::ProjectPresentation::promptRenameWidget(int widgetType,
                                                        int uniqueId,
                                                        const QString& currentTitle)
{
  if (widgetType < 0 || uniqueId < 0)
    return;

  bool ok = false;
  const QString name =
    QInputDialog::getText(nullptr,
                          ProjectModel::tr("Rename Widget"),
                          ProjectModel::tr("Display title (empty restores the original):"),
                          QLineEdit::Normal,
                          currentTitle,
                          &ok);

  if (!ok || name.trimmed() == currentTitle)
    return;

  stageDisplayTitle(extension_scope_key(widgetType, uniqueId, m_model.groups()), name.trimmed());
}

//--------------------------------------------------------------------------------------------------
// Editor view state
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores the editor tree expansion map, marking the project dirty when it changed.
 */
void DataModel::ProjectPresentation::setTreeExpansion(const QJsonObject& expansion)
{
  if (m_treeExpansion == expansion)
    return;

  m_treeExpansion = expansion;
  m_model.setModified(true);
  m_model.scheduleAutoSave();
}

/**
 * @brief Stores the editor tree expansion map a rebuild derived, without dirtying the document:
 *        a rebuild queued behind a save must not re-mark the project modified.
 */
void DataModel::ProjectPresentation::storeTreeExpansion(const QJsonObject& expansion)
{
  m_treeExpansion = expansion;
}

/**
 * @brief Stores the diagram collapse map, marking the project dirty when it changed.
 */
void DataModel::ProjectPresentation::setDiagramCollapse(const QJsonObject& state)
{
  if (m_diagramCollapse == state)
    return;

  m_diagramCollapse = state;
  Q_EMIT m_model.diagramCollapseChanged();
  m_model.setModified(true);
  m_model.scheduleAutoSave();
}

//--------------------------------------------------------------------------------------------------
// QML combo-box and diagram snapshots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns "Time", "Samples", then every dataset label sorted by uniqueId.
 */
QStringList DataModel::ProjectPresentation::xDataSources() const
{
  QStringList list;
  list.append(ProjectModel::tr("Time"));
  list.append(ProjectModel::tr("Samples"));

  QMap<int, QString> datasets;
  for (const auto& group : m_model.groups()) {
    for (const auto& dataset : group.datasets) {
      const auto uid = dataset.uniqueId;
      if (!datasets.contains(uid))
        datasets.insert(uid, QString("%1 (%2)").arg(dataset.title, group.title));
    }
  }

  for (auto it = datasets.cbegin(); it != datasets.cend(); ++it)
    list.append(it.value());

  return list;
}

/**
 * @brief Parallel to xDataSources(): the dataset uniqueId at each combo position
 *        (position 0 -> -2 "Time", 1 -> -1 "Samples", then dataset uniqueIds).
 */
QList<int> DataModel::ProjectPresentation::xDataSourceUniqueIds() const
{
  QList<int> out;
  out.append(kXAxisTime);
  out.append(kXAxisSamples);

  QMap<int, bool> seen;
  for (const auto& group : m_model.groups()) {
    for (const auto& dataset : group.datasets) {
      const auto uid = dataset.uniqueId;
      if (!seen.contains(uid))
        seen.insert(uid, true);
    }
  }

  for (auto it = seen.cbegin(); it != seen.cend(); ++it)
    out.append(it.key());

  return out;
}

/**
 * @brief Returns "Time" plus every dataset label, sorted by uniqueId.
 */
QStringList DataModel::ProjectPresentation::yWaterfallSources() const
{
  QStringList list;
  list.append(ProjectModel::tr("Time"));

  QMap<int, QString> datasets;
  for (const auto& group : m_model.groups()) {
    for (const auto& dataset : group.datasets) {
      const auto uid = dataset.uniqueId;
      if (!datasets.contains(uid))
        datasets.insert(uid, QString("%1 (%2)").arg(dataset.title, group.title));
    }
  }

  for (auto it = datasets.cbegin(); it != datasets.cend(); ++it)
    list.append(it.value());

  return list;
}

/**
 * @brief Parallel to yWaterfallSources(): the dataset uniqueId at each combo position
 *        (position 0 -> 0, the "Time" sentinel).
 */
QList<int> DataModel::ProjectPresentation::yWaterfallSourceUniqueIds() const
{
  QList<int> out;
  out.append(0);

  QMap<int, bool> seen;
  for (const auto& group : m_model.groups()) {
    for (const auto& dataset : group.datasets) {
      const auto uid = dataset.uniqueId;
      if (!seen.contains(uid))
        seen.insert(uid, true);
    }
  }

  for (auto it = seen.cbegin(); it != seen.cend(); ++it)
    out.append(it.key());

  return out;
}

/**
 * @brief Returns a snapshot of all sources suitable for QML diagram consumption.
 */
QVariantList DataModel::ProjectPresentation::sourcesForDiagram() const
{
  const auto& sources = m_model.sources();

  QVariantList result;
  result.reserve(static_cast<qsizetype>(sources.size()));

  for (const auto& src : sources) {
    QVariantMap map;
    map[Keys::SourceId] = src.sourceId;
    map[Keys::BusType]  = src.busType;
    map[Keys::Title]    = src.title;
    result.append(map);
  }

  return result;
}

/**
 * @brief Returns a snapshot of all groups (with their datasets) for QML diagram consumption.
 */
QVariantList DataModel::ProjectPresentation::groupsForDiagram() const
{
  const auto& groups = m_model.groups();

  QVariantList result;
  result.reserve(static_cast<qsizetype>(groups.size()));

  for (const auto& grp : groups) {
    QVariantList datasets;
    datasets.reserve(static_cast<qsizetype>(grp.datasets.size()));

    for (const auto& ds : grp.datasets) {
      QVariantMap dsMap;
      dsMap[Keys::DatasetId]                = ds.datasetId;
      dsMap[Keys::Title]                    = ds.title;
      dsMap[Keys::Units]                    = ds.units;
      dsMap[Keys::Widget]                   = ds.widget;
      dsMap[QStringLiteral("hasTransform")] = !ds.transformCode.trimmed().isEmpty();
      datasets.append(dsMap);
    }

    QVariantMap map;
    map[Keys::GroupId] = grp.groupId;
    QVariantList outputWidgets;
    outputWidgets.reserve(static_cast<qsizetype>(grp.outputWidgets.size()));
    for (const auto& ow : grp.outputWidgets) {
      QVariantMap owMap;
      owMap[Keys::Title]      = ow.title;
      owMap[Keys::OutputType] = static_cast<int>(ow.type);
      outputWidgets.append(owMap);
    }

    map[Keys::SourceId]       = grp.sourceId;
    map[Keys::Title]          = grp.title;
    map[Keys::Widget]         = grp.widget;
    map[Keys::GroupType]      = static_cast<int>(grp.groupType);
    map[Keys::ParentFolderId] = grp.parentFolderId;
    map[Keys::Datasets]       = datasets;
    map[Keys::OutputWidgets]  = outputWidgets;
    result.append(map);
  }

  return result;
}

/**
 * @brief Returns a snapshot of all actions suitable for QML diagram consumption.
 */
QVariantList DataModel::ProjectPresentation::actionsForDiagram() const
{
  const auto& actions = m_model.actions();

  QVariantList result;
  result.reserve(static_cast<qsizetype>(actions.size()));

  for (const auto& act : actions) {
    QVariantMap map;
    map[Keys::ActionId] = act.actionId;
    map[Keys::SourceId] = act.sourceId;
    map[Keys::Title]    = act.title;
    map[Keys::Icon]     = Misc::IconEngine::resolveActionIconSource(act.icon);
    result.append(map);
  }

  return result;
}

/**
 * @brief Returns a snapshot of project data tables (name + register count) for the diagram.
 */
QVariantList DataModel::ProjectPresentation::tablesForDiagram() const
{
  const auto& tables = m_model.tables();

  QVariantList result;
  result.reserve(static_cast<qsizetype>(tables.size()));

  for (const auto& tbl : tables) {
    QVariantMap map;
    map[Keys::Name]                      = tbl.name;
    map[Keys::ParentFolderId]            = tbl.parentFolderId;
    map[QStringLiteral("registerCount")] = static_cast<int>(tbl.registers.size());
    result.append(map);
  }

  return result;
}
