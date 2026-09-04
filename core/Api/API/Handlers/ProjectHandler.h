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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <optional>
#include <QJsonObject>
#include <QSet>
#include <QString>

#include "API/CommandRegistry.h"
#include "API/Handlers/ProjectActionCommands.h"
#include "API/Handlers/ProjectBatchCommands.h"
#include "API/Handlers/ProjectDatasetCommands.h"
#include "API/Handlers/ProjectDatasetFieldCommands.h"
#include "API/Handlers/ProjectDiscoveryCommands.h"
#include "API/Handlers/ProjectDryRunCommands.h"
#include "API/Handlers/ProjectFileCommands.h"
#include "API/Handlers/ProjectGroupCommands.h"
#include "API/Handlers/ProjectListCommands.h"
#include "API/Handlers/ProjectOutputWidgetCommands.h"
#include "API/Handlers/ProjectPainterCommands.h"
#include "API/Handlers/ProjectParserCommands.h"
#include "API/Handlers/ProjectUpdateCommands.h"

namespace DataModel {
struct Dataset;
}  // namespace DataModel

namespace API {
namespace Handlers {

/**
 * @brief Returns the typed schema properties for the dataset verbs, derived from
 *        app/rcc/properties/dataset.json (Generated/DatasetApiFields.cpp).
 */
[[nodiscard]] QJsonObject datasetFieldSchema();

/**
 * @brief Applies simple-mode alarmEnabled / alarmLow / alarmHigh fields to a dataset's alarmBands.
 */
void applySimpleAlarmFields(DataModel::Dataset& d,
                            std::optional<bool> enabled,
                            std::optional<double> low,
                            std::optional<double> high);

/**
 * @brief Facade over the per-domain project command classes: owns one instance of each and fans
 *        registration out to them in the order the registry has always seen.
 */
class ProjectHandler {
public:
  static void registerCommands(CommandRegistry& registry);

  [[nodiscard]] static QString applyDatasetUpdateParams(DataModel::Dataset& d,
                                                        const QJsonObject& params,
                                                        bool& rebuildTree,
                                                        QSet<QString>& consumed);

private:
  explicit ProjectHandler(CommandRegistry& registry);
  void registerAll();

  ProjectFileCommands m_file;
  ProjectGroupCommands m_group;
  ProjectDatasetCommands m_dataset;
  ProjectDatasetFieldCommands m_datasetFields;
  ProjectActionCommands m_action;
  ProjectOutputWidgetCommands m_outputWidget;
  ProjectParserCommands m_parser;
  ProjectPainterCommands m_painter;
  ProjectUpdateCommands m_update;
  ProjectBatchCommands m_batch;
  ProjectDryRunCommands m_dryRun;
  ProjectListCommands m_list;
  ProjectDiscoveryCommands m_discovery;
};

}  // namespace Handlers
}  // namespace API
