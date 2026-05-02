/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "IO/Drivers/HID.h"

#include <QJsonObject>
#include <QMetaObject>
#include <QSet>
#include <QTimer>

#include "IO/ConnectionManager.h"
#include "Misc/Utilities.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

constexpr int kReadBufSize    = 65;
constexpr int kReadTimeoutMs  = 100;
constexpr int kEnumIntervalMs = 2000;

//--------------------------------------------------------------------------------------------------
// Constructor / destructor / singleton
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the HID driver and starts hotplug enumeration.
 */
IO::Drivers::HID::HID()
  : m_handle(nullptr), m_deviceInfoList(nullptr), m_running(false), m_deviceIndex(0)
{
  // Initialize hidapi and restore last selection
  hid_init();
  m_deviceIndex = m_settings.value("HID/deviceIndex", 0).toInt();
  enumerateDevices();

  // Start periodic enumeration for hotplug detection
  m_enumTimer.setInterval(kEnumIntervalMs);
  connect(&m_enumTimer, &QTimer::timeout, this, &HID::enumerateDevices);
  m_enumTimer.start();

  connect(&m_readThread, &QThread::started, this, &HID::readLoop, Qt::DirectConnection);
}

/**
 * @brief Stops the read thread, closes the device handle, and clears usage info.
 */
void IO::Drivers::HID::cleanupDevice()
{
  // Stop the read thread and close the device handle
  m_running = false;

  if (m_readThread.isRunning()) {
    m_readThread.quit();
    m_readThread.wait();
  }

  if (m_handle) {
    hid_close(m_handle);
    m_handle = nullptr;
  }

  m_usagePage.clear();
  m_usage.clear();
}

/**
 * @brief Closes the device and tears down the hidapi enumeration cache.
 */
IO::Drivers::HID::~HID()
{
  cleanupDevice();

  hid_free_enumeration(m_deviceInfoList);
  m_deviceInfoList = nullptr;

  hid_exit();
}

//--------------------------------------------------------------------------------------------------
// HAL_Driver interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Closes the HID device and notifies observers.
 */
void IO::Drivers::HID::close()
{
  cleanupDevice();
  Q_EMIT deviceInfoChanged();
}

/**
 * @brief Returns true when an HID device is currently open.
 */
bool IO::Drivers::HID::isOpen() const noexcept
{
  return m_handle != nullptr;
}

/**
 * @brief Returns true when the HID device can be read.
 */
bool IO::Drivers::HID::isReadable() const noexcept
{
  return isOpen();
}

/**
 * @brief Returns true when the HID device can be written.
 */
bool IO::Drivers::HID::isWritable() const noexcept
{
  return isOpen();
}

/**
 * @brief Returns true when a valid (non-placeholder) device is selected.
 */
bool IO::Drivers::HID::configurationOk() const noexcept
{
  return m_deviceIndex > 0 && m_deviceIndex < m_devicePaths.size();
}

/**
 * @brief Sends @p data to the device as an interrupt OUT / feature report.
 */
qint64 IO::Drivers::HID::write(const QByteArray& data)
{
  if (!isOpen())
    return -1;

  const auto* buf = reinterpret_cast<const unsigned char*>(data.constData());
  const int n     = hid_write(m_handle, buf, static_cast<size_t>(data.size()));
  return static_cast<qint64>(n);
}

/**
 * @brief Opens the selected HID device and starts the read thread.
 */
bool IO::Drivers::HID::open(const QIODevice::OpenMode mode)
{
  (void)mode;

  if (!configurationOk())
    return false;

  // Open the HID device by path
  const QString path = m_devicePaths.at(m_deviceIndex);

  m_handle = hid_open_path(path.toUtf8().constData());
  if (!m_handle) {
    const wchar_t* err = hid_error(nullptr);
    const QString detail =
      (err && err[0] != L'\0') ? QString::fromWCharArray(err) : tr("Unknown error");
    QString info = detail;
#ifdef Q_OS_LINUX
    info += tr("\n\nCheck that your user is in the 'plugdev' group "
               "or that a udev rule grants access to this device.");
#endif
    Misc::Utilities::showMessageBox(
      tr("Failed to open \"%1\"").arg(m_deviceLabels.value(m_deviceIndex)),
      info,
      QMessageBox::Warning);
    return false;
  }

  hid_set_nonblocking(m_handle, 0);

  // Load cached usage info for this device
  const uint16_t up = m_deviceUsagePages.value(m_deviceIndex, 0);
  const uint16_t u  = m_deviceUsages.value(m_deviceIndex, 0);
  m_usagePage       = up
                      ? QStringLiteral("0x") + QString::number(up, 16).rightJustified(4, QLatin1Char('0'))
                      : QString();
  m_usage = u ? QStringLiteral("0x") + QString::number(u, 16).rightJustified(4, QLatin1Char('0'))
              : QString();
  Q_EMIT deviceInfoChanged();

  // Start the interrupt read loop on a worker thread
  m_running = true;
  m_readThread.start();

  return true;
}

//--------------------------------------------------------------------------------------------------
// Property accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the device list with a placeholder entry at index 0.
 */
QStringList IO::Drivers::HID::deviceList() const
{
  return m_deviceLabels;
}

/**
 * @brief Returns the index of the currently selected device.
 */
int IO::Drivers::HID::deviceIndex() const
{
  return m_deviceIndex;
}

/**
 * @brief Returns the HID usage page of the currently open device.
 */
QString IO::Drivers::HID::usagePage() const
{
  return m_usagePage;
}

/**
 * @brief Returns the HID usage of the currently open device.
 */
QString IO::Drivers::HID::usage() const
{
  return m_usage;
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Selects the device at @p index in the device list.
 */
void IO::Drivers::HID::setDeviceIndex(const int index)
{
  if (m_deviceIndex != index) {
    // Ensure device list is populated so the index is meaningful
    if (m_devicePaths.isEmpty())
      enumerateDevices();

    if (index > 0 && index >= m_devicePaths.size())
      return;

    // Persist selection and update cached usage info
    m_deviceIndex = index;
    m_settings.setValue("HID/deviceIndex", index);

    const uint16_t up = m_deviceUsagePages.value(index, 0);
    const uint16_t u  = m_deviceUsages.value(index, 0);
    m_usagePage =
      up ? QStringLiteral("0x") + QString::number(up, 16).rightJustified(4, QLatin1Char('0'))
         : QString();
    m_usage = u ? QStringLiteral("0x") + QString::number(u, 16).rightJustified(4, QLatin1Char('0'))
                : QString();

    Q_EMIT deviceIndexChanged();
    Q_EMIT deviceInfoChanged();
    Q_EMIT configurationChanged();
  }
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles a fatal read error from the read thread.
 */
void IO::Drivers::HID::onReadError()
{
  // Already disconnected from a prior error
  if (!isOpen())
    return;

  ConnectionManager::instance().disconnectDevice(this);
  Misc::Utilities::showMessageBox(tr("HID Device Error"),
                                  tr("The HID device was disconnected or "
                                     "encountered a fatal read error."));
}

/**
 * @brief Re-enumerates all connected HID devices.
 */
void IO::Drivers::HID::enumerateDevices()
{
  hid_free_enumeration(m_deviceInfoList);
  m_deviceInfoList = hid_enumerate(0x0000, 0x0000);

  QList<DeviceEntry> entries = collectHidDeviceEntries();
  std::sort(entries.begin(), entries.end(), [](const DeviceEntry& a, const DeviceEntry& b) {
    return a.label < b.label;
  });

  QStringList newLabels;
  QList<QString> newPaths;
  QList<uint16_t> newUsagePages;
  QList<uint16_t> newUsages;
  composeHidDeviceLists(entries, newLabels, newPaths, newUsagePages, newUsages);

  if (newLabels == m_deviceLabels)
    return;

  const QString prevPath = (m_deviceIndex > 0 && m_deviceIndex < m_devicePaths.size())
                           ? m_devicePaths.at(m_deviceIndex)
                           : QString();

  m_deviceLabels     = newLabels;
  m_devicePaths      = newPaths;
  m_deviceUsagePages = newUsagePages;
  m_deviceUsages     = newUsages;

  const int newIndex = restoreHidSelectionIndex(prevPath);
  if (newIndex != m_deviceIndex) {
    m_deviceIndex = newIndex;
    Q_EMIT deviceIndexChanged();
  }

  updateHidUsageCache();

  Q_EMIT deviceListChanged();
  Q_EMIT deviceInfoChanged();
  Q_EMIT configurationChanged();
}

/**
 * @brief Walks the hidapi device-info linked list and merges duplicates by VID:PID:serial.
 */
QList<IO::Drivers::HID::DeviceEntry> IO::Drivers::HID::collectHidDeviceEntries() const
{
  QList<DeviceEntry> entries;
  QMap<QString, int> seen;

  for (hid_device_info* dev = m_deviceInfoList; dev != nullptr; dev = dev->next) {
    if (dev->vendor_id == 0 && dev->product_id == 0)
      continue;

    const QString serial = (dev->serial_number && dev->serial_number[0] != L'\0')
                           ? QString::fromWCharArray(dev->serial_number)
                           : QString();

    const QString key = QString("%1:%2:%3")
                          .arg(dev->vendor_id, 4, 16, QLatin1Char('0'))
                          .arg(dev->product_id, 4, 16, QLatin1Char('0'))
                          .arg(serial);

    auto it = seen.find(key);
    if (it != seen.end()) {
      auto& existing = entries[it.value()];
      if (existing.usagePage == 0 && dev->usage_page != 0) {
        existing.usagePage = dev->usage_page;
        existing.usage     = dev->usage;
      }

      continue;
    }

    QString name;
    if (dev->product_string && dev->product_string[0] != L'\0')
      name = QString::fromWCharArray(dev->product_string);
    else if (dev->manufacturer_string && dev->manufacturer_string[0] != L'\0')
      name = QString::fromWCharArray(dev->manufacturer_string);

    const QString vidpid = QString("%1:%2")
                             .arg(dev->vendor_id, 4, 16, QLatin1Char('0'))
                             .arg(dev->product_id, 4, 16, QLatin1Char('0'))
                             .toUpper();

    const QString label = name.isEmpty() ? vidpid : (name + " (" + vidpid + ")");

    seen.insert(key, entries.size());
    entries.append({label, QString::fromUtf8(dev->path), dev->usage_page, dev->usage});
  }

  return entries;
}

/**
 * @brief Builds the public lists from sorted entries, prepending a "Select Device" placeholder.
 */
void IO::Drivers::HID::composeHidDeviceLists(const QList<DeviceEntry>& entries,
                                             QStringList& labels,
                                             QList<QString>& paths,
                                             QList<uint16_t>& usagePages,
                                             QList<uint16_t>& usages) const
{
  labels.append(tr("Select Device"));
  paths.append(QString());
  usagePages.append(0);
  usages.append(0);

  for (const auto& e : entries) {
    labels.append(e.label);
    paths.append(e.path);
    usagePages.append(e.usagePage);
    usages.append(e.usage);
  }
}

/**
 * @brief Picks the new device index after a rescan, preserving the prior selection by path.
 */
int IO::Drivers::HID::restoreHidSelectionIndex(const QString& prevPath)
{
  int newIndex = m_deviceIndex;

  if (!prevPath.isEmpty()) {
    const int idx = m_devicePaths.indexOf(prevPath);
    if (idx > 0)
      newIndex = idx;
    else {
      newIndex = 0;
      m_settings.setValue("HID/deviceIndex", 0);
    }
  } else if (newIndex >= m_deviceLabels.size()) {
    newIndex = 0;
    m_settings.setValue("HID/deviceIndex", 0);
  }

  return newIndex;
}

/**
 * @brief Refreshes the formatted usage-page/usage strings for the current selection.
 */
void IO::Drivers::HID::updateHidUsageCache()
{
  const uint16_t up = m_deviceUsagePages.value(m_deviceIndex, 0);
  const uint16_t u  = m_deviceUsages.value(m_deviceIndex, 0);
  m_usagePage       = up
                      ? QStringLiteral("0x") + QString::number(up, 16).rightJustified(4, QLatin1Char('0'))
                      : QString();
  m_usage = u ? QStringLiteral("0x") + QString::number(u, 16).rightJustified(4, QLatin1Char('0'))
              : QString();
}

//--------------------------------------------------------------------------------------------------
// Stable device identification
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns VID, PID, and serial number of the currently selected HID device.
 */
QJsonObject IO::Drivers::HID::deviceIdentifier() const
{
  if (m_deviceIndex < 1 || m_deviceIndex >= m_devicePaths.size())
    return {};

  const auto& selectedPath = m_devicePaths.at(m_deviceIndex);
  if (selectedPath.isEmpty())
    return {};

  for (hid_device_info* dev = m_deviceInfoList; dev != nullptr; dev = dev->next) {
    if (QString::fromUtf8(dev->path) != selectedPath)
      continue;

    QJsonObject id;
    id.insert(QStringLiteral("vid"),
              QString::number(dev->vendor_id, 16).rightJustified(4, '0').toUpper());
    id.insert(QStringLiteral("pid"),
              QString::number(dev->product_id, 16).rightJustified(4, '0').toUpper());

    if (dev->serial_number && dev->serial_number[0] != L'\0')
      id.insert(QStringLiteral("serial"), QString::fromWCharArray(dev->serial_number));

    return id;
  }

  return {};
}

/**
 * @brief Tries to find and select an HID device matching a previously saved VID/PID/serial.
 */
bool IO::Drivers::HID::selectByIdentifier(const QJsonObject& id)
{
  if (id.isEmpty())
    return false;

  // Ensure device list is populated for matching
  if (m_devicePaths.isEmpty())
    enumerateDevices();

  const auto savedVid = id.value(QStringLiteral("vid")).toString();
  const auto savedPid = id.value(QStringLiteral("pid")).toString();
  const auto savedSer = id.value(QStringLiteral("serial")).toString();

  if (savedVid.isEmpty() || savedPid.isEmpty())
    return false;

  for (hid_device_info* dev = m_deviceInfoList; dev != nullptr; dev = dev->next) {
    const auto vid = QString::number(dev->vendor_id, 16).rightJustified(4, '0').toUpper();
    const auto pid = QString::number(dev->product_id, 16).rightJustified(4, '0').toUpper();

    if (vid != savedVid || pid != savedPid)
      continue;

    if (!savedSer.isEmpty()) {
      const QString serial = (dev->serial_number && dev->serial_number[0] != L'\0')
                             ? QString::fromWCharArray(dev->serial_number)
                             : QString();
      if (serial != savedSer)
        continue;
    }

    // Look up the device's display-list index
    const auto path = QString::fromUtf8(dev->path);
    const int idx   = m_devicePaths.indexOf(path);
    if (idx > 0) {
      setDeviceIndex(idx);
      return true;
    }
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Driver property model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the HID configuration as a flat list of editable properties.
 */
QList<IO::DriverProperty> IO::Drivers::HID::driverProperties() const
{
  QList<IO::DriverProperty> props;

  IO::DriverProperty dev;
  dev.key     = QStringLiteral("deviceIndex");
  dev.label   = tr("HID Device");
  dev.type    = IO::DriverProperty::ComboBox;
  dev.value   = m_deviceIndex;
  dev.options = m_deviceLabels;
  props.append(dev);

  return props;
}

/**
 * @brief Applies a single HID configuration change by key.
 */
void IO::Drivers::HID::setDriverProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("deviceIndex"))
    setDeviceIndex(value.toInt());
}

//--------------------------------------------------------------------------------------------------
// Private: read loop (runs on m_readThread)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Interrupt read loop executed on m_readThread.
 */
void IO::Drivers::HID::readLoop()
{
  uint8_t buf[kReadBufSize];

  while (m_running.load()) {
    const int n = hid_read_timeout(m_handle, buf, kReadBufSize, kReadTimeoutMs);

    if (n > 0)
      publishReceivedData(QByteArray(reinterpret_cast<char*>(buf), n));

    else if (n < 0) {
      QMetaObject::invokeMethod(this, "onReadError", Qt::QueuedConnection);
      break;
    }
  }
}
