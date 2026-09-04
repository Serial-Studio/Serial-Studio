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

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <winsock2.h>
#  include <ws2tcpip.h>
#endif

#include <libusb.h>

#include <QHash>
#include <QList>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>

#include "IO/Drivers/USB/UsbTransferPump.h"
#include "IO/HAL_Driver.h"

namespace IO {
namespace Drivers {

/**
 * @brief HAL driver for raw USB devices via libusb.
 */
class USB : public HAL_Driver {
  // clang-format off
  Q_OBJECT

  Q_PROPERTY(int transferMode
             READ  transferMode
             WRITE setTransferMode
             NOTIFY transferModeChanged)
  Q_PROPERTY(QStringList deviceList
             READ  deviceList
             NOTIFY deviceListChanged)
  Q_PROPERTY(int deviceIndex
             READ  deviceIndex
             WRITE setDeviceIndex
             NOTIFY deviceIndexChanged)
  Q_PROPERTY(QStringList inEndpointList
             READ  inEndpointList
             NOTIFY endpointListChanged)
  Q_PROPERTY(QStringList outEndpointList
             READ  outEndpointList
             NOTIFY endpointListChanged)
  Q_PROPERTY(int inEndpointIndex
             READ  inEndpointIndex
             WRITE setInEndpointIndex
             NOTIFY inEndpointIndexChanged)
  Q_PROPERTY(int outEndpointIndex
             READ  outEndpointIndex
             WRITE setOutEndpointIndex
             NOTIFY outEndpointIndexChanged)
  Q_PROPERTY(int isoPacketSize
             READ  isoPacketSize
             WRITE setIsoPacketSize
             NOTIFY isoPacketSizeChanged)
  Q_PROPERTY(bool advancedModeEnabled
             READ  advancedModeEnabled
             NOTIFY transferModeChanged)
  Q_PROPERTY(bool isoModeEnabled
             READ  isoModeEnabled
             NOTIFY transferModeChanged)
  Q_PROPERTY(bool advancedTransferConsent
             READ  advancedTransferConsent
             NOTIFY advancedTransferConsentChanged)
  // clang-format on

signals:
  void transferModeChanged();
  void advancedTransferConsentChanged();
  void deviceListChanged();
  void deviceIndexChanged();
  void endpointListChanged();
  void inEndpointIndexChanged();
  void outEndpointIndexChanged();
  void isoPacketSizeChanged();
  void controlTransferFinished(bool ok,
                               int bytesTransferred,
                               const QString& responseHex,
                               const QString& message);

public:
  enum class TransferMode {
    BulkStream      = 0,
    AdvancedControl = 1,
    Isochronous     = 2,
  };
  Q_ENUM(TransferMode)

public:
  explicit USB();
  ~USB();

  USB(USB&&)                 = delete;
  USB(const USB&)            = delete;
  USB& operator=(USB&&)      = delete;
  USB& operator=(const USB&) = delete;

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

  [[nodiscard]] int transferMode() const;
  [[nodiscard]] bool advancedModeEnabled() const;
  [[nodiscard]] bool isoModeEnabled() const;

  [[nodiscard]] QStringList deviceList() const;
  [[nodiscard]] int deviceIndex() const;

  [[nodiscard]] QStringList inEndpointList() const;
  [[nodiscard]] QStringList outEndpointList() const;
  [[nodiscard]] int inEndpointIndex() const;
  [[nodiscard]] int outEndpointIndex() const;

  [[nodiscard]] int isoPacketSize() const;
  [[nodiscard]] bool advancedTransferConsent() const;

public slots:
  void setDriverProperty(const QString& key, const QVariant& value) override;
  void setDeviceIndex(const int index);
  void setTransferMode(const int mode);
  void grantAdvancedTransferConsent();
  void setInEndpointIndex(const int index);
  void setOutEndpointIndex(const int index);
  void setIsoPacketSize(const int size);
  void setupExternalConnections();
  void sendControlRequest(const QString& bmRequestType,
                          const QString& bRequest,
                          const QString& wValue,
                          const QString& wIndex,
                          const QString& payloadHex,
                          const int readLength);

private slots:
  void onReadError();
  void enumerateDevices();
  void onPumpData(const QByteArray& data, IO::CapturedData::SteadyTimePoint timestamp);
  void onControlTransferCompleted(bool ok,
                                  int bytesTransferred,
                                  const QString& responseHex,
                                  int status);

private:
  struct EndpointInfo {
    uint8_t address;
    uint8_t attributes;
    uint8_t altSetting;
    uint16_t maxPacketSize;
    int interfaceNumber;
    QString label;
  };

  void buildEndpointLists();
  void clearEndpointLists();
  void collectEndpoint(const libusb_endpoint_descriptor& ep,
                       int ifNum,
                       uint8_t altSetting,
                       bool wantIso);

  [[nodiscard]] QString enrichDeviceLabel(libusb_device* dev,
                                          const libusb_device_descriptor& desc,
                                          const QString& base) const;
  [[nodiscard]] QString endpointErrorMessage() const;
  [[nodiscard]] bool activateSelectedEndpoints();

  [[nodiscard]] bool deviceSerialMatches(libusb_device* device,
                                         const libusb_device_descriptor& desc,
                                         const QString& savedSer) const;

  static int LIBUSB_CALL hotplugCallback(libusb_context* ctx,
                                         libusb_device* device,
                                         libusb_hotplug_event event,
                                         void* user_data);

private:
  libusb_context* m_ctx;
  libusb_device_handle* m_handle;
  libusb_hotplug_callback_handle m_hotplugHandle;

  UsbTransferPump m_pump;

  bool m_advancedConsent;

  int m_deviceIndex;
  int m_inEndpointIndex;
  int m_outEndpointIndex;
  int m_isoPacketSize;

  TransferMode m_transferMode;

  QSettings m_settings;

  QStringList m_deviceLabels;
  QList<libusb_device*> m_devicePtrs;
  QHash<QString, QString> m_deviceLabelCache;

  QList<EndpointInfo> m_inEndpoints;
  QList<EndpointInfo> m_outEndpoints;
  QStringList m_inEndpointLabels;
  QStringList m_outEndpointLabels;

  uint8_t m_activeInEp;
  uint8_t m_activeOutEp;
  uint8_t m_activeInEpType;
  uint8_t m_activeOutEpType;
};

}  // namespace Drivers
}  // namespace IO
