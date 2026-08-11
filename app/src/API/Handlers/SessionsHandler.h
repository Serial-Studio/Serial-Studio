/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include "API/CommandProtocol.h"

namespace API {
namespace Handlers {

/**
 * @brief Registers API commands for SQLite session recording (Pro).
 */
class SessionsHandler {
public:
  static void registerCommands();

private:
  static void registerLifecycleCommands();
  static void registerBrowsingCommands();
  static void registerRegressionCommands();
  static void registerTagCommands();

  static CommandResponse getStatus(const QString& id, const QJsonObject& params);
  static CommandResponse setExportEnabled(const QString& id, const QJsonObject& params);
  static CommandResponse close(const QString& id, const QJsonObject& params);
  static CommandResponse getCanonicalDbPath(const QString& id, const QJsonObject& params);

  static CommandResponse openDatabase(const QString& id, const QJsonObject& params);
  static CommandResponse list(const QString& id, const QJsonObject& params);
  static CommandResponse get(const QString& id, const QJsonObject& params);
  static CommandResponse deleteSession(const QString& id, const QJsonObject& params);
  static CommandResponse setNotes(const QString& id, const QJsonObject& params);
  static CommandResponse replay(const QString& id, const QJsonObject& params);
  static CommandResponse exportToCsv(const QString& id, const QJsonObject& params);
  static CommandResponse verify(const QString& id, const QJsonObject& params);
  static CommandResponse getVerification(const QString& id, const QJsonObject& params);
  static CommandResponse regress(const QString& id, const QJsonObject& params);
  static CommandResponse getRegression(const QString& id, const QJsonObject& params);
  static CommandResponse listTags(const QString& id, const QJsonObject& params);
  static CommandResponse addTag(const QString& id, const QJsonObject& params);
  static CommandResponse deleteTag(const QString& id, const QJsonObject& params);
  static CommandResponse renameTag(const QString& id, const QJsonObject& params);
  static CommandResponse assignTag(const QString& id, const QJsonObject& params);
  static CommandResponse removeTag(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API

#endif  // BUILD_COMMERCIAL
