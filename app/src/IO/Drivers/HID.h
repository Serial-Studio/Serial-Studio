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

#pragma once

#include <hidapi.h>

#include <atomic>
#include <QList>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QTimer>

#include "IO/HAL_Driver.h"

namespace IO {
namespace Drivers {

/**
 * @brief HAL driver for Human Interface Devices via hidapi.
 */
class HID : public HAL_Driver {
  // clang-format off
  Q_OBJECT

  Q_PROPERTY(QStringList deviceList
             READ  deviceList
             NOTIFY deviceListChanged)
  Q_PROPERTY(int deviceIndex
             READ  deviceIndex
             WRITE setDeviceIndex
             NOTIFY deviceIndexChanged)
  Q_PROPERTY(QString usagePage
             READ  usagePage
             NOTIFY deviceInfoChanged)
  Q_PROPERTY(QString usage
             READ  usage
             NOTIFY deviceInfoChanged)
  // clang-format on

signals:
  void deviceListChanged();
  void deviceIndexChanged();
  void deviceInfoChanged();

public:
  explicit HID();
  ~HID();

  HID(HID&&)                 = delete;
  HID(const HID&)            = delete;
  HID& operator=(HID&&)      = delete;
  HID& operator=(const HID&) = delete;

  void close() override;

  [[nodiscard]] bool isOpen() const noexcept override;
  [[nodiscard]] bool isReadable() const noexcept override;
  [[nodiscard]] bool isWritable() const noexcept override;
  [[nodiscard]] bool configurationOk() const noexcept override;
  [[nodiscard]] qint64 write(const QByteArray& data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;
  [[nodiscard]] QJsonObject deviceIdentifier() const override;
  [[nodiscard]] QList<IO::DriverProperty> driverProperties() const override;
  bool selectByIdentifier(const QJsonObject& id) override;

  [[nodiscard]] QStringList deviceList() const;
  [[nodiscard]] int deviceIndex() const;
  [[nodiscard]] QString usagePage() const;
  [[nodiscard]] QString usage() const;

public slots:
  void setDriverProperty(const QString& key, const QVariant& value) override;
  void setDeviceIndex(const int index);

private slots:
  void onReadError();
  void enumerateDevices();

private:
  /**
   * @brief Internal description of one enumerated HID device.
   */
  struct DeviceEntry {
    QString label;
    QString path;
    uint16_t usagePage;
    uint16_t usage;
  };

  void cleanupDevice();
  void readLoop();

  [[nodiscard]] QList<DeviceEntry> collectHidDeviceEntries() const;
  void composeHidDeviceLists(const QList<DeviceEntry>& entries,
                             QStringList& labels,
                             QList<QString>& paths,
                             QList<uint16_t>& usagePages,
                             QList<uint16_t>& usages) const;
  [[nodiscard]] int restoreHidSelectionIndex(const QString& prevPath);
  void updateHidUsageCache();

  hid_device* m_handle;
  hid_device_info* m_deviceInfoList;

  QThread m_readThread;
  QTimer m_enumTimer;
  QSettings m_settings;

  std::atomic<bool> m_running;

  int m_deviceIndex;
  QString m_usagePage;
  QString m_usage;

  QStringList m_deviceLabels;
  QList<QString> m_devicePaths;
  QList<uint16_t> m_deviceUsagePages;
  QList<uint16_t> m_deviceUsages;
};

}  // namespace Drivers
}  // namespace IO
