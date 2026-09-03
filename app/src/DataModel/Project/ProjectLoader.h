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

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

namespace DataModel {

class ProjectModel;

/**
 * @brief Read side of the project document: open/import entry points, JSON deserialization,
 *        legacy-schema migrations, and the undo-snapshot restore that shares that pipeline. The
 *        restore path must never emit jsonFileChanged, or BackupManager would snapshot per undo
 *        and the editor path handlers would fire; emitProjectLoadedSignals(false) prevents that.
 */
class ProjectLoader {
public:
  /**
   * @brief Schema facts a document load hands back to its caller for follow-up work.
   */
  struct DocumentLoadFlags {
    int loadedSchema;
    bool olderSchema;
    bool legacyUniqueIds;
  };

  explicit ProjectLoader(ProjectModel& model);
  ProjectLoader(ProjectLoader&&)                 = delete;
  ProjectLoader(const ProjectLoader&)            = delete;
  ProjectLoader& operator=(ProjectLoader&&)      = delete;
  ProjectLoader& operator=(const ProjectLoader&) = delete;

  void openJsonFile();
  [[nodiscard]] bool openJsonFile(const QString& path);

  [[nodiscard]] bool lastOpenReloaded() const noexcept { return m_lastOpenReloaded; }

  [[nodiscard]] bool loadFromJsonDocument(const QJsonDocument& document, const QString& sourcePath);
  void importProjectFromJson(const QJsonObject& project, const QString& suggestedFileName);

  [[nodiscard]] bool applyHistorySnapshot(const QByteArray& state);

  void seedNextUniqueIdFromGroups();
  void deduplicateUniqueIds();
  void resolveDatasetTransformLanguages();
  void resolveDatasetVirtualFlags();
  void emitProjectLoadedSignals(const bool includeJsonFileChanged = true);

private:
  [[nodiscard]] DocumentLoadFlags applyJsonDocumentCore(const QJsonObject& json);

  void loadProjectRootScalars(const QJsonObject& json);
  void loadProjectArrays(const QJsonObject& json, const QString& legacyParserCode);
  void seedDefaultSourceFromUi(const QString& legacyParserCode);
  void enforceGplSingleSource();
  void loadWidgetSettingsAndWorkspaces(const QJsonObject& json);
  void loadCustomWorkspaces(const QJsonObject& json);
  void loadWorkspaceAndGroupFolders(const QJsonObject& json);
  void loadHiddenGroupsAndTables(const QJsonObject& json);
  void loadSinkConfigs(const QJsonObject& json);
  void loadPointCount(const QJsonObject& json);
  void loadPlotTimeRange(const QJsonObject& json);
  void loadFrozen(const QJsonObject& json);
  void loadChangeDrivenTransforms(const QJsonObject& json);
  void loadLuaFastMode(const QJsonObject& json);

  void migrateLegacyWorkspaceRefs();
  void migrateLegacyXAxisIds();
  void migrateLegacyWaterfallYAxisIds();
  void migrateLegacyLayoutKeys();
  void migrateLegacyDashboardLayout(const QJsonObject& json);
  [[nodiscard]] bool migrateLegacySeparator(const QJsonObject& json);
  void persistLegacyMigration();

private:
  ProjectModel& m_model;
  bool m_lastOpenReloaded;
};

}  // namespace DataModel
