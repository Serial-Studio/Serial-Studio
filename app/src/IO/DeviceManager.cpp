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
 * @param deviceId           Opaque identifier for this device (matches ProjectModel sourceId).
 * @param driver             Unique ownership of the HAL driver instance.
 * @param config             Initial FrameReader configuration.
 * @param threadedExtraction When true the FrameReader runs in m_workerThread.
 * @param parent             Optional QObject parent.
 */
IO::DeviceManager::DeviceManager(int deviceId,
                                 std::unique_ptr<HAL_Driver> driver,
                                 const FrameConfig& config,
                                 bool threadedExtraction,
                                 QObject* parent)
  : QObject(parent)
  , m_deviceId(deviceId)
  , m_threadedExtraction(threadedExtraction)
  , m_frameConfig(config)
  , m_driver(std::move(driver))
{
  m_frameScratch.reserve(4096);

  connect(
    m_driver.get(), &IO::HAL_Driver::dataReceived, this, &IO::DeviceManager::onRawDataReceived);

  startFrameReader(config);
}

/**
 * @brief Destructs the DeviceManager, closing the driver and stopping the worker thread.
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
  if (m_driver)
    m_driver->close();

  killFrameReader();
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
 * All setters are called before moveToThread() to satisfy the FrameReader's
 * immutability contract: configuration must be frozen before the reader
 * is handed to its worker thread.
 *
 * @param config FrameReader parameters to apply.
 */
void IO::DeviceManager::startFrameReader(const FrameConfig& config)
{
  if (!m_driver)
    return;

  killFrameReader();

  m_frameReader = new FrameReader();

  m_frameReader->setChecksum(config.checksumAlgorithm);
  m_frameReader->setStartSequence(config.startSequence);
  m_frameReader->setFinishSequence(config.finishSequence);
  m_frameReader->setOperationMode(config.operationMode);
  m_frameReader->setFrameDetectionMode(config.frameDetection);

  if (m_threadedExtraction)
    m_frameReader->moveToThread(&m_workerThread);

  connect(m_driver.get(),
          &IO::HAL_Driver::dataReceived,
          m_frameReader,
          &IO::FrameReader::processData,
          Qt::QueuedConnection);

  connect(m_frameReader,
          &IO::FrameReader::readyRead,
          this,
          &IO::DeviceManager::onReadyRead,
          Qt::QueuedConnection);

  if (m_threadedExtraction && !m_workerThread.isRunning())
    m_workerThread.start();
}

/**
 * @brief Stops and destroys the FrameReader and its worker thread.
 */
void IO::DeviceManager::killFrameReader()
{
  if (!m_frameReader.isNull()) {
    if (m_driver)
      disconnect(m_driver.get(), &IO::HAL_Driver::dataReceived, m_frameReader, nullptr);

    if (m_threadedExtraction && m_workerThread.isRunning()) {
      QMetaObject::invokeMethod(m_frameReader, &QObject::deleteLater, Qt::BlockingQueuedConnection);
      m_workerThread.quit();
      m_workerThread.wait();
      m_frameReader.clear();
    } else {
      m_frameReader->deleteLater();
      m_frameReader.clear();
    }
  }

  else if (m_workerThread.isRunning()) {
    m_workerThread.quit();
    m_workerThread.wait();
  }
}
