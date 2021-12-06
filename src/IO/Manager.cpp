/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
 * Copyright (c) 2021 JP Norair <https://github.com/jpnorair>
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

#include "Manager.h"
#include "Checksum.h"

#include <MQTT/Client.h>
#include <IO/DataSources/Serial.h>
#include <IO/DataSources/Network.h>

#include <QNetworkDatagram>

namespace IO
{
static Manager *MANAGER = Q_NULLPTR;

/**
 * Adds support for C escape sequences to the given @a str.
 * When user inputs "\n" in a textbox, Qt automatically converts that string to "\\n".
 * For our purposes, we need to convert "\\n" back to "\n", and so on with the rest of
 * the escape sequences supported by C.
 *
 * TODO: add support for numbers
 */
static QString ADD_ESCAPE_SEQUENCES(const QString &str)
{
    auto escapedStr = str;
    escapedStr = escapedStr.replace("\\a", "\a");
    escapedStr = escapedStr.replace("\\b", "\b");
    escapedStr = escapedStr.replace("\\f", "\f");
    escapedStr = escapedStr.replace("\\n", "\n");
    escapedStr = escapedStr.replace("\\r", "\r");
    escapedStr = escapedStr.replace("\\t", "\t");
    escapedStr = escapedStr.replace("\\v", "\v");
    return escapedStr;
}

/**
 * Constructor function
 */
Manager::Manager()
    : m_enableCrc(false)
    , m_writeEnabled(true)
    , m_maxBufferSize(1024 * 1024)
    , m_device(Q_NULLPTR)
    , m_dataSource(DataSource::Serial)
    , m_receivedBytes(0)
    , m_startSequence("/*")
    , m_finishSequence("*/")
    , m_separatorSequence(",")
{
    setWatchdogInterval(10000);
    setMaxBufferSize(1024 * 1024);

    // Configure signals/slots
    auto serial = DataSources::Serial::getInstance();
    auto netwrk = DataSources::Network::getInstance();
    connect(netwrk, SIGNAL(portChanged()), this, SIGNAL(configurationChanged()));
    connect(netwrk, SIGNAL(addressChanged()), this, SIGNAL(configurationChanged()));
    connect(this, SIGNAL(dataSourceChanged()), this, SIGNAL(configurationChanged()));
    connect(serial, SIGNAL(portIndexChanged()), this, SIGNAL(configurationChanged()));
}

/**
 * Destructor function
 */
Manager::~Manager() { }

/**
 * Returns the only instance of the class
 */
Manager *Manager::getInstance()
{
    if (!MANAGER)
        MANAGER = new Manager;

    return MANAGER;
}

/**
 * Returns @c true if a device is connected and its open in read-only mode
 */
bool Manager::readOnly()
{
    return connected() && !m_writeEnabled;
}

/**
 * Returns @c true if a device is connected and its open in read/write mode
 */
bool Manager::readWrite()
{
    return connected() && m_writeEnabled;
}

/**
 * Returns @c true if a device is currently selected and opened or if the MQTT client
 * is currently connected as a subscriber.
 */
bool Manager::connected()
{
    if (device())
        return device()->isOpen();

    return MQTT::Client::getInstance()->isSubscribed();
}

/**
 * Returns @c true if a device is currently selected
 */
bool Manager::deviceAvailable()
{
    return device() != Q_NULLPTR;
}

/**
 * Returns @c true if we are able to connect to a device/port with the current config.
 */
bool Manager::configurationOk() const
{
    if (dataSource() == DataSource::Serial)
        return DataSources::Serial::getInstance()->configurationOk();
    else if (dataSource() == DataSource::Network)
        return DataSources::Network::getInstance()->configurationOk();

    return false;
}

/**
 * Returns the interval of the watchdog in milliseconds. The watchdog is used to clear
 * the data buffer if no data has been received in more than x milliseconds (default
 * 15 ms, tested on a 300 baud rate MCU).
 *
 * This is useful for: a) Dealing when a device suddenly is disconnected
 *                     b) Removing extra data between received frames
 *
 * Example:
 *
 *      RRRRRRRRR  watchdog clears buf.  RRRRRRRRR  watchdog clear buf.   RRRRRRRR
 *     [--------] . - . - . - . - . - . [--------] . - . - . - . - . - . [--------]
 *      Frame A   Interval of > 15 ms    Frame B   Interval of > 15 ms    Frame C
 *
 *  Note: in the representation above "R" means that the watchdog is reset, thus, the
 *        data buffer is not cleared until we wait for another frame or communications
 *        are interrupted.
 *
 * TODO: let the JSON project file define the watchdog timeout time.
 */
int Manager::watchdogInterval() const
{
    return m_watchdog.interval();
}

/**
 * Returns the maximum size of the buffer. This is useful to avoid consuming to much
 * memory when a large block of invalid data is received (for example, when the user
 * selects an invalid baud rate configuration).
 */
int Manager::maxBufferSize() const
{
    return m_maxBufferSize;
}

/**
 * Returns a pointer to the currently selected device.
 *
 * @warning you need to check this pointer before using it, it can have a @c Q_NULLPTR
 *          value during normal operations.
 */
QIODevice *Manager::device()
{
    return m_device;
}

/**
 * Returns the currently selected data source, possible return values:
 * - @c DataSource::Serial  use a serial port as a data source
 * - @c DataSource::Network use a network port as a data source
 */
Manager::DataSource Manager::dataSource() const
{
    return m_dataSource;
}

/**
 * Returns the start sequence string used by the application to know where to consider
 * that a frame begins. If the start sequence is empty, then the application shall ignore
 * incoming data. The only thing that wont ignore the incoming data will be the console.
 */
QString Manager::startSequence() const
{
    return m_startSequence;
}

/**
 * Returns the finish sequence string used by the application to know where to consider
 * that a frame ends. If the start sequence is empty, then the application shall ignore
 * incoming data. The only thing that wont ignore the incoming data will be the console.
 */
QString Manager::finishSequence() const
{
    return m_finishSequence;
}

/**
 * Returns the separator sequence string used by the application to know where to consider
 * that a data item ends.
 */
QString Manager::separatorSequence() const
{
    return m_separatorSequence;
}

/**
 * Returns a list with the possible data source options.
 */
StringList Manager::dataSourcesList() const
{
    StringList list;
    list.append(tr("Serial port"));
    list.append(tr("Network port"));
    return list;
}

/**
 * Tries to write the given @a data to the current device.
 *
 * @returns the number of bytes written to the target device
 */
qint64 Manager::writeData(const QByteArray &data)
{
    if (connected())
    {
        qint64 bytes = 0;

        // Check which data source to use to write data
        if (dataSource() == DataSource::Network)
        {
            auto network = DataSources::Network::getInstance();

            // Write to UDP socket
            if (network->socketType() == QAbstractSocket::UdpSocket)
            {
                bytes = network->udpSocket()->writeDatagram(
                    data, QHostAddress(network->remoteAddress()),
                    network->udpRemotePort());
            }

            // Write to TCP socket
            else
            {
                bytes = network->tcpSocket()->write(data);
            }
        }

        // Write to serial device
        else
            bytes = device()->write(data);

        // Show sent data in console
        if (bytes > 0)
        {
            auto writtenData = data;
            writtenData.chop(data.length() - bytes);
            emit dataSent(writtenData);
        }

        // Return number of bytes written
        return bytes;
    }

    return -1;
}

/**
 * Connects/disconnects the application from the currently selected device. This function
 * is used as a convenience for the connect/disconnect button.
 */
void Manager::toggleConnection()
{
    if (connected())
        disconnectDevice();
    else
        connectDevice();
}

/**
 * Closes the currently selected device and tries to open & configure a new connection.
 * A data source must be selected before calling this function.
 */
void Manager::connectDevice()
{
    // Disconnect previous device (if any)
    disconnectDevice();

    // Try to open a serial port connection
    if (dataSource() == DataSource::Serial)
        setDevice(DataSources::Serial::getInstance()->openSerialPort());

    // Try to open a network connection
    else if (dataSource() == DataSource::Network)
        setDevice(DataSources::Network::getInstance()->openNetworkPort());

    // Configure current device
    if (deviceAvailable())
    {
        // Set open flag
        QIODevice::OpenMode mode = QIODevice::ReadOnly;
        if (m_writeEnabled)
            mode = QIODevice::ReadWrite;

        // Open device
        if (device()->open(mode))
            connect(device(), &QIODevice::readyRead, this, &Manager::onDataReceived);

        // Error opening the device
        else
            disconnectDevice();

        // Update UI
        emit connectedChanged();
    }
}

/**
 * Disconnects from the current device and clears temp. data
 */
void Manager::disconnectDevice()
{
    if (deviceAvailable())
    {
        // Disconnect device signals/slots
        device()->disconnect(this, SLOT(onDataReceived()));

        // Call-appropiate interface functions
        if (dataSource() == DataSource::Serial)
            DataSources::Serial::getInstance()->disconnectDevice();
        else if (dataSource() == DataSource::Network)
            DataSources::Network::getInstance()->disconnectDevice();

        // Update device pointer
        m_device = Q_NULLPTR;
        m_receivedBytes = 0;
        m_dataBuffer.clear();
        m_dataBuffer.reserve(maxBufferSize());

        // Update UI
        emit deviceChanged();
        emit connectedChanged();
    }

    // Disable CRC checking
    m_enableCrc = false;
}

/**
 * Enables/disables RW mode
 */
void Manager::setWriteEnabled(const bool enabled)
{
    m_writeEnabled = enabled;
    emit writeEnabledChanged();
}

/**
 * Reads the given payload and emits it as if it were received from a device.
 * This function is for convenience to interact with other application modules & plugins.
 */
void Manager::processPayload(const QByteArray &payload)
{
    if (!payload.isEmpty())
    {
        // Reset watchdog
        feedWatchdog();

        // Update received bytes indicator
        m_receivedBytes += payload.size();
        if (m_receivedBytes >= UINT64_MAX)
            m_receivedBytes = 0;

        // Notify user interface & application modules
        emit dataReceived(payload);
        emit frameReceived(payload);
        emit receivedBytesChanged();
    }
}

/**
 * Changes the maximum permited buffer size. Check the @c maxBufferSize() function for
 * more information.
 */
void Manager::setMaxBufferSize(const int maxBufferSize)
{
    m_maxBufferSize = maxBufferSize;
    emit maxBufferSizeChanged();

    m_dataBuffer.reserve(maxBufferSize);
}

/**
 * Changes the frame start sequence. Check the @c startSequence() function for more
 * information.
 */
void Manager::setStartSequence(const QString &sequence)
{
    m_startSequence = ADD_ESCAPE_SEQUENCES(sequence);
    if (m_startSequence.isEmpty())
        m_startSequence = "/*";

    emit startSequenceChanged();
}

/**
 * Changes the frame end sequence. Check the @c finishSequence() function for more
 * information.
 */
void Manager::setFinishSequence(const QString &sequence)
{
    m_finishSequence = ADD_ESCAPE_SEQUENCES(sequence);
    if (m_finishSequence.isEmpty())
        m_finishSequence = "*/";

    emit finishSequenceChanged();
}

/**
 * Changes the frame separator sequence. Check the @c separatorSequence() function for
 * more information.
 */
void Manager::setSeparatorSequence(const QString &sequence)
{
    m_separatorSequence = ADD_ESCAPE_SEQUENCES(sequence);
    if (m_separatorSequence.isEmpty())
        m_separatorSequence = ",";

    emit separatorSequenceChanged();
}

/**
 * Changes the expiration interval of the watchdog timer. Check the @c watchdogInterval()
 * function for more information.
 */
void Manager::setWatchdogInterval(const int interval)
{
    m_watchdog.setInterval(interval);
    m_watchdog.setTimerType(Qt::PreciseTimer);
    emit watchdogIntervalChanged();
}

/**
 * Changes the data source type. Check the @c dataSource() funciton for more information.
 */
void Manager::setDataSource(const IO::Manager::DataSource source)
{
    // Disconnect current device
    if (connected())
        disconnectDevice();

    // Change data source
    m_dataSource = source;
    emit dataSourceChanged();
}

/**
 * Read frames from temporary buffer, every frame that contains the appropiate start/end
 * sequence is removed from the buffer as soon as its read.
 *
 * This function also checks that the buffer size does not exceed specified size
 * limitations.
 *
 * Implemementation credits: @jpnorair and @alex-spataru
 */
void Manager::readFrames()
{
    // No device connected, abort
    if (!connected())
        return;

    // Read until start/finish combinations are not found
    auto bytes = 0;
    auto prevBytes = 0;
    auto cursor = m_dataBuffer;
    auto start = startSequence().toUtf8();
    auto finish = finishSequence().toUtf8();
    while (cursor.contains(start) && cursor.contains(finish))
    {
        // Remove the part of the buffer prior to, and including, the start sequence.
        auto sIndex = cursor.indexOf(start);
        cursor = cursor.mid(sIndex + start.length(), -1);
        bytes += sIndex + start.length();

        // Copy a sub-buffer that goes until the finish sequence
        auto fIndex = cursor.indexOf(finish);
        auto frame = cursor.left(fIndex);

        // Checksum verification & emit RX frame
        int chop = 0;
        auto result = integrityChecks(frame, cursor, &chop);
        if (result == ValidationStatus::FrameOk)
            emit frameReceived(frame);

        // Checksum data incomplete, try next time...
        else if (result == ValidationStatus::ChecksumIncomplete)
        {
            bytes = prevBytes;
            break;
        }

        // Remove the data including the finish sequence from the master buffer
        cursor = cursor.mid(fIndex + chop, -1);
        bytes += fIndex + chop;

        // Frame read successfully, save the number of bytes to chop.
        // This is used to manage frames with incomplete checksums
        prevBytes = bytes;
    }

    // Remove parsed data from master buffer
    m_dataBuffer.remove(0, bytes);

    // Clear temp. buffer (e.g. device sends a lot of invalid data)
    if (m_dataBuffer.size() > maxBufferSize())
        clearTempBuffer();
}

/**
 * Resets the watchdog timer before it expires. Check the @c watchdogInterval() function
 * for more information.
 */
void Manager::feedWatchdog()
{
    m_watchdog.stop();
    m_watchdog.start();
}

/**
 * Reads incoming data from the serial device, updates the serial console object,
 * registers incoming data to temporary buffer & extracts valid data frames from the
 * buffer using the @c readFrame() function.
 */
void Manager::onDataReceived()
{
    // Verify that device is still valid
    if (!device())
        disconnectDevice();

    // Initialize byte array
    QByteArray data;
    bool udpConnection = false;

    // Check if we need to use UDP socket functions
    if (dataSource() == DataSource::Network)
    {
        auto network = DataSources::Network::getInstance();
        if (network->socketType() == QAbstractSocket::UdpSocket)
        {
            udpConnection = true;
            auto udpSocket = network->udpSocket();
            while (udpSocket->hasPendingDatagrams())
            {
                QByteArray datagram;
                datagram.resize(int(udpSocket->pendingDatagramSize()));
                udpSocket->readDatagram(datagram.data(), datagram.size());
                data.append(datagram);
            }
        }
    }

    // We are using the TCP socket or the serial port
    if (!udpConnection)
        data = device()->readAll();

    // Read data & append it to buffer
    auto bytes = data.length();

    // Feed watchdog (so that data is not cleared automatically)
    feedWatchdog();

    // Obtain frames from data buffer
    m_dataBuffer.append(data);
    readFrames();

    // Update received bytes indicator
    m_receivedBytes += bytes;
    if (m_receivedBytes >= UINT64_MAX)
        m_receivedBytes = 0;

    // Notify user interface
    emit receivedBytesChanged();
    emit dataReceived(data);
}

/**
 * Deletes the contents of the temporary buffer. This function is called automatically by
 * the class when the temporary buffer size exceeds the limit imposed by the
 * @c maxBufferSize() function.
 */
void Manager::clearTempBuffer()
{
    m_dataBuffer.clear();
}

/**
 * Called when the watchdog timer expires. For the moment, this function only clears the
 * temporary data buffer.
 */
void Manager::onWatchdogTriggered()
{
    clearTempBuffer();
}

/**
 * Changes the target device pointer. Deletion should be handled by the interface
 * implementation, not by this class.
 */
void Manager::setDevice(QIODevice *device)
{
    disconnectDevice();
    m_device = device;
    emit deviceChanged();
}

/**
 * Checks if the @c cursor has a checksum corresponding to the given @a frame.
 * If so, the function shall calculate the appropiate checksum to for the @a frame and
 * compare it with the received checksum to verify the integrity of received data.
 *
 * @param frame data in which we shall perform integrity checks
 * @param cursor master buffer, should start with checksum type header
 * @param bytes pointer to the number of bytes that we need to chop from the master buffer
 */
Manager::ValidationStatus Manager::integrityChecks(const QByteArray &frame,
                                                   const QByteArray &cursor, int *bytes)
{
    // Get finish sequence as byte array
    auto finish = finishSequence().toUtf8();
    auto crc8Header = finish + "crc8:";
    auto crc16Header = finish + "crc16:";
    auto crc32Header = finish + "crc32:";

    // Check CRC-8
    if (cursor.contains(crc8Header))
    {
        // Enable the CRC flag
        m_enableCrc = true;
        auto offset = cursor.indexOf(crc8Header) + crc8Header.length() - 1;

        // Check if we have enough data in the buffer
        if (cursor.length() >= offset + 1)
        {
            // Increment the number of bytes to remove from master buffer
            *bytes += crc8Header.length() + 1;

            // Get 8-bit checksum
            quint8 crc = cursor.at(offset + 1);

            // Compare checksums
            if (crc8(frame.data(), frame.length()) == crc)
                return ValidationStatus::FrameOk;
            else
                return ValidationStatus::ChecksumError;
        }
    }

    // Check CRC-16
    else if (cursor.contains(crc16Header))
    {
        // Enable the CRC flag
        m_enableCrc = true;
        auto offset = cursor.indexOf(crc16Header) + crc16Header.length() - 1;

        // Check if we have enough data in the buffer
        if (cursor.length() >= offset + 2)
        {
            // Increment the number of bytes to remove from master buffer
            *bytes += crc16Header.length() + 2;

            // Get 16-bit checksum
            quint8 a = cursor.at(offset + 1);
            quint8 b = cursor.at(offset + 2);
            quint16 crc = (a << 8) | (b & 0xff);

            // Compare checksums
            if (crc16(frame.data(), frame.length()) == crc)
                return ValidationStatus::FrameOk;
            else
                return ValidationStatus::ChecksumError;
        }
    }

    // Check CRC-32
    else if (cursor.contains(crc32Header))
    {
        // Enable the CRC flag
        m_enableCrc = true;
        auto offset = cursor.indexOf(crc32Header) + crc32Header.length() - 1;

        // Check if we have enough data in the buffer
        if (cursor.length() >= offset + 4)
        {
            // Increment the number of bytes to remove from master buffer
            *bytes += crc32Header.length() + 4;

            // Get 32-bit checksum
            quint8 a = cursor.at(offset + 1);
            quint8 b = cursor.at(offset + 2);
            quint8 c = cursor.at(offset + 3);
            quint8 d = cursor.at(offset + 4);
            quint32 crc = (a << 24) | (b << 16) | (c << 8) | (d & 0xff);

            // Compare checksums
            if (crc32(frame.data(), frame.length()) == crc)
                return ValidationStatus::FrameOk;
            else
                return ValidationStatus::ChecksumError;
        }
    }

    // Buffer does not contain CRC code
    else if (!m_enableCrc)
    {
        *bytes += finish.length();
        return ValidationStatus::FrameOk;
    }

    // Checksum data incomplete
    return ValidationStatus::ChecksumIncomplete;
}
}
