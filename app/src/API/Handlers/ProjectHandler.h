/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include "API/CommandProtocol.h"

namespace API
{
namespace Handlers
{
/**
 * @class ProjectHandler
 * @brief Registers API commands for DataModel::ProjectModel operations
 *
 * Provides 24 commands for project/configuration management:
 *
 * File Operations (7):
 * - project.file.new - Create new project
 * - project.file.open - Open project file
 * - project.file.save - Save project
 * - project.loadFromJSON - Load project configuration from JSON object
 * - project.getStatus - Get project info
 * - project.exportJson - Export current project as JSON
 * - project.loadIntoFrameBuilder - Load current project into FrameBuilder
 *
 * Group Management (3):
 * - project.group.add - Add group (optional: title, widgetType)
 * - project.group.delete - Delete current group
 * - project.group.duplicate - Duplicate current group
 *
 * Dataset Management (4):
 * - project.dataset.add - Add dataset (optional: options)
 * - project.dataset.delete - Delete current dataset
 * - project.dataset.duplicate - Duplicate current dataset
 * - project.dataset.setOption - Toggle dataset option
 *
 * Action Management (3):
 * - project.action.add - Add action
 * - project.action.delete - Delete current action
 * - project.action.duplicate - Duplicate current action
 *
 * Frame Parser (4):
 * - project.parser.setCode - Set frame parser code
 * - project.parser.getCode - Get frame parser code
 * - project.frameParser.configure - Configure frame parser settings
 * - project.frameParser.getConfig - Get frame parser configuration
 *
 * Queries (3):
 * - project.groups.list - List all groups with dataset counts
 * - project.datasets.list - List all datasets across all groups
 * - project.actions.list - List all actions
 */
class ProjectHandler
{
public:
  /**
   * @brief Register all Project commands with the CommandRegistry
   */
  static void registerCommands();

private:
  // File operations
  static CommandResponse fileNew(const QString &id, const QJsonObject &params);
  static CommandResponse fileOpen(const QString &id, const QJsonObject &params);
  static CommandResponse fileSave(const QString &id, const QJsonObject &params);
  static CommandResponse setTitle(const QString &id, const QJsonObject &params);
  static CommandResponse getStatus(const QString &id,
                                   const QJsonObject &params);

  // Group management
  static CommandResponse groupAdd(const QString &id, const QJsonObject &params);
  static CommandResponse groupDelete(const QString &id,
                                     const QJsonObject &params);
  static CommandResponse groupDuplicate(const QString &id,
                                        const QJsonObject &params);

  // Dataset management
  static CommandResponse datasetAdd(const QString &id,
                                    const QJsonObject &params);
  static CommandResponse datasetDelete(const QString &id,
                                       const QJsonObject &params);
  static CommandResponse datasetDuplicate(const QString &id,
                                          const QJsonObject &params);
  static CommandResponse datasetSetOption(const QString &id,
                                          const QJsonObject &params);

  // Action management
  static CommandResponse actionAdd(const QString &id,
                                   const QJsonObject &params);
  static CommandResponse actionDelete(const QString &id,
                                      const QJsonObject &params);
  static CommandResponse actionDuplicate(const QString &id,
                                         const QJsonObject &params);

  // Frame parser
  static CommandResponse parserSetCode(const QString &id,
                                       const QJsonObject &params);
  static CommandResponse parserGetCode(const QString &id,
                                       const QJsonObject &params);
  static CommandResponse frameParserConfigure(const QString &id,
                                              const QJsonObject &params);
  static CommandResponse frameParserGetConfig(const QString &id,
                                              const QJsonObject &params);

  // Configuration loading
  static CommandResponse loadFromJSON(const QString &id,
                                      const QJsonObject &params);
  static CommandResponse exportJson(const QString &id,
                                    const QJsonObject &params);
  static CommandResponse loadIntoFrameBuilder(const QString &id,
                                              const QJsonObject &params);

  // List queries
  static CommandResponse groupsList(const QString &id,
                                    const QJsonObject &params);
  static CommandResponse datasetsList(const QString &id,
                                      const QJsonObject &params);
  static CommandResponse actionsList(const QString &id,
                                     const QJsonObject &params);
};

} // namespace Handlers
} // namespace API
