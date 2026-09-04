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

#pragma once

#include <QJsonObject>
#include <QString>

#include "API/CommandRegistry.h"

namespace API {
namespace Handlers {

/**
 * @brief Document-level command surface: new/open/save, undo/redo, status, validation, export,
 *        the composite snapshot read, and the starter templates. Registration is split so the
 *        facade can keep the historical order, where snapshot and templates land after the
 *        list and discovery blocks.
 */
class ProjectFileCommands {
public:
  explicit ProjectFileCommands(CommandRegistry& registry);

  void registerFileCommands();
  void registerSnapshotCommand();
  void registerTemplateCommands();

private:
  void registerHistoryCommands();
  void registerMetadataCommands();
  void registerLifecycleCommands();

  [[nodiscard]] static CommandResponse fileNew(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse fileOpen(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse fileSave(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse setTitle(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse validate(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse getStatus(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse exportJson(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse projectUndo(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse projectRedo(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse loadFromJSON(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse templateList(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse templateApply(const QString& id, const QJsonObject& params);
  [[nodiscard]] static CommandResponse projectSnapshot(const QString& id,
                                                       const QJsonObject& params);
  [[nodiscard]] static CommandResponse loadIntoFrameBuilder(const QString& id,
                                                            const QJsonObject& params);

  CommandRegistry& m_registry;
};

}  // namespace Handlers
}  // namespace API
