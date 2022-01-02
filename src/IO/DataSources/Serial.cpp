/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <IO/Manager.h>
#include <Misc/Utilities.h>
#include <Misc/TimerEvents.h>
#include <IO/DataSources/Serial.h>

/**
 * Constructor function
 */
IO::DataSources::Serial::Serial()
    : m_port(Q_NULLPTR)
    , m_autoReconnect(false)
    , m_lastSerialDeviceIndex(0)
    , m_portIndex(0)
{
    // Read settings
    readSettings();

    // Init serial port configuration variables
    setBaudRate(9600);
    disconnectDevice();
    setDataBits(dataBitsList().indexOf("8"));
    setStopBits(stopBitsList().indexOf("1"));
    setParity(parityList().indexOf(tr("None")));
    setFlowControl(flowControlList().indexOf(tr("None")));

    // Build serial devices list and refresh it every second
    // clang-format off
    connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::timeout1Hz,
            this, &IO::DataSources::Serial::refreshSerialDevices);
    // clang-format on
}

/**
 * Destructor function, closes the serial port before exiting the application and saves
 * the user's baud rate list settings.
 */
IO::DataSources::Serial::~Serial()
{
    writeSettings();

    if (port())
        disconnectDevice();
}

/**
 * Returns the only instance of the class
 */
IO::DataSources::Serial &IO::DataSources::Serial::instance()
{
    static Serial singleton;
    return singleton;
}

/**
 * Returns the pointer to the current serial port handler
 */
QSerialPort *IO::DataSources::Serial::port() const
{
    return m_port;
}

/**
 * Returns the name of the current serial port device
 */
QString IO::DataSources::Serial::portName() const
{
    if (port())
        return port()->portName();

    return tr("No Device");
}

/**
 * Returns @c true if auto-reconnect is enabled
 */
bool IO::DataSources::Serial::autoReconnect() const
{
    return m_autoReconnect;
}

/**
 * Returns @c true if the user selects the appropiate controls & options to be able
 * to connect to a serial device
 */
bool IO::DataSources::Serial::configurationOk() const
{
    return portIndex() > 0;
}

/**
 * Returns the index of the current serial device selected by the program.
 */
quint8 IO::DataSources::Serial::portIndex() const
{
    return m_portIndex;
}

/**
 * Returns the correspoding index of the parity configuration in relation
 * to the @c StringList returned by the @c parityList() function.
 */
quint8 IO::DataSources::Serial::parityIndex() const
{
    return m_parityIndex;
}

/**
 * Returns the correspoding index of the data bits configuration in relation
 * to the @c StringList returned by the @c dataBitsList() function.
 */
quint8 IO::DataSources::Serial::dataBitsIndex() const
{
    return m_dataBitsIndex;
}

/**
 * Returns the correspoding index of the stop bits configuration in relation
 * to the @c StringList returned by the @c stopBitsList() function.
 */
quint8 IO::DataSources::Serial::stopBitsIndex() const
{
    return m_stopBitsIndex;
}

/**
 * Returns the correspoding index of the flow control config. in relation
 * to the @c StringList returned by the @c flowControlList() function.
 */
quint8 IO::DataSources::Serial::flowControlIndex() const
{
    return m_flowControlIndex;
}

/**
 * Returns a list with the available serial devices/ports to use.
 * This function can be used with a combo box to build nice UIs.
 *
 * @note The first item of the list will be invalid, since it's value will
 *       be "Select Serial Device". This is inteded to make the user interface
 *       a little more friendly.
 */
StringList IO::DataSources::Serial::portList() const
{
    return m_portList;
}

/**
 * Returns a list with the available parity configurations.
 * This function can be used with a combo-box to build UIs.
 */
StringList IO::DataSources::Serial::parityList() const
{
    StringList list;
    list.append(tr("None"));
    list.append(tr("Even"));
    list.append(tr("Odd"));
    list.append(tr("Space"));
    list.append(tr("Mark"));
    return list;
}

/**
 * Returns a list with the available baud rate configurations.
 * This function can be used with a combo-box to build UIs.
 */
StringList IO::DataSources::Serial::baudRateList() const
{
    return m_baudRateList;
}

/**
 * Returns a list with the available data bits configurations.
 * This function can be used with a combo-box to build UIs.
 */
StringList IO::DataSources::Serial::dataBitsList() const
{
    return StringList { "5", "6", "7", "8" };
}

/**
 * Returns a list with the available stop bits configurations.
 * This function can be used with a combo-box to build UIs.
 */
StringList IO::DataSources::Serial::stopBitsList() const
{
    return StringList { "1", "1.5", "2" };
}

/**
 * Returns a list with the available flow control configurations.
 * This function can be used with a combo-box to build UIs.
 */
StringList IO::DataSources::Serial::flowControlList() const
{
    StringList list;
    list.append(tr("None"));
    list.append("RTS/CTS");
    list.append("XON/XOFF");
    return list;
}

/**
 * Returns the current parity configuration used by the serial port
 * handler object.
 */
QSerialPort::Parity IO::DataSources::Serial::parity() const
{
    return m_parity;
}

/**
 * Returns the current baud rate configuration used by the serial port
 * handler object.
 */
qint32 IO::DataSources::Serial::baudRate() const
{
    return m_baudRate;
}

/**
 * Returns the current data bits configuration used by the serial port
 * handler object.
 */
QSerialPort::DataBits IO::DataSources::Serial::dataBits() const
{
    return m_dataBits;
}

/**
 * Returns the current stop bits configuration used by the serial port
 * handler object.
 */
QSerialPort::StopBits IO::DataSources::Serial::stopBits() const
{
    return m_stopBits;
}

/**
 * Returns the current flow control configuration used by the serial
 * port handler object.
 */
QSerialPort::FlowControl IO::DataSources::Serial::flowControl() const
{
    return m_flowControl;
}

/**
 * Tries to open the serial port with the current configuration
 */
QSerialPort *IO::DataSources::Serial::openSerialPort()
{
    // Ignore the first item of the list (Select Port)
    auto ports = validPorts();
    auto portId = portIndex() - 1;
    if (portId >= 0 && portId < validPorts().count())
    {
        // Update port index variable & disconnect from current serial port
        disconnectDevice();
        m_portIndex = portId + 1;
        m_lastSerialDeviceIndex = m_portIndex;
        Q_EMIT portIndexChanged();

        // Create new serial port handler
        m_port = new QSerialPort(ports.at(portId));

        // Configure serial port
        port()->setParity(parity());
        port()->setBaudRate(baudRate());
        port()->setDataBits(dataBits());
        port()->setStopBits(stopBits());
        port()->setFlowControl(flowControl());

        // Connect signals/slots
        connect(port(), SIGNAL(errorOccurred(QSerialPort::SerialPortError)), this,
                SLOT(handleError(QSerialPort::SerialPortError)));
    }

    // Disconnect serial port
    else
        disconnectDevice();

    // Return pointer
    return port();
}

/**
 * Disconnects from the current serial device and clears temp. data
 */
void IO::DataSources::Serial::disconnectDevice()
{
    // Check if serial port pointer is valid
    if (port() != Q_NULLPTR)
    {
        // Disconnect signals/slots
        port()->disconnect(this, SLOT(handleError(QSerialPort::SerialPortError)));

        // Close & delete serial port handler
        port()->close();
        port()->deleteLater();
    }

    // Reset pointer
    m_port = Q_NULLPTR;
    Q_EMIT portChanged();
    Q_EMIT availablePortsChanged();
}

/**
 * Changes the baud @a rate of the serial port
 */
void IO::DataSources::Serial::setBaudRate(const qint32 rate)
{
    // Asserts
    Q_ASSERT(rate > 10);

    // Update baud rate
    m_baudRate = rate;

    // Update serial port config
    if (port())
        port()->setBaudRate(baudRate());

    // Update user interface
    Q_EMIT baudRateChanged();
}

/**
 * Changes the port index value, this value is later used by the @c openSerialPort()
 * function.
 */
void IO::DataSources::Serial::setPortIndex(const quint8 portIndex)
{
    auto portId = portIndex - 1;
    if (portId >= 0 && portId < validPorts().count())
        m_portIndex = portIndex;
    else
        m_portIndex = 0;

    Q_EMIT portIndexChanged();
}

/**
 * @brief IO::DataSources::Serial::setParity
 * @param parityIndex
 */
void IO::DataSources::Serial::setParity(const quint8 parityIndex)
{
    // Argument verification
    Q_ASSERT(parityIndex < parityList().count());

    // Update current index
    m_parityIndex = parityIndex;

    // Set parity based on current index
    switch (parityIndex)
    {
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

    // Update serial port config.
    if (port())
        port()->setParity(parity());

    // Notify user interface
    Q_EMIT parityChanged();
}

/**
 * Registers the new baud rate to the list
 */
void IO::DataSources::Serial::appendBaudRate(const QString &baudRate)
{
    if (!m_baudRateList.contains(baudRate))
    {
        m_baudRateList.append(baudRate);
        writeSettings();
        Q_EMIT baudRateListChanged();
        Misc::Utilities::showMessageBox(
            tr("Baud rate registered successfully"),
            tr("Rate \"%1\" has been added to baud rate list").arg(baudRate));
    }
}

/**
 * Changes the data bits of the serial port.
 *
 * @note This function is meant to be used with a combobox in the
 *       QML interface
 */
void IO::DataSources::Serial::setDataBits(const quint8 dataBitsIndex)
{
    // Argument verification
    Q_ASSERT(dataBitsIndex < dataBitsList().count());

    // Update current index
    m_dataBitsIndex = dataBitsIndex;

    // Obtain data bits value from current index
    switch (dataBitsIndex)
    {
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

    // Update serial port configuration
    if (port())
        port()->setDataBits(dataBits());

    // Update user interface
    Q_EMIT dataBitsChanged();
}

/**
 * Changes the stop bits of the serial port.
 *
 * @note This function is meant to be used with a combobox in the
 *       QML interface
 */
void IO::DataSources::Serial::setStopBits(const quint8 stopBitsIndex)
{
    // Argument verification
    Q_ASSERT(stopBitsIndex < stopBitsList().count());

    // Update current index
    m_stopBitsIndex = stopBitsIndex;

    // Obtain stop bits value from current index
    switch (stopBitsIndex)
    {
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

    // Update serial port configuration
    if (port())
        port()->setStopBits(stopBits());

    // Update user interface
    Q_EMIT stopBitsChanged();
}

/**
 * Enables or disables the auto-reconnect feature
 */
void IO::DataSources::Serial::setAutoReconnect(const bool autoreconnect)
{
    m_autoReconnect = autoreconnect;
    Q_EMIT autoReconnectChanged();
}

/**
 * Changes the flow control option of the serial port.
 *
 * @note This function is meant to be used with a combobox in the
 *       QML interface
 */
void IO::DataSources::Serial::setFlowControl(const quint8 flowControlIndex)
{
    // Argument verification
    Q_ASSERT(flowControlIndex < flowControlList().count());

    // Update current index
    m_flowControlIndex = flowControlIndex;

    // Obtain flow control value from current index
    switch (flowControlIndex)
    {
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

    // Update serial port configuration
    if (port())
        port()->setFlowControl(flowControl());

    // Update user interface
    Q_EMIT flowControlChanged();
}

/**
 * Scans for new serial ports available & generates a StringList with current
 * serial ports.
 */
void IO::DataSources::Serial::refreshSerialDevices()
{
    // Create device list, starting with dummy header
    // (for a more friendly UI when no devices are attached)
    StringList ports;
    ports.append(tr("Select Port"));

    // Search for available ports and add them to the lsit
    auto validPortList = validPorts();
    Q_FOREACH (QSerialPortInfo info, validPortList)
    {
        if (!info.isNull())
            ports.append(info.portName());
    }

    // Update list only if necessary
    if (portList() != ports)
    {
        // Update list
        m_portList = ports;

        // Update current port index
        if (port())
        {
            auto name = port()->portName();
            for (int i = 0; i < validPortList.count(); ++i)
            {
                auto info = validPortList.at(i);
                if (info.portName() == name)
                {
                    m_portIndex = i + 1;
                    break;
                }
            }
        }

        // Auto reconnect
        if (Manager::instance().dataSource() == Manager::DataSource::Serial)
        {
            if (autoReconnect() && m_lastSerialDeviceIndex > 0)
            {
                if (m_lastSerialDeviceIndex < portList().count())
                {
                    setPortIndex(m_lastSerialDeviceIndex);
                    Manager::instance().connectDevice();
                }
            }
        }

        // Update UI
        Q_EMIT availablePortsChanged();
    }
}

/**
 * @brief IO::DataSources::Serial::handleError
 * @param error
 */
void IO::DataSources::Serial::handleError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError)
        Manager::instance().disconnectDevice();
}

/**
 * Read saved settings (if any)
 */
void IO::DataSources::Serial::readSettings()
{
    // Register standard baud rates
    QStringList stdBaudRates
        = { "300",   "1200",   "2400",   "4800",   "9600",   "19200",   "38400",  "57600",
            "74880", "115200", "230400", "250000", "500000", "1000000", "2000000" };

    // Get value from settings
    QStringList list;
    list = m_settings.value("IO_DataSource_Serial__BaudRates", stdBaudRates)
               .toStringList();

    // Convert QStringList to QVector
    m_baudRateList.clear();
    for (int i = 0; i < list.count(); ++i)
        m_baudRateList.append(list.at(i));

        // Sort baud rate list
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    for (auto i = 0; i < m_baudRateList.count() - 1; ++i)
    {
        for (auto j = 0; j < m_baudRateList.count() - i - 1; ++j)
        {
            auto a = m_baudRateList.at(j).toInt();
            auto b = m_baudRateList.at(j + 1).toInt();
            if (a > b)
                m_baudRateList.swapItemsAt(j, j + 1);
        }
    }
#endif

    // Notify UI
    Q_EMIT baudRateListChanged();
}

/**
 * Save settings between application runs
 */
void IO::DataSources::Serial::writeSettings()
{
    // Sort baud rate list
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    for (auto i = 0; i < m_baudRateList.count() - 1; ++i)
    {
        for (auto j = 0; j < m_baudRateList.count() - i - 1; ++j)
        {
            auto a = m_baudRateList.at(j).toInt();
            auto b = m_baudRateList.at(j + 1).toInt();
            if (a > b)
            {
                m_baudRateList.swapItemsAt(j, j + 1);
                Q_EMIT baudRateListChanged();
            }
        }
    }
#endif

    // Convert QVector to QStringList
    QStringList list;
    for (int i = 0; i < baudRateList().count(); ++i)
        list.append(baudRateList().at(i));

    // Save list to memory
    m_settings.setValue("IO_DataSource_Serial__BaudRates", list);
}

/**
 * Returns a list with all the valid serial port objects
 */
QVector<QSerialPortInfo> IO::DataSources::Serial::validPorts() const
{
    // Search for available ports and add them to the list
    QVector<QSerialPortInfo> ports;
    Q_FOREACH (QSerialPortInfo info, QSerialPortInfo::availablePorts())
    {
        if (!info.isNull())
        {
            // Only accept *.cu devices on macOS (remove *.tty)
            // https://stackoverflow.com/a/37688347
#ifdef Q_OS_MACOS
            if (info.portName().toLower().startsWith("tty."))
                continue;
#endif
            // Append port to list
            ports.append(info);
        }
    }

    // Return list
    return ports;
}

#ifdef SERIAL_STUDIO_INCLUDE_MOC
#    include "moc_Serial.cpp"
#endif
