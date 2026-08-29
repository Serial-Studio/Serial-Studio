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
#include <QJsonObject>
#include <QObject>
#include <QString>

class QTimer;
class QFileSystemWatcher;

namespace DataModel {

class ProjectModel;

/**
 * @brief Disk side of the project document: serialization, the save/save-as/headless-save paths,
 *        the debounced autosave, and the on-disk change watcher. Constructed inside ProjectModel's
 *        protected ctor closure, so nothing here may reach another module's instance() before
 *        ProjectModel::m_initialized is set.
 */
class ProjectPersistence : public QObject {
  Q_OBJECT

public:
  explicit ProjectPersistence(ProjectModel& model);
  ProjectPersistence(ProjectPersistence&&)                 = delete;
  ProjectPersistence(const ProjectPersistence&)            = delete;
  ProjectPersistence& operator=(ProjectPersistence&&)      = delete;
  ProjectPersistence& operator=(const ProjectPersistence&) = delete;

  [[nodiscard]] QJsonObject serializeToJson() const;

  [[nodiscard]] bool askSave();
  [[nodiscard]] bool saveJsonFile(const bool askPath);
  [[nodiscard]] bool apiSaveJsonFile(const QString& path);
  [[nodiscard]] bool finalizeProjectSave();
  [[nodiscard]] bool writeProjectFile(const QString& path);

  void watchProjectFile();

  [[nodiscard]] bool autoSaveSuspended() const noexcept;
  [[nodiscard]] bool runtimeDirty() const noexcept;
  void setRuntimeDirty(bool dirty) noexcept;

  void flushAutoSave();
  void scheduleAutoSave();
  void setAutoSaveSuspended(bool suspend);
  void stopAutoSaveTimer();
  void syncRuntime();

public slots:
  void autoSave();

private slots:
  void resolveDiskFileChange();

private:
  void serializeDocumentScalars(QJsonObject& json) const;
  void serializeEntityArrays(QJsonObject& json) const;
  void serializeWorkspacesAndTables(QJsonObject& json) const;
  void serializePresentationAndSinks(QJsonObject& json) const;

  void promptDiskFileReload();
  [[nodiscard]] static QByteArray hashProjectFile(const QString& path);

private:
  ProjectModel& m_model;

  QTimer* m_autoSaveTimer;
  bool m_autoSaveSuspended;
  bool m_runtimeDirty;

  QFileSystemWatcher* m_fileWatcher;
  bool m_diskCheckPending;
  bool m_diskPromptActive;
  QByteArray m_diskFileHash;
};

}  // namespace DataModel
