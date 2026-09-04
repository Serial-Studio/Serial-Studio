/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

#include "AI/Providers/Provider.h"

namespace AI {

/**
 * @brief Reply that fires errorOccurred + finished on the next event-loop tick. Used when
 *        sendMessage cannot reach the network at all (e.g. no key); emits only signals
 *        inherited from Reply, so no Q_OBJECT (and no moc pass) is needed.
 */
class ImmediateErrorReply : public Reply {
public:
  /**
   * @brief Schedules an error+finished pair via QTimer::singleShot(0, ...).
   */
  explicit ImmediateErrorReply(const QString& message, QObject* parent = nullptr)
    : Reply(parent), m_message(message)
  {
    QTimer::singleShot(0, this, [this]() {
      Q_EMIT errorOccurred(m_message);
      Q_EMIT finished();
    });
  }

  /**
   * @brief No-op: the timer-driven completion cannot be cancelled.
   */
  void abort() override {}

private:
  QString m_message;
};

}  // namespace AI
