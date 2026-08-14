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

#include "IO/PipelineHost.h"
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
  , m_deviceId(deviceId)
  , m_pipeline(PipelineHost::instance())
  , m_frameConfig(config)
  , m_driver(std::move(driver))
{
  SS_ASSERT_LOG(m_driver);
  SS_ASSERT_LOG(deviceId >= 0);

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
 * @brief Writes @p data to the underlying driver. A driver dialing asynchronously still accepts
 *        writes: QTcpSocket buffers and flushes them on connect, so a control script's
 *        io.connect() + writeData() sequence works without waiting out the dial.
 */
qint64 IO::DeviceManager::write(const QByteArray& data)
{
  SS_ASSERT_LOG(!data.isEmpty());
  SS_ASSERT_LOG(m_driver);

  if (!m_driver || (!m_driver->isOpen() && !m_driver->isConnecting()))
    return -1;

  return m_driver->write(data);
}

//--------------------------------------------------------------------------------------------------
// Connection lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens the device in the given @p mode and ensures the FrameReader is running. The
 *        driver's verdict is returned rather than discarded: for a driver that dials
 *        asynchronously it means the attempt started, not that the link is up.
 */
bool IO::DeviceManager::open(QIODevice::OpenMode mode)
{
  SS_ASSERT_LOG(m_driver);
  SS_ASSERT_LOG(mode != QIODevice::NotOpen);

  if (!m_driver)
    return false;

  if (m_frameReader.isNull())
    startFrameReader(m_frameConfig);

  return m_driver->open(mode);
}

/**
 * @brief Closes the device and stops the FrameReader.
 */
void IO::DeviceManager::close()
{
  SS_ASSERT_LOG(m_driver);

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
  SS_ASSERT_LOG(m_driver);

  m_frameConfig = config;
  killFrameReader();
  startFrameReader(config);
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

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
 * @brief Creates a new FrameReader, configures it while still on this thread, then hands it to
 *        the PipelineHost: configuration runs before the move (no live connections yet), and
 *        after adoption the reader is only reached through queued driver chunks or recreation.
 */
void IO::DeviceManager::startFrameReader(const FrameConfig& config)
{
  SS_ASSERT_LOG(m_driver);
  SS_ASSERT_LOG(m_deviceId >= 0);

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
  m_pipeline.registerFrameReader(m_deviceId, m_frameReader);
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
