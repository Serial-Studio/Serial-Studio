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
 * @brief Constructs a DeviceManager owning a driver and FrameReader for the given device.
 */
IO::DeviceManager::DeviceManager(int deviceId,
                                 std::unique_ptr<HAL_Driver> driver,
                                 const FrameConfig& config,
                                 QObject* parent)
  : QObject(parent), m_deviceId(deviceId), m_frameConfig(config), m_driver(std::move(driver))
{
  Q_ASSERT(m_driver);
  Q_ASSERT(deviceId >= 0);

  connect(
    m_driver.get(), &IO::HAL_Driver::dataReceived, this, &IO::DeviceManager::onRawDataReceived);

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
 * @brief Opens the device in the given @p mode and ensures the FrameReader is running.
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
 * @brief Closes the device and stops the FrameReader.
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
 * @brief Recreates the FrameReader with a new frame configuration.
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
 * @brief Drains the FrameReader queue and forwards each frame as frameReady().
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
 * @brief Re-emits raw bytes from the driver tagged with the device identifier.
 */
void IO::DeviceManager::onRawDataReceived(const IO::CapturedDataPtr& data)
{
  Q_EMIT rawDataReceived(m_deviceId, data);
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Creates a new FrameReader and wires it to the driver's data signal.
 */
void IO::DeviceManager::startFrameReader(const FrameConfig& config)
{
  Q_ASSERT(m_driver);
  Q_ASSERT(m_deviceId >= 0);

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
