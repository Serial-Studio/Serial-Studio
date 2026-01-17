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

namespace API
{
/**
 * @class CommandHandler
 * @brief Main entry point for processing incoming API commands
 *
 * The CommandHandler is responsible for:
 * - Parsing incoming JSON messages
 * - Routing commands to the CommandRegistry
 * - Handling batch requests with ordered execution
 * - Formatting and returning responses
 *
 * It initializes all command handlers on first access, ensuring commands
 * are registered before any processing occurs.
 */
class CommandHandler : public QObject
{
  Q_OBJECT

private:
  explicit CommandHandler(QObject *parent = nullptr);
  CommandHandler(CommandHandler &&) = delete;
  CommandHandler(const CommandHandler &) = delete;
  CommandHandler &operator=(CommandHandler &&) = delete;
  CommandHandler &operator=(const CommandHandler &) = delete;

public:
  /**
   * @brief Gets the singleton instance
   */
  static CommandHandler &instance();

  /**
   * @brief Check if data appears to be an API message
   * @param data Raw bytes to check
   * @return true if data looks like a JSON API command
   */
  [[nodiscard]] bool isApiMessage(const QByteArray &data) const;

  /**
   * @brief Process an incoming message and return the response
   * @param data Raw JSON message bytes
   * @return Response as JSON bytes (ready to send back to client)
   */
  [[nodiscard]] QByteArray processMessage(const QByteArray &data);

  /**
   * @brief Process a single command request
   * @param request The parsed command request
   * @return CommandResponse with result or error
   */
  [[nodiscard]] CommandResponse processCommand(const CommandRequest &request);

  /**
   * @brief Process a batch of commands in order
   * @param batch The parsed batch request
   * @return BatchResponse containing all individual results
   */
  [[nodiscard]] BatchResponse processBatch(const BatchRequest &batch);

  /**
   * @brief Get a list of all available commands
   * @return JSON array of command names and descriptions
   */
  [[nodiscard]] QJsonObject getAvailableCommands() const;

private:
  /**
   * @brief Initialize all command handlers
   *
   * Called once on first access to register all available commands
   * with the CommandRegistry.
   */
  void initializeHandlers();

private:
  bool m_initialized;
};

} // namespace API

