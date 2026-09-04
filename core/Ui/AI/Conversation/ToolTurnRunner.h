/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
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

#include <optional>
#include <QHash>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QVariantList>

#include "AI/Conversation/ToolCallStatus.h"

namespace AI {

class CommandRegistry;

/**
 * @brief The tool half of one assistant turn (spec 0075, J7): the Confirm-tagged calls waiting on
 *        the user, the outstanding-result counter the resume gate reads, and the ToolCallCard
 *        payloads the chat view renders. Owns that state; the Conversation facade keeps the
 *        signals and the dispatch.
 */
class ToolTurnRunner {
public:
  /**
   * @brief Captured Confirm-state info pending user approval.
   */
  struct PendingCall {
    QString name;
    QJsonObject arguments;
  };

  ToolTurnRunner(QVariantList& uiMessages,
                 const int& assistantIndex,
                 const CommandRegistry& commands);

  ToolTurnRunner(ToolTurnRunner&&)                 = delete;
  ToolTurnRunner(const ToolTurnRunner&)            = delete;
  ToolTurnRunner& operator=(ToolTurnRunner&&)      = delete;
  ToolTurnRunner& operator=(const ToolTurnRunner&) = delete;

  void awaitConfirmation(const QString& callId, const QString& name, const QJsonObject& args);
  void clearPending();
  void noteOutstandingResult() noexcept;
  void releaseOutstandingResult() noexcept;
  void resetOutstanding() noexcept;

  [[nodiscard]] std::optional<PendingCall> takePending(const QString& callId);
  [[nodiscard]] QStringList pendingIds() const;
  [[nodiscard]] QStringList pendingInFamily(const QString& family) const;
  [[nodiscard]] bool hasPending() const noexcept;
  [[nodiscard]] int outstandingResults() const noexcept;
  [[nodiscard]] bool batchComplete(bool replyLive) const noexcept;

  [[nodiscard]] bool appendCard(const QString& callId,
                                const QString& name,
                                const QJsonObject& arguments,
                                ToolCallStatus status);
  [[nodiscard]] bool updateCard(const QString& callId,
                                ToolCallStatus status,
                                const QJsonObject& result,
                                const QJsonObject& verification);

private:
  [[nodiscard]] QString categoryOf(const QString& name) const;

private:
  int m_outstandingToolResults;
  QHash<QString, PendingCall> m_awaitingConfirm;

  // Bind the facade's chat model and its active-message cursor; neither address ever moves
  QVariantList& m_uiMessages;
  const int& m_assistantIndex;
  const CommandRegistry& m_commands;
};

}  // namespace AI
