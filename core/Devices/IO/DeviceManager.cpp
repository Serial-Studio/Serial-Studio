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

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs a DeviceManager owning a driver and attaches it to the pipeline for the given
 *        device.
 */
IO::DeviceManager::DeviceManager(int deviceId,
                                 std::unique_ptr<HAL_Driver> driver,
                                 const FrameConfig& config,
                                 IIngestBinder& binder,
                                 QObject* parent)
  : QObject(parent)
  , m_deviceId(deviceId)
  , m_attached(false)
  , m_binder(binder)
  , m_frameConfig(config)
  , m_driver(std::move(driver))
{
  SS_ASSERT_LOG(m_driver);
  SS_ASSERT_LOG(deviceId >= 0);

  connect(
    m_driver.get(), &IO::HAL_Driver::dataReceived, this, &IO::DeviceManager::onRawDataReceived);
  connect(m_driver.get(),
          &IO::HAL_Driver::consoleDataReceived,
          this,
          &IO::DeviceManager::onConsoleDataReceived);
  connect(m_driver.get(), &IO::HAL_Driver::dataSent, this, &IO::DeviceManager::onRawDataSent);

  attachToPipeline(config);
}

/**
 * @brief Closes the device and detaches it from the pipeline.
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
 * @brief Returns true while the pipeline holds a reader for this device.
 */
bool IO::DeviceManager::isAttached() const noexcept
{
  return m_attached;
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
 * @brief Opens the device in the given @p mode and ensures it is attached to the pipeline. The
 *        driver's verdict is returned rather than discarded: for a driver that dials
 *        asynchronously it means the attempt started, not that the link is up.
 */
bool IO::DeviceManager::open(QIODevice::OpenMode mode)
{
  SS_ASSERT_LOG(m_driver);
  SS_ASSERT_LOG(mode != QIODevice::NotOpen);

  if (!m_driver)
    return false;

  if (!m_attached)
    attachToPipeline(m_frameConfig);

  return m_driver->open(mode);
}

/**
 * @brief Closes the device and retires its reader.
 */
void IO::DeviceManager::close()
{
  SS_ASSERT_LOG(m_driver);

  if (m_driver)
    m_driver->close();

  detachFromPipeline();
  SS_ASSERT_LOG(!m_attached);
}

/**
 * @brief Recreates the reader with a new frame configuration (recreate, never lock: the reader is
 *        single-producer); a detached device is attached with it.
 */
void IO::DeviceManager::reconfigure(const FrameConfig& config)
{
  SS_ASSERT_LOG(m_driver);

  m_frameConfig = config;
  if (m_attached)
    m_binder.reconfigure(m_deviceId, config);
  else
    attachToPipeline(config);
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Re-emits bytes the driver wrote on its own, tagged with the device identifier, so the
 *        console echoes them; command rate on the GUI thread, never on the frame path.
 */
void IO::DeviceManager::onRawDataSent(const QByteArray& data)
{
  Q_EMIT rawDataSent(m_deviceId, data);
}

/**
 * @brief Re-emits raw bytes from the driver tagged with the device identifier.
 */
void IO::DeviceManager::onRawDataReceived(const IO::CapturedDataPtr& data)
{
  Q_EMIT rawDataReceived(m_deviceId, data);
}

/**
 * @brief Re-emits a driver's terminal-only bytes tagged with the device identifier.
 */
void IO::DeviceManager::onConsoleDataReceived(const IO::CapturedDataPtr& data)
{
  Q_EMIT consoleDataReceived(m_deviceId, data);
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Hands the driver's byte stream to the pipeline under @p config; the binder creates and
 *        adopts the reader, replacing any earlier one for this device.
 */
void IO::DeviceManager::attachToPipeline(const FrameConfig& config)
{
  SS_ASSERT_LOG(m_driver);
  SS_ASSERT_LOG(m_deviceId >= 0);

  if (!m_driver)
    return;

  m_binder.attach(m_deviceId, m_driver.get(), config);
  m_attached = true;
}

/**
 * @brief Retires this device's reader; idempotent.
 */
void IO::DeviceManager::detachFromPipeline()
{
  if (!m_attached)
    return;

  m_binder.detach(m_deviceId);
  m_attached = false;
}
