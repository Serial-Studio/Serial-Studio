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

#include "IO/Drivers/USB/UsbTransferPump.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <QMetaObject>

#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

constexpr unsigned int kBulkReadTimeout = 100;
constexpr unsigned int kControlTimeout  = 1000;
constexpr int kBulkReadBufSize          = 65536;
constexpr int kIsoNumTransfers          = 8;
constexpr int kIsoPacketsPerTransfer    = 8;
constexpr int kIsoDrainTimeoutMs        = 2000;
constexpr int kIsoIdleSleepMs           = 10;
constexpr int kEventTimeoutUsec         = 100000;

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds an idle pump bound to the owner's libusb context and device handle; both are held
 *        by reference, so a reopened handle is visible without re-injection.
 */
IO::Drivers::UsbTransferPump::UsbTransferPump(libusb_context* const& context,
                                              libusb_device_handle* const& handle,
                                              QObject* parent)
  : QObject(parent)
  , m_context(context)
  , m_handle(handle)
  , m_readEndpoint(0)
  , m_readEndpointType(0)
  , m_isoPacketSize(0)
  , m_running(false)
  , m_eventLoopRunning(false)
  , m_isoInFlight(0)
  , m_controlInFlight(false)
  , m_drainWaiting(false)
  , m_controlTransfer(nullptr)
{}

/**
 * @brief Joins both threads as a safety net. It deliberately does NOT free the transfer pool: the
 *        owner drains and frees while its context is still alive, and freeing after libusb_exit()
 *        would touch a destroyed context, so a non-empty pool here is a teardown-order defect and
 *        is reported as one.
 */
IO::Drivers::UsbTransferPump::~UsbTransferPump()
{
  stopReadThread();
  stopEventThread();

  SS_ASSERT_LOG(m_isoTransfers.isEmpty());
  SS_ASSERT_LOG(m_controlTransfer == nullptr);
}

//--------------------------------------------------------------------------------------------------
// Event thread
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts the libusb event thread; every completion callback runs there.
 */
void IO::Drivers::UsbTransferPump::startEventThread()
{
  SS_ASSERT(m_context != nullptr, return);

  if (m_eventThread.isRunning())
    return;

  m_eventLoopRunning.store(true, std::memory_order_release);
  connect(
    &m_eventThread, &QThread::started, this, &UsbTransferPump::eventLoop, Qt::DirectConnection);

  m_eventThread.start();
}

/**
 * @brief Stops and joins the libusb event thread. quit() before wait() because eventLoop runs on
 * the started+DirectConnection idiom and would otherwise fall into exec().
 */
void IO::Drivers::UsbTransferPump::stopEventThread()
{
  m_eventLoopRunning.store(false, std::memory_order_release);

  if (m_eventThread.isRunning()) {
    m_eventThread.quit();
    m_eventThread.wait();
  }

  disconnect(&m_eventThread, &QThread::started, this, &UsbTransferPump::eventLoop);
}

/**
 * @brief Returns true while the libusb event thread pumps; the owner uses it to keep synchronous
 *        string-descriptor reads off a device whose backend is already being pumped.
 */
bool IO::Drivers::UsbTransferPump::eventThreadRunning() const
{
  return m_eventThread.isRunning();
}

/**
 * @brief Runs the libusb event loop on m_eventThread.
 */
void IO::Drivers::UsbTransferPump::eventLoop()
{
  while (m_eventLoopRunning.load(std::memory_order_acquire)) {
    struct timeval tv = {0, kEventTimeoutUsec};
    libusb_handle_events_timeout(m_context, &tv);
  }
}

//--------------------------------------------------------------------------------------------------
// Read thread
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts the synchronous read loop for the bulk and interrupt transfer modes.
 */
void IO::Drivers::UsbTransferPump::startBulkRead(const uint8_t endpoint, const uint8_t endpointType)
{
  SS_ASSERT(m_handle != nullptr, return);
  SS_ASSERT(endpoint != 0, return);

  m_readEndpoint     = endpoint;
  m_readEndpointType = endpointType;
  m_running.store(true, std::memory_order_release);

  connect(&m_readThread, &QThread::started, this, &UsbTransferPump::readLoop, Qt::DirectConnection);
  m_readThread.start();
}

/**
 * @brief Submits the isochronous transfer pool and parks the read thread; the pool then
 *        self-resubmits from the event thread until the run flag drops.
 */
void IO::Drivers::UsbTransferPump::startIsochronousRead(const uint8_t endpoint,
                                                        const int packetSize)
{
  SS_ASSERT(m_handle != nullptr, return);
  SS_ASSERT(packetSize > 0, return);

  m_readEndpoint     = endpoint;
  m_readEndpointType = LIBUSB_TRANSFER_TYPE_ISOCHRONOUS;
  m_isoPacketSize    = packetSize;
  m_running.store(true, std::memory_order_release);

  allocateIsoTransfers();

  connect(
    &m_readThread, &QThread::started, this, &UsbTransferPump::isoReadLoop, Qt::DirectConnection);

  m_readThread.start();
}

/**
 * @brief Stops and joins the read thread. Mirrors HID::cleanupDevice: the started+DirectConnection
 * idiom drops the thread into exec() once the loop returns, so quit() before wait() is mandatory
 * and terminate() (which corrupts libusb mid-transfer) is never used. Both loop connections are
 * detached here so the next open() cycle wires exactly one read slot instead of double-connecting.
 */
void IO::Drivers::UsbTransferPump::stopReadThread()
{
  m_running.store(false, std::memory_order_release);

  if (m_readThread.isRunning()) {
    m_readThread.quit();
    m_readThread.wait();
  }

  disconnect(&m_readThread, &QThread::started, this, &UsbTransferPump::readLoop);
  disconnect(&m_readThread, &QThread::started, this, &UsbTransferPump::isoReadLoop);
}

/**
 * @brief Synchronous read loop for the bulk and interrupt modes; the acquisition stamp is taken
 *        the moment the transfer returns, and dataReceived() is emitted on this thread so the
 *        owner publishes exactly where it used to.
 */
void IO::Drivers::UsbTransferPump::readLoop()
{
  unsigned char buf[kBulkReadBufSize];
  const bool interruptEp = (m_readEndpointType == LIBUSB_TRANSFER_TYPE_INTERRUPT);

  while (m_running.load(std::memory_order_relaxed)) {
    int transferred = 0;
    int rc;
    if (interruptEp)
      rc = libusb_interrupt_transfer(
        m_handle, m_readEndpoint, buf, kBulkReadBufSize, &transferred, kBulkReadTimeout);
    else
      rc = libusb_bulk_transfer(
        m_handle, m_readEndpoint, buf, kBulkReadBufSize, &transferred, kBulkReadTimeout);

    if (rc == LIBUSB_ERROR_TIMEOUT)
      continue;

    if (rc == 0 && transferred > 0) {
      const auto timestamp = IO::CapturedData::SteadyClock::now();
      Q_EMIT dataReceived(QByteArray(reinterpret_cast<const char*>(buf), transferred), timestamp);
      continue;
    }

    if (rc != 0) {
      Q_EMIT readError();
      break;
    }
  }
}

/**
 * @brief Idle loop for the isochronous mode; the transfers themselves are driven by the event
 *        thread, so this only holds the read thread alive until the run flag drops.
 */
void IO::Drivers::UsbTransferPump::isoReadLoop()
{
  while (m_running.load(std::memory_order_relaxed))
    QThread::msleep(kIsoIdleSleepMs);
}

//--------------------------------------------------------------------------------------------------
// Isochronous transfer pool
//--------------------------------------------------------------------------------------------------

/**
 * @brief Allocates and submits the isochronous transfer pool on the owner's thread.
 */
void IO::Drivers::UsbTransferPump::allocateIsoTransfers()
{
  const int totalBufSize = m_isoPacketSize * kIsoPacketsPerTransfer;

  for (int i = 0; i < kIsoNumTransfers; ++i) {
    libusb_transfer* t = libusb_alloc_transfer(kIsoPacketsPerTransfer);
    if (!t)
      break;

    auto* buf = new (std::nothrow) unsigned char[totalBufSize];
    if (!buf) {
      libusb_free_transfer(t);
      break;
    }

    libusb_fill_iso_transfer(t,
                             m_handle,
                             m_readEndpoint,
                             buf,
                             totalBufSize,
                             kIsoPacketsPerTransfer,
                             &UsbTransferPump::isoTransferCallback,
                             this,
                             0);

    libusb_set_iso_packet_lengths(t, static_cast<unsigned int>(m_isoPacketSize));

    if (libusb_submit_transfer(t) < 0) {
      delete[] buf;
      libusb_free_transfer(t);
    } else {
      m_isoTransfers.append(t);
      m_isoInFlight.fetch_add(1, std::memory_order_acq_rel);
    }
  }
}

/**
 * @brief Static libusb callback for each completed iso transfer; stamps acquisition time before
 *        queueing. Buffer ownership lives solely with freeTransfers() (frees post-join).
 */
void LIBUSB_CALL IO::Drivers::UsbTransferPump::isoTransferCallback(libusb_transfer* transfer)
{
  auto* self = static_cast<UsbTransferPump*>(transfer->user_data);

  if (transfer->status == LIBUSB_TRANSFER_COMPLETED || transfer->status == LIBUSB_TRANSFER_ERROR) {
    int totalLen = 0;
    for (int i = 0; i < transfer->num_iso_packets; ++i)
      totalLen += static_cast<int>(transfer->iso_packet_desc[i].actual_length);

    QByteArray received;
    received.reserve(totalLen);

    for (int i = 0; i < transfer->num_iso_packets; ++i) {
      const libusb_iso_packet_descriptor& pkt = transfer->iso_packet_desc[i];
      if (pkt.actual_length == 0)
        continue;

      const unsigned char* data =
        libusb_get_iso_packet_buffer_simple(transfer, static_cast<unsigned int>(i));

      received.append(reinterpret_cast<const char*>(data), static_cast<int>(pkt.actual_length));
    }

    if (!received.isEmpty()) {
      const auto timestamp = IO::CapturedData::SteadyClock::now();
      QMetaObject::invokeMethod(
        self,
        [self, received, timestamp] { Q_EMIT self->dataReceived(received, timestamp); },
        Qt::QueuedConnection);
    }
  }

  if (!self->m_running.load(std::memory_order_relaxed)) {
    self->m_isoInFlight.fetch_sub(1, std::memory_order_acq_rel);
    self->notifyDrainWaiter();
    return;
  }

  if (libusb_submit_transfer(transfer) < 0) {
    self->m_running.store(false, std::memory_order_release);
    self->m_isoInFlight.fetch_sub(1, std::memory_order_acq_rel);
    self->notifyDrainWaiter();
    Q_EMIT self->readError();
  }
}

//--------------------------------------------------------------------------------------------------
// Transfer teardown
//--------------------------------------------------------------------------------------------------

/**
 * @brief Cancels every iso transfer and waits until all callbacks have reported back. libusb
 * forbids freeing an in-flight transfer, so this must drain to zero before the pool is freed;
 * the completion callbacks wake the wait as soon as the last one lands, and the bounded
 * deadline covers a dead device whose cancellations never complete.
 */
void IO::Drivers::UsbTransferPump::cancelAndDrainTransfers()
{
  for (auto* t : std::as_const(m_isoTransfers))
    libusb_cancel_transfer(t);

  if (m_controlTransfer)
    libusb_cancel_transfer(m_controlTransfer);

  m_drainWaiting.store(true, std::memory_order_release);
  {
    std::unique_lock<std::mutex> lock(m_drainMutex);
    (void)m_drainCv.wait_for(lock, std::chrono::milliseconds(kIsoDrainTimeoutMs), [this] {
      return m_isoInFlight.load(std::memory_order_acquire) == 0
          && !m_controlInFlight.load(std::memory_order_acquire);
    });
  }
  m_drainWaiting.store(false, std::memory_order_release);
}

/**
 * @brief Wakes a drain wait once an in-flight counter dropped (event thread). The empty lock
 * before notify is what closes the race with a waiter that checked the counters but has not
 * gone to sleep yet; the steady state (no drain pending) costs one atomic load.
 */
void IO::Drivers::UsbTransferPump::notifyDrainWaiter()
{
  if (!m_drainWaiting.load(std::memory_order_acquire))
    return;

  {
    std::lock_guard<std::mutex> lock(m_drainMutex);
  }
  m_drainCv.notify_all();
}

/**
 * @brief Frees the iso transfer pool and any leftover control transfer plus their buffers. Sole
 * owner of transfer->buffer: only valid once the read and event threads are joined, so no callback
 * can race the free.
 */
void IO::Drivers::UsbTransferPump::freeTransfers()
{
  for (auto* t : std::as_const(m_isoTransfers)) {
    delete[] t->buffer;
    libusb_free_transfer(t);
  }

  m_isoTransfers.clear();
  m_isoInFlight.store(0, std::memory_order_release);

  if (m_controlTransfer) {
    delete[] m_controlTransfer->buffer;
    libusb_free_transfer(m_controlTransfer);
    m_controlTransfer = nullptr;
  }

  m_controlInFlight.store(false, std::memory_order_release);
}

//--------------------------------------------------------------------------------------------------
// Interface claims
//--------------------------------------------------------------------------------------------------

/**
 * @brief Claims @p interfaceNumber on the open device handle (no-op when already claimed).
 */
bool IO::Drivers::UsbTransferPump::claimInterface(const int interfaceNumber)
{
  SS_ASSERT(m_handle != nullptr, return false);

  if (m_claimedInterfaces.contains(interfaceNumber))
    return true;

  if (libusb_claim_interface(m_handle, interfaceNumber) < 0)
    return false;

  m_claimedInterfaces.append(interfaceNumber);
  return true;
}

/**
 * @brief Releases every claimed interface.
 */
void IO::Drivers::UsbTransferPump::releaseInterfaces()
{
  if (m_handle)
    for (const int iface : std::as_const(m_claimedInterfaces))
      libusb_release_interface(m_handle, iface);

  m_claimedInterfaces.clear();
}

//--------------------------------------------------------------------------------------------------
// Control transfers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true while a control transfer is still outstanding.
 */
bool IO::Drivers::UsbTransferPump::controlTransferInFlight() const
{
  return m_controlInFlight.load(std::memory_order_acquire);
}

/**
 * @brief Composes and submits an async USB control transfer from an already-validated setup
 *        packet, reporting later through @ref controlTransferCompleted. Async so the event thread
 *        owns all event handling and the UI never blocks; the previous transfer is freed here, on
 *        the owner's thread, because the callback never touches that lifetime.
 */
IO::Drivers::UsbTransferPump::ControlSubmitResult IO::Drivers::UsbTransferPump::
  submitControlTransfer(const ControlSetup& setup, int& libusbError)
{
  SS_ASSERT(m_handle != nullptr, return ControlSubmitResult::AllocationFailed);
  SS_ASSERT(setup.length >= 0, return ControlSubmitResult::AllocationFailed);

  libusbError = 0;

  if (m_controlTransfer) {
    delete[] m_controlTransfer->buffer;
    libusb_free_transfer(m_controlTransfer);
    m_controlTransfer = nullptr;
  }

  libusb_transfer* transfer = libusb_alloc_transfer(0);
  auto* buffer =
    transfer ? new (std::nothrow) unsigned char[LIBUSB_CONTROL_SETUP_SIZE + setup.length] : nullptr;
  if (!transfer || !buffer) {
    delete[] buffer;
    if (transfer)
      libusb_free_transfer(transfer);

    return ControlSubmitResult::AllocationFailed;
  }

  libusb_fill_control_setup(buffer,
                            setup.requestType,
                            setup.request,
                            setup.value,
                            setup.index,
                            static_cast<uint16_t>(setup.length));

  const int payloadSize = static_cast<int>(setup.payload.size());
  if (payloadSize > 0)
    std::memcpy(buffer + LIBUSB_CONTROL_SETUP_SIZE,
                setup.payload.constData(),
                static_cast<size_t>(std::min(payloadSize, setup.length)));

  libusb_fill_control_transfer(
    transfer, m_handle, buffer, &UsbTransferPump::controlTransferCallback, this, kControlTimeout);

  m_controlTransfer = transfer;
  m_controlInFlight.store(true, std::memory_order_release);

  const int rc = libusb_submit_transfer(transfer);
  if (rc < 0) {
    m_controlInFlight.store(false, std::memory_order_release);
    m_controlTransfer = nullptr;
    delete[] buffer;
    libusb_free_transfer(transfer);
    libusbError = rc;
    return ControlSubmitResult::SubmitFailed;
  }

  return ControlSubmitResult::Submitted;
}

/**
 * @brief Static libusb completion callback (event thread): marshals the outcome via a queued
 * emit, then clears the in-flight flag last. It never frees the transfer nor writes
 * m_controlTransfer; the owner thread owns that lifetime (freed at the next send or in
 * freeTransfers), mirroring the iso pool so teardown cannot race a free.
 */
void LIBUSB_CALL IO::Drivers::UsbTransferPump::controlTransferCallback(libusb_transfer* transfer)
{
  auto* self = static_cast<UsbTransferPump*>(transfer->user_data);
  SS_ASSERT(self != nullptr, return);
  SS_ASSERT(transfer->buffer != nullptr, {
    self->m_controlInFlight.store(false, std::memory_order_release);
    self->notifyDrainWaiter();
    return;
  });

  const bool ok    = (transfer->status == LIBUSB_TRANSFER_COMPLETED);
  const int bytes  = transfer->actual_length;
  const int status = static_cast<int>(transfer->status);
  const bool isIn  = (transfer->buffer[0] & LIBUSB_ENDPOINT_IN) != 0;

  QString responseHex;
  if (ok && isIn && bytes > 0) {
    const unsigned char* data = libusb_control_transfer_get_data(transfer);
    responseHex =
      QString::fromLatin1(QByteArray(reinterpret_cast<const char*>(data), bytes).toHex(' '));
  }

  QMetaObject::invokeMethod(
    self,
    [self, ok, bytes, responseHex, status] {
      Q_EMIT self->controlTransferCompleted(ok, bytes, responseHex, status);
    },
    Qt::QueuedConnection);

  self->m_controlInFlight.store(false, std::memory_order_release);
  self->notifyDrainWaiter();
}
