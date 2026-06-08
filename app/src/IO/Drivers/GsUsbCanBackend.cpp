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

#include "IO/Drivers/GsUsbCanBackend.h"

#include <libusb.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <QMetaObject>
#include <QVariant>

//--------------------------------------------------------------------------------------------------
// gs_usb wire protocol (candleLight firmware / Linux kernel gs_usb driver)
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

// gs_usb control requests (bRequest)
constexpr std::uint8_t kBreqHostFormat = 0;
constexpr std::uint8_t kBreqBitTiming  = 1;
constexpr std::uint8_t kBreqMode       = 2;
constexpr std::uint8_t kBreqBtConst    = 4;

// gs_usb mode values
constexpr std::uint32_t kModeReset = 0;
constexpr std::uint32_t kModeStart = 1;

// SocketCAN-style CAN ID flags carried in gs_host_frame::can_id
constexpr std::uint32_t kCanEffFlag = 0x80000000u;
constexpr std::uint32_t kCanRtrFlag = 0x40000000u;
constexpr std::uint32_t kCanErrFlag = 0x20000000u;
constexpr std::uint32_t kCanEffMask = 0x1fffffffu;
constexpr std::uint32_t kCanSffMask = 0x000007ffu;

// Sentinel echo_id marking a received (non-echo) frame
constexpr std::uint32_t kHostFrameRx = 0xffffffffu;

// Host byte-order marker requesting little-endian framing from the device
constexpr std::uint32_t kHostFormatLE = 0x0000beefu;

// Transfer geometry
constexpr int kClassicFrameSize       = 20;
constexpr int kBulkReadBufSize        = 2048;
constexpr unsigned int kReadTimeoutMs = 100;
constexpr unsigned int kCtrlTimeoutMs = 1000;
constexpr unsigned int kWriteTimeout  = 1000;

#pragma pack(push, 1)

struct GsHostConfig {
  std::uint32_t byteOrder;
};

struct GsDeviceMode {
  std::uint32_t mode;
  std::uint32_t flags;
};

struct GsDeviceBitTiming {
  std::uint32_t propSeg;
  std::uint32_t phaseSeg1;
  std::uint32_t phaseSeg2;
  std::uint32_t sjw;
  std::uint32_t brp;
};

struct GsDeviceBtConst {
  std::uint32_t feature;
  std::uint32_t fclkCan;
  std::uint32_t tseg1Min;
  std::uint32_t tseg1Max;
  std::uint32_t tseg2Min;
  std::uint32_t tseg2Max;
  std::uint32_t sjwMax;
  std::uint32_t brpMin;
  std::uint32_t brpMax;
  std::uint32_t brpInc;
};

struct GsHostFrame {
  std::uint32_t echoId;
  std::uint32_t canId;
  std::uint8_t canDlc;
  std::uint8_t channel;
  std::uint8_t flags;
  std::uint8_t reserved;
  std::uint8_t data[8];
};

#pragma pack(pop)

static_assert(sizeof(GsHostFrame) == kClassicFrameSize, "gs_host_frame must be 20 bytes");

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
  Q_ASSERT(device != nullptr);

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
 * @brief Solves gs_usb bit-timing segments for a target bitrate from BT_CONST limits.
 */
[[nodiscard]] static bool solveBitTiming(const GsDeviceBtConst& bt,
                                         std::uint32_t bitrate,
                                         GsDeviceBitTiming& out)
{
  if (bitrate == 0 || bt.fclkCan == 0)
    return false;

  const std::uint32_t step = std::max<std::uint32_t>(1, bt.brpInc);
  for (std::uint32_t brp = bt.brpMin; brp <= bt.brpMax; brp += step) {
    const std::uint32_t divisor = bitrate * brp;
    if (divisor == 0 || (bt.fclkCan % divisor) != 0)
      continue;

    const std::uint32_t total = bt.fclkCan / divisor;
    if (total < 1 + bt.tseg1Min + bt.tseg2Min || total > 1 + bt.tseg1Max + bt.tseg2Max)
      continue;

    std::uint32_t tseg2 = static_cast<std::uint32_t>(std::lround(total * 0.125));
    tseg2               = std::clamp(tseg2, std::max<std::uint32_t>(1, bt.tseg2Min), bt.tseg2Max);
    if (total <= 1 + tseg2)
      continue;

    const std::uint32_t tseg1 = total - 1 - tseg2;
    if (tseg1 < bt.tseg1Min || tseg1 > bt.tseg1Max)
      continue;

    out.phaseSeg2 = tseg2;
    out.phaseSeg1 = std::max<std::uint32_t>(1, tseg1 / 2);
    out.propSeg   = tseg1 - out.phaseSeg1;
    out.sjw       = std::min(bt.sjwMax, tseg2);
    out.brp       = brp;
    return true;
  }

  return false;
}

/**
 * @brief Decodes a received gs_host_frame; returns false for TX echoes that must be dropped.
 */
[[nodiscard]] static bool decodeRxFrame(const GsHostFrame& host, QCanBusFrame& out)
{
  if (host.echoId != kHostFrameRx)
    return false;

  const bool extended    = (host.canId & kCanEffFlag) != 0;
  const bool remote      = (host.canId & kCanRtrFlag) != 0;
  const bool errored     = (host.canId & kCanErrFlag) != 0;
  const std::uint32_t id = host.canId & (extended ? kCanEffMask : kCanSffMask);
  const int dlc          = std::min<int>(host.canDlc, sizeof(host.data));

  out = QCanBusFrame(id, QByteArray(reinterpret_cast<const char*>(host.data), dlc));
  out.setExtendedFrameFormat(extended);
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
  return {pluginKey(), QStringLiteral("CANable USB"), supported(), &availableInterfaces, &create};
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
  libusb_context* ctx = nullptr;
  if (libusb_init(&ctx) < 0)
    return false;

  libusb_exit(ctx);
  return true;
}

/**
 * @brief Enumerates connected gs_usb adapters and returns their interface labels.
 */
QStringList IO::Drivers::GsUsbCanBackend::availableInterfaces()
{
  QStringList interfaces;

  libusb_context* ctx = nullptr;
  if (libusb_init(&ctx) < 0)
    return interfaces;

  libusb_device** devices = nullptr;
  const ssize_t count     = libusb_get_device_list(ctx, &devices);
  if (count < 0) {
    libusb_exit(ctx);
    return interfaces;
  }

  for (ssize_t i = 0; i < count; ++i) {
    libusb_device_descriptor desc{};
    if (libusb_get_device_descriptor(devices[i], &desc) < 0)
      continue;

    if (isGsUsbDevice(desc))
      interfaces.append(deviceLabel(devices[i], desc));
  }

  libusb_free_device_list(devices, 1);
  libusb_exit(ctx);
  return interfaces;
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
  , m_running(false)
{}

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

  if (m_ctx) {
    libusb_exit(m_ctx);
    m_ctx = nullptr;
  }
}

//--------------------------------------------------------------------------------------------------
// QCanBusDevice interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens the selected adapter, programs the bitrate, and starts the read loop.
 */
bool IO::Drivers::GsUsbCanBackend::open()
{
  Q_ASSERT(m_handle == nullptr);

  if (libusb_init(&m_ctx) < 0) {
    setError(tr("Failed to initialize libusb for the CANable adapter."),
             QCanBusDevice::ConnectionError);
    setState(QCanBusDevice::UnconnectedState);
    return false;
  }

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
    libusb_close(m_handle);
    m_handle = nullptr;
    setState(QCanBusDevice::UnconnectedState);
    return false;
  }

  const auto bitrate = configurationParameter(QCanBusDevice::BitRateKey).toUInt();
  if (!configureDevice(bitrate == 0 ? 500000 : bitrate)) {
    libusb_release_interface(m_handle, m_interfaceNumber);
    libusb_close(m_handle);
    m_handle = nullptr;
    setState(QCanBusDevice::UnconnectedState);
    return false;
  }

  m_running.store(true, std::memory_order_release);
  connect(&m_readThread, &QThread::started, this, &GsUsbCanBackend::readLoop, Qt::DirectConnection);
  m_readThread.start();

  setState(QCanBusDevice::ConnectedState);
  return true;
}

/**
 * @brief Stops the adapter, releases the interface, and returns to the unconnected state.
 */
void IO::Drivers::GsUsbCanBackend::close()
{
  setState(QCanBusDevice::ClosingState);

  stopReadThread();

  if (m_handle) {
    GsDeviceMode mode{kModeReset, 0};
    (void)controlOut(kBreqMode, 0, &mode, sizeof(mode));

    if (m_interfaceNumber >= 0)
      libusb_release_interface(m_handle, m_interfaceNumber);

    libusb_close(m_handle);
    m_handle = nullptr;
  }

  if (m_ctx) {
    libusb_exit(m_ctx);
    m_ctx = nullptr;
  }

  m_interfaceNumber = -1;
  m_inEndpoint      = 0;
  m_outEndpoint     = 0;
  m_rxCarry.clear();

  setState(QCanBusDevice::UnconnectedState);
}

/**
 * @brief Encodes a QCanBusFrame as a gs_host_frame and writes it via bulk OUT.
 */
bool IO::Drivers::GsUsbCanBackend::writeFrame(const QCanBusFrame& frame)
{
  if (!m_handle || m_outEndpoint == 0) {
    setError(tr("CANable adapter is not open for writing."), QCanBusDevice::WriteError);
    return false;
  }

  if (!frame.isValid())
    return false;

  GsHostFrame host{};
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
  host.canDlc = static_cast<std::uint8_t>(std::min<int>(payload.size(), sizeof(host.data)));
  std::memcpy(host.data, payload.constData(), host.canDlc);

  int transferred = 0;
  const int rc    = libusb_bulk_transfer(m_handle,
                                      m_outEndpoint,
                                      reinterpret_cast<unsigned char*>(&host),
                                      kClassicFrameSize,
                                      &transferred,
                                      kWriteTimeout);
  if (rc < 0 || transferred != kClassicFrameSize) {
    setError(tr("Failed to transmit CAN frame to the adapter."), QCanBusDevice::WriteError);
    return false;
  }

  Q_EMIT framesWritten(1);
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

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Bulk-read loop: reassembles gs_host_frames, drops TX echoes, enqueues RX frames.
 */
void IO::Drivers::GsUsbCanBackend::readLoop()
{
  unsigned char buffer[kBulkReadBufSize];

  while (m_running.load(std::memory_order_relaxed)) {
    int transferred = 0;
    const int rc    = libusb_bulk_transfer(
      m_handle, m_inEndpoint, buffer, kBulkReadBufSize, &transferred, kReadTimeoutMs);

    if (rc == LIBUSB_ERROR_TIMEOUT)
      continue;

    if (rc < 0) {
      const QString reason = QString::fromUtf8(libusb_strerror(static_cast<libusb_error>(rc)));
      QMetaObject::invokeMethod(
        this, "handleReadError", Qt::QueuedConnection, Q_ARG(QString, reason));
      break;
    }

    if (transferred <= 0)
      continue;

    m_rxCarry.append(reinterpret_cast<const char*>(buffer), transferred);

    QList<QCanBusFrame> received;
    while (m_rxCarry.size() >= kClassicFrameSize) {
      GsHostFrame host{};
      std::memcpy(&host, m_rxCarry.constData(), sizeof(host));
      m_rxCarry.remove(0, kClassicFrameSize);

      QCanBusFrame canFrame;
      if (decodeRxFrame(host, canFrame))
        received.append(canFrame);
    }

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
  Q_ASSERT(m_handle != nullptr);

  GsHostConfig config{kHostFormatLE};
  if (controlOut(kBreqHostFormat, 1, &config, sizeof(config)) < 0) {
    setError(tr("CANable adapter rejected the host-format handshake."),
             QCanBusDevice::ConfigurationError);
    return false;
  }

  GsDeviceBtConst bt{};
  if (controlIn(kBreqBtConst, 0, &bt, sizeof(bt)) < 0) {
    setError(tr("Could not read CANable timing constants."), QCanBusDevice::ConfigurationError);
    return false;
  }

  GsDeviceBitTiming timing{};
  if (!solveBitTiming(bt, bitrate, timing)) {
    setError(tr("The bitrate %1 bps is not supported by this CANable adapter.").arg(bitrate),
             QCanBusDevice::ConfigurationError);
    return false;
  }

  if (controlOut(kBreqBitTiming, 0, &timing, sizeof(timing)) < 0) {
    setError(tr("CANable adapter rejected the requested bitrate."),
             QCanBusDevice::ConfigurationError);
    return false;
  }

  GsDeviceMode mode{kModeStart, 0};
  if (controlOut(kBreqMode, 0, &mode, sizeof(mode)) < 0) {
    setError(tr("Could not start the CANable channel."), QCanBusDevice::ConfigurationError);
    return false;
  }

  return true;
}

/**
 * @brief Finds the vendor interface and its bulk IN/OUT endpoints, then claims it.
 */
bool IO::Drivers::GsUsbCanBackend::claimGsUsbInterface()
{
  Q_ASSERT(m_handle != nullptr);

  libusb_device* device         = libusb_get_device(m_handle);
  libusb_config_descriptor* cfg = nullptr;
  if (libusb_get_active_config_descriptor(device, &cfg) < 0)
    return false;

  for (int i = 0; i < cfg->bNumInterfaces && m_interfaceNumber < 0; ++i) {
    const libusb_interface_descriptor& alt = cfg->interface[i].altsetting[0];

    std::uint8_t inEp  = 0;
    std::uint8_t outEp = 0;
    for (int e = 0; e < alt.bNumEndpoints; ++e) {
      const libusb_endpoint_descriptor& ep = alt.endpoint[e];
      const bool isBulk =
        (ep.bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_BULK;
      if (!isBulk)
        continue;

      if (ep.bEndpointAddress & LIBUSB_ENDPOINT_IN)
        inEp = ep.bEndpointAddress;
      else
        outEp = ep.bEndpointAddress;
    }

    if (inEp != 0 && outEp != 0) {
      m_interfaceNumber = alt.bInterfaceNumber;
      m_inEndpoint      = inEp;
      m_outEndpoint     = outEp;
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
  Q_ASSERT(m_handle != nullptr);

  const std::uint8_t type =
    static_cast<std::uint8_t>(static_cast<unsigned>(LIBUSB_REQUEST_TYPE_VENDOR)
                              | static_cast<unsigned>(LIBUSB_RECIPIENT_INTERFACE)
                              | static_cast<unsigned>(LIBUSB_ENDPOINT_OUT));
  return libusb_control_transfer(m_handle,
                                 type,
                                 request,
                                 value,
                                 0,
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
  Q_ASSERT(m_handle != nullptr);

  const std::uint8_t type =
    static_cast<std::uint8_t>(static_cast<unsigned>(LIBUSB_REQUEST_TYPE_VENDOR)
                              | static_cast<unsigned>(LIBUSB_RECIPIENT_INTERFACE)
                              | static_cast<unsigned>(LIBUSB_ENDPOINT_IN));
  return libusb_control_transfer(m_handle,
                                 type,
                                 request,
                                 value,
                                 0,
                                 reinterpret_cast<unsigned char*>(data),
                                 length,
                                 kCtrlTimeoutMs);
}
