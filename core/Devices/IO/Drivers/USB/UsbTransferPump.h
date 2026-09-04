/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
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

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <new>
#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>
#include <QThread>

#include "IO/HAL_Driver.h"

namespace IO {
namespace Drivers {

/**
 * @brief Owns every libusb transfer of the USB driver and the two threads that drive them: the
 *        event thread that pumps @c libusb_handle_events_timeout and the read thread. The drain
 *        handshake lives here whole because libusb forbids freeing an in-flight transfer, and
 *        splitting it from the completion callbacks would separate the lock from what it guards.
 */
class UsbTransferPump : public QObject {
  Q_OBJECT

signals:
  void readError();
  void dataReceived(const QByteArray& data, IO::CapturedData::SteadyTimePoint timestamp);
  void controlTransferCompleted(bool ok,
                                int bytesTransferred,
                                const QString& responseHex,
                                int status);

public:
  /**
   * @brief Outcome of a control-transfer submission; the owner turns each into the message the
   *        composer shows, so every user-visible string stays in the driver's translation unit.
   */
  enum class ControlSubmitResult {
    Submitted        = 0,
    AllocationFailed = 1,
    SubmitFailed     = 2,
  };

  /**
   * @brief One control-transfer request, already parsed and bounds-checked by the owner.
   */
  struct ControlSetup {
    uint8_t requestType;
    uint8_t request;
    uint16_t value;
    uint16_t index;
    int length;
    QByteArray payload;
  };

public:
  explicit UsbTransferPump(libusb_context* const& context,
                           libusb_device_handle* const& handle,
                           QObject* parent = nullptr);
  ~UsbTransferPump() override;

  UsbTransferPump(UsbTransferPump&&)                 = delete;
  UsbTransferPump(const UsbTransferPump&)            = delete;
  UsbTransferPump& operator=(UsbTransferPump&&)      = delete;
  UsbTransferPump& operator=(const UsbTransferPump&) = delete;

  void freeTransfers();
  void stopReadThread();
  void stopEventThread();
  void startEventThread();
  void releaseInterfaces();
  void cancelAndDrainTransfers();
  void startBulkRead(const uint8_t endpoint, const uint8_t endpointType);
  void startIsochronousRead(const uint8_t endpoint, const int packetSize);

  [[nodiscard]] bool claimInterface(const int interfaceNumber);
  [[nodiscard]] bool eventThreadRunning() const;
  [[nodiscard]] bool controlTransferInFlight() const;
  [[nodiscard]] ControlSubmitResult submitControlTransfer(const ControlSetup& setup,
                                                          int& libusbError);

private:
  void readLoop();
  void eventLoop();
  void isoReadLoop();
  void notifyDrainWaiter();
  void allocateIsoTransfers();

  static void LIBUSB_CALL isoTransferCallback(libusb_transfer* transfer);
  static void LIBUSB_CALL controlTransferCallback(libusb_transfer* transfer);

private:
  libusb_context* const& m_context;
  libusb_device_handle* const& m_handle;

  uint8_t m_readEndpoint;
  uint8_t m_readEndpointType;
  int m_isoPacketSize;

  static constexpr std::size_t kCacheLine = 64;
  alignas(kCacheLine) std::atomic<bool> m_running;
  alignas(kCacheLine) std::atomic<bool> m_eventLoopRunning;
  alignas(kCacheLine) std::atomic<int> m_isoInFlight;
  alignas(kCacheLine) std::atomic<bool> m_controlInFlight;
  alignas(kCacheLine) std::atomic<bool> m_drainWaiting;

  std::mutex m_drainMutex;
  std::condition_variable m_drainCv;

  QThread m_readThread;
  QThread m_eventThread;

  QList<int> m_claimedInterfaces;
  QList<libusb_transfer*> m_isoTransfers;
  libusb_transfer* m_controlTransfer;
};

}  // namespace Drivers
}  // namespace IO
