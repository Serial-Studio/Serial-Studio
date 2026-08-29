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

#include <QJsonObject>
#include <QString>

#include "DataModel/Frame.h"

namespace DataModel {

class ProjectModel;

/**
 * @brief Data-source (device) editing for the project document: source CRUD, the per-source frame
 *        parser settings, and the driver connection-settings capture/restore pair. Source 0
 *        doubles as the project-wide frame-detection record, so the setSource0* and
 *        setFrameParser* entry points exist beside the indexed ones.
 */
class ProjectSources {
public:
  explicit ProjectSources(ProjectModel& model);
  ProjectSources(ProjectSources&&)                 = delete;
  ProjectSources(const ProjectSources&)            = delete;
  ProjectSources& operator=(ProjectSources&&)      = delete;
  ProjectSources& operator=(const ProjectSources&) = delete;

  static void seedDefaultFrameParser(Source& source);

  void addSource();
  void deleteSource(int sourceId, bool confirm);
  void duplicateSource(int sourceId);
  void updateSource(int sourceId, const Source& source, const bool rebuildTree);
  void updateSourceTitle(int sourceId, const QString& title, const bool rebuildTree);
  void updateSourceBusType(int sourceId, int busType);
  void promptRenameSource(int sourceId);

  void captureSourceSettings(int sourceId);
  void restoreSourceSettings(int sourceId);
  void setSource0ConnectionSettings(const QJsonObject& settings);
  void setSource0BusType(int busType);

  void setFrameParserCode(const QString& code);
  void setFrameParserLanguage(int language);
  void setFrameParserTemplate(const QString& templateId);
  void setFrameParserParams(const QJsonObject& params);

  void updateSourceFrameParser(int sourceId, const QString& code);
  void updateSourceFrameParserLanguage(int sourceId, int language);
  void updateSourceFrameParserTemplate(int sourceId, const QString& templateId);
  void updateSourceFrameParserParams(int sourceId, const QJsonObject& params);
  void updateSourceStreamLane(int sourceId, const QString& lane);
  void storeFrameParserCode(int sourceId, const QString& code);

private:
  ProjectModel& m_model;
};

}  // namespace DataModel
