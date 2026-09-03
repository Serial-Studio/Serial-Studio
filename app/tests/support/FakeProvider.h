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

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

#include "AI/Providers/Provider.h"

namespace Test {

/**
 * @brief One step of a scripted assistant reply.
 *
 * BudgetBreach charges past Reply::kMaxStreamedReplyBytes instead of emitting text, which is the
 * only way to reach the streamed-byte cap in a test without moving eight megabytes.
 */
struct ReplyEvent {
  enum Kind {
    Text,
    ToolCall,
    Error,
    Done,
    BudgetBreach,
  } kind;

  QString text;
  QString toolName;
  QJsonObject args;
};

/**
 * @brief Reply that replays a scripted event list, one event per event-loop turn.
 *
 * A zero-millisecond timer per event, rather than emitting the whole script inline, is what makes
 * the double able to reproduce the ordering bugs the review found: an abort or an approval that
 * lands mid-stream has somewhere to land.
 */
class FakeReply final : public AI::Reply {
  Q_OBJECT

public:
  explicit FakeReply(QList<ReplyEvent> script, QObject* parent = nullptr);
  ~FakeReply() override;

  void abort() override;

  [[nodiscard]] bool aborted() const;
  [[nodiscard]] int deliveredEvents() const;

private slots:
  void deliverNext();

private:
  QList<ReplyEvent> m_script;
  int m_index;
  bool m_aborted;
};

/**
 * @brief Provider that hands out FakeReply objects replaying a scripted event list.
 */
class FakeProvider final : public AI::Provider {
public:
  FakeProvider();
  ~FakeProvider() override;

  void script(QList<ReplyEvent> events);

  [[nodiscard]] int sendCount() const;
  [[nodiscard]] QJsonArray lastHistory() const;
  [[nodiscard]] QJsonArray lastTools() const;
  [[nodiscard]] bool lastForbidToolUse() const;

  [[nodiscard]] QString displayName() const override;
  [[nodiscard]] QString keyVendorUrl() const override;
  [[nodiscard]] QStringList availableModels() const override;
  [[nodiscard]] QString defaultModel() const override;
  [[nodiscard]] AI::ProviderCapabilities capabilities() const override;

  [[nodiscard]] AI::Reply* sendMessage(const QJsonArray& history,
                                       const QJsonArray& tools,
                                       bool forbidToolUse = false) override;

private:
  QList<ReplyEvent> m_script;
  QJsonArray m_lastHistory;
  QJsonArray m_lastTools;
  int m_sendCount;
  bool m_lastForbidToolUse;
};

}  // namespace Test
