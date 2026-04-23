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

IO::DeviceManager::~DeviceManager()
{
  close();
}

//--------------------------------------------------------------------------------------------------
// Status queries
//--------------------------------------------------------------------------------------------------

int IO::DeviceManager::deviceId() const noexcept
{
  return m_deviceId;
}

bool IO::DeviceManager::isOpen() const
{
  return m_driver && m_driver->isOpen();
}

bool IO::DeviceManager::isWritable() const
{
  return m_driver && m_driver->isOpen() && m_driver->isWritable();
}

IO::HAL_Driver* IO::DeviceManager::driver() const noexcept
{
  return m_driver.get();
}

//--------------------------------------------------------------------------------------------------
// Data transmission
//--------------------------------------------------------------------------------------------------

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

void IO::DeviceManager::close()
{
  Q_ASSERT(m_driver);

  if (m_driver)
    m_driver->close();

  killFrameReader();
  Q_ASSERT(m_frameReader.isNull());
}

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

void IO::DeviceManager::onReadyRead()
{
  Q_ASSERT(m_driver);

  if (!m_frameReader)
    return;

  auto& queue = m_frameReader->queue();
  while (queue.try_dequeue(m_frameScratch))
    Q_EMIT frameReady(m_deviceId, m_frameScratch);
}

void IO::DeviceManager::onRawDataReceived(const IO::CapturedDataPtr& data)
{
  Q_EMIT rawDataReceived(m_deviceId, data);
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

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

void IO::DeviceManager::killFrameReader()
{
  if (m_frameReader.isNull())
    return;

  if (m_driver)
    disconnect(m_driver.get(), &IO::HAL_Driver::dataReceived, m_frameReader, nullptr);

  m_frameReader->deleteLater();
  m_frameReader.clear();
}
