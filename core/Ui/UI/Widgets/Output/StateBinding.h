/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QString>

#include "Core/DataModel/Frame.h"

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

  void configure(const DataModel::OutputWidget& config);
  [[nodiscard]] bool bound() const noexcept;
  [[nodiscard]] int datasetId() const noexcept;
  [[nodiscard]] const QString& table() const noexcept;
  [[nodiscard]] const QString& variable() const noexcept;
  [[nodiscard]] bool readsTable() const noexcept;

  void observe(const double value, const QString& text, const qint64 sampleMs);
  void observeTable(const double value, const QString& text, const quint64 writeClock);
  void forget();

  [[nodiscard]] static qint64 nowMs() noexcept;

  void noteRequested(const qint64 nowMs);
  void beginInteraction();
  void endInteraction(const qint64 nowMs);
  [[nodiscard]] bool holdsDisplay(const qint64 nowMs) const;
  [[nodiscard]] bool pending(const qint64 nowMs) const;

  [[nodiscard]] Verdict verdict(const qint64 nowMs) const;

  [[nodiscard]] static bool truth(const QString& onValue, const double value, const QString& text);

private:
  [[nodiscard]] qint64 staleWindowMs() const noexcept;

private:
  bool m_hasValue          = false;
  bool m_interacting       = false;
  int m_datasetId          = -1;
  int m_confirmMs          = 0;
  double m_value           = 0;
  qint64 m_sampleMs        = 0;
  qint64 m_requestedMs     = -1;
  quint64 m_lastWriteClock = 0;
  QString m_text;
  QString m_table;
  QString m_variable;
  QString m_onValue;
  DataModel::OutputStateSource m_source = DataModel::OutputStateSource::None;
};

}  // namespace Output
}  // namespace Widgets
