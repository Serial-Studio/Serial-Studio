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
  static void registerGroupCommands();
  static void registerDatasetCommands();
  static void registerActionCommands();
  static void registerOutputWidgetCommands();
  static void registerParserCommands();
  static void registerListCommands();

  static CommandResponse fileNew(const QString& id, const QJsonObject& params);
  static CommandResponse fileOpen(const QString& id, const QJsonObject& params);
  static CommandResponse fileSave(const QString& id, const QJsonObject& params);
  static CommandResponse setTitle(const QString& id, const QJsonObject& params);
  static CommandResponse getStatus(const QString& id, const QJsonObject& params);

  static CommandResponse groupAdd(const QString& id, const QJsonObject& params);
  static CommandResponse groupDelete(const QString& id, const QJsonObject& params);
  static CommandResponse groupDuplicate(const QString& id, const QJsonObject& params);
  static CommandResponse groupSelect(const QString& id, const QJsonObject& params);

  static CommandResponse datasetAdd(const QString& id, const QJsonObject& params);
  static CommandResponse datasetDelete(const QString& id, const QJsonObject& params);
  static CommandResponse datasetDuplicate(const QString& id, const QJsonObject& params);
  static CommandResponse datasetSetOption(const QString& id, const QJsonObject& params);

  static CommandResponse actionAdd(const QString& id, const QJsonObject& params);
  static CommandResponse actionDelete(const QString& id, const QJsonObject& params);
  static CommandResponse actionDuplicate(const QString& id, const QJsonObject& params);

  static CommandResponse outputWidgetAdd(const QString& id, const QJsonObject& params);
  static CommandResponse outputWidgetDelete(const QString& id, const QJsonObject& params);
  static CommandResponse outputWidgetDuplicate(const QString& id, const QJsonObject& params);

  static CommandResponse parserSetCode(const QString& id, const QJsonObject& params);
  static CommandResponse parserGetCode(const QString& id, const QJsonObject& params);
  static CommandResponse parserSetLanguage(const QString& id, const QJsonObject& params);
  static CommandResponse parserGetLanguage(const QString& id, const QJsonObject& params);
  static CommandResponse frameParserConfigure(const QString& id, const QJsonObject& params);
  static CommandResponse frameParserGetConfig(const QString& id, const QJsonObject& params);

  static CommandResponse loadFromJSON(const QString& id, const QJsonObject& params);
  static CommandResponse exportJson(const QString& id, const QJsonObject& params);
  static CommandResponse loadIntoFrameBuilder(const QString& id, const QJsonObject& params);

  static CommandResponse groupsList(const QString& id, const QJsonObject& params);
  static CommandResponse datasetsList(const QString& id, const QJsonObject& params);
  static CommandResponse actionsList(const QString& id, const QJsonObject& params);

  static CommandResponse datasetSetVirtual(const QString& id, const QJsonObject& params);
  static CommandResponse datasetSetTransformCode(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API
