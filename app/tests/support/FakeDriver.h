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

#include <QByteArray>
#include <QIODevice>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QVariant>

#include "IO/HAL_Driver.h"

namespace Test {

/**
 * @brief HAL_Driver double with a scripted open outcome.
 *
 * The verdict matrix is what the connect path actually has to survive: a synchronous refusal, a
 * synchronous success, and both async outcomes arriving after open() already returned. The async
 * cases are the ones that wedge the connect button in production when a driver reports only
 * success, so this double reports through reportOpenFinished() exactly once for either verdict,
 * and answers isConnecting() true while the dial is in flight.
 */
class FakeDriver final : public IO::HAL_Driver {
  Q_OBJECT

public:
  /**
   * @brief What the scripted open() does: answer immediately, or latch and report later.
   */
  enum class Outcome {
    SyncOk,
    SyncFail,
    AsyncOk,
    AsyncFail,
  };

  explicit FakeDriver(QObject* parent = nullptr);
  ~FakeDriver() override;

  void setOpenOutcome(Outcome outcome);
  void setAsyncDelayMs(int milliseconds);
  void dropLink();

  [[nodiscard]] int openCalls() const;
  [[nodiscard]] int closeCalls() const;
  [[nodiscard]] int writeCalls() const;
  [[nodiscard]] QByteArray lastWrite() const;

  void feed(const QByteArray& data);

  void close() override;
  [[nodiscard]] bool isOpen() const noexcept override;
  [[nodiscard]] bool isConnecting() const noexcept override;
  [[nodiscard]] bool isReadable() const noexcept override;
  [[nodiscard]] bool isWritable() const noexcept override;
  [[nodiscard]] bool configurationOk() const noexcept override;
  [[nodiscard]] qint64 write(const QByteArray& data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;
  [[nodiscard]] QList<IO::DriverProperty> driverProperties() const override;

  [[nodiscard]] QJsonObject deviceIdentifier() const override;
  [[nodiscard]] QVariant driverPropertyValue(const QString& key) const;

public slots:
  void setDriverProperty(const QString& key, const QVariant& value) override;

private slots:
  void finishAsyncOpen();

private:
  Outcome m_outcome;
  int m_asyncDelayMs;
  int m_openCalls;
  int m_closeCalls;
  int m_writeCalls;
  bool m_open;
  bool m_connecting;
  QByteArray m_lastWrite;
  QVariantMap m_properties;
};

}  // namespace Test
