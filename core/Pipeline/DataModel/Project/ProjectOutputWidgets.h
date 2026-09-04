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

#include <QString>

#include "DataModel/Frame.h"
#include "SerialStudio.h"

namespace DataModel {

class ProjectModel;

/**
 * @brief Output (control) widget editing: creation inside an output group, type/icon changes,
 *        update, duplication, deletion and reordering. Output widgets live inside their owning
 *        Group, so this class owns behaviour only. Deleting the last widget of an output group
 *        cascades into deleting the group, so the delete path runs the same repair as a group.
 */
class ProjectOutputWidgets {
public:
  explicit ProjectOutputWidgets(ProjectModel& model);
  ProjectOutputWidgets(ProjectOutputWidgets&&)                 = delete;
  ProjectOutputWidgets(const ProjectOutputWidgets&)            = delete;
  ProjectOutputWidgets& operator=(ProjectOutputWidgets&&)      = delete;
  ProjectOutputWidgets& operator=(const ProjectOutputWidgets&) = delete;

  void addOutputPanel(int sourceId);
  void addOutputControl(const SerialStudio::OutputWidgetType type, int sourceId);

  void setOutputWidgetType(int type);
  void setOutputWidgetIcon(const QString& icon);
  void updateOutputWidget(int groupId, int widgetId, const OutputWidget& widget, bool rebuildTree);

  void deleteCurrentOutputWidget();
  void duplicateCurrentOutputWidget();
  void deleteOutputWidget(int groupId, int widgetId, bool confirm);
  void duplicateOutputWidget(int groupId, int widgetId);
  void moveOutputWidget(int groupId, int fromWidgetId, int toWidgetId);

private:
  ProjectModel& m_model;
};

}  // namespace DataModel
