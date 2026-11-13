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

#include <QObject>

#include "Core/Api/CommandProtocol.h"
#include "Core/Api/ICommandExecutor.h"

namespace API {
/**
 * @brief Identifies who issued a command: internal modules are trusted; a project's own scripts
 *        are trusted except for the few commands no project script needs (spec 0120); network
 *        clients must pass the device-write consent gate.
 */
enum class CommandOrigin : quint8 {
  Trusted,
  Remote,
  Script,
};

/**
 * @brief Main entry point for processing incoming API commands. The composition root registers
 *        the handler sets (registerCoreHandlers(), then the Ui and licensing sets) and binds this
 *        object as the scripting layer's ICommandExecutor (spec 0077 T60).
 */
class CommandHandler
  : public QObject
  , public ICommandExecutor {
  Q_OBJECT

private:
  explicit CommandHandler(QObject* parent = nullptr);
  CommandHandler(CommandHandler&&)                 = delete;
  CommandHandler(const CommandHandler&)            = delete;
  CommandHandler& operator=(CommandHandler&&)      = delete;
  CommandHandler& operator=(const CommandHandler&) = delete;

public:
  [[nodiscard]] static CommandHandler& instance();

  [[nodiscard]] bool isApiMessage(const QByteArray& data) const;
  [[nodiscard]] QByteArray processMessage(const QByteArray& data,
                                          const CommandOrigin origin = CommandOrigin::Trusted);
  [[nodiscard]] CommandResponse processCommand(const CommandRequest& request,
                                               const CommandOrigin origin = CommandOrigin::Trusted);
  [[nodiscard]] BatchResponse processBatch(const BatchRequest& batch,
                                           const CommandOrigin origin = CommandOrigin::Trusted);
  [[nodiscard]] QJsonObject getAvailableCommands() const;

  [[nodiscard]] CommandResponse execute(const CommandRequest& request) override;
  [[nodiscard]] bool hasCommand(const QString& name) const override;
  [[nodiscard]] QStringList commandNames() const override;

  void registerCoreHandlers();

private:
  bool m_initialized;
};

}  // namespace API
