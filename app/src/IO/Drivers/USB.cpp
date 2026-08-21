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

#include "IO/Drivers/USB.h"

#include <chrono>
#include <cstring>
#include <QApplication>
#include <QJsonObject>
#include <QMessageBox>
#include <QMetaObject>
#include <QThread>
#include <QTimer>

#include "IO/ConnectionManager.h"
#include "Misc/Utilities.h"
#include "SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

constexpr unsigned int kBulkReadTimeout  = 100;
constexpr unsigned int kBulkWriteTimeout = 1000;
constexpr unsigned int kControlTimeout   = 1000;
constexpr int kBulkReadBufSize           = 65536;
constexpr int kDefaultIsoPacketSize      = 1024;
constexpr int kIsoNumTransfers           = 8;
constexpr int kIsoPacketsPerTransfer     = 8;
constexpr int kHotplugFallbackIntervalMs = 2000;
constexpr int kIsoDrainTimeoutMs         = 2000;
constexpr int kMaxControlLength          = 4096;

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
  , m_isoPacketSize(kDefaultIsoPacketSize)
  , m_transferMode(TransferMode::BulkStream)
  , m_running(false)
  , m_eventLoopRunning(false)
  , m_isoInFlight(0)
  , m_controlInFlight(false)
  , m_drainWaiting(false)
  , m_activeInEp(0)
  , m_activeOutEp(0)
  , m_activeInEpType(0)
  , m_activeOutEpType(0)
  , m_controlTransfer(nullptr)
{
  if (libusb_init(&m_ctx) < 0)
    m_ctx = nullptr;

  m_deviceIndex      = m_settings.value("USB/deviceIndex", 0).toInt();
  m_inEndpointIndex  = m_settings.value("USB/inEndpointIndex", 0).toInt();
  m_outEndpointIndex = m_settings.value("USB/outEndpointIndex", 0).toInt();
  m_isoPacketSize    = m_settings.value("USB/isoPacketSize", kDefaultIsoPacketSize).toInt();
  m_transferMode     = static_cast<TransferMode>(m_settings.value("USB/transferMode", 0).toInt());

  enumerateDevices();

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

  if (m_ctx) {
    m_eventLoopRunning.store(true, std::memory_order_release);
    connect(&m_eventThread, &QThread::started, this, &USB::eventLoop, Qt::DirectConnection);
    m_eventThread.start();
  }
}

/**
 * @brief Destructor: tears down threads and libusb state in the documented safe order (stop
 *        reader, drain iso callbacks while the event thread pumps, join it, then free).
 */
IO::Drivers::USB::~USB()
{
  stopReadThread();
  cancelAndDrainTransfers();
  stopEventThread();

  if (m_ctx && m_hotplugHandle) {
    libusb_hotplug_deregister_callback(m_ctx, m_hotplugHandle);
    m_hotplugHandle = 0;
  }

  freeTransfers();

  for (auto* dev : std::as_const(m_devicePtrs))
    libusb_unref_device(dev);

  if (m_handle) {
    releaseInterfaces();
    libusb_close(m_handle);
    m_handle = nullptr;
  }

  if (m_ctx) {
    libusb_exit(m_ctx);
    m_ctx = nullptr;
  }
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

  if (!m_ctx) {
    logDriverError(tr("USB Error"),
                   tr("Failed to initialize the USB subsystem. "
                      "Check that libusb is available on your system."));
    return false;
  }

  if (m_deviceIndex <= 0 || (m_deviceIndex - 1) >= m_devicePtrs.size()) {
    logDriverError(tr("USB Error"), tr("No USB device selected. Select a device and try again."));
    return false;
  }

  libusb_device* dev     = m_devicePtrs.at(m_deviceIndex - 1);
  const auto deviceLabel = m_deviceLabels.value(m_deviceIndex - 1, tr("Unknown Device"));
  const int openRc       = libusb_open(dev, &m_handle);
  if (openRc < 0) {
    m_handle = nullptr;
    logDriverError(tr("Failed to open \"%1\"").arg(deviceLabel),
                   tr("Could not open the USB device: %1.\n\n"
                      "On Linux, ensure you have read/write permission on the device node "
                      "(add a udev rule or run as root). "
                      "On macOS, the kernel driver may need to be detached first.")
                     .arg(QString::fromUtf8(libusb_strerror(static_cast<libusb_error>(openRc)))));
    return false;
  }

#ifdef __linux__
  libusb_set_auto_detach_kernel_driver(m_handle, 1);
#endif

  buildEndpointLists();
  Q_EMIT endpointListChanged();
  Q_EMIT inEndpointIndexChanged();
  Q_EMIT outEndpointIndexChanged();

  if (m_inEndpointIndex <= 0 || (m_inEndpointIndex - 1) >= m_inEndpoints.size()) {
    libusb_close(m_handle);
    m_handle = nullptr;
    logDriverError(tr("USB Device Error"), endpointErrorMessage());
    return false;
  }

  if (!activateSelectedEndpoints()) {
    releaseInterfaces();
    libusb_close(m_handle);
    m_handle = nullptr;
    return false;
  }

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
 * @brief Closes the device, tears down all active transfers, and stops the read thread. The
 * QThread::started connections are detached here so the next open() cycle re-wires exactly one
 * read slot instead of double-connecting.
 */
void IO::Drivers::USB::close()
{
  SS_ASSERT_LOG(m_ctx != nullptr);
  SS_ASSERT_LOG(m_activeInEp != 0 || m_handle == nullptr);

  stopReadThread();
  cancelAndDrainTransfers();
  freeTransfers();

  disconnect(&m_readThread, &QThread::started, this, &USB::readLoop);
  disconnect(&m_readThread, &QThread::started, this, &USB::isoReadLoop);

  if (m_handle) {
    releaseInterfaces();
    libusb_close(m_handle);
    m_handle = nullptr;
  }

  m_activeInEp      = 0;
  m_activeOutEp     = 0;
  m_activeInEpType  = 0;
  m_activeOutEpType = 0;

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
 * @brief Returns true when a device and a usable IN endpoint are selected, enabling connect.
 */
bool IO::Drivers::USB::configurationOk() const noexcept
{
  return m_deviceIndex > 0 && (m_deviceIndex - 1) < m_devicePtrs.size() && m_inEndpointIndex > 0
      && (m_inEndpointIndex - 1) < m_inEndpoints.size();
}

/**
 * @brief Sends @p data to the device via a synchronous bulk or interrupt OUT transfer. A mutable
 * copy of @p data is required because libusb takes a non-const buffer and may write into it.
 */
qint64 IO::Drivers::USB::write(const QByteArray& data)
{
  SS_ASSERT(!data.isEmpty(), return -1);
  SS_ASSERT(m_handle != nullptr, return -1);

  if (!isWritable())
    return -1;

  int transferred = 0;
  QByteArray mutableData(data);
  auto* buf      = reinterpret_cast<unsigned char*>(mutableData.data());
  const int size = static_cast<int>(data.size());

  int rc;
  if (m_activeOutEpType == LIBUSB_TRANSFER_TYPE_INTERRUPT)
    rc = libusb_interrupt_transfer(
      m_handle, m_activeOutEp, buf, size, &transferred, kBulkWriteTimeout);
  else
    rc = libusb_bulk_transfer(m_handle, m_activeOutEp, buf, size, &transferred, kBulkWriteTimeout);

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
  const bool deviceSelected = m_deviceIndex > 0 && (m_deviceIndex - 1) < m_devicePtrs.size();

  QStringList list;
  if (deviceSelected && m_inEndpointLabels.isEmpty())
    list.append(tr("No Usable IN Endpoints"));
  else
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
 * @brief Selects a device by combo index and rebuilds the endpoint lists so the user can pick
 * IN/OUT endpoints before connecting (descriptors are readable without opening the device).
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

  buildEndpointLists();

  m_settings.setValue("USB/deviceIndex", index);
  m_settings.setValue("USB/inEndpointIndex", m_inEndpointIndex);
  m_settings.setValue("USB/outEndpointIndex", m_outEndpointIndex);

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

  m_transferMode     = requested;
  m_inEndpointIndex  = 0;
  m_outEndpointIndex = 0;

  buildEndpointLists();

  m_settings.setValue("USB/transferMode", mode);
  m_settings.setValue("USB/inEndpointIndex", m_inEndpointIndex);
  m_settings.setValue("USB/outEndpointIndex", m_outEndpointIndex);

  Q_EMIT endpointListChanged();
  Q_EMIT inEndpointIndexChanged();
  Q_EMIT outEndpointIndexChanged();
  Q_EMIT transferModeChanged();
  Q_EMIT configurationChanged();
}

/**
 * @brief Sets the active IN endpoint by combo index; in iso mode the packet size follows the
 * selected endpoint's effective max transfer size.
 */
void IO::Drivers::USB::setInEndpointIndex(const int index)
{
  if (m_inEndpointIndex == index || index < 0 || index > m_inEndpoints.size())
    return;

  m_inEndpointIndex = index;
  m_settings.setValue("USB/inEndpointIndex", index);

  if (m_transferMode == TransferMode::Isochronous && index > 0) {
    const int suggested = m_inEndpoints.at(index - 1).maxPacketSize;
    if (suggested > 0 && suggested != m_isoPacketSize) {
      m_isoPacketSize = suggested;
      m_settings.setValue("USB/isoPacketSize", suggested);
      Q_EMIT isoPacketSizeChanged();
    }
  }

  Q_EMIT inEndpointIndexChanged();
  Q_EMIT configurationChanged();
}

/**
 * @brief Sets the active OUT endpoint by combo index (0 = none, receive-only).
 */
void IO::Drivers::USB::setOutEndpointIndex(const int index)
{
  if (m_outEndpointIndex == index || index < 0 || index > m_outEndpoints.size())
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
 * @brief Connects the driver to application-level lifecycle signals. On quit, the read thread is
 * joined, iso transfers are cancelled and drained, and the event thread is joined before the
 * transfer pool is freed, matching the teardown order in close()/~USB().
 */
void IO::Drivers::USB::setupExternalConnections()
{
  connect(qApp, &QApplication::aboutToQuit, this, [this] {
    stopReadThread();
    cancelAndDrainTransfers();
    stopEventThread();

    if (m_ctx && m_hotplugHandle) {
      libusb_hotplug_deregister_callback(m_ctx, m_hotplugHandle);
      m_hotplugHandle = 0;
    }

    freeTransfers();
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
  if (!isOpen())
    return;

  static auto& connectionManager = ConnectionManager::instance();
  connectionManager.disconnectDevice(this);
  logDriverError(tr("USB Device Error"),
                 tr("The USB device was disconnected or encountered a fatal read error."));
}

/**
 * @brief Scans the USB bus and rebuilds the device list, re-anchoring the current selection by
 * label so a hotplug event cannot silently retarget it to a different device.
 */
void IO::Drivers::USB::enumerateDevices()
{
  if (!m_ctx)
    return;

  const QStringList previous  = m_deviceLabels;
  const QString selectedLabel = m_deviceIndex > 0 ? previous.value(m_deviceIndex - 1) : QString();

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

  QHash<QString, QString> refreshedCache;
  for (ssize_t i = 0; i < count; ++i) {
    libusb_device* dev = devs[i];
    libusb_device_descriptor desc{};

    if (libusb_get_device_descriptor(dev, &desc) < 0)
      continue;

    QString label = QStringLiteral("%1:%2")
                      .arg(desc.idVendor, 4, 16, QChar('0'))
                      .arg(desc.idProduct, 4, 16, QChar('0'))
                      .toUpper();

    const QString key = QStringLiteral("%1:%2:%3")
                          .arg(label)
                          .arg(static_cast<int>(libusb_get_bus_number(dev)))
                          .arg(static_cast<int>(libusb_get_device_address(dev)));

    if (m_deviceLabelCache.contains(key))
      label = m_deviceLabelCache.value(key);
    else if (!m_eventThread.isRunning())
      label = enrichDeviceLabel(dev, desc, label);

    refreshedCache.insert(key, label);

    libusb_ref_device(dev);
    m_devicePtrs.append(dev);
    m_deviceLabels.append(label);
  }

  m_deviceLabelCache.swap(refreshedCache);
  libusb_free_device_list(devs, 1);

  int newIndex = m_deviceIndex;
  if (!selectedLabel.isEmpty())
    newIndex = m_deviceLabels.indexOf(selectedLabel) + 1;
  else if (m_deviceIndex > m_devicePtrs.size())
    newIndex = 0;

  const bool indexMoved = (newIndex != m_deviceIndex);
  m_deviceIndex         = newIndex;

  if (m_deviceLabels != previous || indexMoved) {
    buildEndpointLists();

    if (indexMoved)
      Q_EMIT deviceIndexChanged();

    Q_EMIT deviceListChanged();
    Q_EMIT endpointListChanged();
    Q_EMIT inEndpointIndexChanged();
    Q_EMIT outEndpointIndexChanged();
    Q_EMIT configurationChanged();
  }
}

//--------------------------------------------------------------------------------------------------
// Private helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens @p dev to append its manufacturer/product strings to @p base; the synchronous
 * string-descriptor transfer runs only on the single-threaded startup scan, since issuing it while
 * m_eventThread pumps libusb_handle_events deadlocks the macOS backend (hotplug rescans reuse the
 * cached label instead).
 */
QString IO::Drivers::USB::enrichDeviceLabel(libusb_device* dev,
                                            const libusb_device_descriptor& desc,
                                            const QString& base) const
{
  SS_ASSERT(dev != nullptr, return base);
  SS_ASSERT(m_ctx != nullptr, return base);

  QString label             = base;
  libusb_device_handle* tmp = nullptr;
  if (libusb_open(dev, &tmp) != 0)
    return label;

  const auto fetchStr = [tmp](uint8_t idx) -> QString {
    if (!idx)
      return {};

    unsigned char buf[256] = {};
    const int rc = libusb_get_string_descriptor_ascii(tmp, idx, buf, static_cast<int>(sizeof(buf)));

    if (rc > 0)
      return QString::fromLatin1(reinterpret_cast<const char*>(buf), rc).trimmed();

    return {};
  };

  const QString mfr  = fetchStr(desc.iManufacturer);
  const QString prod = fetchStr(desc.iProduct);

  // code-verify off
  if (!mfr.isEmpty() || !prod.isEmpty())
    label += QStringLiteral(" – %1 %2").arg(mfr, prod).trimmed();
  // code-verify on

  libusb_close(tmp);
  return label;
}

/**
 * @brief Returns true when @p type is streamable in the current transfer mode (iso mode wants
 * isochronous endpoints; every other mode streams bulk and interrupt endpoints).
 */
static bool typeUsableInMode(uint8_t type, bool wantIso)
{
  if (wantIso)
    return type == LIBUSB_TRANSFER_TYPE_ISOCHRONOUS;

  return type == LIBUSB_TRANSFER_TYPE_BULK || type == LIBUSB_TRANSFER_TYPE_INTERRUPT;
}

/**
 * @brief Returns true if the alt-setting exposes at least one endpoint usable in the other mode.
 */
static bool altSettingHasOtherMode(const libusb_interface_descriptor& alt, bool wantIso)
{
  for (int e = 0; e < alt.bNumEndpoints; ++e) {
    const uint8_t type = alt.endpoint[e].bmAttributes & LIBUSB_TRANSFER_TYPE_MASK;
    if (typeUsableInMode(type, !wantIso))
      return true;
  }

  return false;
}

/**
 * @brief Returns true if the configuration exposes at least one endpoint usable in the other mode.
 */
static bool configHasOtherMode(const libusb_config_descriptor* cfg, bool wantIso)
{
  for (int i = 0; i < cfg->bNumInterfaces; ++i) {
    const libusb_interface& iface = cfg->interface[i];
    for (int a = 0; a < iface.num_altsetting; ++a)
      if (altSettingHasOtherMode(iface.altsetting[a], wantIso))
        return true;
  }

  return false;
}

/**
 * @brief Reads the config descriptor of @p dev without opening it, falling back to configuration
 * 0 when no active configuration is reported (common for unconfigured devices).
 */
static libusb_config_descriptor* readConfigDescriptor(libusb_device* dev)
{
  libusb_config_descriptor* config = nullptr;
  if (libusb_get_active_config_descriptor(dev, &config) == 0)
    return config;

  if (libusb_get_config_descriptor(dev, 0, &config) == 0)
    return config;

  return nullptr;
}

/**
 * @brief Builds an actionable error message when no IN endpoint is found.
 */
QString IO::Drivers::USB::endpointErrorMessage() const
{
  const bool wantIso = (m_transferMode == TransferMode::Isochronous);
  bool hasOtherType  = false;

  libusb_config_descriptor* cfg = readConfigDescriptor(m_devicePtrs.at(m_deviceIndex - 1));
  if (cfg) {
    hasOtherType = configHasOtherMode(cfg, wantIso);
    libusb_free_config_descriptor(cfg);
  }

  if (hasOtherType && wantIso)
    return tr("No isochronous IN endpoint was found on this device, but bulk or "
              "interrupt endpoints are available.\n\n"
              "Switch the Transfer Mode to \"Bulk/Interrupt Stream\" and try again.");

  if (hasOtherType && !wantIso)
    return tr("No bulk or interrupt IN endpoint was found on this device, but "
              "isochronous endpoints are available.\n\n"
              "Switch the Transfer Mode to \"Isochronous\" and try again.");

  return tr("No usable IN endpoint was found on this device.\n\n"
            "The device may not expose data endpoints in its active configuration, "
            "or it may speak a dedicated protocol. Protocol adapters (e.g. CAN or "
            "Modbus interfaces) should be connected through their own driver.");
}

/**
 * @brief Inspects a single endpoint descriptor and appends it to the IN or OUT list if it matches
 * the mode; skips zero-bandwidth endpoints and duplicates repeated across alt-settings.
 */
void IO::Drivers::USB::collectEndpoint(const libusb_endpoint_descriptor& ep,
                                       int ifNum,
                                       uint8_t altSetting,
                                       bool wantIso)
{
  const uint8_t type = ep.bmAttributes & LIBUSB_TRANSFER_TYPE_MASK;
  if (!typeUsableInMode(type, wantIso))
    return;

  const int basePacketSize = ep.wMaxPacketSize & 0x07FF;
  const int transactions   = 1 + ((ep.wMaxPacketSize >> 11) & 0x03);
  const int effectiveSize  = basePacketSize * transactions;
  if (effectiveSize <= 0)
    return;

  const bool isIn                = (ep.bEndpointAddress & LIBUSB_ENDPOINT_IN) != 0;
  QList<EndpointInfo>& endpoints = isIn ? m_inEndpoints : m_outEndpoints;
  QStringList& labels            = isIn ? m_inEndpointLabels : m_outEndpointLabels;

  for (const auto& existing : std::as_const(endpoints))
    if (existing.address == ep.bEndpointAddress && (!wantIso || existing.altSetting == altSetting))
      return;

  QString typeStr = QStringLiteral("Bulk");
  if (type == LIBUSB_TRANSFER_TYPE_ISOCHRONOUS)
    typeStr = QStringLiteral("Iso");
  else if (type == LIBUSB_TRANSFER_TYPE_INTERRUPT)
    typeStr = QStringLiteral("Int");

  EndpointInfo info;
  info.address         = ep.bEndpointAddress;
  info.attributes      = ep.bmAttributes;
  info.altSetting      = altSetting;
  info.maxPacketSize   = static_cast<uint16_t>(effectiveSize);
  info.interfaceNumber = ifNum;

  const QString dirStr = isIn ? QStringLiteral("IN") : QStringLiteral("OUT");
  const QString epHex  = QString::number(ep.bEndpointAddress, 16).toUpper().rightJustified(2, '0');
  QString ifStr        = QStringLiteral("IF%1").arg(ifNum);
  if (altSetting > 0)
    ifStr += QStringLiteral(" ALT%1").arg(altSetting);

  // code-verify off
  info.label = QStringLiteral("EP 0x%1 – %2 %3  (%4, max %5 B)")
                 .arg(epHex, typeStr, dirStr, ifStr)
                 .arg(effectiveSize);
  // code-verify on

  endpoints.append(info);
  labels.append(info.label);
}

/**
 * @brief Scans all interfaces and endpoints of the selected device and populates the IN/OUT
 * lists. Runs on cached descriptors (no open handle needed), preserves the current selection by
 * endpoint address across rebuilds, and auto-selects the first entry as a default.
 */
void IO::Drivers::USB::buildEndpointLists()
{
  const auto selectedAddress = [](const QList<EndpointInfo>& list, int index) -> uint8_t {
    if (index > 0 && (index - 1) < list.size())
      return list.at(index - 1).address;

    return 0;
  };

  const auto matchAddress = [](const QList<EndpointInfo>& list, uint8_t address) -> int {
    for (int i = 0; i < list.size(); ++i)
      if (list.at(i).address == address)
        return i + 1;

    return 0;
  };

  const uint8_t prevIn  = selectedAddress(m_inEndpoints, m_inEndpointIndex);
  const uint8_t prevOut = selectedAddress(m_outEndpoints, m_outEndpointIndex);

  clearEndpointLists();

  if (m_deviceIndex <= 0 || (m_deviceIndex - 1) >= m_devicePtrs.size()) {
    m_inEndpointIndex  = 0;
    m_outEndpointIndex = 0;
    return;
  }

  libusb_config_descriptor* config = readConfigDescriptor(m_devicePtrs.at(m_deviceIndex - 1));
  if (!config) {
    m_inEndpointIndex  = 0;
    m_outEndpointIndex = 0;
    return;
  }

  const bool wantIso = (m_transferMode == TransferMode::Isochronous);

  for (int i = 0; i < config->bNumInterfaces; ++i) {
    const libusb_interface& iface = config->interface[i];

    for (int a = 0; a < iface.num_altsetting; ++a) {
      const libusb_interface_descriptor& alt = iface.altsetting[a];

      for (int e = 0; e < alt.bNumEndpoints; ++e)
        collectEndpoint(alt.endpoint[e], alt.bInterfaceNumber, alt.bAlternateSetting, wantIso);
    }
  }

  libusb_free_config_descriptor(config);

  if (prevIn != 0)
    m_inEndpointIndex = matchAddress(m_inEndpoints, prevIn);
  else if (m_inEndpointIndex > m_inEndpoints.size())
    m_inEndpointIndex = 0;

  if (prevOut != 0)
    m_outEndpointIndex = matchAddress(m_outEndpoints, prevOut);
  else if (m_outEndpointIndex > m_outEndpoints.size())
    m_outEndpointIndex = 0;

  if (m_inEndpointIndex == 0 && !m_inEndpoints.isEmpty())
    m_inEndpointIndex = 1;

  if (m_outEndpointIndex == 0 && !m_outEndpoints.isEmpty())
    m_outEndpointIndex = 1;

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
  while (m_eventLoopRunning.load(std::memory_order_acquire)) {
    struct timeval tv = {0, 100000};
    libusb_handle_events_timeout(m_ctx, &tv);
  }
}

/**
 * @brief Stops and joins the bulk read thread. Mirrors HID::cleanupDevice: the started+
 * DirectConnection idiom drops the thread into exec() once readLoop returns, so quit() before
 * wait() is mandatory and terminate() (which corrupts libusb mid-transfer) is never used.
 */
void IO::Drivers::USB::stopReadThread()
{
  m_running.store(false, std::memory_order_release);

  if (m_readThread.isRunning()) {
    m_readThread.quit();
    m_readThread.wait();
  }
}

/**
 * @brief Cancels every iso transfer and waits until all callbacks have reported back. libusb
 * forbids freeing an in-flight transfer, so this must drain to zero before the pool is freed;
 * the completion callbacks wake the wait as soon as the last one lands, and the bounded
 * deadline covers a dead device whose cancellations never complete.
 */
void IO::Drivers::USB::cancelAndDrainTransfers()
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
void IO::Drivers::USB::notifyDrainWaiter()
{
  if (!m_drainWaiting.load(std::memory_order_acquire))
    return;

  {
    std::lock_guard<std::mutex> lock(m_drainMutex);
  }
  m_drainCv.notify_all();
}

/**
 * @brief Stops and joins the libusb event thread. quit() before wait() because eventLoop runs on
 * the started+DirectConnection idiom and would otherwise fall into exec().
 */
void IO::Drivers::USB::stopEventThread()
{
  m_eventLoopRunning.store(false, std::memory_order_release);

  if (m_eventThread.isRunning()) {
    m_eventThread.quit();
    m_eventThread.wait();
  }
}

/**
 * @brief Frees the iso transfer pool and any leftover control transfer plus their buffers. Sole
 * owner of transfer->buffer: only valid once the read and event threads are joined, so no callback
 * can race the free.
 */
void IO::Drivers::USB::freeTransfers()
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
 * @brief Claims @p ifaceNum on the open device handle (no-op when already claimed).
 */
bool IO::Drivers::USB::claimInterface(int ifaceNum)
{
  SS_ASSERT(m_handle != nullptr, return false);

  if (m_claimedInterfaces.contains(ifaceNum))
    return true;

  if (libusb_claim_interface(m_handle, ifaceNum) < 0)
    return false;

  m_claimedInterfaces.append(ifaceNum);
  return true;
}

/**
 * @brief Releases every claimed interface.
 */
void IO::Drivers::USB::releaseInterfaces()
{
  if (m_handle)
    for (const int iface : std::as_const(m_claimedInterfaces))
      libusb_release_interface(m_handle, iface);

  m_claimedInterfaces.clear();
}

/**
 * @brief Claims the interfaces of the selected IN/OUT endpoints and activates their alt-settings
 * (isochronous endpoints live in non-zero alt-settings; alt 0 is typically zero-bandwidth). A
 * failing OUT endpoint degrades to read-only with a warning instead of aborting the connection.
 */
bool IO::Drivers::USB::activateSelectedEndpoints()
{
  SS_ASSERT(m_handle != nullptr, return false);
  SS_ASSERT(m_inEndpointIndex > 0 && (m_inEndpointIndex - 1) < m_inEndpoints.size(), return false);

  const EndpointInfo in = m_inEndpoints.at(m_inEndpointIndex - 1);
  if (!claimInterface(in.interfaceNumber)) {
    logDriverError(tr("USB Device Error"),
                   tr("Could not claim interface %1 on the USB device.\n\n"
                      "Another driver or application may already have it open. "
                      "On Linux, try unloading the kernel driver (e.g. cdc_acm) "
                      "or adding a udev rule.")
                     .arg(in.interfaceNumber));
    return false;
  }

  if (in.altSetting != 0
      && libusb_set_interface_alt_setting(m_handle, in.interfaceNumber, in.altSetting) < 0) {
    logDriverError(tr("USB Device Error"),
                   tr("Could not activate alternate setting %1 on interface %2. "
                      "The selected endpoint is not reachable.")
                     .arg(in.altSetting)
                     .arg(in.interfaceNumber));
    return false;
  }

  m_activeInEp      = in.address;
  m_activeInEpType  = in.attributes & LIBUSB_TRANSFER_TYPE_MASK;
  m_activeOutEp     = 0;
  m_activeOutEpType = 0;

  if (m_outEndpointIndex <= 0 || (m_outEndpointIndex - 1) >= m_outEndpoints.size())
    return true;

  const EndpointInfo out = m_outEndpoints.at(m_outEndpointIndex - 1);
  bool outOk             = true;

  if (out.interfaceNumber == in.interfaceNumber)
    outOk = (out.altSetting == in.altSetting);
  else {
    outOk = claimInterface(out.interfaceNumber);
    if (outOk && out.altSetting != 0)
      outOk = libusb_set_interface_alt_setting(m_handle, out.interfaceNumber, out.altSetting) >= 0;
  }

  if (outOk) {
    m_activeOutEp     = out.address;
    m_activeOutEpType = out.attributes & LIBUSB_TRANSFER_TYPE_MASK;
  } else {
    logDriverError(tr("USB Device Warning"),
                   tr("The selected OUT endpoint could not be activated. "
                      "Continuing in read-only mode."));
  }

  return true;
}

/**
 * @brief Synchronous read loop for BulkStream and AdvancedControl modes; dispatches bulk or
 * interrupt transfers based on the active IN endpoint's transfer type.
 */
void IO::Drivers::USB::readLoop()
{
  unsigned char buf[kBulkReadBufSize];
  const bool interruptEp = (m_activeInEpType == LIBUSB_TRANSFER_TYPE_INTERRUPT);

  while (m_running.load(std::memory_order_relaxed)) {
    int transferred = 0;
    int rc;
    if (interruptEp)
      rc = libusb_interrupt_transfer(
        m_handle, m_activeInEp, buf, kBulkReadBufSize, &transferred, kBulkReadTimeout);
    else
      rc = libusb_bulk_transfer(
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
      m_isoInFlight.fetch_add(1, std::memory_order_acq_rel);
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
 * @brief Static libusb callback for each completed iso transfer; stamps acquisition time before
 *        queueing. Buffer ownership lives solely with freeTransfers() (frees post-join).
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
      const auto timestamp = IO::CapturedData::SteadyClock::now();
      QMetaObject::invokeMethod(
        self,
        [self, received, timestamp] { self->publishReceivedData(received, timestamp); },
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
    QMetaObject::invokeMethod(self, "onReadError", Qt::QueuedConnection);
  }
}

//--------------------------------------------------------------------------------------------------
// Control transfers (Advanced Control mode)
//--------------------------------------------------------------------------------------------------

/**
 * @brief True when @p c is a hexadecimal digit (0-9, a-f, A-F).
 */
[[nodiscard]] static bool isHexChar(const QChar c)
{
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

/**
 * @brief Parses a hex string (optional 0x prefix) into an unsigned value bounded by @p max.
 */
[[nodiscard]] static unsigned int parseHexUInt(const QString& text,
                                               const unsigned int max,
                                               bool& ok)
{
  QString cleaned = text.trimmed();
  if (cleaned.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive))
    cleaned.remove(0, 2);

  const unsigned int value = cleaned.toUInt(&ok, 16);
  if (!ok || value > max)
    ok = false;

  return value;
}

/**
 * @brief Parses a whitespace-tolerant hex byte string into raw bytes; ok=false on any non-hex.
 */
[[nodiscard]] static QByteArray parseHexBytes(const QString& text, bool& ok)
{
  QString cleaned;
  cleaned.reserve(text.size());

  for (const QChar c : text) {
    if (c.isSpace())
      continue;

    if (!isHexChar(c)) {
      ok = false;
      return {};
    }

    cleaned.append(c);
  }

  ok = (cleaned.size() % 2 == 0);
  return ok ? QByteArray::fromHex(cleaned.toLatin1()) : QByteArray{};
}

/**
 * @brief Maps a libusb transfer status to a short human-readable failure reason.
 */
[[nodiscard]] static QString controlStatusText(const int status)
{
  switch (status) {
    case LIBUSB_TRANSFER_TIMED_OUT:
      return QObject::tr("timed out");
    case LIBUSB_TRANSFER_CANCELLED:
      return QObject::tr("cancelled");
    case LIBUSB_TRANSFER_STALL:
      return QObject::tr("stalled (request not supported)");
    case LIBUSB_TRANSFER_NO_DEVICE:
      return QObject::tr("device disconnected");
    case LIBUSB_TRANSFER_OVERFLOW:
      return QObject::tr("buffer overflow");
    default:
      return QObject::tr("transfer error");
  }
}

/**
 * @brief Composes and submits an async USB control transfer from the setup-packet fields entered
 * in the composer; the result is reported later via @ref controlTransferFinished. Async (not the
 * blocking libusb_control_transfer) so the event thread owns all event handling and the UI never
 * blocks; a bounded kControlTimeout caps a non-responding device.
 */
void IO::Drivers::USB::sendControlRequest(const QString& bmRequestType,
                                          const QString& bRequest,
                                          const QString& wValue,
                                          const QString& wIndex,
                                          const QString& payloadHex,
                                          const int readLength)
{
  const auto fail = [this](const QString& m) {
    Q_EMIT controlTransferFinished(false, 0, {}, m);
  };

  if (!m_handle || !advancedModeEnabled()) {
    fail(tr("No device connected in Advanced Control mode."));
    return;
  }

  if (m_controlInFlight.load(std::memory_order_acquire)) {
    fail(tr("A control transfer is already in progress."));
    return;
  }

  bool ok                    = false;
  const unsigned int type    = parseHexUInt(bmRequestType, 0xFF, ok);
  const unsigned int request = ok ? parseHexUInt(bRequest, 0xFF, ok) : 0;
  const unsigned int value   = ok ? parseHexUInt(wValue, 0xFFFF, ok) : 0;
  const unsigned int index   = ok ? parseHexUInt(wIndex, 0xFFFF, ok) : 0;
  if (!ok) {
    fail(tr("Invalid setup field: request type, request, wValue, and wIndex must be hex."));
    return;
  }

  const bool isIn = (type & LIBUSB_ENDPOINT_IN) != 0;
  QByteArray payload;
  if (!isIn)
    payload = parseHexBytes(payloadHex, ok);

  if (!isIn && !ok) {
    fail(tr("Invalid data payload: expected a sequence of hex bytes."));
    return;
  }

  const int wLength = isIn ? readLength : static_cast<int>(payload.size());
  if (wLength < 0 || wLength > kMaxControlLength) {
    fail(tr("Invalid transfer length (0-%1 bytes).").arg(kMaxControlLength));
    return;
  }

  if (m_controlTransfer) {
    delete[] m_controlTransfer->buffer;
    libusb_free_transfer(m_controlTransfer);
    m_controlTransfer = nullptr;
  }

  libusb_transfer* transfer = libusb_alloc_transfer(0);
  auto* buffer =
    transfer ? new (std::nothrow) unsigned char[LIBUSB_CONTROL_SETUP_SIZE + wLength] : nullptr;
  if (!transfer || !buffer) {
    delete[] buffer;
    if (transfer)
      libusb_free_transfer(transfer);

    fail(tr("Could not allocate the control transfer."));
    return;
  }

  libusb_fill_control_setup(buffer,
                            static_cast<uint8_t>(type),
                            static_cast<uint8_t>(request),
                            static_cast<uint16_t>(value),
                            static_cast<uint16_t>(index),
                            static_cast<uint16_t>(wLength));

  if (!isIn && wLength > 0)
    std::memcpy(
      buffer + LIBUSB_CONTROL_SETUP_SIZE, payload.constData(), static_cast<size_t>(wLength));

  libusb_fill_control_transfer(
    transfer, m_handle, buffer, &USB::controlTransferCallback, this, kControlTimeout);

  m_controlTransfer = transfer;
  m_controlInFlight.store(true, std::memory_order_release);

  const int rc = libusb_submit_transfer(transfer);
  if (rc < 0) {
    m_controlInFlight.store(false, std::memory_order_release);
    m_controlTransfer = nullptr;
    delete[] buffer;
    libusb_free_transfer(transfer);
    fail(tr("Failed to submit control transfer: %1.")
           .arg(QString::fromUtf8(libusb_strerror(static_cast<libusb_error>(rc)))));
  }
}

/**
 * @brief Static libusb completion callback (event thread): marshals the outcome via a queued
 * emit, then clears the in-flight flag last. It never frees the transfer nor writes
 * m_controlTransfer; the main thread owns that lifetime (freed at the next send or in
 * freeTransfers), mirroring the iso pool so teardown cannot race a free.
 */
void LIBUSB_CALL IO::Drivers::USB::controlTransferCallback(libusb_transfer* transfer)
{
  auto* self = static_cast<USB*>(transfer->user_data);
  SS_ASSERT(self != nullptr, return);
  SS_ASSERT(transfer->buffer != nullptr, {
    self->m_controlInFlight.store(false, std::memory_order_release);
    self->notifyDrainWaiter();
    return;
  });

  const bool ok   = (transfer->status == LIBUSB_TRANSFER_COMPLETED);
  const int bytes = transfer->actual_length;
  const bool isIn = (transfer->buffer[0] & LIBUSB_ENDPOINT_IN) != 0;

  QString responseHex;
  QString message;
  if (ok) {
    message = tr("Transfer complete: %1 byte(s).").arg(bytes);
    if (isIn && bytes > 0) {
      const unsigned char* data = libusb_control_transfer_get_data(transfer);
      responseHex =
        QString::fromLatin1(QByteArray(reinterpret_cast<const char*>(data), bytes).toHex(' '));
    }
  } else
    message = tr("Control transfer failed: %1.").arg(controlStatusText(transfer->status));

  QMetaObject::invokeMethod(
    self,
    [self, ok, bytes, responseHex, message] {
      Q_EMIT self->controlTransferFinished(ok, bytes, responseHex, message);
    },
    Qt::QueuedConnection);

  self->m_controlInFlight.store(false, std::memory_order_release);
  self->notifyDrainWaiter();
}

//--------------------------------------------------------------------------------------------------
// Stable device identification
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns VID, PID, and serial number of the currently selected USB device. The serial is
 * omitted while m_eventThread pumps libusb_handle_events: the synchronous string-descriptor
 * transfer deadlocks the macOS backend (same rule as enrichDeviceLabel).
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

  libusb_device_handle* tmp = nullptr;
  if (desc.iSerialNumber && !m_eventThread.isRunning() && libusb_open(dev, &tmp) == 0) {
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

    if (!deviceSerialMatches(m_devicePtrs.at(i), desc, savedSer))
      continue;

    setDeviceIndex(i + 1);
    return true;
  }

  return false;
}

/**
 * @brief Returns true when the device serial matches savedSer (or savedSer is empty / unreadable).
 * Falls back to a VID/PID-only match while m_eventThread runs, because the synchronous
 * string-descriptor read deadlocks the macOS backend (same rule as enrichDeviceLabel).
 */
bool IO::Drivers::USB::deviceSerialMatches(libusb_device* device,
                                           const libusb_device_descriptor& desc,
                                           const QString& savedSer) const
{
  if (savedSer.isEmpty() || !desc.iSerialNumber)
    return true;

  if (m_eventThread.isRunning())
    return true;

  libusb_device_handle* tmp = nullptr;
  if (libusb_open(device, &tmp) != 0)
    return true;

  unsigned char buf[256] = {};
  const int rc =
    libusb_get_string_descriptor_ascii(tmp, desc.iSerialNumber, buf, static_cast<int>(sizeof(buf)));
  libusb_close(tmp);

  if (rc <= 0)
    return true;

  const auto serial = QString::fromLatin1(reinterpret_cast<const char*>(buf), rc).trimmed();
  return serial == savedSer;
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
  mode.options = {tr("Bulk/Interrupt Stream"), tr("Advanced Control"), tr("Isochronous")};
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
  if (key == QLatin1String("deviceIndex")) {
    setDeviceIndex(value.toInt());
    return;
  }

  if (key == QLatin1String("transferMode")) {
    setTransferMode(value.toInt());
    return;
  }

  if (key == QLatin1String("inEndpointIndex")) {
    setInEndpointIndex(value.toInt());
    return;
  }

  if (key == QLatin1String("outEndpointIndex")) {
    setOutEndpointIndex(value.toInt());
    return;
  }

  if (key == QLatin1String("isoPacketSize"))
    setIsoPacketSize(value.toInt());
}
