#include "Manager.h"

#include <Logger.h>
#include <IO/DataSources/Serial.h>
#include <IO/DataSources/Network.h>

using namespace IO;

static Manager *INSTANCE = nullptr;

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
}

Manager::~Manager() { }

Manager *Manager::getInstance()
{
    if (!INSTANCE)
        INSTANCE = new Manager;

    return INSTANCE;
}

bool Manager::readOnly()
{
    return connected() && !m_writeEnabled;
}

bool Manager::readWrite()
{
    return connected() && m_writeEnabled;
}

bool Manager::connected()
{
    if (device())
        return device()->isOpen();

    return false;
}

bool Manager::deviceAvailable()
{
    return device() != nullptr;
}

int Manager::watchdogInterval() const
{
    return m_watchdog.interval();
}

int Manager::maxBufferSize() const
{
    return m_maxBuzzerSize;
}

QIODevice *Manager::device()
{
    return m_device;
}

Manager::DataSource Manager::dataSource() const
{
    return m_dataSource;
}

QString Manager::startSequence() const
{
    return m_startSequence;
}

QString Manager::finishSequence() const
{
    return m_finishSequence;
}

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

QStringList Manager::dataSourcesList() const
{
    QStringList list;
    list.append(tr("Serial port"));
    list.append(tr("Network port"));
    return list;
}

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

void Manager::toggleConnection()
{
    if (connected())
        disconnectDevice();
    else
        connectDevice();
}

void Manager::connectDevice()
{
    // Disconnect previous device (if any)
    disconnectDevice();

    // Set device handler
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

        // Update UI
        emit deviceChanged();
        emit connectedChanged();
    }
}

void Manager::setWriteEnabled(const bool enabled)
{
    m_writeEnabled = enabled;
    emit writeEnabledChanged();
}

void Manager::setDataSource(const DataSource source)
{
    // Disconnect current device
    if (connected())
        disconnectDevice();

    // Change data source
    m_dataSource = source;
    emit dataSourceChanged();
}

void Manager::setMaxBufferSize(const int maxBufferSize)
{
    m_maxBuzzerSize = maxBufferSize;
    emit maxBufferSizeChanged();
}

void Manager::setStartSequence(const QString &sequence)
{
    m_startSequence = sequence;
    emit startSequenceChanged();
}

void Manager::setFinishSequence(const QString &sequence)
{
    m_finishSequence = sequence;
    emit finishSequenceChanged();
}

void Manager::setWatchdogInterval(const int interval)
{
    m_watchdog.setInterval(interval);
    m_watchdog.setTimerType(Qt::PreciseTimer);
    emit watchdogIntervalChanged();
}

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

void Manager::feedWatchdog()
{
    m_watchdog.stop();
    m_watchdog.start();
}

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

void Manager::clearTempBuffer()
{
    m_dataBuffer.clear();
}

void Manager::onWatchdogTriggered()
{
    clearTempBuffer();
}

void Manager::setDevice(QIODevice *device)
{
    disconnectDevice();
    m_device = device;
    emit deviceChanged();
}
