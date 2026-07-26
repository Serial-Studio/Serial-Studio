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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "IO/DeviceManager.h"

#include "IO/ConnectionFlows.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a DeviceManager owning a driver and FrameReader for the given device.
 */
IO::DeviceManager::DeviceManager(int deviceId,
                                 std::unique_ptr<HAL_Driver> driver,
                                 const FrameConfig& config,
                                 QObject* parent)
  : QObject(parent)
  , m_linkUp(false)
  , m_opening(false)
  , m_deviceId(deviceId)
  , m_linkEstablished(false)
  , m_frameConfig(config)
  , m_driver(std::move(driver))
  , m_runner(this)
{
  SS_ASSERT_LOG(m_driver != nullptr);
  SS_ASSERT_LOG(deviceId >= 0);

  connect(
    m_driver.get(), &IO::HAL_Driver::dataReceived, this, &IO::DeviceManager::onRawDataReceived);

  connect(
    m_driver.get(), &IO::HAL_Driver::openFinished, this, &IO::DeviceManager::onDriverOpenFinished);

  connect(
    m_driver.get(), &IO::HAL_Driver::linkDropped, this, &IO::DeviceManager::onDriverLinkDropped);

  connect(&m_runner, &Async::TaskRunner::finished, this, &IO::DeviceManager::onFlowFinished);

  startFrameReader(config);
}

/**
 * @brief Closes the device and tears down its FrameReader.
 */
IO::DeviceManager::~DeviceManager()
{
  close();
}

//--------------------------------------------------------------------------------------------------
// Status queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the device identifier.
 */
int IO::DeviceManager::deviceId() const noexcept
{
  return m_deviceId;
}

/**
 * @brief Returns true when the underlying driver is open.
 */
bool IO::DeviceManager::isOpen() const
{
  return m_driver && m_driver->isOpen();
}

/**
 * @brief Returns true while an orchestrated open attempt is still in flight.
 */
bool IO::DeviceManager::isOpening() const noexcept
{
  return m_opening;
}

/**
 * @brief Returns true while an orchestration flow owns this device, which stays true for the
 *        life of a supervised link because the supervisor keeps watching it for a drop.
 */
bool IO::DeviceManager::hasActiveFlow() const noexcept
{
  return m_runner.isRunning();
}

/**
 * @brief Returns how many open attempts the flow has made in the sequence it is working on, and
 *        zero once the link is up: a caller reads this to tell a retrying source from a dead one.
 */
int IO::DeviceManager::reconnectAttempt() const
{
  const auto* supervisor = qobject_cast<const IO::SupervisorTask*>(m_runner.root());
  return supervisor ? supervisor->attempt() : 0;
}

/**
 * @brief Returns true when the device is open and writable.
 */
bool IO::DeviceManager::isWritable() const
{
  return m_driver && m_driver->isOpen() && m_driver->isWritable();
}

/**
 * @brief Returns the underlying HAL driver instance.
 */
IO::HAL_Driver* IO::DeviceManager::driver() const noexcept
{
  return m_driver.get();
}

//--------------------------------------------------------------------------------------------------
// Data transmission
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes @p data to the underlying driver.
 */
qint64 IO::DeviceManager::write(const QByteArray& data)
{
  SS_ASSERT(!data.isEmpty(), return -1);

  if (!m_driver || !m_driver->isOpen())
    return -1;

  return m_driver->write(data);
}

//--------------------------------------------------------------------------------------------------
// Connection lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens the device in the given @p mode and ensures the FrameReader is running. A driver
 *        that opted into orchestration is opened by a supervised flow, on a short connect policy
 *        because the owner waits behind it; a drop that follows recovers on the longer schedule.
 *        Every other driver takes the same synchronous call, with its result no longer discarded.
 */
void IO::DeviceManager::open(QIODevice::OpenMode mode)
{
  SS_ASSERT(mode != QIODevice::NotOpen, return);

  if (!m_driver)
    return;

  if (m_frameReader.isNull())
    startFrameReader(m_frameConfig);

  if (!m_driver->supportsAsyncOpen()) {
    const bool ok = m_driver->open(mode);
    Q_EMIT openFinished(
      m_deviceId, ok, ok ? QString() : QStringLiteral("driver reported open failure"));
    return;
  }

  m_opening         = true;
  m_linkEstablished = false;
  m_runner.run(Flows::makeSupervised(m_driver.get(),
                                     Flows::makeOpenFlow(m_driver.get(), mode, m_runner.clock()),
                                     Async::RetryPolicy::initialConnect(),
                                     Async::RetryPolicy::autoReconnect(),
                                     m_runner.clock()));
}

/**
 * @brief Closes the device and stops the FrameReader. The flow is cancelled first so a pending
 *        attempt cannot resurrect the link, or touch the reader, while it is being torn down.
 */
void IO::DeviceManager::close()
{
  m_linkUp          = false;
  m_opening         = false;
  m_linkEstablished = false;
  m_runner.cancel();

  if (m_driver)
    m_driver->close();

  killFrameReader();
  SS_ASSERT_LOG(m_frameReader.isNull());
}

/**
 * @brief Recreates the FrameReader with a new frame configuration.
 */
void IO::DeviceManager::reconfigure(const FrameConfig& config)
{
  SS_ASSERT(m_driver != nullptr, return);

  m_frameConfig = config;
  killFrameReader();
  startFrameReader(config);
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Drains the FrameReader queue and forwards each frame as frameReady().
 */
void IO::DeviceManager::onReadyRead()
{
  SS_ASSERT_LOG(m_driver != nullptr);

  if (!m_frameReader)
    return;

  auto& queue = m_frameReader->queue();
  while (queue.try_dequeue(m_frameScratch))
    Q_EMIT frameReady(m_deviceId, m_frameScratch);
}

/**
 * @brief Re-emits raw bytes from the driver tagged with the device identifier.
 */
void IO::DeviceManager::onRawDataReceived(const IO::CapturedDataPtr& data)
{
  Q_EMIT rawDataReceived(m_deviceId, data);
}

/**
 * @brief Reports a link that came up. A failed attempt inside a running flow is swallowed here
 *        because the flow will retry it; only its final verdict reaches the owner. A link that
 *        came up is remembered, so a later give-up can be told from a connect that never landed.
 */
void IO::DeviceManager::onDriverOpenFinished(bool ok, const QString& reason)
{
  SS_ASSERT_LOG(m_driver != nullptr);
  SS_ASSERT_LOG(ok || !reason.isEmpty());

  if (!ok && m_runner.isRunning())
    return;

  m_opening = false;
  if (ok)
    m_linkEstablished = true;

  Q_EMIT openFinished(m_deviceId, ok, reason);

  if (ok && !m_linkUp) {
    m_linkUp = true;
    Q_EMIT linkStateChanged(m_deviceId);
  }
}

/**
 * @brief Reports the up-to-down edge of a supervised link exactly once per drop, so the UI can
 *        reflect the outage the moment it happens while the silent recovery keeps retrying
 *        without any per-attempt signal.
 */
void IO::DeviceManager::onDriverLinkDropped()
{
  if (!m_linkUp)
    return;

  m_linkUp = false;
  Q_EMIT linkStateChanged(m_deviceId);
}

/**
 * @brief Reports a flow that gave up. Success needs no report (the link already announced
 *        itself) and a cancel is a close the owner asked for, so neither emits. A give-up that
 *        ends the recovery of a link which had been up is also reported as a lost link, which is
 *        what lets the owner tear the source down and name the last reason exactly once.
 */
void IO::DeviceManager::onFlowFinished(Async::Outcome outcome, const Async::StepError& error)
{
  SS_ASSERT_LOG(m_driver != nullptr);

  m_opening = false;
  if (outcome == Async::Outcome::Success || outcome == Async::Outcome::Cancelled)
    return;

  m_linkUp          = false;
  const bool lost   = m_linkEstablished;
  m_linkEstablished = false;

  Q_EMIT openFinished(m_deviceId, false, error.reason);
  if (lost)
    Q_EMIT linkLost(m_deviceId, error.reason);
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates a new FrameReader and wires it to the driver's data signal.
 */
void IO::DeviceManager::startFrameReader(const FrameConfig& config)
{
  SS_ASSERT(m_deviceId >= 0, return);

  if (!m_driver)
    return;

  killFrameReader();

  m_frameReader = new FrameReader();
  m_frameReader->setChecksum(config.checksumAlgorithm);
  m_frameReader->setStartSequences(config.startSequences);
  m_frameReader->setFinishSequences(config.finishSequences);
  m_frameReader->setOperationMode(config.operationMode);
  m_frameReader->setFrameDetectionMode(config.frameDetection);

  connect(
    m_driver.get(), &IO::HAL_Driver::dataReceived, m_frameReader, &IO::FrameReader::processData);
  connect(m_frameReader, &IO::FrameReader::readyRead, this, &IO::DeviceManager::onReadyRead);
}

/**
 * @brief Disconnects and schedules deletion of the current FrameReader.
 */
void IO::DeviceManager::killFrameReader()
{
  if (m_frameReader.isNull())
    return;

  if (m_driver)
    disconnect(m_driver.get(), &IO::HAL_Driver::dataReceived, m_frameReader, nullptr);

  m_frameReader->deleteLater();
  m_frameReader.clear();
}
