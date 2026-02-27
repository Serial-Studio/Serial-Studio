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
#ifdef BUILD_COMMERCIAL

#  include <libusb.h>

#  include <atomic>
#  include <QList>
#  include <QObject>
#  include <QSettings>
#  include <QString>
#  include <QStringList>
#  include <QThread>

#  include "IO/HAL_Driver.h"

namespace IO {
namespace Drivers {

/**
 * @brief Raw USB driver supporting bulk, control, and isochronous transfers.
 *
 * Device enumeration is driven by libusb hotplug callbacks when the platform
 * supports them (macOS, Linux, Windows). A dedicated event thread runs
 * libusb_handle_events_timeout() continuously so both hotplug notifications
 * and isochronous transfer callbacks are processed without polling. On
 * platforms where hotplug is unavailable a 2-second QTimer fallback is used.
 *
 * Three transfer modes:
 *   BulkStream      — synchronous bulk IN/OUT (default).
 *   AdvancedControl — bulk + control transfers; requires user confirmation.
 *   Isochronous     — async isochronous transfers using the event thread.
 *
 * Threading:
 *   m_eventThread runs libusb_handle_events_timeout() at all times.
 *   BulkStream / AdvancedControl: blocking libusb_bulk_transfer loop on
 *     m_readThread.
 *   Isochronous: submitted transfers are driven by m_eventThread; callbacks
 *     resubmit completed transfers.
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
  // clang-format on

signals:
  void transferModeChanged();
  void deviceListChanged();
  void deviceIndexChanged();
  void endpointListChanged();
  void inEndpointIndexChanged();
  void outEndpointIndexChanged();
  void isoPacketSizeChanged();

public:
  /**
   * @brief USB transfer mode.
   */
  enum class TransferMode {
    BulkStream      = 0, /**< Synchronous bulk IN/OUT (default). */
    AdvancedControl = 1, /**< Bulk + control transfers. */
    Isochronous     = 2, /**< Async isochronous transfers. */
  };
  Q_ENUM(TransferMode)

private:
  explicit USB();
  ~USB();

  USB(USB&&)                 = delete;
  USB(const USB&)            = delete;
  USB& operator=(USB&&)      = delete;
  USB& operator=(const USB&) = delete;

public:
  static USB& instance();

  void close() override;

  [[nodiscard]] bool isOpen() const noexcept override;
  [[nodiscard]] bool isReadable() const noexcept override;
  [[nodiscard]] bool isWritable() const noexcept override;
  [[nodiscard]] bool configurationOk() const noexcept override;
  [[nodiscard]] qint64 write(const QByteArray& data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;

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

public slots:
  void setDeviceIndex(const int index);
  void setTransferMode(const int mode);
  void setInEndpointIndex(const int index);
  void setOutEndpointIndex(const int index);
  void setIsoPacketSize(const int size);
  void setupExternalConnections();

private slots:
  void onReadError();
  void enumerateDevices();

private:
  struct EndpointInfo {
    uint8_t address;
    uint8_t attributes;
    uint16_t maxPacketSize;
    int interfaceNumber;
    QString label;
  };

  void buildEndpointLists();
  void clearEndpointLists();
  void eventLoop();

  [[nodiscard]] QString endpointErrorMessage() const;

  void readLoop();
  void isoReadLoop();

  bool claimInterface(int ifaceNum);
  void releaseInterface();

  [[nodiscard]] qint64 sendControlTransfer(uint8_t bmRequestType,
                                           uint8_t bRequest,
                                           uint16_t wValue,
                                           uint16_t wIndex,
                                           const QByteArray& data,
                                           unsigned int timeout_ms);

  static void LIBUSB_CALL isoTransferCallback(libusb_transfer* transfer);
  static int LIBUSB_CALL hotplugCallback(libusb_context* ctx,
                                         libusb_device* device,
                                         libusb_hotplug_event event,
                                         void* user_data);

private:
  libusb_context* m_ctx;
  libusb_device_handle* m_handle;
  libusb_hotplug_callback_handle m_hotplugHandle;

  int m_deviceIndex;
  int m_inEndpointIndex;
  int m_outEndpointIndex;
  int m_claimedInterface;
  int m_isoPacketSize;

  TransferMode m_transferMode;

  std::atomic<bool> m_running;
  std::atomic<bool> m_eventLoopRunning;

  QThread m_readThread;
  QThread m_eventThread;
  QSettings m_settings;

  QStringList m_deviceLabels;
  QList<libusb_device*> m_devicePtrs;

  QList<EndpointInfo> m_inEndpoints;
  QList<EndpointInfo> m_outEndpoints;
  QStringList m_inEndpointLabels;
  QStringList m_outEndpointLabels;

  uint8_t m_activeInEp;
  uint8_t m_activeOutEp;

  QList<libusb_transfer*> m_isoTransfers;
};

}  // namespace Drivers
}  // namespace IO

#endif  // BUILD_COMMERCIAL
