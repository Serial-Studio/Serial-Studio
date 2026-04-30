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

#include "IO/Drivers/USB.h"

#include <QApplication>
#include <QJsonObject>
#include <QMessageBox>
#include <QMetaObject>
#include <QTimer>

#include "Misc/Utilities.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

constexpr unsigned int kBulkReadTimeout  = 100;
constexpr unsigned int kBulkWriteTimeout = 1000;
constexpr int kBulkReadBufSize           = 65536;
constexpr int kDefaultIsoPacketSize      = 1024;
constexpr int kIsoNumTransfers           = 8;
constexpr int kIsoPacketsPerTransfer     = 8;
constexpr int kHotplugFallbackIntervalMs = 2000;

//--------------------------------------------------------------------------------------------------
// Constructor, destructor & singleton
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the USB driver, initializes libusb, and registers hotplug detection.
 */
IO::Drivers::USB::USB()
  : m_ctx(nullptr)
  , m_handle(nullptr)
  , m_hotplugHandle(0)
  , m_deviceIndex(0)
  , m_inEndpointIndex(0)
  , m_outEndpointIndex(0)
  , m_claimedInterface(-1)
  , m_isoPacketSize(kDefaultIsoPacketSize)
  , m_transferMode(TransferMode::BulkStream)
  , m_running(false)
  , m_eventLoopRunning(false)
  , m_activeInEp(0)
  , m_activeOutEp(0)
{
  // Initialize libusb context
  if (libusb_init(&m_ctx) < 0)
    m_ctx = nullptr;

  // Restore persisted settings
  m_deviceIndex      = m_settings.value("USB/deviceIndex", 0).toInt();
  m_inEndpointIndex  = m_settings.value("USB/inEndpointIndex", 0).toInt();
  m_outEndpointIndex = m_settings.value("USB/outEndpointIndex", 0).toInt();
  m_isoPacketSize    = m_settings.value("USB/isoPacketSize", kDefaultIsoPacketSize).toInt();
  m_transferMode     = static_cast<TransferMode>(m_settings.value("USB/transferMode", 0).toInt());

  // Run initial device enumeration
  enumerateDevices();

  // Register hotplug callback or fall back to polling timer
  if (m_ctx && libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
    libusb_hotplug_register_callback(
      m_ctx,
      static_cast<libusb_hotplug_event>(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED
                                        | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT),
      LIBUSB_HOTPLUG_NO_FLAGS,
      LIBUSB_HOTPLUG_MATCH_ANY,
      LIBUSB_HOTPLUG_MATCH_ANY,
      LIBUSB_HOTPLUG_MATCH_ANY,
      &USB::hotplugCallback,
      this,
      &m_hotplugHandle);
  } else {
    auto* timer = new QTimer(this);
    timer->setInterval(kHotplugFallbackIntervalMs);
    connect(timer, &QTimer::timeout, this, &USB::enumerateDevices);
    timer->start();
  }

  // Start the event thread for hotplug and transfer completion
  m_eventLoopRunning = true;
  connect(&m_eventThread, &QThread::started, this, &USB::eventLoop, Qt::DirectConnection);
  m_eventThread.start();
}

/**
 * @brief Stops worker threads, releases libusb resources, and closes the device.
 */
IO::Drivers::USB::~USB()
{
  // Signal both threads to stop
  m_running          = false;
  m_eventLoopRunning = false;

  // Deregister hotplug and cancel pending transfers
  if (m_ctx && m_hotplugHandle) {
    libusb_hotplug_deregister_callback(m_ctx, m_hotplugHandle);
    m_hotplugHandle = 0;
  }

  for (auto* t : std::as_const(m_isoTransfers))
    libusb_cancel_transfer(t);

  if (m_readThread.isRunning()) {
    if (!m_readThread.wait(2000))
      m_readThread.terminate();

    m_readThread.wait();
  }

  if (m_eventThread.isRunning()) {
    if (!m_eventThread.wait(2000))
      m_eventThread.terminate();

    m_eventThread.wait();
  }

  for (auto* t : std::as_const(m_isoTransfers))
    libusb_free_transfer(t);

  for (auto* dev : std::as_const(m_devicePtrs))
    libusb_unref_device(dev);

  if (m_handle) {
    releaseInterface();
    libusb_close(m_handle);
    m_handle = nullptr;
  }

  if (m_ctx)
    libusb_exit(m_ctx);
}

//--------------------------------------------------------------------------------------------------
// HAL_Driver interface
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens the selected USB device and starts the appropriate transfer loop.
 */
bool IO::Drivers::USB::open(const QIODevice::OpenMode mode)
{
  Q_UNUSED(mode)
  Q_ASSERT(configurationOk());
  Q_ASSERT(m_ctx != nullptr);

  // Abort if libusb is not available
  if (!m_ctx) {
    Misc::Utilities::showMessageBox(tr("USB Error"),
                                    tr("Failed to initialize the USB subsystem. "
                                       "Check that libusb is available on your system."),
                                    QMessageBox::Critical);
    return false;
  }

  // Require a real device to be selected
  if (m_deviceIndex <= 0 || (m_deviceIndex - 1) >= m_devicePtrs.size()) {
    Misc::Utilities::showMessageBox(tr("USB Error"),
                                    tr("No USB device selected. Select a device and try again."),
                                    QMessageBox::Critical);
    return false;
  }

  // Open device handle
  libusb_device* dev     = m_devicePtrs.at(m_deviceIndex - 1);
  const auto deviceLabel = m_deviceLabels.value(m_deviceIndex - 1, tr("Unknown Device"));
  const int openRc       = libusb_open(dev, &m_handle);
  if (openRc < 0) {
    m_handle = nullptr;
    Misc::Utilities::showMessageBox(
      tr("Failed to open \"%1\"").arg(deviceLabel),
      tr("Could not open the USB device: %1.\n\n"
         "On Linux, ensure you have read/write permission on the device node "
         "(add a udev rule or run as root). "
         "On macOS, the kernel driver may need to be detached first.")
        .arg(QString::fromUtf8(libusb_strerror(static_cast<libusb_error>(openRc)))),
      QMessageBox::Critical);
    return false;
  }

#ifdef __linux__
  libusb_set_auto_detach_kernel_driver(m_handle, 1);
#endif

  // Scan endpoints and populate comboboxes
  buildEndpointLists();
  Q_EMIT endpointListChanged();
  Q_EMIT inEndpointIndexChanged();
  Q_EMIT outEndpointIndexChanged();

  // At least one IN endpoint is required
  if (m_inEndpointIndex <= 0 || (m_inEndpointIndex - 1) >= m_inEndpoints.size()) {
    libusb_close(m_handle);
    m_handle = nullptr;
    Misc::Utilities::showMessageBox(
      tr("USB Device Error"), endpointErrorMessage(), QMessageBox::Critical);
    return false;
  }

  // Resolve the selected endpoint addresses
  const EndpointInfo& inEp = m_inEndpoints.at(m_inEndpointIndex - 1);
  m_activeInEp             = inEp.address;

  if (m_outEndpointIndex > 0 && (m_outEndpointIndex - 1) < m_outEndpoints.size())
    m_activeOutEp = m_outEndpoints.at(m_outEndpointIndex - 1).address;
  else
    m_activeOutEp = 0;

  // Claim the interface that owns the selected IN endpoint
  if (!claimInterface(inEp.interfaceNumber)) {
    libusb_close(m_handle);
    m_handle = nullptr;
    Misc::Utilities::showMessageBox(tr("USB Device Error"),
                                    tr("Could not claim interface %1 on the USB device.\n\n"
                                       "Another driver or application may already have it open. "
                                       "On Linux, try unloading the kernel driver (e.g. cdc_acm) "
                                       "or adding a udev rule.")
                                      .arg(inEp.interfaceNumber),
                                    QMessageBox::Critical);
    return false;
  }

  // Start read loop based on transfer mode
  m_running = true;

  if (m_transferMode == TransferMode::Isochronous) {
    allocateIsoTransfers();
    connect(&m_readThread, &QThread::started, this, &USB::isoReadLoop, Qt::DirectConnection);
  } else {
    connect(&m_readThread, &QThread::started, this, &USB::readLoop, Qt::DirectConnection);
  }

  m_readThread.start();

  Q_EMIT configurationChanged();
  return true;
}

/**
 * @brief Closes the device, tears down all active transfers, and stops the read thread.
 */
void IO::Drivers::USB::close()
{
  Q_ASSERT(m_ctx != nullptr);
  Q_ASSERT(m_activeInEp != 0 || m_handle == nullptr);

  // Signal the read loop to exit
  m_running = false;

  // Cancel in-flight isochronous transfers so the event loop can drain
  for (auto* t : std::as_const(m_isoTransfers))
    libusb_cancel_transfer(t);

  // Wait for read thread to finish
  if (m_readThread.isRunning()) {
    if (!m_readThread.wait(2000))
      m_readThread.terminate();

    m_readThread.wait();
  }

  // Free the isochronous transfer pool
  for (auto* t : std::as_const(m_isoTransfers))
    libusb_free_transfer(t);

  m_isoTransfers.clear();

  // Detach thread-started connections for the next open() cycle
  disconnect(&m_readThread, &QThread::started, this, &USB::readLoop);
  disconnect(&m_readThread, &QThread::started, this, &USB::isoReadLoop);

  // Release the claimed interface and close the device handle
  if (m_handle) {
    releaseInterface();
    libusb_close(m_handle);
    m_handle = nullptr;
  }

  m_activeInEp  = 0;
  m_activeOutEp = 0;

  Q_EMIT configurationChanged();
}

/**
 * @brief Returns true when a USB device handle is open.
 */
bool IO::Drivers::USB::isOpen() const noexcept
{
  return m_handle != nullptr;
}

/**
 * @brief Returns true if the device is open and an IN endpoint is active.
 */
bool IO::Drivers::USB::isReadable() const noexcept
{
  return m_handle != nullptr && m_activeInEp != 0;
}

/**
 * @brief Returns true if the device is open and an OUT endpoint is active.
 */
bool IO::Drivers::USB::isWritable() const noexcept
{
  return m_handle != nullptr && m_activeOutEp != 0;
}

/**
 * @brief Returns true when a device is selected and the connect button should be enabled.
 */
bool IO::Drivers::USB::configurationOk() const noexcept
{
  return m_deviceIndex > 0 && (m_deviceIndex - 1) < m_devicePtrs.size();
}

/**
 * @brief Sends @p data to the device via a synchronous bulk OUT transfer.
 */
qint64 IO::Drivers::USB::write(const QByteArray& data)
{
  Q_ASSERT(!data.isEmpty());
  Q_ASSERT(m_handle != nullptr);

  if (!isWritable())
    return -1;

  // Mutable copy needed because libusb may write to the buffer
  int transferred = 0;
  QByteArray mutableData(data);
  auto* buf = reinterpret_cast<unsigned char*>(mutableData.data());

  const int rc = libusb_bulk_transfer(
    m_handle, m_activeOutEp, buf, static_cast<int>(data.size()), &transferred, kBulkWriteTimeout);
  if (rc < 0)
    return -1;

  Q_EMIT dataSent(data.left(transferred));
  return static_cast<qint64>(transferred);
}

//--------------------------------------------------------------------------------------------------
// Property getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the current transfer mode as an integer (0 = BulkStream, 1 = AdvancedControl, 2 =
 * Isochronous).
 */
int IO::Drivers::USB::transferMode() const
{
  return static_cast<int>(m_transferMode);
}

/**
 * @brief Returns true when Advanced (Bulk + Control) mode is active.
 */
bool IO::Drivers::USB::advancedModeEnabled() const
{
  return m_transferMode == TransferMode::AdvancedControl;
}

/**
 * @brief Returns true when Isochronous mode is active.
 */
bool IO::Drivers::USB::isoModeEnabled() const
{
  return m_transferMode == TransferMode::Isochronous;
}

/**
 * @brief Returns the device list with a "Select Device" placeholder at index 0.
 */
QStringList IO::Drivers::USB::deviceList() const
{
  QStringList list;
  list.append(tr("Select Device"));
  list.append(m_deviceLabels);
  return list;
}

/**
 * @brief Returns the index of the currently selected USB device.
 */
int IO::Drivers::USB::deviceIndex() const
{
  return m_deviceIndex;
}

/**
 * @brief Returns the IN endpoint list with a placeholder entry at index 0.
 */
QStringList IO::Drivers::USB::inEndpointList() const
{
  QStringList list;
  list.append(tr("Select IN Endpoint"));
  list.append(m_inEndpointLabels);
  return list;
}

/**
 * @brief Returns the OUT endpoint list with a "None (Read-only)" entry at index 0.
 */
QStringList IO::Drivers::USB::outEndpointList() const
{
  QStringList list;
  list.append(tr("None (Read-only)"));
  list.append(m_outEndpointLabels);
  return list;
}

/**
 * @brief Returns the index of the currently selected IN endpoint.
 */
int IO::Drivers::USB::inEndpointIndex() const
{
  return m_inEndpointIndex;
}

/**
 * @brief Returns the index of the currently selected OUT endpoint.
 */
int IO::Drivers::USB::outEndpointIndex() const
{
  return m_outEndpointIndex;
}

/**
 * @brief Returns the isochronous packet size in bytes.
 */
int IO::Drivers::USB::isoPacketSize() const
{
  return m_isoPacketSize;
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Selects a device by combo index and resets cached endpoint data.
 */
void IO::Drivers::USB::setDeviceIndex(const int index)
{
  if (m_deviceIndex == index)
    return;

  if (m_devicePtrs.isEmpty())
    enumerateDevices();

  if (index > 0 && (index - 1) >= m_devicePtrs.size())
    return;

  m_deviceIndex      = index;
  m_inEndpointIndex  = 0;
  m_outEndpointIndex = 0;

  clearEndpointLists();

  m_settings.setValue("USB/deviceIndex", index);
  m_settings.setValue("USB/inEndpointIndex", 0);
  m_settings.setValue("USB/outEndpointIndex", 0);

  Q_EMIT deviceIndexChanged();
  Q_EMIT endpointListChanged();
  Q_EMIT inEndpointIndexChanged();
  Q_EMIT outEndpointIndexChanged();
  Q_EMIT configurationChanged();
}

/**
 * @brief Sets the transfer mode, prompting for confirmation on AdvancedControl.
 */
void IO::Drivers::USB::setTransferMode(const int mode)
{
  // Require user confirmation before enabling AdvancedControl
  const auto requested = static_cast<TransferMode>(mode);

  if (requested == TransferMode::AdvancedControl
      && m_transferMode != TransferMode::AdvancedControl) {
    const int choice = Misc::Utilities::showMessageBox(
      tr("Enable Advanced USB Control Transfers?"),
      tr("This enables control transfers in addition to bulk transfers. "
         "Sending incorrect control requests can crash or damage connected "
         "hardware. Only enable this if you know what you are doing."),
      QMessageBox::Warning,
      tr("Advanced USB Mode"),
      QMessageBox::Yes | QMessageBox::No,
      QMessageBox::No);

    if (choice != QMessageBox::Yes) {
      Q_EMIT transferModeChanged();
      return;
    }
  }

  m_transferMode = requested;
  m_settings.setValue("USB/transferMode", mode);

  // Stale endpoints from previous mode are no longer valid
  clearEndpointLists();
  m_inEndpointIndex  = 0;
  m_outEndpointIndex = 0;

  Q_EMIT endpointListChanged();
  Q_EMIT inEndpointIndexChanged();
  Q_EMIT outEndpointIndexChanged();
  Q_EMIT transferModeChanged();
  Q_EMIT configurationChanged();
}

/**
 * @brief Sets the active IN endpoint by combo index.
 */
void IO::Drivers::USB::setInEndpointIndex(const int index)
{
  if (m_inEndpointIndex == index)
    return;

  m_inEndpointIndex = index;
  m_settings.setValue("USB/inEndpointIndex", index);

  Q_EMIT inEndpointIndexChanged();
  Q_EMIT configurationChanged();
}

/**
 * @brief Sets the active OUT endpoint by combo index (0 = none, receive-only).
 */
void IO::Drivers::USB::setOutEndpointIndex(const int index)
{
  if (m_outEndpointIndex == index)
    return;

  m_outEndpointIndex = index;
  m_settings.setValue("USB/outEndpointIndex", index);

  Q_EMIT outEndpointIndexChanged();
  Q_EMIT configurationChanged();
}

/**
 * @brief Sets the isochronous packet size in bytes.
 */
void IO::Drivers::USB::setIsoPacketSize(const int size)
{
  if (m_isoPacketSize == size || size <= 0)
    return;

  m_isoPacketSize = size;
  m_settings.setValue("USB/isoPacketSize", size);

  Q_EMIT isoPacketSizeChanged();
  Q_EMIT configurationChanged();
}

/**
 * @brief Connects the driver to application-level lifecycle signals.
 */
void IO::Drivers::USB::setupExternalConnections()
{
  connect(qApp, &QApplication::aboutToQuit, this, [this] {
    m_running          = false;
    m_eventLoopRunning = false;

    // Deregister hotplug first to wake the event loop out of its poll
    if (m_ctx && m_hotplugHandle) {
      libusb_hotplug_deregister_callback(m_ctx, m_hotplugHandle);
      m_hotplugHandle = 0;
    }

    for (auto* t : std::as_const(m_isoTransfers))
      libusb_cancel_transfer(t);

    if (m_readThread.isRunning()) {
      if (!m_readThread.wait(2000))
        m_readThread.terminate();

      m_readThread.wait();
    }

    if (m_eventThread.isRunning()) {
      if (!m_eventThread.wait(2000))
        m_eventThread.terminate();

      m_eventThread.wait();
    }
  });
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles a fatal read error by closing the device on the main thread.
 */
void IO::Drivers::USB::onReadError()
{
  close();
}

/**
 * @brief Scans the USB bus and rebuilds the device list.
 */
void IO::Drivers::USB::enumerateDevices()
{
  if (!m_ctx)
    return;

  // Save previous labels for change detection
  const QStringList previous = m_deviceLabels;
  for (auto* dev : std::as_const(m_devicePtrs))
    libusb_unref_device(dev);

  m_devicePtrs.clear();
  m_deviceLabels.clear();

  libusb_device** devs = nullptr;
  const ssize_t count  = libusb_get_device_list(m_ctx, &devs);

  if (count < 0) {
    if (!previous.isEmpty()) {
      Q_EMIT deviceListChanged();
      Q_EMIT configurationChanged();
    }

    return;
  }

  for (ssize_t i = 0; i < count; ++i) {
    libusb_device* dev = devs[i];
    libusb_device_descriptor desc{};

    if (libusb_get_device_descriptor(dev, &desc) < 0)
      continue;

    QString label = QStringLiteral("%1:%2")
                      .arg(desc.idVendor, 4, 16, QChar('0'))
                      .arg(desc.idProduct, 4, 16, QChar('0'))
                      .toUpper();

    libusb_device_handle* tmp = nullptr;
    if (libusb_open(dev, &tmp) == 0) {
      auto fetchStr = [&](uint8_t idx) -> QString {
        if (!idx)
          return {};

        unsigned char buf[256] = {};
        const int rc =
          libusb_get_string_descriptor_ascii(tmp, idx, buf, static_cast<int>(sizeof(buf)));

        if (rc > 0)
          return QString::fromLatin1(reinterpret_cast<const char*>(buf), rc).trimmed();

        return {};
      };

      const QString mfr  = fetchStr(desc.iManufacturer);
      const QString prod = fetchStr(desc.iProduct);

      if (!mfr.isEmpty() || !prod.isEmpty())
        label += QStringLiteral(" – %1 %2").arg(mfr, prod).trimmed();

      libusb_close(tmp);
    }

    libusb_ref_device(dev);
    m_devicePtrs.append(dev);
    m_deviceLabels.append(label);
  }

  libusb_free_device_list(devs, 1);

  if (m_deviceIndex > m_devicePtrs.size())
    m_deviceIndex = 0;

  if (m_deviceLabels != previous) {
    Q_EMIT deviceListChanged();
    Q_EMIT configurationChanged();
  }
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true if the device's active configuration exposes at least one endpoint of @p
 * targetType.
 */
static bool configHasTransferType(const libusb_config_descriptor* cfg, uint8_t targetType)
{
  for (int i = 0; i < cfg->bNumInterfaces; ++i) {
    const libusb_interface& iface = cfg->interface[i];
    for (int a = 0; a < iface.num_altsetting; ++a) {
      const libusb_interface_descriptor& alt = iface.altsetting[a];
      for (int e = 0; e < alt.bNumEndpoints; ++e) {
        const uint8_t type = alt.endpoint[e].bmAttributes & LIBUSB_TRANSFER_TYPE_MASK;
        if (type == targetType)
          return true;
      }
    }
  }

  return false;
}

/**
 * @brief Builds an actionable error message when no IN endpoint is found.
 */
QString IO::Drivers::USB::endpointErrorMessage() const
{
  // Detect whether switching transfer mode would help
  const bool wantIso = (m_transferMode == TransferMode::Isochronous);
  bool hasOtherType  = false;

  libusb_config_descriptor* cfg = nullptr;
  if (libusb_get_active_config_descriptor(m_devicePtrs.at(m_deviceIndex - 1), &cfg) == 0) {
    const uint8_t otherType =
      wantIso ? LIBUSB_TRANSFER_TYPE_BULK : LIBUSB_TRANSFER_TYPE_ISOCHRONOUS;
    hasOtherType = configHasTransferType(cfg, otherType);
    libusb_free_config_descriptor(cfg);
  }

  if (hasOtherType && wantIso)
    return tr("No isochronous IN endpoint was found on this device, but bulk "
              "endpoints are available.\n\n"
              "Switch the Transfer Mode to \"Bulk Stream\" and try again.");

  if (hasOtherType && !wantIso)
    return tr("No bulk IN endpoint was found on this device, but isochronous "
              "endpoints are available.\n\n"
              "Switch the Transfer Mode to \"Isochronous\" and try again.");

  return tr("No usable IN endpoint was found on this device.\n\n"
            "The device may not expose data endpoints in its active "
            "configuration, or it may require a specific driver.");
}

/**
 * @brief Inspects a single endpoint descriptor and appends it to the IN or OUT list if it matches
 * the mode.
 */
void IO::Drivers::USB::collectEndpoint(const libusb_endpoint_descriptor& ep,
                                       int ifNum,
                                       bool wantIso)
{
  const uint8_t type = ep.bmAttributes & LIBUSB_TRANSFER_TYPE_MASK;
  const bool isBulk  = (type == LIBUSB_TRANSFER_TYPE_BULK);
  const bool isIso   = (type == LIBUSB_TRANSFER_TYPE_ISOCHRONOUS);

  if (wantIso && !isIso)
    return;

  if (!wantIso && !isBulk)
    return;

  const bool isIn       = (ep.bEndpointAddress & LIBUSB_ENDPOINT_IN) != 0;
  const QString typeStr = isIso ? QStringLiteral("Iso") : QStringLiteral("Bulk");
  const QString dirStr  = isIn ? QStringLiteral("IN") : QStringLiteral("OUT");

  EndpointInfo info;
  info.address         = ep.bEndpointAddress;
  info.attributes      = ep.bmAttributes;
  info.maxPacketSize   = ep.wMaxPacketSize;
  info.interfaceNumber = ifNum;
  const QString epHex  = QString::number(ep.bEndpointAddress, 16).toUpper().rightJustified(2, '0');
  info.label           = QStringLiteral("EP 0x%1 – %2 %3  (IF%4, max %5 B)")
                 .arg(epHex, typeStr, dirStr)
                 .arg(ifNum)
                 .arg(ep.wMaxPacketSize);

  if (isIn) {
    m_inEndpoints.append(info);
    m_inEndpointLabels.append(info.label);
  } else {
    m_outEndpoints.append(info);
    m_outEndpointLabels.append(info.label);
  }
}

/**
 * @brief Scans all interfaces and endpoints of the selected device and populates the IN/OUT lists.
 */
void IO::Drivers::USB::buildEndpointLists()
{
  clearEndpointLists();

  if (m_deviceIndex <= 0 || (m_deviceIndex - 1) >= m_devicePtrs.size())
    return;

  // Walk all interfaces on the active configuration
  libusb_device* dev               = m_devicePtrs.at(m_deviceIndex - 1);
  libusb_config_descriptor* config = nullptr;

  if (libusb_get_active_config_descriptor(dev, &config) < 0)
    return;

  const bool wantIso = (m_transferMode == TransferMode::Isochronous);

  for (int i = 0; i < config->bNumInterfaces; ++i) {
    const libusb_interface& iface = config->interface[i];

    for (int a = 0; a < iface.num_altsetting; ++a) {
      const libusb_interface_descriptor& alt = iface.altsetting[a];

      for (int e = 0; e < alt.bNumEndpoints; ++e)
        collectEndpoint(alt.endpoint[e], alt.bInterfaceNumber, wantIso);
    }
  }

  libusb_free_config_descriptor(config);

  // Clamp saved indices to new list sizes
  if (m_inEndpointIndex > m_inEndpoints.size())
    m_inEndpointIndex = 0;

  if (m_outEndpointIndex > m_outEndpoints.size())
    m_outEndpointIndex = 0;

  // Auto-select the first available endpoint when nothing is selected yet
  if (m_inEndpointIndex == 0 && !m_inEndpoints.isEmpty())
    m_inEndpointIndex = 1;

  if (m_outEndpointIndex == 0 && !m_outEndpoints.isEmpty())
    m_outEndpointIndex = 1;

  // Auto-suggest the endpoint's max packet size into the isochronous packet size field
  if (wantIso && m_inEndpointIndex > 0) {
    const int suggested = m_inEndpoints.at(m_inEndpointIndex - 1).maxPacketSize;
    if (suggested > 0 && suggested != m_isoPacketSize) {
      m_isoPacketSize = suggested;
      Q_EMIT isoPacketSizeChanged();
    }
  }
}

/**
 * @brief Runs the libusb event loop on m_eventThread.
 */
void IO::Drivers::USB::eventLoop()
{
  while (m_eventLoopRunning.load(std::memory_order_relaxed)) {
    struct timeval tv = {0, 100000};
    libusb_handle_events_timeout(m_ctx, &tv);
  }
}

/**
 * @brief Static libusb hotplug callback invoked when a device arrives or leaves.
 */
int LIBUSB_CALL IO::Drivers::USB::hotplugCallback(libusb_context*,
                                                  libusb_device*,
                                                  libusb_hotplug_event,
                                                  void* user_data)
{
  auto* self = static_cast<USB*>(user_data);
  QMetaObject::invokeMethod(self, &USB::enumerateDevices, Qt::QueuedConnection);
  return 0;
}

/**
 * @brief Clears all endpoint lists and their corresponding label lists.
 */
void IO::Drivers::USB::clearEndpointLists()
{
  m_inEndpoints.clear();
  m_outEndpoints.clear();
  m_inEndpointLabels.clear();
  m_outEndpointLabels.clear();
}

/**
 * @brief Claims @p ifaceNum on the open device handle.
 */
bool IO::Drivers::USB::claimInterface(int ifaceNum)
{
  if (libusb_claim_interface(m_handle, ifaceNum) < 0)
    return false;

  m_claimedInterface = ifaceNum;
  return true;
}

/**
 * @brief Releases the currently claimed interface.
 */
void IO::Drivers::USB::releaseInterface()
{
  if (m_handle && m_claimedInterface >= 0) {
    libusb_release_interface(m_handle, m_claimedInterface);
    m_claimedInterface = -1;
  }
}

/**
 * @brief Synchronous bulk read loop for BulkStream and AdvancedControl modes.
 */
void IO::Drivers::USB::readLoop()
{
  unsigned char buf[kBulkReadBufSize];

  while (m_running.load(std::memory_order_relaxed)) {
    int transferred = 0;
    const int rc    = libusb_bulk_transfer(
      m_handle, m_activeInEp, buf, kBulkReadBufSize, &transferred, kBulkReadTimeout);

    if (rc == LIBUSB_ERROR_TIMEOUT)
      continue;

    if (rc == 0 && transferred > 0) {
      publishReceivedData(QByteArray(reinterpret_cast<const char*>(buf), transferred));
      continue;
    }

    if (rc != 0) {
      QMetaObject::invokeMethod(this, "onReadError", Qt::QueuedConnection);
      break;
    }
  }
}

/**
 * @brief Allocates and submits the isochronous transfer pool on the main thread.
 */
void IO::Drivers::USB::allocateIsoTransfers()
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
                             m_activeInEp,
                             buf,
                             totalBufSize,
                             kIsoPacketsPerTransfer,
                             &USB::isoTransferCallback,
                             this,
                             0);

    libusb_set_iso_packet_lengths(t, static_cast<unsigned int>(m_isoPacketSize));

    if (libusb_submit_transfer(t) < 0) {
      delete[] buf;
      libusb_free_transfer(t);
    } else {
      m_isoTransfers.append(t);
    }
  }
}

/**
 * @brief Async isochronous read loop for Isochronous mode.
 */
void IO::Drivers::USB::isoReadLoop()
{
  while (m_running.load(std::memory_order_relaxed))
    QThread::msleep(10);
}

/**
 * @brief Static libusb callback invoked for each completed isochronous transfer.
 */
void LIBUSB_CALL IO::Drivers::USB::isoTransferCallback(libusb_transfer* transfer)
{
  auto* self = static_cast<USB*>(transfer->user_data);

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
      // Stamp at acquisition, before queueing to the main thread
      const auto timestamp = IO::CapturedData::SteadyClock::now();
      QMetaObject::invokeMethod(
        self,
        [self, received, timestamp] { self->publishReceivedData(received, timestamp); },
        Qt::QueuedConnection);
    }
  }

  if (self->m_running.load(std::memory_order_relaxed))
    libusb_submit_transfer(transfer);
  else
    delete[] transfer->buffer;
}

/**
 * @brief Issues a USB control transfer (AdvancedControl mode only).
 */
qint64 IO::Drivers::USB::sendControlTransfer(uint8_t bmRequestType,
                                             uint8_t bRequest,
                                             uint16_t wValue,
                                             uint16_t wIndex,
                                             const QByteArray& data,
                                             unsigned int timeout_ms)
{
  Q_ASSERT(m_handle != nullptr);
  Q_ASSERT(advancedModeEnabled());

  if (!advancedModeEnabled() || !m_handle)
    return -1;

  // Mutable copy needed because libusb may write to the buffer
  QByteArray mutableData(data);
  auto* buf = reinterpret_cast<unsigned char*>(mutableData.data());

  const int rc = libusb_control_transfer(m_handle,
                                         bmRequestType,
                                         bRequest,
                                         wValue,
                                         wIndex,
                                         buf,
                                         static_cast<uint16_t>(data.size()),
                                         timeout_ms);
  return rc < 0 ? -1 : static_cast<qint64>(rc);
}

//--------------------------------------------------------------------------------------------------
// Stable device identification
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns VID, PID, and serial number of the currently selected USB device.
 */
QJsonObject IO::Drivers::USB::deviceIdentifier() const
{
  if (!m_ctx || m_deviceIndex < 1 || m_deviceIndex > m_devicePtrs.size())
    return {};

  auto* dev = m_devicePtrs.at(m_deviceIndex - 1);
  libusb_device_descriptor desc{};
  if (libusb_get_device_descriptor(dev, &desc) < 0)
    return {};

  QJsonObject id;
  id.insert(QStringLiteral("vid"),
            QString::number(desc.idVendor, 16).rightJustified(4, '0').toUpper());
  id.insert(QStringLiteral("pid"),
            QString::number(desc.idProduct, 16).rightJustified(4, '0').toUpper());

  // Append serial number if the device exposes one
  libusb_device_handle* tmp = nullptr;
  if (desc.iSerialNumber && libusb_open(dev, &tmp) == 0) {
    unsigned char buf[256] = {};
    const int rc           = libusb_get_string_descriptor_ascii(
      tmp, desc.iSerialNumber, buf, static_cast<int>(sizeof(buf)));

    if (rc > 0) {
      id.insert(QStringLiteral("serial"),
                QString::fromLatin1(reinterpret_cast<const char*>(buf), rc).trimmed());
    }

    libusb_close(tmp);
  }

  return id;
}

/**
 * @brief Tries to find and select a USB device matching a previously saved VID/PID/serial.
 */
bool IO::Drivers::USB::selectByIdentifier(const QJsonObject& id)
{
  if (id.isEmpty() || !m_ctx)
    return false;

  if (m_devicePtrs.isEmpty())
    enumerateDevices();

  const auto savedVid = id.value(QStringLiteral("vid")).toString();
  const auto savedPid = id.value(QStringLiteral("pid")).toString();
  const auto savedSer = id.value(QStringLiteral("serial")).toString();

  if (savedVid.isEmpty() || savedPid.isEmpty())
    return false;

  for (int i = 0; i < m_devicePtrs.size(); ++i) {
    libusb_device_descriptor desc{};
    if (libusb_get_device_descriptor(m_devicePtrs.at(i), &desc) < 0)
      continue;

    const auto vid = QString::number(desc.idVendor, 16).rightJustified(4, '0').toUpper();
    const auto pid = QString::number(desc.idProduct, 16).rightJustified(4, '0').toUpper();

    if (vid != savedVid || pid != savedPid)
      continue;

    // Narrow match by serial number when available
    if (!savedSer.isEmpty() && desc.iSerialNumber) {
      libusb_device_handle* tmp = nullptr;
      if (libusb_open(m_devicePtrs.at(i), &tmp) == 0) {
        unsigned char buf[256] = {};
        const int rc           = libusb_get_string_descriptor_ascii(
          tmp, desc.iSerialNumber, buf, static_cast<int>(sizeof(buf)));
        libusb_close(tmp);

        if (rc > 0) {
          const auto serial = QString::fromLatin1(reinterpret_cast<const char*>(buf), rc).trimmed();
          if (serial != savedSer)
            continue;
        }
      }
    }

    setDeviceIndex(i + 1);
    return true;
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Driver property model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the USB configuration as a flat list of editable properties.
 */
QList<IO::DriverProperty> IO::Drivers::USB::driverProperties() const
{
  QList<IO::DriverProperty> props;

  IO::DriverProperty dev;
  dev.key     = QStringLiteral("deviceIndex");
  dev.label   = tr("USB Device");
  dev.type    = IO::DriverProperty::ComboBox;
  dev.value   = m_deviceIndex;
  dev.options = deviceList();
  props.append(dev);

  IO::DriverProperty mode;
  mode.key     = QStringLiteral("transferMode");
  mode.label   = tr("Transfer Mode");
  mode.type    = IO::DriverProperty::ComboBox;
  mode.value   = static_cast<int>(m_transferMode);
  mode.options = {tr("Bulk Stream"), tr("Advanced Control"), tr("Isochronous")};
  props.append(mode);

  IO::DriverProperty inEp;
  inEp.key     = QStringLiteral("inEndpointIndex");
  inEp.label   = tr("IN Endpoint");
  inEp.type    = IO::DriverProperty::ComboBox;
  inEp.value   = m_inEndpointIndex;
  inEp.options = inEndpointList();
  props.append(inEp);

  IO::DriverProperty outEp;
  outEp.key     = QStringLiteral("outEndpointIndex");
  outEp.label   = tr("OUT Endpoint");
  outEp.type    = IO::DriverProperty::ComboBox;
  outEp.value   = m_outEndpointIndex;
  outEp.options = outEndpointList();
  props.append(outEp);

  IO::DriverProperty iso;
  iso.key   = QStringLiteral("isoPacketSize");
  iso.label = tr("ISO Packet Size");
  iso.type  = IO::DriverProperty::IntField;
  iso.value = m_isoPacketSize;
  iso.min   = 1;
  iso.max   = 65535;
  props.append(iso);

  return props;
}

/**
 * @brief Applies a single USB configuration change by key.
 */
void IO::Drivers::USB::setDriverProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("deviceIndex"))
    setDeviceIndex(value.toInt());

  else if (key == QLatin1String("transferMode"))
    setTransferMode(value.toInt());

  else if (key == QLatin1String("inEndpointIndex"))
    setInEndpointIndex(value.toInt());

  else if (key == QLatin1String("outEndpointIndex"))
    setOutEndpointIndex(value.toInt());

  else if (key == QLatin1String("isoPacketSize"))
    setIsoPacketSize(value.toInt());
}
