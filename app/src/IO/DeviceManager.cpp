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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "IO/DeviceManager.h"

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a DeviceManager that owns the given driver.
 *
 * Takes ownership of @p driver, stores the initial @p config, and connects the
 * driver's dataReceived() signal so raw bytes are forwarded to consumers.
 *
 * The FrameReader is created immediately and will be recreated each time
 * open() is called after a close().
 *
 * @param deviceId Opaque identifier for this device (matches ProjectModel sourceId).
 * @param driver   Unique ownership of the HAL driver instance.
 * @param config   Initial FrameReader configuration.
 * @param parent   Optional QObject parent.
 */
IO::DeviceManager::DeviceManager(int deviceId,
                                 std::unique_ptr<HAL_Driver> driver,
                                 const FrameConfig& config,
                                 QObject* parent)
  : QObject(parent), m_deviceId(deviceId), m_frameConfig(config), m_driver(std::move(driver))
{
  Q_ASSERT(m_driver);
  Q_ASSERT(deviceId >= 0);

  m_frameScratch.reserve(4096);

  connect(
    m_driver.get(), &IO::HAL_Driver::dataReceived, this, &IO::DeviceManager::onRawDataReceived);

  startFrameReader(config);
}

/**
 * @brief Destructs the DeviceManager, closing the driver.
 */
IO::DeviceManager::~DeviceManager()
{
  close();
}

//--------------------------------------------------------------------------------------------------
// Status queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the device identifier passed at construction.
 */
int IO::DeviceManager::deviceId() const noexcept
{
  return m_deviceId;
}

/**
 * @brief Returns true if the driver is currently open.
 */
bool IO::DeviceManager::isOpen() const
{
  return m_driver && m_driver->isOpen();
}

/**
 * @brief Returns true if the driver is open and writable.
 */
bool IO::DeviceManager::isWritable() const
{
  return m_driver && m_driver->isOpen() && m_driver->isWritable();
}

/**
 * @brief Returns a raw pointer to the owned driver (never null after construction).
 */
IO::HAL_Driver* IO::DeviceManager::driver() const noexcept
{
  return m_driver.get();
}

//--------------------------------------------------------------------------------------------------
// Data transmission
//--------------------------------------------------------------------------------------------------

/**
 * @brief Writes @p data to the driver.
 * @param data Bytes to transmit.
 * @return Number of bytes written, or -1 on failure.
 */
qint64 IO::DeviceManager::write(const QByteArray& data)
{
  Q_ASSERT(!data.isEmpty());
  Q_ASSERT(m_driver);

  if (!m_driver || !m_driver->isOpen())
    return -1;

  return m_driver->write(data);
}

//--------------------------------------------------------------------------------------------------
// Connection lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens the driver in the given mode and restarts the FrameReader if needed.
 *
 * The FrameReader is destroyed by close() to discard stale buffered data.
 * open() recreates it from the stored config so the device is fully ready
 * to extract frames on the next connection without requiring a rebuildDevices().
 *
 * @param mode Open mode (default ReadWrite).
 */
void IO::DeviceManager::open(QIODevice::OpenMode mode)
{
  Q_ASSERT(m_driver);
  Q_ASSERT(mode != QIODevice::NotOpen);

  if (!m_driver)
    return;

  if (m_frameReader.isNull())
    startFrameReader(m_frameConfig);

  (void)m_driver->open(mode);
}

/**
 * @brief Closes the driver and stops the FrameReader.
 */
void IO::DeviceManager::close()
{
  Q_ASSERT(m_driver);

  if (m_driver)
    m_driver->close();

  killFrameReader();
  Q_ASSERT(m_frameReader.isNull());
}

/**
 * @brief Replaces the FrameReader configuration without closing the driver.
 *
 * Destroys the current FrameReader (if any) and starts a new one configured
 * with @p config. Useful for applying updated delimiters or checksum settings
 * while keeping the driver connection alive.
 *
 * @param config New FrameReader parameters.
 */
void IO::DeviceManager::reconfigure(const FrameConfig& config)
{
  Q_ASSERT(m_driver);

  m_frameConfig = config;
  killFrameReader();
  startFrameReader(config);
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Dequeues all available frames from the FrameReader and emits frameReady().
 */
void IO::DeviceManager::onReadyRead()
{
  Q_ASSERT(m_driver);

  if (!m_frameReader)
    return;

  auto& queue = m_frameReader->queue();
  while (queue.try_dequeue(m_frameScratch))
    Q_EMIT frameReady(m_deviceId, m_frameScratch);
}

/**
 * @brief Forwards raw driver data to consumers via rawDataReceived().
 * @param data Incoming bytes from the driver.
 */
void IO::DeviceManager::onRawDataReceived(const IO::ByteArrayPtr& data)
{
  Q_EMIT rawDataReceived(m_deviceId, data);
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates and starts a new FrameReader configured with @p config.
 *
 * Uses AutoConnection so that Qt picks DirectConnection when sender and
 * receiver share a thread (zero-copy) and QueuedConnection when they
 * don't (e.g. HID/USB drivers that emit from their own read threads).
 *
 * @param config FrameReader parameters to apply.
 */
void IO::DeviceManager::startFrameReader(const FrameConfig& config)
{
  Q_ASSERT(m_driver);
  Q_ASSERT(m_deviceId >= 0);

  // Bail out if the driver has been destroyed
  if (!m_driver)
    return;

  // Destroy any existing FrameReader and create a new one
  killFrameReader();

  m_frameReader = new FrameReader();

  m_frameReader->setChecksum(config.checksumAlgorithm);
  m_frameReader->setStartSequence(config.startSequence);
  m_frameReader->setFinishSequence(config.finishSequence);
  m_frameReader->setOperationMode(config.operationMode);
  m_frameReader->setFrameDetectionMode(config.frameDetection);

  connect(
    m_driver.get(), &IO::HAL_Driver::dataReceived, m_frameReader, &IO::FrameReader::processData);

  connect(m_frameReader, &IO::FrameReader::readyRead, this, &IO::DeviceManager::onReadyRead);
}

/**
 * @brief Stops and destroys the FrameReader.
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
