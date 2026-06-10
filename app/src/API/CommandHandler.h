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

#include <QObject>

#include "API/CommandProtocol.h"

namespace API {
/**
 * @brief Identifies who issued a command: in-process callers (scripts, internal modules) are
 *        trusted; network clients must pass the device-write consent gate.
 */
enum class CommandOrigin : quint8 {
  Trusted,
  Remote,
};

/**
 * @brief Main entry point for processing incoming API commands.
 */
class CommandHandler : public QObject {
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

private:
  void initializeHandlers();

private:
  bool m_initialized;
};

}  // namespace API
