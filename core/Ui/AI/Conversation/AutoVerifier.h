/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QCoreApplication>
#include <QJsonObject>
#include <QString>

namespace AI {

class CommandRegistry;
class ToolDispatcher;

/**
 * @brief Harness-enforced read-back verification for apply-class mutations: after a tool
 *        reports success, the matching Safe-tier check is re-run through the dispatcher and
 *        its verdict travels with the tool result. The safety tier is asserted from the
 *        injected registry, never assumed, so a read-back can never itself mutate.
 */
class AutoVerifier {
  Q_DECLARE_TR_FUNCTIONS(AI::AutoVerifier)

public:
  explicit AutoVerifier(const CommandRegistry& commands);

  void setDispatcher(ToolDispatcher* dispatcher);
  [[nodiscard]] QJsonObject verify(const QString& name,
                                   const QJsonObject& arguments,
                                   const QJsonObject& reply);

private:
  [[nodiscard]] QJsonObject verifySourceUpdate(const QJsonObject& arguments);

private:
  const CommandRegistry& m_commands;
  ToolDispatcher* m_dispatcher;
};

}  // namespace AI
