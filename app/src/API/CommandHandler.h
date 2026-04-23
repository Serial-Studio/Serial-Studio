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
  [[nodiscard]] QByteArray processMessage(const QByteArray& data);
  [[nodiscard]] CommandResponse processCommand(const CommandRequest& request);
  [[nodiscard]] BatchResponse processBatch(const BatchRequest& batch);
  [[nodiscard]] QJsonObject getAvailableCommands() const;

private:
  void initializeHandlers();

private:
  bool m_initialized;
};

}  // namespace API
