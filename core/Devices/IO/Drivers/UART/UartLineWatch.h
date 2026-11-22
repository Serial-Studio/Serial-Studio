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

#include <functional>
#include <QObject>
#include <QTimer>

namespace IO {
namespace Drivers {
class UART;

/**
 * @brief The four input control lines of the UART driver's open port (spec 0127 R22), for the
 *        setup pane's indicators. A serial port raises no event when a modem line changes, so the
 *        lines are polled, and only while the port is open and at least one view asked to watch:
 *        a hidden pane costs no system call.
 */
class UartLineWatch : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool available
             READ available
             NOTIFY linesChanged)
  Q_PROPERTY(bool cts
             READ cts
             NOTIFY linesChanged)
  Q_PROPERTY(bool dsr
             READ dsr
             NOTIFY linesChanged)
  Q_PROPERTY(bool dcd
             READ dcd
             NOTIFY linesChanged)
  Q_PROPERTY(bool ri
             READ ri
             NOTIFY linesChanged)
  // clang-format on

signals:
  void linesChanged();

public:
  static constexpr int kPollIntervalMs = 250;

  using Source = std::function<UART*()>;

  explicit UartLineWatch(UART& driver);

  void bindSource(Source source);

  [[nodiscard]] bool ri() const noexcept;
  [[nodiscard]] bool cts() const noexcept;
  [[nodiscard]] bool dsr() const noexcept;
  [[nodiscard]] bool dcd() const noexcept;
  [[nodiscard]] bool available() const noexcept;
  [[nodiscard]] int watchers() const noexcept;

public slots:
  void attach();
  void detach();
  void poll();

private:
  void store(bool available, bool cts, bool dsr, bool dcd, bool ri);

private:
  UART& m_driver;
  Source m_source;
  QTimer m_timer;
  int m_watchers;
  bool m_available;
  bool m_cts;
  bool m_dsr;
  bool m_dcd;
  bool m_ri;
};

}  // namespace Drivers
}  // namespace IO
