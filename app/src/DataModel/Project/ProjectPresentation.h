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

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QVariantList>

namespace DataModel {

class ProjectModel;

/**
 * @brief The project's presentation blobs and the read-only views QML builds its lists from:
 *        per-widget settings, title overrides, the editor tree expansion map, the overview diagram
 *        collapse map, and the diagram/combo-box snapshots. All are opaque to the document core,
 *        which is why the cluster is outside the undo history by spec (0031 whitelist).
 */
class ProjectPresentation {
public:
  explicit ProjectPresentation(ProjectModel& model);
  ProjectPresentation(ProjectPresentation&&)                 = delete;
  ProjectPresentation(const ProjectPresentation&)            = delete;
  ProjectPresentation& operator=(ProjectPresentation&&)      = delete;
  ProjectPresentation& operator=(const ProjectPresentation&) = delete;

  [[nodiscard]] const QJsonObject& widgetSettingsBlob() const noexcept;
  [[nodiscard]] const QJsonObject& widgetDisplayBlob() const noexcept;
  [[nodiscard]] const QJsonObject& treeExpansion() const noexcept;
  [[nodiscard]] const QJsonObject& diagramCollapse() const noexcept;

  [[nodiscard]] QJsonObject& mutableWidgetSettings() noexcept;

  void resetDocument();
  [[nodiscard]] bool clearWidgetSettings();
  void loadBlobs(const QJsonObject& widgetSettings,
                 const QJsonObject& widgetDisplay,
                 const QJsonObject& treeExpansion,
                 const QJsonObject& diagramCollapse);

  [[nodiscard]] int activeGroupId() const;
  void setActiveGroupId(const int groupId);

  [[nodiscard]] QJsonObject groupLayout(int groupId) const;
  [[nodiscard]] QJsonObject groupLayout(const QString& scope, int groupId) const;
  [[nodiscard]] QJsonObject layoutChoice(const QString& scope, int groupId) const;
  void setLayoutChoice(const QString& scope, int groupId, const QString& pattern, int ratio);
  void setGroupLayout(const int groupId, const QJsonObject& layout);

  [[nodiscard]] QJsonArray externalWindows() const;
  void setExternalWindows(const QJsonArray& windows);

  [[nodiscard]] QJsonObject widgetSettings(const QString& widgetId) const;
  void saveWidgetSetting(const QString& widgetId, const QString& key, const QVariant& value);
  [[nodiscard]] QJsonObject pluginState(const QString& pluginId) const;
  void savePluginState(const QString& pluginId, const QJsonObject& state);

  [[nodiscard]] QString displayTitle(int uniqueId) const;
  [[nodiscard]] QString widgetDisplayTitle(int widgetType, int uniqueId) const;
  [[nodiscard]] QString freezeTitleMode(int widgetType, int uniqueId) const;
  [[nodiscard]] QJsonObject displayTitles() const;
  void setDisplayTitle(int uniqueId, const QString& title);
  void setWidgetDisplayTitle(int widgetType, int uniqueId, const QString& title);
  void setFreezeTitleMode(int widgetType, int uniqueId, const QString& mode);
  void promptRenameWidget(int widgetType, int uniqueId, const QString& currentTitle);

  void setTreeExpansion(const QJsonObject& expansion);
  void setDiagramCollapse(const QJsonObject& state);

  [[nodiscard]] QStringList xDataSources() const;
  [[nodiscard]] QList<int> xDataSourceUniqueIds() const;
  [[nodiscard]] QStringList yWaterfallSources() const;
  [[nodiscard]] QList<int> yWaterfallSourceUniqueIds() const;

  [[nodiscard]] QVariantList sourcesForDiagram() const;
  [[nodiscard]] QVariantList groupsForDiagram() const;
  [[nodiscard]] QVariantList actionsForDiagram() const;
  [[nodiscard]] QVariantList tablesForDiagram() const;

private:
  void stageDisplayTitle(const QString& key, const QString& title);

private:
  ProjectModel& m_model;

  QJsonObject m_widgetSettings;
  QJsonObject m_widgetDisplay;
  QJsonObject m_treeExpansion;
  QJsonObject m_diagramCollapse;
};

}  // namespace DataModel
