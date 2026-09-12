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

#include <QCoreApplication>

class QStandardItem;

namespace InfluxDB {
class Export;
}  // namespace InfluxDB

namespace DataModel {

class ProjectEditor;

/**
 * @brief The InfluxDB Sink form: builds its rows from the live export object, pushes edits
 *        straight back into it, and rebuilds when the object changes underneath the open view.
 *        Without BUILD_COMMERCIAL the form is an empty model.
 */
class EditorInflux {
  Q_DECLARE_TR_FUNCTIONS(DataModel::ProjectEditor)

public:
  explicit EditorInflux(ProjectEditor& editor);
  EditorInflux(EditorInflux&&)                 = delete;
  EditorInflux(const EditorInflux&)            = delete;
  EditorInflux& operator=(EditorInflux&&)      = delete;
  EditorInflux& operator=(const EditorInflux&) = delete;

  void buildInfluxSinkModel();

private:
  void onInfluxSinkItemChanged(QStandardItem* item);
#ifdef BUILD_COMMERCIAL
  void buildSinkSection(bool enabled);
  void buildServerSection(bool enabled);
  void appendTextRow(int type,
                     const QString& name,
                     const QString& value,
                     const QString& placeholder,
                     const QString& description,
                     bool enabled);
#endif

private:
  ProjectEditor& m_editor;
  bool m_applying;
#ifdef BUILD_COMMERCIAL
  InfluxDB::Export& m_export;
#endif
};

}  // namespace DataModel
