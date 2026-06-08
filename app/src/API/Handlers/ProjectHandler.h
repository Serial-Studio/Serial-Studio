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

#include <QSet>
#include <QString>

#include "API/CommandProtocol.h"

namespace DataModel {
struct Dataset;
}  // namespace DataModel

namespace API {
namespace Handlers {
/**
 * @brief Registers API commands for DataModel::ProjectModel operations.
 */
class ProjectHandler {
public:
  static void registerCommands();

private:
  static void registerFileCommands();
  static void registerFileLifecycleCommands();
  static void registerFileMetadataCommands();
  static void registerGroupCommands();
  static void registerDatasetCommands();
  static void registerDatasetCrudCommands();
  static void registerDatasetCreateCommands();
  static void registerDatasetLifecycleCommands();
  static void registerDatasetOptionCommands();
  static void registerDatasetFieldCommands();
  static void registerDatasetAlarmCommands();
  static void registerActionCommands();
  static void registerOutputWidgetCommands();
  static void registerParserCommands();
  static void registerParserCodeCommands();
  static void registerParserTemplateCommands();
  static void registerParserConfigCommands();
  static void registerPainterCommands();
  static void registerPainterCodeCommands();
  static void registerUpdateCommands();
  static void registerEntityUpdateCommands();
  static void registerBatchCommand();
  static void registerDryRunCommands();
  static void registerFrameParserDryRunCommands();
  static void registerScriptDryRunCommands();
  static void registerEndToEndDryRunCommand();
  static void registerListCommands();
  static void registerResolverCommands();
  static void registerSnapshotAndMoveCommands();
  static void registerTemplateCommands();

  static CommandResponse fileNew(const QString& id, const QJsonObject& params);
  static CommandResponse fileOpen(const QString& id, const QJsonObject& params);
  static CommandResponse fileSave(const QString& id, const QJsonObject& params);
  static CommandResponse setTitle(const QString& id, const QJsonObject& params);
  static CommandResponse getStatus(const QString& id, const QJsonObject& params);

  static CommandResponse groupAdd(const QString& id, const QJsonObject& params);
  static CommandResponse groupDelete(const QString& id, const QJsonObject& params);
  static CommandResponse groupDuplicate(const QString& id, const QJsonObject& params);

  static CommandResponse datasetAdd(const QString& id, const QJsonObject& params);
  static CommandResponse datasetAddMany(const QString& id, const QJsonObject& params);
  static CommandResponse datasetDelete(const QString& id, const QJsonObject& params);
  static CommandResponse datasetDuplicate(const QString& id, const QJsonObject& params);
  static CommandResponse datasetSetOption(const QString& id, const QJsonObject& params);
  static CommandResponse datasetSetOptions(const QString& id, const QJsonObject& params);

  static CommandResponse actionAdd(const QString& id, const QJsonObject& params);
  static CommandResponse actionDelete(const QString& id, const QJsonObject& params);
  static CommandResponse actionDuplicate(const QString& id, const QJsonObject& params);

  static CommandResponse outputWidgetAdd(const QString& id, const QJsonObject& params);
  static CommandResponse outputWidgetDelete(const QString& id, const QJsonObject& params);
  static CommandResponse outputWidgetDuplicate(const QString& id, const QJsonObject& params);
  static CommandResponse outputWidgetGet(const QString& id, const QJsonObject& params);

  static CommandResponse parserSetCode(const QString& id, const QJsonObject& params);
  static CommandResponse parserGetCode(const QString& id, const QJsonObject& params);
  static CommandResponse parserSetLanguage(const QString& id, const QJsonObject& params);
  static CommandResponse parserGetLanguage(const QString& id, const QJsonObject& params);
  static CommandResponse parserListTemplates(const QString& id, const QJsonObject& params);
  static CommandResponse parserGetTemplateSchema(const QString& id, const QJsonObject& params);
  static CommandResponse parserGetTemplate(const QString& id, const QJsonObject& params);
  static CommandResponse parserSetTemplate(const QString& id, const QJsonObject& params);
  static CommandResponse frameParserConfigure(const QString& id, const QJsonObject& params);
  static CommandResponse frameParserGetConfig(const QString& id, const QJsonObject& params);

  static CommandResponse painterSetCode(const QString& id, const QJsonObject& params);
  static CommandResponse painterGetCode(const QString& id, const QJsonObject& params);

  static CommandResponse frameParserDryRun(const QString& id, const QJsonObject& params);
  static CommandResponse frameParserDryCompile(const QString& id, const QJsonObject& params);
  static CommandResponse transformDryRun(const QString& id, const QJsonObject& params);
  static CommandResponse painterDryRun(const QString& id, const QJsonObject& params);
  static CommandResponse outputWidgetDryRun(const QString& id, const QJsonObject& params);
  static CommandResponse endToEndDryRun(const QString& id, const QJsonObject& params);

  static CommandResponse groupUpdate(const QString& id, const QJsonObject& params);
  static CommandResponse datasetUpdate(const QString& id, const QJsonObject& params);
  static CommandResponse actionUpdate(const QString& id, const QJsonObject& params);
  static CommandResponse outputWidgetUpdate(const QString& id, const QJsonObject& params);

  static CommandResponse loadFromJSON(const QString& id, const QJsonObject& params);
  static CommandResponse exportJson(const QString& id, const QJsonObject& params);
  static CommandResponse loadIntoFrameBuilder(const QString& id, const QJsonObject& params);

  static CommandResponse templateList(const QString& id, const QJsonObject& params);
  static CommandResponse templateApply(const QString& id, const QJsonObject& params);

  static CommandResponse validate(const QString& id, const QJsonObject& params);

  static CommandResponse groupsList(const QString& id, const QJsonObject& params);
  static CommandResponse datasetsList(const QString& id, const QJsonObject& params);
  static CommandResponse actionsList(const QString& id, const QJsonObject& params);

  static CommandResponse datasetGetByUniqueId(const QString& id, const QJsonObject& params);
  static CommandResponse datasetGetByTitle(const QString& id, const QJsonObject& params);
  static CommandResponse datasetGetByPath(const QString& id, const QJsonObject& params);
  static CommandResponse datasetGetExecutionOrder(const QString& id, const QJsonObject& params);

  static CommandResponse projectSnapshot(const QString& id, const QJsonObject& params);

  static CommandResponse datasetMove(const QString& id, const QJsonObject& params);
  static CommandResponse groupMove(const QString& id, const QJsonObject& params);

  static CommandResponse datasetSetVirtual(const QString& id, const QJsonObject& params);
  static CommandResponse datasetSetTransformCode(const QString& id, const QJsonObject& params);

  static CommandResponse datasetGetAlarmBands(const QString& id, const QJsonObject& params);
  static CommandResponse datasetSetAlarmBands(const QString& id, const QJsonObject& params);

  static CommandResponse projectBatch(const QString& id, const QJsonObject& params);

  static void applyDatasetVisualizationFlags(DataModel::Dataset& d, int options);
  static QString widgetForDatasetOptions(int options);
  static QString applyDatasetUpdateParams(DataModel::Dataset& d,
                                          const QJsonObject& params,
                                          bool& rebuildTree,
                                          QSet<QString>& consumed);
};

}  // namespace Handlers
}  // namespace API
