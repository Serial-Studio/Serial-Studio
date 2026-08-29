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

#include "API/Handlers/ProjectHandler.h"

/**
 * @brief Wires every command class to the shared registry.
 */
API::Handlers::ProjectHandler::ProjectHandler(CommandRegistry& registry)
  : m_file(registry)
  , m_group(registry)
  , m_dataset(registry)
  , m_datasetFields(registry)
  , m_action(registry)
  , m_outputWidget(registry)
  , m_parser(registry)
  , m_painter(registry)
  , m_update(registry)
  , m_batch(registry)
  , m_dryRun(registry)
  , m_list(registry)
  , m_discovery(registry)
{}

/**
 * @brief Register all Project commands with the registry. The facade instance outlives the
 *        registry entries it installs, which hold plain function pointers into the command
 *        classes rather than bound member functions.
 */
void API::Handlers::ProjectHandler::registerCommands(CommandRegistry& registry)
{
  static ProjectHandler handler(registry);
  handler.registerAll();
}

/**
 * @brief Fans registration out to the per-domain command classes. The call order reproduces the
 *        historical registration order exactly; do not reorder.
 */
void API::Handlers::ProjectHandler::registerAll()
{
  m_file.registerFileCommands();
  m_group.registerCommands();
  m_dataset.registerCommands();
  m_datasetFields.registerCommands();
  m_action.registerCommands();
  m_outputWidget.registerCommands();
  m_parser.registerCommands();
  m_painter.registerCommands();
  m_update.registerCommands();
  m_batch.registerCommands();
  m_dryRun.registerCommands();
  m_list.registerListCommands();
  m_discovery.registerCommands();
  m_file.registerSnapshotCommand();
  m_list.registerMoveCommands();
  m_file.registerTemplateCommands();
}
