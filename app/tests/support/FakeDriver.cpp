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

#include "FakeDriver.h"

#include <QTimer>

namespace Test {

FakeDriver::FakeDriver(QObject* parent)
  : IO::HAL_Driver(parent)
  , m_outcome(Outcome::SyncOk)
  , m_asyncDelayMs(0)
  , m_openCalls(0)
  , m_closeCalls(0)
  , m_writeCalls(0)
  , m_open(false)
  , m_connecting(false)
{}

FakeDriver::~FakeDriver() = default;

/**
 * @brief Selects what the next open() does.
 */
void FakeDriver::setOpenOutcome(Outcome outcome)
{
  m_outcome = outcome;
}

/**
 * @brief How long an async verdict stays in flight. Zero posts it on the next event-loop turn.
 */
void FakeDriver::setAsyncDelayMs(int milliseconds)
{
  m_asyncDelayMs = milliseconds < 0 ? 0 : milliseconds;
}

/**
 * @brief Simulates an established link going away without a close() call.
 */
void FakeDriver::dropLink()
{
  if (!m_open)
    return;

  m_open = false;
  Q_EMIT configurationChanged();
}

int FakeDriver::openCalls() const
{
  return m_openCalls;
}

int FakeDriver::closeCalls() const
{
  return m_closeCalls;
}

int FakeDriver::writeCalls() const
{
  return m_writeCalls;
}

QByteArray FakeDriver::lastWrite() const
{
  return m_lastWrite;
}

/**
 * @brief Publishes @p data as if the device had produced it.
 */
void FakeDriver::feed(const QByteArray& data)
{
  publishReceivedData(data);
}

void FakeDriver::close()
{
  ++m_closeCalls;
  m_open       = false;
  m_connecting = false;
}

bool FakeDriver::isOpen() const noexcept
{
  return m_open;
}

bool FakeDriver::isConnecting() const noexcept
{
  return m_connecting;
}

bool FakeDriver::isReadable() const noexcept
{
  return m_open;
}

bool FakeDriver::isWritable() const noexcept
{
  return m_open;
}

bool FakeDriver::configurationOk() const noexcept
{
  return true;
}

qint64 FakeDriver::write(const QByteArray& data)
{
  ++m_writeCalls;
  m_lastWrite = data;
  return m_open ? static_cast<qint64>(data.size()) : -1;
}

/**
 * @brief Runs the scripted outcome. The async cases return true with isConnecting() latched, the
 *        way a driver that dials behind a synchronous open() behaves.
 */
bool FakeDriver::open(const QIODevice::OpenMode mode)
{
  Q_UNUSED(mode);
  ++m_openCalls;

  switch (m_outcome) {
    case Outcome::SyncOk:
      m_open       = true;
      m_connecting = false;
      reportOpenFinished(true);
      return true;

    case Outcome::SyncFail:
      m_open       = false;
      m_connecting = false;
      reportOpenFinished(false, QStringLiteral("scripted synchronous failure"));
      return false;

    case Outcome::AsyncOk:
    case Outcome::AsyncFail:
      m_open       = false;
      m_connecting = true;
      QTimer::singleShot(m_asyncDelayMs, this, &FakeDriver::finishAsyncOpen);
      return true;
  }

  return false;
}

QList<IO::DriverProperty> FakeDriver::driverProperties() const
{
  IO::DriverProperty property;
  property.key   = QStringLiteral("fakeValue");
  property.label = QStringLiteral("Fake Value");
  property.type  = IO::DriverProperty::Text;
  property.value = m_properties.value(property.key);
  return {property};
}

QJsonObject FakeDriver::deviceIdentifier() const
{
  return QJsonObject{
    {QStringLiteral("kind"), QStringLiteral("fake")}
  };
}

/**
 * @brief The last value setDriverProperty() stored for @p key.
 */
QVariant FakeDriver::driverPropertyValue(const QString& key) const
{
  return m_properties.value(key);
}

void FakeDriver::setDriverProperty(const QString& key, const QVariant& value)
{
  if (m_properties.value(key) == value)
    return;

  m_properties.insert(key, value);
  Q_EMIT configurationChanged();
}

/**
 * @brief Delivers the async verdict. reportOpenFinished() latches, so a second call is a no-op
 *        and a test can assert the verdict arrived exactly once.
 */
void FakeDriver::finishAsyncOpen()
{
  const bool ok = m_outcome == Outcome::AsyncOk;
  m_connecting  = false;
  m_open        = ok;
  reportOpenFinished(ok, ok ? QString() : QStringLiteral("scripted asynchronous failure"));
}

}  // namespace Test
