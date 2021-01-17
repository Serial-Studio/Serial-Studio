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

#include "Logger.h"
#include "SerialManager.h"
#include "ConsoleAppender.h"

#include <QMessageBox>

/**
 * Pointer to the only instance of the class.
 */
static SerialManager *INSTANCE = nullptr;

/**
 * Shows a macOS-like message box with the given properties
 */
static int NiceMessageBox(QString text, QString informativeText,
                          QString windowTitle = qAppName(),
                          QMessageBox::StandardButtons bt = QMessageBox::Ok)
{
    // clang-format off
    auto icon = QPixmap(":/images/icon.png").scaled(64, 64,
                                                    Qt::IgnoreAspectRatio,
                                                    Qt::SmoothTransformation);
    // clang-format on

    // Create message box & set options
    QMessageBox box;
    box.setIconPixmap(icon);
    box.setStandardButtons(bt);
    box.setWindowTitle(windowTitle);
    box.setText("<h3>" + text + "</h3>");
    box.setInformativeText(informativeText);

    // Show message box & return user decision to caller
    return box.exec();
}

/**
 * Constructor function, initializes the serial port configuration and
 * sets the maximum buffer size to 5 MB.
 */
SerialManager::SerialManager()
{
    // Ensure that pointers are NULL
    m_port = nullptr;
    m_receivedBytes = 0;

    // Init serial port configuration variables
    setPort(0);
    setParity(parityList().indexOf(tr("None")));
    setBaudRate(baudRateList().indexOf("9600"));
    setDataBits(dataBitsList().indexOf("8"));
    setStopBits(stopBitsList().indexOf("1"));
    setFlowControl(flowControlList().indexOf(tr("None")));

    // Init start/finish sequence strings
    setStartSequence("/*");
    setFinishSequence("*/");
    setMaxBufferSize(5 * 1024 * 1024);

    // Build serial devices list
    refreshSerialDevices();

    // Start frame interpretation loop (~60 Hz)
    readFrames();
}

/**
 * Destructor function, closes the serial port before exiting the application
 */
SerialManager::~SerialManager()
{
    if (port())
        disconnectDevice();
}

/**
 * Returns the only instance of the class
 */
SerialManager *SerialManager::getInstance()
{
    if (INSTANCE == nullptr)
        INSTANCE = new SerialManager;

    return INSTANCE;
}

/**
 * Returns the pointer to the current serial port handler
 */
QSerialPort *SerialManager::port() const
{
    return m_port;
}

/**
 * Returns @c true if the serial port is open in read-only mode
 */
bool SerialManager::readOnly() const
{
    if (connected())
        return port()->openMode() == QIODevice::ReadOnly;

    return false;
}

/**
 * Returns @c true if the serial port is open in read/write mode
 */
bool SerialManager::readWrite() const
{
    if (connected())
        return port()->openMode() == QIODevice::ReadWrite;

    return false;
}

/**
 * Returns @c true if the serial port is open
 */
bool SerialManager::connected() const
{
    if (port())
        return port()->isOpen();

    return false;
}

/**
 * Returns the name of the current serial port device
 */
QString SerialManager::portName() const
{
    if (port())
        return port()->portName();

    return tr("No Device");
}

/**
 * Returns the max. temporary buffer size allowed. If the temporary
 * buffer size exceeds this limit, then the program shall delete the
 * contents of the temporary buffer to avoid using too much memory.
 *
 * The temporary buffer is used to store incoming serial data until a
 * packet is read. The temporary buffer is automatically handled by the
 * application if it contains valid data, however, there may be cases
 * in which the data is corrupt (e.g. incorret baud rate), so it's necessary
 * to define a maximum size in order to avoid running into memory problems.
 */
int SerialManager::maxBufferSize() const
{
    return m_maxBufferSize;
}

/**
 * Returns the size of the data received (and successfully read) from the
 * serial device.
 */
QString SerialManager::receivedBytes() const
{
    QString value;
    QString units;

    if (m_receivedBytes < 1024)
    {
        value = QString::number(m_receivedBytes);
        units = "bytes";
    }

    else if (m_receivedBytes >= 1024 && m_receivedBytes < 1024 * 1024)
    {
        double kb = (double)m_receivedBytes / 1024.0;
        value = QString::number(kb, 'f', 2);
        units = "KB";
    }

    else
    {
        double mb = (double)m_receivedBytes / (1024 * 1024.0);
        value = QString::number(mb, 'f', 2);
        units = "MB";
    }

    return tr("Received: %1 %2").arg(value).arg(units);
}

/**
 * Returns the start sequence string used by the application to know where
 * to consider that a packet begins. If the start sequence is empty, then
 * the application shall select packets by selecting matching JSON brackets.
 */
QString SerialManager::startSequence() const
{
    return m_startSeq;
}

/**
 * Returns the end sequence string used by the application to know where
 * to consider that a packet ends. If the finish sequence is empty, then
 * the application shall select packets by selecting matching JSON brackets.
 */
QString SerialManager::finishSequence() const
{
    return m_finishSeq;
}

/**
 * Returns the index of the current serial device selected by the program.
 */
quint8 SerialManager::portIndex() const
{
    return m_portIndex;
}

/**
 * Returns the correspoding index of the parity configuration in relation
 * to the @c QStringList returned by the @c parityList() function.
 */
quint8 SerialManager::parityIndex() const
{
    return m_parityIndex;
}

/**
 * Returns the correspoding index of the baud rate configuration in relation
 * to the @c QStringList returned by the @c baudRateList() function.
 */
quint8 SerialManager::baudRateIndex() const
{
    return m_baudRateIndex;
}

/**
 * Returns the correspoding index of the data bits configuration in relation
 * to the @c QStringList returned by the @c dataBitsList() function.
 */
quint8 SerialManager::dataBitsIndex() const
{
    return m_dataBitsIndex;
}

/**
 * Returns the correspoding index of the stop bits configuration in relation
 * to the @c QStringList returned by the @c stopBitsList() function.
 */
quint8 SerialManager::stopBitsIndex() const
{
    return m_stopBitsIndex;
}

/**
 * Returns the correspoding index of the flow control config. in relation
 * to the @c QStringList returned by the @c flowControlList() function.
 */
quint8 SerialManager::flowControlIndex() const
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
QStringList SerialManager::portList() const
{
    return m_portList;
}

/**
 * Returns a list with the available parity configurations.
 * This function can be used with a combo-box to build UIs.
 */
QStringList SerialManager::parityList() const
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
 * Returns a list with the available baud rate configurations.
 * This function can be used with a combo-box to build UIs.
 */
QStringList SerialManager::baudRateList() const
{
    return QStringList { "1200",  "2400",  "4800",  "9600",
                         "19200", "38400", "57600", "115200" };
}

/**
 * Returns a list with the available data bits configurations.
 * This function can be used with a combo-box to build UIs.
 */
QStringList SerialManager::dataBitsList() const
{
    return QStringList { "5", "6", "7", "8" };
}

/**
 * Returns a list with the available stop bits configurations.
 * This function can be used with a combo-box to build UIs.
 */
QStringList SerialManager::stopBitsList() const
{
    return QStringList { "1", "1.5", "2" };
}

/**
 * Returns a list with the available flow control configurations.
 * This function can be used with a combo-box to build UIs.
 */
QStringList SerialManager::flowControlList() const
{
    QStringList list;
    list.append(tr("None"));
    list.append("RTS/CTS");
    list.append("XON/XOFF");
    return list;
}

/**
 * Returns the current parity configuration used by the serial port
 * handler object.
 */
QSerialPort::Parity SerialManager::parity() const
{
    return m_parity;
}

/**
 * Returns the current baud rate configuration used by the serial port
 * handler object.
 */
QSerialPort::BaudRate SerialManager::baudRate() const
{
    return m_baudRate;
}

/**
 * Returns the current data bits configuration used by the serial port
 * handler object.
 */
QSerialPort::DataBits SerialManager::dataBits() const
{
    return m_dataBits;
}

/**
 * Returns the current stop bits configuration used by the serial port
 * handler object.
 */
QSerialPort::StopBits SerialManager::stopBits() const
{
    return m_stopBits;
}

/**
 * Returns the current flow control configuration used by the serial
 * port handler object.
 */
QSerialPort::FlowControl SerialManager::flowControl() const
{
    return m_flowControl;
}

/**
 * Configures the text document to make it fit for logging purposes
 */
void SerialManager::configureTextDocument(QQuickTextDocument *doc)
{
    if (doc)
        doc->textDocument()->setUndoRedoEnabled(false);
}

/**
 * Deletes the contents of the temporary buffer. This function is called
 * automatically by the class when the temporary buffer size exceeds the
 * limit imposed by the @c maxBufferSize() function.
 */
void SerialManager::clearTempBuffer()
{
    LOG_INFO() << "Deleting temp. buffer to avoid excesive memory usage..";
    m_tempBuffer.clear();
    LOG_INFO() << "Temporary data buffer cleared";
}

/**
 * Disconnects from the current serial device and clears temp. data
 */
void SerialManager::disconnectDevice()
{
    // Check if serial port pointer is valid
    if (port() != nullptr)
    {
        // Get serial port name (used for warning messages)
        auto name = portName();

        // Disconnect signals/slots
        port()->disconnect(this, SLOT(handleError()));
        port()->disconnect(this, SLOT(onDataReceived()));
        port()->disconnect(this, SLOT(disconnectDevice()));

        // Close & delete serial port handler
        port()->close();
        port()->deleteLater();

        // Reset pointer
        m_port = nullptr;

        // Reset received bytes
        m_receivedBytes = 0;
        emit receivedBytesChanged();

        // Warn user
        if (!name.isEmpty())
            emit connectionError(name);

        // Log changes
        LOG_INFO() << "Disconnected from" << name;

        // Clear buffer
        clearTempBuffer();
    }

    // Update user interface
    emit connectedChanged();
}

/**
 * Tries to write the given @a data to the current serial port device.
 * Upon data write, the class emits the @a rx() signal for UI updating
 * or for write verification purposes.
 */
void SerialManager::sendData(const QString &data)
{
    if (!data.isEmpty() && connected())
    {
        // Convert string to byte array and write to serial port
        auto bin = data.toUtf8();
        auto bytes = port()->write(bin);

        // Write success, notify UI & log bytes written
        if (bytes > 0)
        {
            // Get sent byte array
            auto sent = bin;
            sent.chop(bin.length() - bytes);
            LOG_INFO() << "Written" << bytes << "bytes to serial port";

            // Bytes not equal to data length
            if (bytes != bin.length())
                LOG_WARNING()
                    << "Written data length not equal to request data length";

            // Emit signals
            else
                emit tx(data);
        }

        // Write error
        else
            LOG_INFO() << "Write error" << port()->errorString();
    }
}

/**
 * Closes the current serial port and tries to open & configure a new serial
 * port connection with the device at the given @a port index.
 *
 * Upon serial port configuration the function emits the @c connectionChanged()
 * signal and the portChanged() signal.
 *
 * @note If the @a portIndex is the same as the current port index, then the
 *       function shall not try to reconfigure or close the current serial port.
 *
 * @note If another device is connected through a serial port, then the
 * connection with that device will be canceled/closed before configuring the
 * new serial port connection.
 */
void SerialManager::setPort(const quint8 portIndex)
{
    // Argument verification
    Q_ASSERT(portIndex < portList().count() + 1);

    // Abort if portIndex is the same as the actual port index
    if (portIndex == m_portIndex)
        return;

    // Update port index variable & disconnect from current serial port
    m_portIndex = portIndex;
    disconnectDevice();

    // Ignore the first item of the list (Select Port)
    if (portIndex > 0)
    {
        // Get the actual port ID & serial devices
        auto portId = portIndex - 1;
        auto ports = QSerialPortInfo::availablePorts();

        // Check if port ID is valid
        if (portId < ports.count())
        {
            // Create new serial port handler
            m_port = new QSerialPort(ports.at(portId));

            // Configure serial port
            port()->setParity(parity());
            port()->setBaudRate(baudRate());
            port()->setDataBits(dataBits());
            port()->setStopBits(stopBits());
            port()->setFlowControl(flowControl());

            // Connect signals/slots
            // clang-format off
            connect(port(), SIGNAL(readyRead()),
                    this,     SLOT(onDataReceived()));
            connect(port(), SIGNAL(aboutToClose()),
                    this,     SLOT(disconnectDevice()));
            connect(port(), SIGNAL(errorOccurred(QSerialPort::SerialPortError)),
                    this,     SLOT(handleError(QSerialPort::SerialPortError)));
            // clang-format on

            // Try to open the serial port in R/W mode
            if (port()->open(QIODevice::ReadWrite))
            {
                emit connectedChanged();
                LOG_INFO() << Q_FUNC_INFO
                           << "Serial port opened successfully in RW mode";
            }

            // Try to open the serial port only for reading
            else if (port()->open(QIODevice::ReadOnly))
            {
                emit connectedChanged();
                LOG_INFO() << Q_FUNC_INFO
                           << "Serial port opened successfully in READ mode";
            }

            // Close serial port on error
            else
            {
                disconnectDevice();
                LOG_INFO() << "Serial port open error" << port()->errorString();
            }
        }
    }

    // Notify UI that the port status changed
    emit portChanged();
    LOG_INFO() << "Serial port selection set to" << portName();
}

/**
 * @brief SerialManager::setParity
 * @param parityIndex
 */
void SerialManager::setParity(const quint8 parityIndex)
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
        default:
            m_parity = QSerialPort::UnknownParity;
            break;
    }

    // Update serial port config.
    if (port())
        port()->setParity(parity());

    // Notify user interface
    emit parityChanged();

    // Log changes
    LOG_INFO() << "Serial port parity set to" << parity();
}

/**
 * Changes the baud rate of the serial port.
 *
 * @note This function is meant to be used with a combobox in the
 *       QML interface
 */
void SerialManager::setBaudRate(const quint8 baudRateIndex)
{
    // Argument verifications
    Q_ASSERT(baudRateIndex < baudRateList().count());

    // Update current index
    m_baudRateIndex = baudRateIndex;

    // Obtain baud rate value from current index
    switch (baudRateIndex)
    {
        case 0:
            m_baudRate = QSerialPort::Baud1200;
            break;
        case 1:
            m_baudRate = QSerialPort::Baud2400;
            break;
        case 2:
            m_baudRate = QSerialPort::Baud4800;
            break;
        case 3:
            m_baudRate = QSerialPort::Baud9600;
            break;
        case 4:
            m_baudRate = QSerialPort::Baud19200;
            break;
        case 5:
            m_baudRate = QSerialPort::Baud38400;
            break;
        case 6:
            m_baudRate = QSerialPort::Baud57600;
            break;
        case 7:
            m_baudRate = QSerialPort::Baud115200;
            break;
        default:
            m_baudRate = QSerialPort::UnknownBaud;
            break;
    }

    // Update serial port config
    if (port())
        port()->setBaudRate(baudRate());

    // Update user interface
    emit baudRateChanged();

    // Log changes
    LOG_INFO() << "Baud rate set to" << baudRate();
}

/**
 * Changes the data bits of the serial port.
 *
 * @note This function is meant to be used with a combobox in the
 *       QML interface
 */
void SerialManager::setDataBits(const quint8 dataBitsIndex)
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
        default:
            m_dataBits = QSerialPort::UnknownDataBits;
            break;
    }

    // Update serial port configuration
    if (port())
        port()->setDataBits(dataBits());

    // Update user interface
    emit dataBitsChanged();

    // Log changes
    LOG_INFO() << "Data bits set to" << dataBits();
}

/**
 * Changes the stop bits of the serial port.
 *
 * @note This function is meant to be used with a combobox in the
 *       QML interface
 */
void SerialManager::setStopBits(const quint8 stopBitsIndex)
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
        default:
            m_stopBits = QSerialPort::UnknownStopBits;
            break;
    }

    // Update serial port configuration
    if (port())
        port()->setStopBits(stopBits());

    // Update user interface
    emit stopBitsChanged();

    // Log changes
    LOG_INFO() << "Stop bits set to" << stopBits();
}

/**
 * Changes the maximum allowed size of the incoming data buffer.
 *
 * @note Incoming data is stored instide a temp. buffer, which is
 *       later sliced into sections so that each data frame can
 *       be interpreted separately.
 *       If we receive invalid data from the serial port, this
 *       buffer shall be automatically managed in order to avoid
 *       excessive memory consumption.
 */
void SerialManager::setMaxBufferSize(const int maxBufferSize)
{
    // Update max. buffer size if it's different from current value
    if (maxBufferSize > 1 && m_maxBufferSize != maxBufferSize)
    {
        // Update buffer size
        m_maxBufferSize = maxBufferSize;

        // Clear buffer if necessary
        if (m_tempBuffer.size() > maxBufferSize)
            clearTempBuffer();

        // Update user interface
        emit maxBufferSizeChanged();

        // Log changes
        LOG_INFO() << "Max. buffer size set to" << maxBufferSize << "bytes";
    }
}

/**
 * Changes the start sequence used for the communication protocol.
 *
 * We need a start and an end sequence in order to separate incoming data
 * and generate 'slices' of valid data frames.
 *
 * @note By default, the start sequence is set to '/ *', we have removed
 *       the option to change the start sequence from the QML interface.
 *       However, this function was left in order to maintain a nice
 *       software architecture.
 */
void SerialManager::setStartSequence(const QString &sequence)
{
    // Update start sequency only if necessary
    if (m_startSeq != sequence)
    {
        m_startSeq = sequence;
        emit startSequenceChanged();
        LOG_INFO() << "Start sequence set to" << startSequence();
    }
}

/**
 * Changes the end sequence used for the communication protocol.
 *
 * We need a start and an end sequence in order to separate incoming data
 * and generate 'slices' of valid data frames.
 *
 * @note By default, the end sequence is set to '* /', we have removed
 *       the option to change the start sequence from the QML interface.
 *       However, this function was left in order to maintain a nice
 *       software architecture.
 */
void SerialManager::setFinishSequence(const QString &sequence)
{
    // Update end sequence only if necesessary
    if (m_finishSeq != sequence)
    {
        m_finishSeq = sequence;
        emit finishSequenceChanged();
        LOG_INFO() << "Finish sequence set to" << finishSequence();
    }
}

/**
 * Changes the flow control option of the serial port.
 *
 * @note This function is meant to be used with a combobox in the
 *       QML interface
 */
void SerialManager::setFlowControl(const quint8 flowControlIndex)
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
        case 3:
            m_flowControl = QSerialPort::UnknownFlowControl;
            break;
    }

    // Update serial port configuration
    if (port())
        port()->setFlowControl(flowControl());

    // Update user interface
    emit flowControlChanged();

    // Log changes
    LOG_INFO() << "Flow control set to" << flowControl();
}

/**
 * Reads incoming data from the serial device, updates the serial console
 * object, registers incoming data to temporary buffer & extracts valid data
 * frames from the buffer.
 */
void SerialManager::onDataReceived()
{
    // Verify that port is still alive
    if (port() == nullptr)
        return;

    // Get data & calculate received bytes
    auto data = port()->readAll();
    auto bytes = data.length();

    // Add data to temp. buffer
    m_tempBuffer.append(data);

    // Update received bytes indicator
    m_receivedBytes += bytes;
    if (m_receivedBytes >= UINT64_MAX)
        m_receivedBytes = 0;

    // Notify user interface
    emit receivedBytesChanged();
    emit rx(QString::fromUtf8(data));
}

/**
 * Scans for new serial ports available & generates a QStringList with current
 * serial ports.
 */
void SerialManager::refreshSerialDevices()
{
    // Create device list, starting with dummy header
    // (for a more friendly UI when no devices are attached)
    QStringList ports;
    ports.append(tr("Select Port"));

    // Search for available ports and add them to the lsit
    foreach (QSerialPortInfo info, QSerialPortInfo::availablePorts())
    {
        if (!info.isNull())
            ports.append(info.portName());
    }

    // Update list only if necessary
    if (portList() != ports)
    {
        m_portList = ports;
        emit availablePortsChanged();
    }

    // Call this function again in one second
    QTimer::singleShot(100, this, SLOT(refreshSerialDevices()));
}

/**
 * @brief SerialManager::handleError
 * @param error
 */
void SerialManager::handleError(QSerialPort::SerialPortError error)
{
    LOG_INFO() << "Serial port error" << port()->errorString();

    if (error == QSerialPort::ResourceError)
    {
        NiceMessageBox(tr("Critical error"), port()->errorString());
        setPort(0);
    }
}

/**
 * Read frames from temporary buffer, every frame that contains the appropiate
 * start/end sequence is removed from the buffer as soon as its read.
 *
 * This function also checks that the buffer size does not exceed specified
 * size limitations.
 */
void SerialManager::readFrames()
{
    // Only execute code if we are connected to a serial device
    if (connected())
    {
        auto start = startSequence().toUtf8();
        auto finish = finishSequence().toUtf8();
        while (m_tempBuffer.contains(start) && m_tempBuffer.contains(finish))
        {
            // Begin reading from start sequence index
            auto buffer = m_tempBuffer;
            auto sIndex = m_tempBuffer.indexOf(start);
            buffer.remove(0, sIndex + start.length());

            // Check that new buffer contains finish sequence
            if (!buffer.contains(finish))
                break;

            // Remove bytes outside start/end sequence range
            auto fIndex = buffer.indexOf(finish);
            buffer.remove(fIndex, buffer.length() - fIndex);

            // Buffer is not empty, notify app & remove read data from buffer
            if (!buffer.isEmpty())
            {
                emit packetReceived(buffer);
                m_tempBuffer.remove(0, sIndex + fIndex + finish.length());
            }
        }
    }

    // Clear temp. buffer
    if (m_tempBuffer.size() > maxBufferSize())
        clearTempBuffer();

    // Execute this function at about 100 Hz
    QTimer::singleShot(10, Qt::PreciseTimer, this, SLOT(readFrames()));
}
