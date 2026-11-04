/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QString>
#include <QVariant>

#include "Core/DataModel/Frame.h"
#include "Core/SerialStudio.h"

namespace Widgets {
namespace Output {

/**
 * @brief What an output control should display, given what its state source reports (spec 0080).
 *        Deliberately holds no transmit target and includes nothing that owns one: reflecting
 *        equipment state is not an operator action, and a control able to transmit from here
 *        would command its own equipment in a loop.
 */
class StateBinding {
public:
  /**
   * @brief One reading: whether anything is known, and what it says.
   */
  struct Verdict {
    bool known   = false;
    bool on      = false;
    double value = 0;
    QString text;
  };

  /**
   * @brief Where a reply-bound control's last command stands (spec 0110); the shared enum, so QML
   *        compares the same values.
   */
  using Outcome = SerialStudio::OutputOutcome;

  void configure(const DataModel::OutputWidget& config);
  [[nodiscard]] bool bound() const noexcept;
  [[nodiscard]] bool expectsReply() const noexcept;
  [[nodiscard]] int datasetId() const noexcept;
  [[nodiscard]] int replyCodeId() const noexcept;
  [[nodiscard]] int replyTokenId() const noexcept;
  [[nodiscard]] int replyMessageId() const noexcept;
  [[nodiscard]] const QString& table() const noexcept;
  [[nodiscard]] const QString& variable() const noexcept;
  [[nodiscard]] bool readsTable() const noexcept;

  void observe(const double value, const QString& text, const qint64 sampleMs);
  void observeTable(const double value, const QString& text, const quint64 writeClock);
  void forget();

  [[nodiscard]] static qint64 nowMs() noexcept;

  void noteRequested(const qint64 nowMs, const QVariant& request = QVariant());
  void beginInteraction();
  void endInteraction(const qint64 nowMs);
  void settle(const qint64 nowMs);
  [[nodiscard]] bool holdsDisplay(const qint64 nowMs) const;
  [[nodiscard]] bool pending(const qint64 nowMs) const;

  static void resetTokens();
  [[nodiscard]] int issueToken();
  void noteSent(const qint64 nowMs, const int token);
  void observeReply(const qint64 nowMs, const int token, const int code, const QString& message);
  [[nodiscard]] Outcome outcome(const qint64 nowMs) const;
  [[nodiscard]] int expectedToken() const noexcept;
  [[nodiscard]] int replyCode() const noexcept;
  [[nodiscard]] const QString& replyMessage() const noexcept;

  [[nodiscard]] Verdict verdict(const qint64 nowMs) const;

  [[nodiscard]] static bool truth(const QString& onValue, const double value, const QString& text);

  [[nodiscard]] qint64 windowMs() const noexcept;

private:
  [[nodiscard]] qint64 staleWindowMs() const noexcept;
  [[nodiscard]] bool stateConfirms(const qint64 nowMs) const;

private:
  bool m_twoState          = false;
  bool m_hasValue          = false;
  bool m_hasRequest        = false;
  bool m_interacting       = false;
  bool m_textRequest       = false;
  int m_datasetId          = -1;
  int m_confirmMs          = 0;
  int m_replyCode          = 0;
  int m_seenToken          = 0;
  int m_replyCodeId        = -1;
  int m_replyTokenId       = -1;
  int m_expectedToken      = 0;
  int m_replyMessageId     = -1;
  double m_value           = 0;
  double m_requestValue    = 0;
  qint64 m_sampleMs        = 0;
  qint64 m_requestedMs     = -1;
  quint64 m_lastWriteClock = 0;
  QString m_text;
  QString m_table;
  QString m_variable;
  QString m_onValue;
  QString m_requestText;
  QString m_replyMessage;
  Outcome m_outcome                     = Outcome::None;
  DataModel::OutputStateSource m_source = DataModel::OutputStateSource::None;
};

}  // namespace Output
}  // namespace Widgets
