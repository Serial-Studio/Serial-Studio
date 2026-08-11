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

#include "IO/Drivers/CANBus/GsUsbCanBackend.h"

#include <libusb.h>

#ifdef _WIN32
#  include <winsock2.h>
#else
#  include <sys/time.h>
#endif

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <mutex>
#include <QMetaObject>
#include <QScopeGuard>
#include <QVariant>
#include <thread>

#include "IO/Drivers/CANBus/GsUsbProtocol.h"
#include "SSAssert.h"

using namespace IO::Drivers::GsUsb;

//--------------------------------------------------------------------------------------------------
// gs_usb device identity & transfer geometry
//--------------------------------------------------------------------------------------------------

/**
 * @brief USB vendor/product identifiers known to expose the gs_usb interface.
 */
struct GsUsbId {
  std::uint16_t vid;
  std::uint16_t pid;
};

constexpr GsUsbId kGsUsbIds[] = {
  {0x1d50, 0x606f}, // OpenMoko: candleLight, CANable, CANtact Pro, RH-02
  {0x1209, 0x2323}, // CANtact
  {0x1cd2, 0x606f}, // CANalyze
  {0x16d0, 0x10b8}, // CES CANext FD
  {0x1209, 0x1234}, // generic gs_usb clones
};

// Transfer geometry
constexpr int kBulkReadBufSize        = 2048;
constexpr unsigned int kReadTimeoutMs = 100;
constexpr unsigned int kCtrlTimeoutMs = 1000;
constexpr unsigned int kWriteTimeout  = 1000;

// TX echo confirmation: poll cadence and the deadline after which an un-echoed transmit fails
constexpr int kTxTimeoutPollMs       = 250;
constexpr qint64 kTxConfirmTimeoutMs = 1000;

/**
 * @brief Returns true when the given descriptor matches a known gs_usb adapter.
 */
[[nodiscard]] static bool isGsUsbDevice(const libusb_device_descriptor& desc)
{
  for (const GsUsbId& id : kGsUsbIds)
    if (desc.idVendor == id.vid && desc.idProduct == id.pid)
      return true;

  return false;
}

/**
 * @brief Builds the stable interface label shown in the CANBus interface combo.
 */
[[nodiscard]] static QString deviceLabel(libusb_device* device,
                                         const libusb_device_descriptor& desc)
{
  SS_ASSERT(device != nullptr, return {});

  QString serial;
  libusb_device_handle* handle = nullptr;
  if (desc.iSerialNumber && libusb_open(device, &handle) == 0) {
    unsigned char buf[256] = {};
    const int rc = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, buf, sizeof(buf));
    if (rc > 0)
      serial = QString::fromLatin1(reinterpret_cast<const char*>(buf), rc).trimmed();

    libusb_close(handle);
  }

  if (serial.isEmpty()) {
    serial = QStringLiteral("USB %1.%2")
               .arg(libusb_get_bus_number(device))
               .arg(libusb_get_device_address(device));
  }

  return QStringLiteral("CANable %1:%2 (%3)")
    .arg(desc.idVendor, 4, 16, QChar('0'))
    .arg(desc.idProduct, 4, 16, QChar('0'))
    .arg(serial);
}

/**
 * @brief Returns a process-lifetime libusb context shared by every probe and connection.
 *
 * libusb_exit() on macOS signals its CFRunLoop event thread and pthread_join()s it; that
 * join deadlocks (seen at startup from GsUsbCanBackend::supported(), and aggravated by the
 * mimalloc override). Initializing one context once and never calling libusb_exit() removes
 * every join: the event thread lives until the process exits. Ownership is the static;
 * callers borrow the pointer and must never libusb_exit() it.
 */
[[nodiscard]] static libusb_context* sharedUsbContext()
{
  static libusb_context* const ctx = [] {
    libusb_context* c = nullptr;
    if (libusb_init(&c) != 0)
      c = nullptr;

    return c;
  }();

  return ctx;
}

/**
 * @brief Decodes a received host frame (classic or FD slot); returns false for TX echoes that
 *        must be dropped. FD flag handling is gated on @p fdMode so the classic decode path
 *        stays bit-identical to the pre-FD behavior.
 */
[[nodiscard]] static bool decodeRxFrame(const GsHostFrameFd& host, bool fdMode, QCanBusFrame& out)
{
  if (host.echoId != kHostFrameRx)
    return false;

  const bool extended    = (host.canId & kCanEffFlag) != 0;
  const bool remote      = (host.canId & kCanRtrFlag) != 0;
  const bool errored     = (host.canId & kCanErrFlag) != 0;
  const std::uint32_t id = host.canId & (extended ? kCanEffMask : kCanSffMask);

  const bool fdFrame = fdMode && (host.flags & kFrameFlagFd) != 0;
  const int length   = fdFrame ? dlc2len(host.canDlc) : std::min<int>(host.canDlc, 8);

  out = QCanBusFrame(id, QByteArray(reinterpret_cast<const char*>(host.data), length));
  out.setExtendedFrameFormat(extended);
  if (fdFrame) {
    out.setFlexibleDataRateFormat(true);
    out.setBitrateSwitch((host.flags & kFrameFlagBrs) != 0);
    out.setErrorStateIndicator((host.flags & kFrameFlagEsi) != 0);
  }

  if (errored)
    out.setFrameType(QCanBusFrame::ErrorFrame);
  else if (remote)
    out.setFrameType(QCanBusFrame::RemoteRequestFrame);

  return true;
}

//--------------------------------------------------------------------------------------------------
// Static plugin identity & enumeration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns this backend's registry entry.
 */
IO::Drivers::CanBackends::Entry IO::Drivers::GsUsbCanBackend::registration()
{
  return {pluginKey(),
          QStringLiteral("CANable USB"),
          supported(),
          &availableInterfaces,
          &create,
          &interfaceSupportsFD};
}

/**
 * @brief Returns the synthetic plugin key used by IO::Drivers::CANBus.
 */
const QString& IO::Drivers::GsUsbCanBackend::pluginKey()
{
  static const QString key = QStringLiteral("canable_gsusb");
  return key;
}

/**
 * @brief Returns true when libusb is available on this platform.
 */
bool IO::Drivers::GsUsbCanBackend::supported()
{
  return sharedUsbContext() != nullptr;
}

/**
 * @brief Per-label FD capability cache: filled for new labels and pruned to the enumerated
 *        set on every availableInterfaces() pass, so it is self-bounding and a present
 *        (possibly claimed) adapter is never re-probed. Main-thread only.
 */
[[nodiscard]] static QHash<QString, bool>& fdCapabilityCache()
{
  static QHash<QString, bool> cache;
  return cache;
}

/**
 * @brief Reads BT_CONST from an unclaimed device and reports the FD feature bit; any failure
 *        degrades to "not FD-capable" so a busy or odd device never blocks classic use.
 */
[[nodiscard]] static bool probeFdCapability(libusb_device* device)
{
  SS_ASSERT(device != nullptr, return false);

  libusb_device_handle* handle = nullptr;
  if (libusb_open(device, &handle) < 0)
    return false;

  GsDeviceBtConst bt{};
  const std::uint8_t type =
    static_cast<std::uint8_t>(static_cast<unsigned>(LIBUSB_REQUEST_TYPE_VENDOR)
                              | static_cast<unsigned>(LIBUSB_RECIPIENT_INTERFACE)
                              | static_cast<unsigned>(LIBUSB_ENDPOINT_IN));
  const int rc = libusb_control_transfer(handle,
                                         type,
                                         kBreqBtConst,
                                         0,
                                         0,
                                         reinterpret_cast<unsigned char*>(&bt),
                                         sizeof(bt),
                                         kCtrlTimeoutMs);
  libusb_close(handle);

  return rc == static_cast<int>(sizeof(bt)) && (bt.feature & kFeatureFd) != 0;
}

/**
 * @brief Enumerates connected gs_usb adapters and returns their interface labels.
 */
QStringList IO::Drivers::GsUsbCanBackend::availableInterfaces()
{
  QStringList interfaces;

  libusb_context* ctx = sharedUsbContext();
  if (!ctx)
    return interfaces;

  libusb_device** devices = nullptr;
  const ssize_t count     = libusb_get_device_list(ctx, &devices);
  if (count < 0)
    return interfaces;

  for (ssize_t i = 0; i < count; ++i) {
    libusb_device_descriptor desc{};
    if (libusb_get_device_descriptor(devices[i], &desc) < 0)
      continue;

    if (!isGsUsbDevice(desc))
      continue;

    const QString label = deviceLabel(devices[i], desc);
    interfaces.append(label);
    if (!fdCapabilityCache().contains(label))
      fdCapabilityCache().insert(label, probeFdCapability(devices[i]));
  }

  libusb_free_device_list(devices, 1);

  QHash<QString, bool>& cache = fdCapabilityCache();
  for (auto it = cache.begin(); it != cache.end();)
    if (interfaces.contains(it.key()))
      ++it;
    else
      it = cache.erase(it);

  return interfaces;
}

/**
 * @brief Reports the cached FD capability of a previously enumerated interface label.
 */
bool IO::Drivers::GsUsbCanBackend::interfaceSupportsFD(const QString& interfaceName)
{
  return fdCapabilityCache().value(interfaceName, false);
}

/**
 * @brief Hot-plug notifier target. The mutex is the lifecycle boundary between the pumping
 *        thread delivering the libusb callback and the main thread registering/clearing the
 *        target; it is never touched per frame.
 */
struct HotplugNotifier {
  std::mutex lock;
  QObject* context;
  std::function<void()> notify;
};

[[nodiscard]] static HotplugNotifier& hotplugNotifier()
{
  static HotplugNotifier notifier;
  return notifier;
}

/**
 * @brief libusb hot-plug callback: runs on whichever thread pumps libusb events (the dedicated
 *        pump thread, or any thread inside a sync transfer). Non-gs_usb devices are filtered on
 *        the cached descriptor; the rest of the body is one queued invoke under the notifier
 *        lock -- no other Qt state, no USB I/O here.
 */
static int LIBUSB_CALL onGsUsbHotplug(libusb_context* context,
                                      libusb_device* device,
                                      libusb_hotplug_event event,
                                      void* user_data)
{
  Q_UNUSED(context)
  Q_UNUSED(event)
  Q_UNUSED(user_data)

  libusb_device_descriptor desc{};
  if (libusb_get_device_descriptor(device, &desc) < 0 || !isGsUsbDevice(desc))
    return 0;

  HotplugNotifier& notifier = hotplugNotifier();
  const std::lock_guard<std::mutex> guard(notifier.lock);
  if (notifier.context && notifier.notify)
    QMetaObject::invokeMethod(notifier.context, notifier.notify, Qt::QueuedConnection);

  return 0;
}

/**
 * @brief Starts the process-lifetime event pump for the shared context. libusb delivers
 *        hot-plug callbacks only from libusb_handle_events(); without this thread nothing
 *        pumps the shared context while no gs_usb transfer is in flight, and arrival events
 *        would sit undelivered. Never joined, mirroring the never-exit context invariant.
 */
static void startHotplugPump(libusb_context* ctx)
{
  static std::once_flag once;
  std::call_once(once, [ctx] {
    std::thread([ctx] {
      timeval timeout{1, 0};
      // code-verify off
      // Intentional process-lifetime service loop, like the readLoop worker: the thread dies
      // with the process, mirroring the never-libusb_exit shared-context invariant.
      while (true)
        libusb_handle_events_timeout(ctx, &timeout);
      // code-verify on
    }).detach();
  });
}

/**
 * @brief Registers @p notifier for gs_usb arrive/left events. The libusb callback and pump
 *        thread persist for the process lifetime; only the notifier target is mutable, and
 *        clearHotplugNotifier() must disarm it before the context object is destroyed.
 */
void IO::Drivers::GsUsbCanBackend::setHotplugNotifier(QObject* context,
                                                      std::function<void()> notifier)
{
  SS_ASSERT(context != nullptr, return);

  libusb_context* ctx = sharedUsbContext();
  if (!ctx || !libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG))
    return;

  static const bool registered = [ctx] {
    libusb_hotplug_callback_handle handle = 0;
    return libusb_hotplug_register_callback(
             ctx,
             static_cast<libusb_hotplug_event>(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED
                                               | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT),
             static_cast<libusb_hotplug_flag>(0),
             LIBUSB_HOTPLUG_MATCH_ANY,
             LIBUSB_HOTPLUG_MATCH_ANY,
             LIBUSB_HOTPLUG_MATCH_ANY,
             &onGsUsbHotplug,
             nullptr,
             &handle)
        == LIBUSB_SUCCESS;
  }();
  if (!registered)
    return;

  {
    const std::lock_guard<std::mutex> guard(hotplugNotifier().lock);
    SS_ASSERT(hotplugNotifier().context == nullptr, return);
    hotplugNotifier().context = context;
    hotplugNotifier().notify  = std::move(notifier);
  }

  startHotplugPump(ctx);
}

/**
 * @brief Disarms the hot-plug notifier when @p context owns it; called from the registering
 *        object's destructor so a late callback finds a null target instead of freed memory.
 */
void IO::Drivers::GsUsbCanBackend::clearHotplugNotifier(QObject* context)
{
  HotplugNotifier& notifier = hotplugNotifier();
  const std::lock_guard<std::mutex> guard(notifier.lock);
  if (notifier.context != context)
    return;

  notifier.context = nullptr;
  notifier.notify  = nullptr;
}

/**
 * @brief Factory used by the CAN backend registry.
 */
QCanBusDevice* IO::Drivers::GsUsbCanBackend::create(const QString& interfaceName)
{
  return new GsUsbCanBackend(interfaceName);
}

//--------------------------------------------------------------------------------------------------
// Constructor/destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the backend bound to a previously enumerated interface label.
 */
IO::Drivers::GsUsbCanBackend::GsUsbCanBackend(const QString& interfaceName, QObject* parent)
  : QCanBusDevice(parent)
  , m_ctx(nullptr)
  , m_handle(nullptr)
  , m_interfaceName(interfaceName)
  , m_interfaceNumber(-1)
  , m_inEndpoint(0)
  , m_outEndpoint(0)
  , m_echoCounter(0)
  , m_fdActive(false)
  , m_padTxToMaxPacket(false)
  , m_rxFrameSize(kClassicFrameSize)
  , m_outMaxPacket(0)
  , m_running(false)
{
  m_txTimeoutTimer.setInterval(kTxTimeoutPollMs);
  m_txTimeoutTimer.setTimerType(Qt::CoarseTimer);
  connect(&m_txTimeoutTimer, &QTimer::timeout, this, &GsUsbCanBackend::checkTxTimeouts);
}

/**
 * @brief Tears down the read thread and releases all libusb resources.
 */
IO::Drivers::GsUsbCanBackend::~GsUsbCanBackend()
{
  if (state() != QCanBusDevice::UnconnectedState)
    close();

  stopReadThread();

  if (m_handle) {
    libusb_close(m_handle);
    m_handle = nullptr;
  }

  m_ctx = nullptr;
}

//--------------------------------------------------------------------------------------------------
// QCanBusDevice interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens the selected adapter, programs the bitrate, and starts the read loop.
 */
bool IO::Drivers::GsUsbCanBackend::open()
{
  SS_ASSERT(m_handle == nullptr, return false);

  m_ctx = sharedUsbContext();
  if (!m_ctx) {
    setError(tr("Failed to initialize libusb for the CANable adapter."),
             QCanBusDevice::ConnectionError);
    setState(QCanBusDevice::UnconnectedState);
    return false;
  }

  bool opened           = false;
  bool interfaceClaimed = false;
  const auto cleanup    = qScopeGuard([&] {
    if (opened)
      return;

    if (m_handle) {
      if (interfaceClaimed)
        libusb_release_interface(m_handle, m_interfaceNumber);

      libusb_close(m_handle);
      m_handle = nullptr;
    }

    m_ctx             = nullptr;
    m_interfaceNumber = -1;
  });

  libusb_device** devices = nullptr;
  const ssize_t count     = libusb_get_device_list(m_ctx, &devices);
  if (count < 0) {
    setError(tr("Unable to enumerate USB devices."), QCanBusDevice::ConnectionError);
    setState(QCanBusDevice::UnconnectedState);
    return false;
  }

  for (ssize_t i = 0; i < count && !m_handle; ++i) {
    libusb_device_descriptor desc{};
    if (libusb_get_device_descriptor(devices[i], &desc) < 0)
      continue;

    if (!isGsUsbDevice(desc) || deviceLabel(devices[i], desc) != m_interfaceName)
      continue;

    if (libusb_open(devices[i], &m_handle) < 0)
      m_handle = nullptr;
  }

  libusb_free_device_list(devices, 1);

  if (!m_handle) {
    setError(tr("The selected CANable adapter is no longer connected, or another "
                "application has it open. On Windows the device must use the WinUSB "
                "driver (candleLight installs it automatically)."),
             QCanBusDevice::ConnectionError);
    setState(QCanBusDevice::UnconnectedState);
    return false;
  }

#ifdef Q_OS_LINUX
  libusb_set_auto_detach_kernel_driver(m_handle, 1);
#endif

  if (!claimGsUsbInterface()) {
    setError(tr("Could not claim the CANable USB interface."), QCanBusDevice::ConnectionError);
    setState(QCanBusDevice::UnconnectedState);
    return false;
  }

  interfaceClaimed = true;

  const auto bitrate = configurationParameter(QCanBusDevice::BitRateKey).toUInt();
  if (!configureDevice(bitrate == 0 ? 500000 : bitrate)) {
    setState(QCanBusDevice::UnconnectedState);
    return false;
  }

  m_txClock.start();
  m_txTimeoutTimer.start();

  m_running.store(true, std::memory_order_release);
  connect(&m_readThread, &QThread::started, this, &GsUsbCanBackend::readLoop, Qt::DirectConnection);
  m_readThread.start();

  opened = true;
  setState(QCanBusDevice::ConnectedState);
  return true;
}

/**
 * @brief Stops the adapter, releases the interface, and returns to the unconnected state.
 */
void IO::Drivers::GsUsbCanBackend::close()
{
  setState(QCanBusDevice::ClosingState);

  m_txTimeoutTimer.stop();
  m_pendingTx.clear();

  stopReadThread();

  if (m_handle) {
    GsDeviceMode mode{kModeReset, 0};
    (void)controlOut(kBreqMode, 0, &mode, sizeof(mode));

    if (m_interfaceNumber >= 0)
      libusb_release_interface(m_handle, m_interfaceNumber);

    libusb_close(m_handle);
    m_handle = nullptr;
  }

  m_ctx = nullptr;

  m_interfaceNumber  = -1;
  m_inEndpoint       = 0;
  m_outEndpoint      = 0;
  m_fdActive         = false;
  m_padTxToMaxPacket = false;
  m_rxFrameSize      = kClassicFrameSize;
  m_outMaxPacket     = 0;
  m_rxCarry.clear();

  setState(QCanBusDevice::UnconnectedState);
}

/**
 * @brief Encodes a QCanBusFrame as a gs_host_frame and writes it via bulk OUT. BRS is forced
 *        on FD frames on purpose: the app-level producer has no per-frame BRS control, and a
 *        configured data bitrate implies switching.
 */
bool IO::Drivers::GsUsbCanBackend::writeFrame(const QCanBusFrame& frame)
{
  if (!m_handle || m_outEndpoint == 0) {
    setError(tr("CANable adapter is not open for writing."), QCanBusDevice::WriteError);
    return false;
  }

  if (!frame.isValid())
    return false;

  GsHostFrameFd host{};
  host.echoId   = m_echoCounter;
  m_echoCounter = (m_echoCounter + 1) % kHostFrameRx;
  host.channel  = 0;

  std::uint32_t id = frame.frameId();
  if (frame.hasExtendedFrameFormat())
    id = (id & kCanEffMask) | kCanEffFlag;
  else
    id &= kCanSffMask;

  if (frame.frameType() == QCanBusFrame::RemoteRequestFrame)
    id |= kCanRtrFlag;

  host.canId = id;

  const QByteArray payload = frame.payload();
  int wireSize             = m_fdActive ? kFdFrameSize : kClassicFrameSize;
  if (m_fdActive && frame.hasFlexibleDataRateFormat()) {
    const int capped = std::min<int>(payload.size(), kFdPayloadMax);
    host.canDlc      = len2dlc(capped);
    host.flags       = kFrameFlagFd | kFrameFlagBrs;
    std::memcpy(host.data, payload.constData(), static_cast<std::size_t>(capped));
  } else {
    host.canDlc = static_cast<std::uint8_t>(std::min<qsizetype>(payload.size(), 8));
    std::memcpy(host.data, payload.constData(), host.canDlc);
  }

  int transferred = 0;
  int rc          = 0;
  if (m_padTxToMaxPacket && m_outMaxPacket > wireSize) {
    unsigned char padded[kMaxBulkPacketSize] = {};
    SS_ASSERT(m_outMaxPacket <= static_cast<int>(sizeof(padded)), return false);
    std::memcpy(padded, &host, static_cast<std::size_t>(wireSize));
    wireSize = m_outMaxPacket;
    rc =
      libusb_bulk_transfer(m_handle, m_outEndpoint, padded, wireSize, &transferred, kWriteTimeout);
  } else {
    rc = libusb_bulk_transfer(m_handle,
                              m_outEndpoint,
                              reinterpret_cast<unsigned char*>(&host),
                              wireSize,
                              &transferred,
                              kWriteTimeout);
  }

  if (rc < 0 || transferred != wireSize) {
    setError(tr("Failed to transmit CAN frame to the adapter."), QCanBusDevice::WriteError);
    return false;
  }

  m_pendingTx.insert(host.echoId, m_txClock.elapsed());
  return true;
}

/**
 * @brief Produces a human-readable description for a CAN error frame.
 */
QString IO::Drivers::GsUsbCanBackend::interpretErrorFrame(const QCanBusFrame& frame)
{
  if (frame.frameType() != QCanBusFrame::ErrorFrame)
    return {};

  return tr("CAN bus error reported by the CANable adapter.");
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Marshals a fatal read-loop error onto the device thread and disconnects.
 */
void IO::Drivers::GsUsbCanBackend::handleReadError(const QString& reason)
{
  if (state() == QCanBusDevice::UnconnectedState)
    return;

  setError(reason, QCanBusDevice::ReadError);
  close();
}

/**
 * @brief Confirms transmits whose TX echo returned, emitting framesWritten once per echo.
 */
void IO::Drivers::GsUsbCanBackend::completeTransmits(const QList<quint32>& echoIds)
{
  for (const quint32 echoId : echoIds) {
    const auto it = m_pendingTx.find(echoId);
    if (it == m_pendingTx.end())
      continue;

    m_pendingTx.erase(it);
    Q_EMIT framesWritten(1);
  }
}

/**
 * @brief Fails transmits that were never echoed back within the confirmation deadline.
 */
void IO::Drivers::GsUsbCanBackend::checkTxTimeouts()
{
  if (m_pendingTx.isEmpty())
    return;

  const qint64 now = m_txClock.elapsed();
  bool expired     = false;
  for (auto it = m_pendingTx.begin(); it != m_pendingTx.end();) {
    if (now - it.value() < kTxConfirmTimeoutMs) {
      ++it;
      continue;
    }

    it      = m_pendingTx.erase(it);
    expired = true;
  }

  if (expired)
    setError(tr("A CAN frame was not acknowledged on the bus."), QCanBusDevice::WriteError);
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Bulk-read loop: reassembles gs_host_frames, drops TX echoes, enqueues RX frames.
 */
void IO::Drivers::GsUsbCanBackend::readLoop()
{
  unsigned char buffer[kBulkReadBufSize];

  const bool fdMode   = m_fdActive;
  const int frameSize = m_rxFrameSize;

  while (m_running.load(std::memory_order_relaxed)) {
    int transferred = 0;
    const int rc    = libusb_bulk_transfer(
      m_handle, m_inEndpoint, buffer, kBulkReadBufSize, &transferred, kReadTimeoutMs);

    if (rc == LIBUSB_ERROR_TIMEOUT)
      continue;

    if (rc < 0) {
      bool removed = rc == LIBUSB_ERROR_NO_DEVICE;
      if (!removed && rc == LIBUSB_ERROR_IO) {
        int configuration = 0;
        removed = libusb_get_configuration(m_handle, &configuration) == LIBUSB_ERROR_NO_DEVICE;
      }

      const QString reason = removed
                             ? tr("The CANable adapter was disconnected.")
                             : QString::fromUtf8(libusb_strerror(static_cast<libusb_error>(rc)));
      QMetaObject::invokeMethod(
        this, "handleReadError", Qt::QueuedConnection, Q_ARG(QString, reason));
      break;
    }

    if (transferred <= 0)
      continue;

    m_rxCarry.append(reinterpret_cast<const char*>(buffer), transferred);

    const qint64 arrivalUsec = std::chrono::duration_cast<std::chrono::microseconds>(
                                 std::chrono::steady_clock::now().time_since_epoch())
                                 .count();

    QList<quint32> echoIds;
    QList<QCanBusFrame> received;
    while (m_rxCarry.size() >= frameSize) {
      GsHostFrameFd host{};
      std::memcpy(&host, m_rxCarry.constData(), static_cast<std::size_t>(frameSize));
      m_rxCarry.remove(0, frameSize);

      if (host.echoId != kHostFrameRx) {
        echoIds.append(host.echoId);
        continue;
      }

      QCanBusFrame canFrame;
      if (decodeRxFrame(host, fdMode, canFrame)) {
        canFrame.setTimeStamp(QCanBusFrame::TimeStamp::fromMicroSeconds(arrivalUsec));
        received.append(canFrame);
      }
    }

    if (!echoIds.isEmpty())
      QMetaObject::invokeMethod(
        this, [this, echoIds] { completeTransmits(echoIds); }, Qt::QueuedConnection);

    if (received.isEmpty())
      continue;

    // code-verify off
    // enqueueReceivedFrames() is not thread-safe; it must run on the device thread, never here.
    QMetaObject::invokeMethod(
      this, [this, received] { enqueueReceivedFrames(received); }, Qt::QueuedConnection);
    // code-verify on
  }
}

/**
 * @brief Signals the read loop to exit and joins the worker thread.
 */
void IO::Drivers::GsUsbCanBackend::stopReadThread()
{
  m_running.store(false, std::memory_order_release);

  if (m_readThread.isRunning()) {
    m_readThread.quit();
    if (!m_readThread.wait(2000))
      m_readThread.terminate();

    m_readThread.wait();
  }

  disconnect(&m_readThread, &QThread::started, this, &GsUsbCanBackend::readLoop);
}

/**
 * @brief Runs the gs_usb handshake: host format, bit timing, and start mode.
 */
bool IO::Drivers::GsUsbCanBackend::configureDevice(quint32 bitrate)
{
  SS_ASSERT(m_handle != nullptr, return false);

  GsHostConfig config{kHostFormatLE};
  if (controlOut(kBreqHostFormat, 1, &config, sizeof(config)) != static_cast<int>(sizeof(config))) {
    setError(tr("CANable adapter rejected the host-format handshake."),
             QCanBusDevice::ConfigurationError);
    return false;
  }

  GsDeviceBtConst bt{};
  if (controlIn(kBreqBtConst, 0, &bt, sizeof(bt)) != static_cast<int>(sizeof(bt))) {
    setError(tr("Could not read CANable timing constants."), QCanBusDevice::ConfigurationError);
    return false;
  }

  const bool fdRequested = configurationParameter(QCanBusDevice::CanFdKey).toBool();
  if (fdRequested && (bt.feature & kFeatureFd) == 0) {
    setError(tr("The adapter firmware does not support CAN FD. Flash candleLight FD firmware "
                "or disable the flexible data-rate option."),
             QCanBusDevice::ConfigurationError);
    return false;
  }

  GsDeviceBitTiming timing{};
  if (!solveBitTiming(bt, bitrate, timing)) {
    setError(tr("The bitrate %1 bps is not supported by this CANable adapter.").arg(bitrate),
             QCanBusDevice::ConfigurationError);
    return false;
  }

  if (controlOut(kBreqBitTiming, 0, &timing, sizeof(timing)) != static_cast<int>(sizeof(timing))) {
    setError(tr("CANable adapter rejected the requested bitrate."),
             QCanBusDevice::ConfigurationError);
    return false;
  }

  if (fdRequested && !configureFdDataTiming(bt.feature))
    return false;

  m_fdActive         = fdRequested;
  m_rxFrameSize      = fdRequested ? kFdFrameSize : kClassicFrameSize;
  m_padTxToMaxPacket = fdRequested && (bt.feature & kFeaturePadPkts) != 0;

  std::uint32_t modeFlags = kModeBerrReporting;
  if (m_fdActive)
    modeFlags |= kModeFd;

  if (configurationParameter(QCanBusDevice::LoopbackKey).toBool())
    modeFlags |= kModeLoopBack;

  if (configurationParameter(IO::Drivers::kListenOnlyConfigKey).toBool())
    modeFlags |= kModeListenOnly;

  GsDeviceMode mode{kModeStart, modeFlags};
  if (controlOut(kBreqMode, 0, &mode, sizeof(mode)) != static_cast<int>(sizeof(mode))) {
    setError(tr("Could not start the CANable channel."), QCanBusDevice::ConfigurationError);
    return false;
  }

  return true;
}

/**
 * @brief Solves and programs the FD data-phase bit timing; limits come from BT_CONST_EXT when
 *        the firmware advertises it, else fall back to the classic limits (kernel behavior).
 */
bool IO::Drivers::GsUsbCanBackend::configureFdDataTiming(quint32 feature)
{
  SS_ASSERT(m_handle != nullptr, return false);

  const auto configured  = configurationParameter(QCanBusDevice::DataBitRateKey).toUInt();
  const quint32 dataRate = configured == 0 ? kDefaultFdDataBitrate : configured;

  GsDeviceBtConst limits{};
  if ((feature & kFeatureBtConstExt) != 0) {
    GsDeviceBtConstExt ext{};
    if (controlIn(kBreqBtConstExt, 0, &ext, sizeof(ext)) != static_cast<int>(sizeof(ext))) {
      setError(tr("Could not read CANable FD timing constants."),
               QCanBusDevice::ConfigurationError);
      return false;
    }

    limits = timingLimits(ext, true);
  } else if (controlIn(kBreqBtConst, 0, &limits, sizeof(limits))
             != static_cast<int>(sizeof(limits))) {
    setError(tr("Could not read CANable timing constants."), QCanBusDevice::ConfigurationError);
    return false;
  }

  GsDeviceBitTiming timing{};
  if (!solveBitTiming(limits, dataRate, timing)) {
    setError(tr("The data bitrate %1 bps is not supported by this CANable adapter.").arg(dataRate),
             QCanBusDevice::ConfigurationError);
    return false;
  }

  if (controlOut(kBreqDataBitTiming, 0, &timing, sizeof(timing))
      != static_cast<int>(sizeof(timing))) {
    setError(tr("CANable adapter rejected the requested data bitrate."),
             QCanBusDevice::ConfigurationError);
    return false;
  }

  return true;
}

/**
 * @brief Finds the vendor interface and its bulk IN/OUT endpoints, then claims it.
 */
bool IO::Drivers::GsUsbCanBackend::claimGsUsbInterface()
{
  SS_ASSERT(m_handle != nullptr, return false);

  libusb_device* device         = libusb_get_device(m_handle);
  libusb_config_descriptor* cfg = nullptr;
  if (libusb_get_active_config_descriptor(device, &cfg) < 0)
    return false;

  for (int i = 0; i < cfg->bNumInterfaces && m_interfaceNumber < 0; ++i) {
    if (cfg->interface[i].num_altsetting < 1)
      continue;

    const libusb_interface_descriptor& alt = cfg->interface[i].altsetting[0];

    std::uint8_t inEp  = 0;
    std::uint8_t outEp = 0;
    int outMax         = 0;
    for (int e = 0; e < alt.bNumEndpoints; ++e) {
      const libusb_endpoint_descriptor& ep = alt.endpoint[e];
      const bool isBulk =
        (ep.bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_BULK;
      if (!isBulk)
        continue;

      if (ep.bEndpointAddress & LIBUSB_ENDPOINT_IN)
        inEp = ep.bEndpointAddress;
      else {
        outEp  = ep.bEndpointAddress;
        outMax = ep.wMaxPacketSize;
      }
    }

    if (inEp != 0 && outEp != 0) {
      m_interfaceNumber = alt.bInterfaceNumber;
      m_inEndpoint      = inEp;
      m_outEndpoint     = outEp;
      m_outMaxPacket    = std::min(outMax, kMaxBulkPacketSize);
    }
  }

  libusb_free_config_descriptor(cfg);

  if (m_interfaceNumber < 0)
    return false;

  return libusb_claim_interface(m_handle, m_interfaceNumber) == 0;
}

/**
 * @brief Issues a vendor OUT control transfer to the gs_usb interface.
 */
int IO::Drivers::GsUsbCanBackend::controlOut(std::uint8_t request,
                                             std::uint16_t value,
                                             void* data,
                                             std::uint16_t length)
{
  SS_ASSERT(m_handle != nullptr, return LIBUSB_ERROR_NO_DEVICE);
  SS_ASSERT(m_interfaceNumber >= 0, return LIBUSB_ERROR_NOT_FOUND);

  const std::uint8_t type =
    static_cast<std::uint8_t>(static_cast<unsigned>(LIBUSB_REQUEST_TYPE_VENDOR)
                              | static_cast<unsigned>(LIBUSB_RECIPIENT_INTERFACE)
                              | static_cast<unsigned>(LIBUSB_ENDPOINT_OUT));
  return libusb_control_transfer(m_handle,
                                 type,
                                 request,
                                 value,
                                 static_cast<std::uint16_t>(m_interfaceNumber),
                                 reinterpret_cast<unsigned char*>(data),
                                 length,
                                 kCtrlTimeoutMs);
}

/**
 * @brief Issues a vendor IN control transfer from the gs_usb interface.
 */
int IO::Drivers::GsUsbCanBackend::controlIn(std::uint8_t request,
                                            std::uint16_t value,
                                            void* data,
                                            std::uint16_t length)
{
  SS_ASSERT(m_handle != nullptr, return LIBUSB_ERROR_NO_DEVICE);
  SS_ASSERT(m_interfaceNumber >= 0, return LIBUSB_ERROR_NOT_FOUND);

  const std::uint8_t type =
    static_cast<std::uint8_t>(static_cast<unsigned>(LIBUSB_REQUEST_TYPE_VENDOR)
                              | static_cast<unsigned>(LIBUSB_RECIPIENT_INTERFACE)
                              | static_cast<unsigned>(LIBUSB_ENDPOINT_IN));
  return libusb_control_transfer(m_handle,
                                 type,
                                 request,
                                 value,
                                 static_cast<std::uint16_t>(m_interfaceNumber),
                                 reinterpret_cast<unsigned char*>(data),
                                 length,
                                 kCtrlTimeoutMs);
}
