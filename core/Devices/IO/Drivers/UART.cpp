/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "IO/Drivers/UART.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "Core/Bus/MessageBus.h"
#include "Core/Bus/Messages.h"
#include "Core/Services.h"
#include "Core/TimerEvents.h"
#include "Core/Translator.h"
#include "IO/ConnectionManager.h"
#include "IO/Drivers/SerialPortIdentity.h"
#include "IO/Drivers/UART/SequenceRunner.h"
#include "IO/Drivers/UART/UartPolicy.h"
#include "IO/Drivers/UART/UartSequencePort.h"

//--------------------------------------------------------------------------------------------------
// Static utility functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief The connection manager a driver hands its drops and recoveries to; one cached reach.
 */
static IO::ConnectionManager& connectionManager()
{
  static auto& manager = IO::ConnectionManager::instance();
  return manager;
}

//--------------------------------------------------------------------------------------------------
// Constructor/destructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the UART driver and restores persisted port settings.
 */
IO::Drivers::UART::UART()
  : m_port(nullptr)
  , m_dtrEnabled(true)
  , m_autoReconnect(false)
  , m_pendingReconnect(false)
  , m_usingCustomSerialPort(false)
  , m_rtsEnabled(false)
  , m_breakHeld(false)
  , m_sequenceConnecting(false)
  , m_portIndex(0)
  , m_breakTimer(Async::kInvalidTimerId)
{
  populateErrors();

  m_baudRate = m_settings.value("IO_Serial_Baud_Rate", 9600).toInt();

  int defParity   = parityList().indexOf(tr("None"));
  int defFlow     = flowControlList().indexOf(tr("None"));
  int defDataBits = dataBitsList().indexOf(QStringLiteral("8"));
  int defStopBits = stopBitsList().indexOf(QStringLiteral("1"));

  setDtrEnabled(m_settings.value("UartDriver/dtr", 1).toBool());
  setParity(m_settings.value("UartDriver/parity", defParity).toInt());
  setDataBits(m_settings.value("UartDriver/dataBits", defDataBits).toInt());
  setStopBits(m_settings.value("UartDriver/stopBits", defStopBits).toInt());
  setAutoReconnect(m_settings.value("UartDriver/autoReconnect", 0).toBool());
  setFlowControl(m_settings.value("UartDriver/flowControl", defFlow).toInt());
  setRtsEnabled(m_settings.value("UartDriver/rts", 0).toBool());
  setConnectionSequence(m_settings.value("UartDriver/connectionSequence").toString());

  connect(
    this, &IO::Drivers::UART::portIndexChanged, this, &IO::Drivers::UART::configurationChanged);
  connect(
    this, &IO::Drivers::UART::baudRateChanged, this, &IO::Drivers::UART::configurationChanged);
  connect(this, &IO::Drivers::UART::parityChanged, this, &IO::Drivers::UART::configurationChanged);
  connect(
    this, &IO::Drivers::UART::dataBitsChanged, this, &IO::Drivers::UART::configurationChanged);
  connect(
    this, &IO::Drivers::UART::stopBitsChanged, this, &IO::Drivers::UART::configurationChanged);
  connect(
    this, &IO::Drivers::UART::flowControlChanged, this, &IO::Drivers::UART::configurationChanged);
  connect(
    this, &IO::Drivers::UART::dtrEnabledChanged, this, &IO::Drivers::UART::configurationChanged);
  connect(
    this, &IO::Drivers::UART::autoReconnectChanged, this, &IO::Drivers::UART::configurationChanged);
  connect(
    this, &IO::Drivers::UART::rtsEnabledChanged, this, &IO::Drivers::UART::configurationChanged);
  connect(this,
          &IO::Drivers::UART::connectionSequenceChanged,
          this,
          &IO::Drivers::UART::configurationChanged);

  connect(this, &IO::Drivers::UART::languageChanged, this, &IO::Drivers::UART::populateErrors);

  m_reconnectTimer.setInterval(1000);
  connect(&m_reconnectTimer, &QTimer::timeout, this, &IO::Drivers::UART::pollAutoReconnect);
}

/**
 * @brief Closes the active port and releases its handle.
 */
IO::Drivers::UART::~UART()
{
  if (m_runner)
    m_runner->cancel();

  if (port()) {
    if (port()->isOpen())
      port()->close();

    port()->deleteLater();
  }
}

//--------------------------------------------------------------------------------------------------
// HAL-driver implementation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Closes the serial port and clears its signal/slot connections.
 */
void IO::Drivers::UART::close()
{
  stopSequence();
  releaseBreak();

  if (port() != nullptr) {
    disconnect(port(), &QSerialPort::errorOccurred, this, &IO::Drivers::UART::handleError);
    disconnect(port(), &QIODevice::readyRead, this, &IO::Drivers::UART::onReadyRead);

    if (dtrEnabled())
      port()->setDataTerminalReady(false);

    port()->close();
    port()->deleteLater();
  }

  m_port                  = nullptr;
  m_pendingReconnect      = false;
  m_usingCustomSerialPort = false;
  m_reconnectTimer.stop();

  Q_EMIT portChanged();
  Q_EMIT availablePortsChanged();
}

/**
 * @brief Returns true when the link is connected: the port is open and no connect-time
 *        Connection Sequence still owns it. Internal "does the port exist" checks use portOpen().
 */
bool IO::Drivers::UART::isOpen() const noexcept
{
  return portOpen() && !m_sequenceConnecting;
}

/**
 * @brief Returns true while a connect-time Connection Sequence runs on the open port.
 */
bool IO::Drivers::UART::isConnecting() const noexcept
{
  return portOpen() && m_sequenceConnecting;
}

/**
 * @brief Returns true when the serial port itself is open, whatever the connected state.
 */
bool IO::Drivers::UART::portOpen() const noexcept
{
  return m_port != nullptr && m_port->isOpen();
}

/**
 * @brief Returns true when the serial port can be read.
 */
bool IO::Drivers::UART::isReadable() const noexcept
{
  if (isOpen())
    return port()->isReadable();

  return false;
}

/**
 * @brief Returns true when the serial port can be written.
 */
bool IO::Drivers::UART::isWritable() const noexcept
{
  if (isOpen())
    return port()->isWritable();

  return false;
}

/**
 * @brief Returns @c true when the user has selected a non-placeholder port.
 */
bool IO::Drivers::UART::configurationOk() const noexcept
{
  return portIndex() > 0;
}

/**
 * @brief Writes data to the serial port.
 */
qint64 IO::Drivers::UART::write(const QByteArray& data)
{
  if (m_runner || m_breakHeld)
    return 0;

  if (isWritable())
    return port()->write(data);

  return 0;
}

/**
 * @brief Returns the index of the port whose system location matches, or -1 when none does.
 */
static int matchPortByLocation(const QVector<QSerialPortInfo>& ports, const QString& location)
{
  for (int i = 0; i < ports.count(); ++i)
    if (ports.at(i).systemLocation() == location)
      return i;

  return -1;
}

/**
 * @brief Opens the currently selected serial port with the specified mode.
 */
bool IO::Drivers::UART::open(const QIODevice::OpenMode mode)
{
  if (m_deviceNames.isEmpty())
    refreshSerialDevices();

  auto ports  = portList();
  auto portId = portIndex();
  if (portId >= 1 && portId < ports.count()) {
    close();
    m_portIndex = portId;
    Q_EMIT portIndexChanged();

    const auto name = ports.at(portId);
    m_lastPortName  = name;

    if (m_deviceNames.contains(name)) {
      const auto target =
        portId < m_deviceLocations.count() ? m_deviceLocations.at(portId) : QString();
      const auto live = SerialPorts::visiblePorts();

      const int matchIndex = matchPortByLocation(live, target);
      if (matchIndex < 0) {
        close();
        return false;
      }

      m_usingCustomSerialPort = false;
      m_port                  = new QSerialPort(live.at(matchIndex));
    }

    else if (m_customDevices.contains(name)) {
      m_usingCustomSerialPort = true;
      m_port                  = new QSerialPort(name);
    }

    if (!m_port)
      return false;

    port()->setParity(parity());
    if (!port()->setBaudRate(baudRate())) {
      logDriverError(tr("Failed to set baud rate"),
                     tr("Baud rate %1 rejected for port \"%2\": %3")
                       .arg(QString::number(baudRate()), name, port()->errorString()));
    }
    port()->setDataBits(dataBits());
    port()->setStopBits(stopBits());
    port()->setFlowControl(flowControl());
    port()->setReadBufferSize(UartSequencePort::readBufferSize(baudRate()));

    if (port()->open(mode)) {
      connect(port(), &QSerialPort::errorOccurred, this, &IO::Drivers::UART::handleError);
      connect(port(), &QIODevice::readyRead, this, &IO::Drivers::UART::onReadyRead);
      port()->setDataTerminalReady(dtrEnabled());
      if (m_rtsEnabled && UartPolicy::rtsSettable(flowControl()))
        applyRts(true);

      UartSequencePort::tuneNativeBuffer(port(), baudRate());
      if (!m_sequence.isEmpty())
        startSequence(SequenceStart::OnConnect);

      return true;
    }

    else {
      logDriverError(tr("Failed to connect to serial port \"%1\"").arg(name),
                     port()->errorString());
    }
  }

  close();
  return false;
}

//--------------------------------------------------------------------------------------------------
// Driver specifics
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the underlying QSerialPort instance.
 */
QSerialPort* IO::Drivers::UART::port() const
{
  return m_port;
}

/**
 * @brief Returns true when auto-reconnect on resource loss is enabled.
 */
bool IO::Drivers::UART::autoReconnect() const
{
  return m_autoReconnect;
}

/**
 * @brief Returns true when the DTR signal is asserted.
 */
bool IO::Drivers::UART::dtrEnabled() const
{
  return m_dtrEnabled;
}

/**
 * @brief Returns true when RTS is asserted on open and held for the connection (Hold RTS).
 */
bool IO::Drivers::UART::rtsEnabled() const
{
  return m_rtsEnabled;
}

/**
 * @brief Returns the index of the currently selected serial port.
 */
quint8 IO::Drivers::UART::portIndex() const
{
  return m_portIndex;
}

/**
 * @brief Returns the index of the currently selected parity setting.
 */
quint8 IO::Drivers::UART::parityIndex() const
{
  return m_parityIndex;
}

/**
 * @brief Returns the index of the currently selected data bits setting.
 */
quint8 IO::Drivers::UART::dataBitsIndex() const
{
  return m_dataBitsIndex;
}

/**
 * @brief Returns the index of the currently selected stop bits setting.
 */
quint8 IO::Drivers::UART::stopBitsIndex() const
{
  return m_stopBitsIndex;
}

/**
 * @brief Returns the index of the currently selected flow control setting.
 */
quint8 IO::Drivers::UART::flowControlIndex() const
{
  return m_flowControlIndex;
}

/**
 * @brief Returns a list with the available serial devices/ports (with "Select Port" placeholder at
 * index 0).
 */
QStringList IO::Drivers::UART::portList() const
{
  if (m_deviceNames.count() > 0)
    return m_deviceNames + m_customDevices;

  else
    return QStringList{tr("Select Port")};
}

/**
 * @brief Returns a list with the available baud rate configurations.
 */
QStringList IO::Drivers::UART::baudRateList() const
{
  QSet<qint32> baudSet = {110,
                          150,
                          300,
                          1200,
                          2400,
                          4800,
                          9600,
                          19200,
                          38400,
                          57600,
                          115200,
                          230400,
                          256000,
                          460800,
                          576000,
                          921600};

  QList<qint32> sortedList = baudSet.values();
  std::sort(sortedList.begin(), sortedList.end());

  QStringList result;
  result.reserve(sortedList.size());
  for (qint32 rate : std::as_const(sortedList))
    result.append(QString::number(rate));

  return result;
}

/**
 * @brief Returns a list with the available parity configurations.
 */
QStringList IO::Drivers::UART::parityList() const
{
  QStringList list;
  list.append(tr("None"));
  list.append(tr("Even"));
  list.append(tr("Odd"));
  list.append(tr("Space"));
  list.append(tr("Mark"));
  return list;
}

/**
 * @brief Returns a list with the available data bits configurations.
 */
QStringList IO::Drivers::UART::dataBitsList() const
{
  QStringList list;
  list.append(QStringLiteral("5"));
  list.append(QStringLiteral("6"));
  list.append(QStringLiteral("7"));
  list.append(QStringLiteral("8"));
  return list;
}

/**
 * @brief Returns a list with the available stop bits configurations.
 */
QStringList IO::Drivers::UART::stopBitsList() const
{
  QStringList list;
  list.append(QStringLiteral("1"));
  list.append(QStringLiteral("1.5"));
  list.append(QStringLiteral("2"));
  return list;
}

/**
 * @brief Returns a list with the available flow control configurations.
 */
QStringList IO::Drivers::UART::flowControlList() const
{
  QStringList list;
  list.append(tr("None"));
  list.append(tr("RTS/CTS"));
  list.append(tr("XON/XOFF"));
  return list;
}

/**
 * @brief Returns the active parity setting as a QSerialPort enum.
 */
QSerialPort::Parity IO::Drivers::UART::parity() const
{
  return m_parity;
}

/**
 * @brief Returns the configured baud rate.
 */
qint32 IO::Drivers::UART::baudRate() const
{
  return m_baudRate;
}

/**
 * @brief Returns the rate the open port runs at, which a Connection Sequence or the API may have
 *        changed; the saved baudRate() when the port is closed.
 */
qint32 IO::Drivers::UART::liveBaudRate() const
{
  return portOpen() ? port()->baudRate() : m_baudRate;
}

/**
 * @brief Returns the active data bits setting as a QSerialPort enum.
 */
QSerialPort::DataBits IO::Drivers::UART::dataBits() const
{
  return m_dataBits;
}

/**
 * @brief Returns the active stop bits setting as a QSerialPort enum.
 */
QSerialPort::StopBits IO::Drivers::UART::stopBits() const
{
  return m_stopBits;
}

/**
 * @brief Returns the active flow control setting as a QSerialPort enum.
 */
QSerialPort::FlowControl IO::Drivers::UART::flowControl() const
{
  return m_flowControl;
}

/**
 * @brief Configures the signal/slot connections with the rest of the modules.
 */
void IO::Drivers::UART::setupExternalConnections()
{
  connect(&Core::services().timerEvents,
          &Misc::TimerEvents::timeout1Hz,
          this,
          &IO::Drivers::UART::refreshSerialDevices);

  connect(&Core::services().translator,
          &Misc::Translator::languageChanged,
          this,
          &IO::Drivers::UART::languageChanged);
}

/**
 * @brief Changes the baud rate of the serial port.
 */
void IO::Drivers::UART::setBaudRate(const qint32 rate)
{
  if (m_baudRate != rate && rate > 0) {
    m_baudRate = rate;
    m_settings.setValue("IO_Serial_Baud_Rate", rate);

    if (port() && !m_runner) {
      if (!port()->setBaudRate(baudRate())) {
        logDriverError(
          tr("Failed to set baud rate"),
          tr("Baud rate %1 rejected: %2").arg(QString::number(baudRate()), port()->errorString()));
      }
    }

    Q_EMIT baudRateChanged();
  }
}

/**
 * @brief Sets the Data Terminal Ready (DTR) signal state.
 */
void IO::Drivers::UART::setDtrEnabled(const bool enabled)
{
  if (m_dtrEnabled == enabled)
    return;

  m_dtrEnabled = enabled;
  m_settings.setValue("UartDriver/dtr", enabled);

  if (portOpen() && !m_runner)
    port()->setDataTerminalReady(enabled);

  Q_EMIT dtrEnabledChanged();
}

/**
 * @brief Sets Hold RTS. Applied to an open port only outside RTS/CTS flow control, where the port
 *        driver owns the line, and never while a Connection Sequence drives the port.
 */
void IO::Drivers::UART::setRtsEnabled(const bool enabled)
{
  if (m_rtsEnabled == enabled)
    return;

  m_rtsEnabled = enabled;
  m_settings.setValue("UartDriver/rts", enabled);

  if (portOpen() && !m_runner && UartPolicy::rtsSettable(port()->flowControl()))
    applyRts(enabled);

  Q_EMIT rtsEnabledChanged();
}

/**
 * @brief Changes the port index value, later used by openSerialPort(). Selecting the placeholder
 *        CLEARS the persisted selection: leaving it behind let the 1 Hz rescan re-select the old
 *        port within a second of the user choosing "Select Port".
 */
void IO::Drivers::UART::setPortIndex(const quint8 portIndex)
{
  if (m_deviceNames.isEmpty())
    refreshSerialDevices();

  const quint8 clamped = (portIndex < portList().count()) ? portIndex : 0;
  if (portIndex == m_portIndex && clamped == m_portIndex)
    return;

  m_portIndex = clamped;

  const auto name = portList().at(m_portIndex);
  if (!name.isEmpty() && m_portIndex > 0)
    m_settings.setValue("IO_Serial_SelectedDevice", name);

  else if (m_portIndex == 0)
    m_settings.remove("IO_Serial_SelectedDevice");

  Q_EMIT portIndexChanged();
}

/**
 * @brief Registers a custom serial device by path. A bad path is reported through the console and
 *        a queued notification, never a modal: this is reachable from a project load and from the
 *        API, where a dialog would block a non-interactive caller (spec 0056).
 */
void IO::Drivers::UART::registerDevice(const QString& device)
{
  const auto trimmedPath = device.simplified();

  QFile path(trimmedPath);
  if (!path.exists()) {
    logDriverError(tr("\"%1\" is not a valid path").arg(trimmedPath),
                   tr("Please type another path to register a custom serial device"));

    auto* bus = messageBus();
    if (bus)
      bus->publish<Core::Bus::NotificationRaised>(
        Core::Bus::kSeverityWarning,
        QStringLiteral("UART"),
        QString(),
        tr("Serial device not registered"),
        tr("\"%1\" is not a valid path; type another path to register a custom serial device.")
          .arg(trimmedPath));

    return;
  }

  if (m_customDevices.contains(trimmedPath))
    return;

  m_customDevices.append(trimmedPath);
  Q_EMIT availablePortsChanged();
}

/**
 * @brief Sets the serial port parity by index from parityList().
 */
void IO::Drivers::UART::setParity(const quint8 parityIndex)
{
  if (parityIndex >= parityList().count()) {
    qWarning() << "UART::setParity: index" << parityIndex << "out of range";
    return;
  }

  if (m_parityIndex == parityIndex)
    return;

  m_parityIndex = parityIndex;
  m_settings.setValue("UartDriver/parity", parityIndex);

  switch (parityIndex) {
    case 0:
      m_parity = QSerialPort::NoParity;
      break;
    case 1:
      m_parity = QSerialPort::EvenParity;
      break;
    case 2:
      m_parity = QSerialPort::OddParity;
      break;
    case 3:
      m_parity = QSerialPort::SpaceParity;
      break;
    case 4:
      m_parity = QSerialPort::MarkParity;
      break;
  }

  if (port() && !m_runner)
    port()->setParity(parity());

  Q_EMIT parityChanged();
}

/**
 * @brief Changes the data bits of the serial port.
 */
void IO::Drivers::UART::setDataBits(const quint8 dataBitsIndex)
{
  if (dataBitsIndex >= dataBitsList().count()) {
    qWarning() << "UART::setDataBits: index" << dataBitsIndex << "out of range";
    return;
  }

  if (m_dataBitsIndex == dataBitsIndex)
    return;

  m_dataBitsIndex = dataBitsIndex;
  m_settings.setValue("UartDriver/dataBits", dataBitsIndex);

  switch (dataBitsIndex) {
    case 0:
      m_dataBits = QSerialPort::Data5;
      break;
    case 1:
      m_dataBits = QSerialPort::Data6;
      break;
    case 2:
      m_dataBits = QSerialPort::Data7;
      break;
    case 3:
      m_dataBits = QSerialPort::Data8;
      break;
  }

  if (port() && !m_runner)
    port()->setDataBits(dataBits());

  Q_EMIT dataBitsChanged();
}

/**
 * @brief Changes the stop bits of the serial port.
 */
void IO::Drivers::UART::setStopBits(const quint8 stopBitsIndex)
{
  if (stopBitsIndex >= stopBitsList().count()) {
    qWarning() << "UART::setStopBits: index" << stopBitsIndex << "out of range";
    return;
  }

  if (m_stopBitsIndex == stopBitsIndex)
    return;

  m_stopBitsIndex = stopBitsIndex;
  m_settings.setValue("UartDriver/stopBits", stopBitsIndex);

  switch (stopBitsIndex) {
    case 0:
      m_stopBits = QSerialPort::OneStop;
      break;
    case 1:
      m_stopBits = QSerialPort::OneAndHalfStop;
      break;
    case 2:
      m_stopBits = QSerialPort::TwoStop;
      break;
  }

  if (port() && !m_runner)
    port()->setStopBits(stopBits());

  Q_EMIT stopBitsChanged();
}

/**
 * @brief Enables or disables the auto-reconnect feature.
 */
void IO::Drivers::UART::setAutoReconnect(const bool autoreconnect)
{
  if (m_autoReconnect == autoreconnect)
    return;

  m_autoReconnect = autoreconnect;
  m_settings.setValue("UartDriver/autoReconnect", autoreconnect);
  Q_EMIT autoReconnectChanged();
}

/**
 * @brief Changes the flow control option of the serial port.
 */
void IO::Drivers::UART::setFlowControl(const quint8 flowControlIndex)
{
  if (flowControlIndex >= flowControlList().count()) {
    qWarning() << "UART::setFlowControl: index" << flowControlIndex << "out of range";
    return;
  }

  if (m_flowControlIndex == flowControlIndex)
    return;

  m_flowControlIndex = flowControlIndex;
  m_settings.setValue("UartDriver/flowControl", flowControlIndex);

  switch (flowControlIndex) {
    case 0:
      m_flowControl = QSerialPort::NoFlowControl;
      break;
    case 1:
      m_flowControl = QSerialPort::HardwareControl;
      break;
    case 2:
      m_flowControl = QSerialPort::SoftwareControl;
      break;
  }

  if (port() && !m_runner)
    port()->setFlowControl(flowControl());

  Q_EMIT flowControlChanged();
}

/**
 * @brief Scans for available serial ports and rebuilds the device list.
 */
void IO::Drivers::UART::refreshSerialDevices()
{
  const auto listing          = SerialPorts::listPorts();
  const QStringList names     = QStringList(tr("Select Port")) + listing.labels;
  const QStringList locations = QStringList(QStringLiteral("/dev/null")) + listing.locations;

  if (m_deviceNames == names)
    return;

  m_deviceNames     = names;
  m_deviceLocations = locations;

  const bool indexChanged = relocateOpenPortIndex(listing.ports);

  Q_EMIT availablePortsChanged();

  if (indexChanged)
    Q_EMIT portIndexChanged();

  if (m_portIndex != 0)
    return;

  const auto lastPort = m_settings.value("IO_Serial_SelectedDevice", "").toString();
  const int lastIndex = SerialPorts::indexOfPort(portList(), lastPort);
  if (lastIndex > 0)
    setPortIndex(static_cast<quint8>(lastIndex));
}

/**
 * @brief Retry poll after a resource-loss drop, on this instance's own timer so live drivers
 *        recover too. handleError() sets the flag and close() clears it, so a manual disconnect
 *        ends the retry; the port is matched by name and the reopen is scoped to this driver.
 */
void IO::Drivers::UART::pollAutoReconnect()
{
  if (!m_pendingReconnect || !autoReconnect() || portOpen()) {
    m_reconnectTimer.stop();
    return;
  }

  refreshSerialDevices();
  const int index = SerialPorts::indexOfPort(portList(), m_lastPortName);
  if (index < 1)
    return;

  m_pendingReconnect = false;
  m_reconnectTimer.stop();
  setPortIndex(static_cast<quint8>(index));
  connectionManager().connectDevice(this);
}

/**
 * @brief Ends the link on a fatal port error and hands the recovery to the driver's own opt-in
 *        auto-reconnect when it owns it. A custom device path no longer swallows ResourceError:
 *        an unplugged by-id node or pty left the port "open" for the rest of the session. During
 *        a connect-time Connection Sequence the error is that sequence's verdict, naming the step.
 */
void IO::Drivers::UART::handleError(QSerialPort::SerialPortError error)
{
  if (!portOpen())
    return;

  if (!UartPolicy::isFatalPortError(error, m_usingCustomSerialPort))
    return;

  const bool dialing = m_sequenceConnecting;
  const auto name    = port()->portName();
  const auto reason  = m_errorDescriptions.value(error, tr("Unknown error"));
  if (dialing) {
    const int step  = m_runner ? qMax(m_runner->currentStep(), 0) : 0;
    const auto kind = IO::ConnectionSequence::kindLabel(m_sequence.steps.value(step).kind);
    onSequenceFinished(false, UartPolicy::sequenceFailureText(kind, step, reason, {}), step);
  } else {
    connectionManager().disconnectDevice(this);
  }

  if (!UartPolicy::shouldAutoReconnect(error, m_autoReconnect)) {
    if (!dialing)
      logDriverError(tr("Critical error on serial port \"%1\"").arg(name), reason);

    return;
  }

  m_pendingReconnect = true;
  m_reconnectTimer.start();
}

/**
 * @brief Reads all the data from the serial port. While a Connection Sequence owns the port the
 *        bytes reach its runner and the console only, never the frame pipeline; while BREAK is
 *        held they are dropped.
 */
void IO::Drivers::UART::onReadyRead()
{
  if (!portOpen())
    return;

  auto data = port()->readAll();
  if (m_breakHeld)
    return;

  if (!m_runner) {
    publishReceivedData(std::move(data));
    return;
  }

  if (m_runner->breakHeld())
    return;

  auto* runner = m_runner.get();
  publishConsoleData(QByteArray(data));
  runner->feed(data);
}

/**
 * @brief Populates the error descriptions for the serial port driver.
 */
void IO::Drivers::UART::populateErrors()
{
  // clang-format off
  m_errorDescriptions.clear();
  m_errorDescriptions.insert(QSerialPort::NoError, tr("No error occurred."));
  m_errorDescriptions.insert(QSerialPort::DeviceNotFoundError, tr("The specified device could not be found. Check the connection and try again."));
  m_errorDescriptions.insert(QSerialPort::PermissionError, tr("Permission denied. Ensure the application has the necessary access rights to the device."));
  m_errorDescriptions.insert(QSerialPort::OpenError, tr("Failed to open the device. It may already be in use or unavailable."));
  m_errorDescriptions.insert(QSerialPort::WriteError, tr("An error occurred while writing data to the device."));
  m_errorDescriptions.insert(QSerialPort::ReadError, tr("An error occurred while reading data from the device."));
  m_errorDescriptions.insert(QSerialPort::ResourceError, tr("A critical resource error occurred. The device may have been disconnected or is no longer accessible."));
  m_errorDescriptions.insert(QSerialPort::UnsupportedOperationError, tr("The requested operation is not supported on this device."));
  m_errorDescriptions.insert(QSerialPort::UnknownError, tr("An unknown error occurred. Check the device and try again."));
  m_errorDescriptions.insert(QSerialPort::TimeoutError, tr("The operation timed out. The device may not be responding."));
  m_errorDescriptions.insert(QSerialPort::NotOpenError, tr("The device is not open. Open the device before attempting this operation."));
  // clang-format on
}

/**
 * @brief Re-locates the open port in the new device list, updating m_portIndex if matched.
 */
bool IO::Drivers::UART::relocateOpenPortIndex(const QVector<QSerialPortInfo>& ports)
{
  if (!port())
    return false;

  const auto name = port()->portName();
  for (int i = 0; i < ports.count(); ++i) {
    if (ports.at(i).portName() != name)
      continue;

    m_portIndex = i + 1;
    return true;
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Line control & Connection Sequence (spec 0103)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Reads the modem lines of the open port; empty when it is closed.
 */
QJsonObject IO::Drivers::UART::lineStates() const
{
  if (!portOpen())
    return {};

  const auto pins = port()->pinoutSignals();
  QJsonObject states;
  states.insert(QStringLiteral("cts"), pins.testFlag(QSerialPort::ClearToSendSignal));
  states.insert(QStringLiteral("dsr"), pins.testFlag(QSerialPort::DataSetReadySignal));
  states.insert(QStringLiteral("dcd"), pins.testFlag(QSerialPort::DataCarrierDetectSignal));
  states.insert(QStringLiteral("ri"), pins.testFlag(QSerialPort::RingIndicatorSignal));
  states.insert(QStringLiteral("rts"), pins.testFlag(QSerialPort::RequestToSendSignal));
  states.insert(QStringLiteral("dtr"), pins.testFlag(QSerialPort::DataTerminalReadySignal));
  return states;
}

/**
 * @brief Holds the open line in BREAK for @p ms milliseconds, then releases it and discards what
 *        arrived meanwhile. Refused while a BREAK or a Connection Sequence already owns the line.
 */
bool IO::Drivers::UART::sendBreak(int ms)
{
  if (!isOpen() || m_runner || m_breakHeld || ms < 1 || ms > IO::kMaxBreakMs)
    return false;

  SS_ASSERT_LOG(m_breakTimer == Async::kInvalidTimerId);
  if (!port()->setBreakEnabled(true))
    return false;

  m_breakHeld  = true;
  m_breakTimer = m_clock.schedule(ms, [this]() {
    m_breakTimer = Async::kInvalidTimerId;
    releaseBreak();
  });

  SS_ASSERT_LOG(m_breakTimer != Async::kInvalidTimerId);
  return true;
}

/**
 * @brief Ends a BREAK sendBreak() started: stops its timer, releases the line and discards the
 *        bytes the device sent while it was held. A port that refuses the release would swallow
 *        every write that follows, so the link is closed and the failure logged instead.
 */
void IO::Drivers::UART::releaseBreak()
{
  m_clock.cancel(m_breakTimer);
  m_breakTimer = Async::kInvalidTimerId;
  if (!m_breakHeld)
    return;

  m_breakHeld = false;
  if (!portOpen())
    return;

  if (!port()->setBreakEnabled(false)) [[unlikely]] {
    logDriverError(tr("BREAK release failed"),
                   tr("The serial port refused to release the BREAK condition; disconnecting."));
    connectionManager().disconnectDevice(this);
    return;
  }

  (void)port()->clear(QSerialPort::Input);
  SS_ASSERT_LOG(!m_breakHeld);
}

/**
 * @brief Changes the rate of the open port without closing it; the saved rate, and so every
 *        reopen, stays as it was. Refused while a Connection Sequence owns the port.
 */
bool IO::Drivers::UART::setLiveBaudRate(qint32 rate)
{
  if (!isOpen() || m_runner || !UartSequencePort::applyBaud(*port(), rate))
    return false;

  Q_EMIT liveBaudRateChanged();
  return true;
}

/**
 * @brief Re-runs the Connection Sequence on the connected port, for a device power-cycled while
 *        the port stayed open. The link stays connected; frames pause until the run ends.
 */
bool IO::Drivers::UART::runSequence()
{
  if (!isOpen() || m_runner || m_breakHeld || m_sequence.isEmpty())
    return false;

  startSequence(SequenceStart::OnDemand);
  return true;
}

/**
 * @brief Returns the Connection Sequence as compact JSON text, the editor dialog's payload.
 */
QString IO::Drivers::UART::connectionSequenceJson() const
{
  return QString::fromUtf8(QJsonDocument(m_sequence.toJson()).toJson(QJsonDocument::Compact));
}

/**
 * @brief Validates a Connection Sequence given as JSON text: the first problem's step index (-1
 *        for the whole sequence) and message, an empty message when it is valid.
 */
QVariantMap IO::Drivers::UART::validateConnectionSequence(const QString& json) const
{
  const auto error = IO::ConnectionSequence::fromVariant(json).validate();
  QVariantMap result;
  result.insert(QStringLiteral("step"), error.step);
  result.insert(QStringLiteral("message"), error.message);
  return result;
}

/**
 * @brief Replaces the Connection Sequence from a QJsonObject, a QVariantMap or JSON text; an
 *        empty value clears it. It runs on the next open, never on the one in progress.
 */
void IO::Drivers::UART::setConnectionSequence(const QVariant& value)
{
  const auto sequence = IO::ConnectionSequence::fromVariant(value);
  if (sequence.toJson() == m_sequence.toJson())
    return;

  m_sequence = sequence;
  m_settings.setValue("UartDriver/connectionSequence", connectionSequenceJson());
  Q_EMIT connectionSequenceChanged();
}

/**
 * @brief Hands the open port to a runner. A connect-time run keeps isOpen() false until its
 *        verdict; a re-run keeps the link connected. Either way bytes go to the runner and writes
 *        are refused until it ends. The start is queued, so open() returns before the first step.
 */
void IO::Drivers::UART::startSequence(SequenceStart start)
{
  SS_ASSERT(portOpen(), return);
  SS_ASSERT_LOG(!m_sequence.isEmpty());

  stopSequence();
  m_sequenceConnecting = start == SequenceStart::OnConnect;
  m_sequencePort       = std::make_unique<UartSequencePort>(port());
  m_runner             = std::make_unique<SequenceRunner>(m_sequence, *m_sequencePort, m_clock);

  auto* runner    = m_runner.get();
  const auto note = [this](int, const QString& text) {
    publishSequenceNote(text);
  };
  connect(runner, &SequenceRunner::stepNote, this, note);
  connect(runner, &SequenceRunner::stepStarted, this, note);
  connect(runner, &SequenceRunner::bytesSent, this, &UART::publishSentData);
  connect(runner, &SequenceRunner::finished, this, &UART::onSequenceFinished);
  QMetaObject::invokeMethod(runner, &SequenceRunner::start, Qt::QueuedConnection);
}

/**
 * @brief Drives RTS on the open port and logs a refusal (an adapter without the line, or one whose
 *        flow control owns it), so a Hold RTS the device never saw is reported, not silent.
 */
void IO::Drivers::UART::applyRts(bool on)
{
  SS_ASSERT(portOpen(), return);

  if (!port()->setRequestToSend(on)) [[unlikely]]
    logDriverError(tr("Hold RTS failed"),
                   tr("The serial port refused to set RTS: %1").arg(port()->errorString()));
}

/**
 * @brief Cancels a running sequence without a verdict (a user disconnect is not a failed step)
 *        and retires the runner later, since this may run inside the runner's own emission.
 */
void IO::Drivers::UART::stopSequence()
{
  m_sequenceConnecting = false;
  if (!m_runner)
    return;

  m_runner->cancel();
  m_runner.release()->deleteLater();
  SS_ASSERT_LOG(!m_runner);
}

/**
 * @brief Settles a run. A connect-time run reports the open verdict once through the latch:
 *        connected after the last step, or a logged driver error naming the step. A re-run
 *        reports through the console and, on failure, the notification center.
 */
void IO::Drivers::UART::onSequenceFinished(bool ok, const QString& reason, int stepIndex)
{
  SS_ASSERT_LOG(stepIndex >= -1);
  SS_ASSERT_LOG(m_runner != nullptr);

  const bool connecting = m_sequenceConnecting;
  stopSequence();
  Q_EMIT liveBaudRateChanged();
  publishSequenceNote(ok ? tr("Completed, the device is ready") : reason);
  if (ok) {
    if (connecting) {
      reportOpenFinished(true);
      Q_EMIT configurationChanged();
    }

    return;
  }

  logDriverError(tr("Connection Sequence failed"), reason);
  if (connecting) {
    reportOpenFinished(false, reason);
    if (portOpen())
      connectionManager().disconnectDevice(this);

    return;
  }

  auto* bus = messageBus();
  if (bus)
    bus->publish<Core::Bus::NotificationRaised>(Core::Bus::kSeverityWarning,
                                                QStringLiteral("UART"),
                                                QString(),
                                                tr("Connection Sequence failed"),
                                                reason);
}

/**
 * @brief Publishes one Connection Sequence console line for the source this driver backs.
 */
void IO::Drivers::UART::publishSequenceNote(const QString& text)
{
  auto* bus = messageBus();
  if (!bus || text.isEmpty())
    return;

  const int sourceId = qMax(0, connectionManager().deviceIdForDriver(this));
  bus->publish<Core::Bus::LinkSequenceNote>(sourceId, text);
}

//--------------------------------------------------------------------------------------------------
// Stable device identification
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns cross-platform hardware identifiers for the currently selected serial port.
 */
QJsonObject IO::Drivers::UART::deviceIdentifier() const
{
  if (m_portIndex < 1)
    return {};

  const auto ports = SerialPorts::visiblePorts();
  const int idx    = m_portIndex - 1;
  if (idx < 0 || idx >= ports.count())
    return {};

  return SerialPorts::identity(ports.at(idx));
}

/**
 * @brief Names the serial port this driver claims, so a project pointing two sources at one port
 *        is refused before either of them opens it.
 */
QString IO::Drivers::UART::exclusiveResource() const
{
  if (m_portIndex < 1)
    return {};

  return SerialPorts::resourceName(portList().value(m_portIndex));
}

/**
 * @brief Tries to find and select a serial port matching a previously saved identifier.
 */
bool IO::Drivers::UART::selectByIdentifier(const QJsonObject& id)
{
  if (id.isEmpty())
    return false;

  if (m_deviceNames.isEmpty())
    refreshSerialDevices();

  const auto ports = SerialPorts::visiblePorts();

  int bestScore = 0;
  int bestIndex = -1;

  for (int i = 0; i < ports.count(); ++i) {
    const int score = SerialPorts::scoreIdentityMatch(SerialPorts::identity(ports.at(i)), id);
    if (score > bestScore) {
      bestScore = score;
      bestIndex = i;
    }
  }

  if (bestIndex < 0)
    return false;

  setPortIndex(static_cast<quint8>(bestIndex + 1));
  return true;
}

//--------------------------------------------------------------------------------------------------
// Driver property model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the UART configuration as a flat list of editable properties.
 */
QList<IO::DriverProperty> IO::Drivers::UART::driverProperties() const
{
  QList<IO::DriverProperty> props;

  IO::DriverProperty port;
  port.key     = QStringLiteral("portIndex");
  port.label   = tr("Serial Port");
  port.type    = IO::DriverProperty::ComboBox;
  port.value   = m_portIndex;
  port.options = portList();
  props.append(port);

  IO::DriverProperty baud;
  baud.key   = QStringLiteral("baudRate");
  baud.label = tr("Baud Rate");
  baud.type  = IO::DriverProperty::IntField;
  baud.value = m_baudRate;
  baud.min   = 1;
  props.append(baud);

  IO::DriverProperty parity;
  parity.key     = QStringLiteral("parityIndex");
  parity.label   = tr("Parity");
  parity.type    = IO::DriverProperty::ComboBox;
  parity.value   = m_parityIndex;
  parity.options = parityList();
  props.append(parity);

  IO::DriverProperty data;
  data.key     = QStringLiteral("dataBitsIndex");
  data.label   = tr("Data Bits");
  data.type    = IO::DriverProperty::ComboBox;
  data.value   = m_dataBitsIndex;
  data.options = dataBitsList();
  props.append(data);

  IO::DriverProperty stop;
  stop.key     = QStringLiteral("stopBitsIndex");
  stop.label   = tr("Stop Bits");
  stop.type    = IO::DriverProperty::ComboBox;
  stop.value   = m_stopBitsIndex;
  stop.options = stopBitsList();
  props.append(stop);

  IO::DriverProperty flow;
  flow.key     = QStringLiteral("flowControlIndex");
  flow.label   = tr("Flow Control");
  flow.type    = IO::DriverProperty::ComboBox;
  flow.value   = m_flowControlIndex;
  flow.options = flowControlList();
  props.append(flow);

  IO::DriverProperty dtr;
  dtr.key   = QStringLiteral("dtr");
  dtr.label = tr("DTR");
  dtr.type  = IO::DriverProperty::CheckBox;
  dtr.value = m_dtrEnabled;
  props.append(dtr);

  IO::DriverProperty rts;
  rts.key   = QStringLiteral("rts");
  rts.label = tr("Hold RTS");
  rts.type  = IO::DriverProperty::CheckBox;
  rts.value = m_rtsEnabled;
  rts.showWhen(QStringLiteral("flowControlIndex"), {0, 2});
  props.append(rts);

  IO::DriverProperty reconnect;
  reconnect.key   = QStringLiteral("autoReconnect");
  reconnect.label = tr("Auto-Reconnect");
  reconnect.type  = IO::DriverProperty::CheckBox;
  reconnect.value = m_autoReconnect;
  props.append(reconnect);

  IO::DriverProperty sequence;
  sequence.key   = QStringLiteral("connectionSequence");
  sequence.type  = IO::DriverProperty::Text;
  sequence.value = QVariant::fromValue(m_sequence.toJson());
  props.append(sequence);

  return props;
}

/**
 * @brief Applies a single UART configuration change by key.
 */
void IO::Drivers::UART::setDriverProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("portIndex")) {
    setPortIndex(static_cast<quint8>(value.toInt()));
    return;
  }

  if (key == QLatin1String("parityIndex")) {
    setParity(static_cast<quint8>(value.toInt()));
    return;
  }

  if (key == QLatin1String("dataBitsIndex")) {
    setDataBits(static_cast<quint8>(value.toInt()));
    return;
  }

  if (key == QLatin1String("stopBitsIndex")) {
    setStopBits(static_cast<quint8>(value.toInt()));
    return;
  }

  if (key == QLatin1String("flowControlIndex")) {
    setFlowControl(static_cast<quint8>(value.toInt()));
    return;
  }

  if (key == QLatin1String("dtr")) {
    setDtrEnabled(value.toBool());
    return;
  }

  if (key == QLatin1String("rts")) {
    setRtsEnabled(value.toBool());
    return;
  }

  if (key == QLatin1String("connectionSequence")) {
    setConnectionSequence(value);
    return;
  }

  if (key == QLatin1String("autoReconnect")) {
    setAutoReconnect(value.toBool());
    return;
  }

  if (key == QLatin1String("baudRate")) {
    applyBaudRateProperty(value);
    return;
  }

  if (key == QLatin1String("device"))
    applyDeviceProperty(value);
}

/**
 * @brief Applies a source's saved settings. A non-empty set saved before spec 0103 has no "rts"
 *        or "connectionSequence" key and must behave as it did then, never inherit whatever the
 *        Setup Panel last held: a sequence meant for another device would run on this one.
 */
void IO::Drivers::UART::applyConnectionSettings(const QJsonObject& settings)
{
  HAL_Driver::applyConnectionSettings(settings);
  if (settings.isEmpty())
    return;

  if (!settings.contains(QStringLiteral("rts")))
    setRtsEnabled(false);

  if (!settings.contains(QStringLiteral("connectionSequence")))
    setConnectionSequence(QVariant());
}

/**
 * @brief Applies the "baudRate" property by raw rate or by index into baudRateList().
 */
void IO::Drivers::UART::applyBaudRateProperty(const QVariant& value)
{
  const int v = value.toInt();
  if (v >= 110) {
    setBaudRate(v);
    return;
  }

  const auto list = baudRateList();
  if (v >= 0 && v < list.size())
    setBaudRate(list.at(v).toInt());
}

/**
 * @brief Applies the "device" property by registering a custom path and selecting it.
 */
void IO::Drivers::UART::applyDeviceProperty(const QVariant& value)
{
  const auto path = value.toString().simplified();
  if (path.isEmpty())
    return;

  if (m_deviceNames.isEmpty())
    refreshSerialDevices();

  registerDevice(path);
  const auto ports = portList();
  const int idx    = ports.indexOf(path);
  if (idx >= 1)
    setPortIndex(static_cast<quint8>(idx));
}
