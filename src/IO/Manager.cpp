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

#include "Manager.h"

#include <Logger.h>
#include <IO/DataSources/Serial.h>
#include <IO/DataSources/Network.h>

using namespace IO;

/**
 * Pointer to the only instance of the class.
 */
static Manager *INSTANCE = nullptr;

/**
 * Constructor function
 */
Manager::Manager()
    : m_writeEnabled(true)
    , m_maxBuzzerSize(1024 * 1024)
    , m_device(nullptr)
    , m_dataSource(DataSource::Serial)
    , m_receivedBytes(0)
    , m_startSequence("/*")
    , m_finishSequence("*/")
{
    setWatchdogInterval(15);
    setMaxBufferSize(1024 * 1024);
    LOG_TRACE() << "Class initialized";
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
    if (!INSTANCE)
        INSTANCE = new Manager;

    return INSTANCE;
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
 * Returns @c true if a device is currently selected and opened
 */
bool Manager::connected()
{
    if (device())
        return device()->isOpen();

    return false;
}

/**
 * Returns @c true if a device is currently selected
 */
bool Manager::deviceAvailable()
{
    return device() != nullptr;
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
    return m_maxBuzzerSize;
}

/**
 * Returns a pointer to the currently selected device.
 *
 * @warning you need to check this pointer before using it, it can have a @c nullptr
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
 * Returns the size of the data received (and successfully read) from the device in the
 * format of a string.
 */
QString Manager::receivedDataLength() const
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
        double kb = static_cast<double>(m_receivedBytes) / 1024.0;
        value = QString::number(kb, 'f', 2);
        units = "KB";
    }

    else
    {
        double mb = static_cast<double>(m_receivedBytes) / (1024 * 1024.0);
        value = QString::number(mb, 'f', 2);
        units = "MB";
    }

    return QString("%1 %2").arg(value).arg(units);
}

/**
 * Returns a list with the possible data source options.
 */
QStringList Manager::dataSourcesList() const
{
    QStringList list;
    list.append(tr("Serial port"));
    list.append(tr("Network port"));
    return list;
}

/**
 * Tries to write the given @a data to the current device. Upon data write, the class
 * emits the @a tx() signal for UI updating.
 *
 * @returns the number of bytes written to the target device
 */
qint64 Manager::writeData(const QByteArray &data)
{
    // Write data to device
    qint64 bytes = 0;
    if (readWrite())
        bytes = device()->write(data);

    // Flash UI lights (if any)
    if (bytes > 0)
        emit tx();

    // Return number of bytes written
    return bytes;
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

    // Configure current device
    if (deviceAvailable())
    {
        // Set open flag
        QIODevice::OpenMode mode = QIODevice::ReadOnly;
        if (m_writeEnabled)
            mode = QIODevice::ReadWrite;

        // Open device
        if (device()->open(mode))
        {
            connect(device(), &QIODevice::readyRead, this, &Manager::onDataReceived);
            LOG_INFO() << "Device opened successfully";
        }

        // Error opening the device
        else
        {
            LOG_WARNING() << "Failed to open device, closing...";
            disconnectDevice();
        }

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

        // Update device pointer
        m_device = nullptr;
        m_receivedBytes = 0;
        m_dataBuffer.clear();
        m_dataBuffer.reserve(maxBufferSize());

        // Update UI
        emit deviceChanged();
        emit connectedChanged();

        // Log changes
        LOG_INFO() << "Device disconnected";
    }
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
 * Changes the data source type. Check the @c dataSource() funciton for more information.
 */
void Manager::setDataSource(const DataSource source)
{
    // Disconnect current device
    if (connected())
        disconnectDevice();

    // Change data source
    m_dataSource = source;
    emit dataSourceChanged();
}

/**
 * Changes the maximum permited buffer size. Check the @c maxBufferSize() function for
 * more information.
 */
void Manager::setMaxBufferSize(const int maxBufferSize)
{
    m_maxBuzzerSize = maxBufferSize;
    emit maxBufferSizeChanged();

    m_dataBuffer.reserve(maxBufferSize);
}

/**
 * Changes the frame start sequence. Check the @c startSequence() function for more
 * information.
 */
void Manager::setStartSequence(const QString &sequence)
{
    m_startSequence = sequence;
    emit startSequenceChanged();
}

/**
 * Changes the frame end sequence. Check the @c finishSequence() function for more
 * information.
 */
void Manager::setFinishSequence(const QString &sequence)
{
    m_finishSequence = sequence;
    emit finishSequenceChanged();
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
 * Read frames from temporary buffer, every frame that contains the appropiate start/end
 * sequence is removed from the buffer as soon as its read.
 *
 * This function also checks that the buffer size does not exceed specified size
 * limitations.
 */
void Manager::readFrames()
{
    // No device connected, abort
    if (!connected())
        return;

    // Read until start/finish combinations are not found
    auto start = startSequence().toUtf8();
    auto finish = finishSequence().toUtf8();
    while (m_dataBuffer.contains(start) && m_dataBuffer.contains(finish))
    {
        // Begin reading from start sequence index
        auto buffer = m_dataBuffer;
        auto sIndex = m_dataBuffer.indexOf(start);
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
            emit frameReceived(buffer);
            m_dataBuffer.remove(0, sIndex + fIndex + finish.length());
        }
    }

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

    // Read data & append it to buffer
    auto data = device()->readAll();
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
    emit rx();
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

    LOG_TRACE() << "Device pointer set to" << m_device;
}
