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

#include "IO/Drivers/Audio.h"

#include <algorithm>
#include <chrono>
#include <QtEndian>

#include "IO/ConnectionManager.h"
#include "Misc/TimerEvents.h"
#include "Misc/Translator.h"

//--------------------------------------------------------------------------------------------------
// Utility functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Compares two device lists to determine if they differ.
 */
static bool deviceListsDiffer(const QVector<ma_device_info>& a, const QVector<ma_device_info>& b)
{
  // Size mismatch is an immediate difference
  if (a.size() != b.size())
    return true;

  // Check each device ID and name for equality
  for (int i = 0; i < a.size(); ++i) {
    if (memcmp(&a[i].id, &b[i].id, sizeof(ma_device_id)) != 0)
      return true;

    if (strcmp(a[i].name, b[i].name) != 0)
      return true;
  }

  return false;
}

/**
 * @brief Extracts audio device capabilities using MiniAudio's backend context.
 */
static IO::Drivers::Audio::AudioDeviceInfo extractCapabilities(ma_context* context,
                                                               const ma_device_info& info,
                                                               ma_device_type type)
{
  const QSet<int> defaultChannels    = {1, 2};
  const QSet<int> defaultSampleRates = {
    8000, 11025, 16000, 22050, 44100, 48000, 88200, 96000, 176400, 192000, 352800, 384000};

  const QSet<ma_format> defaultFormats = {
    ma_format_u8, ma_format_s16, ma_format_s24, ma_format_s32, ma_format_f32};

  // Query full device info from the backend
  ma_device_info fullInfo = {};
  auto r                  = ma_context_get_device_info(context, type, &info.id, &fullInfo);
  if (r != MA_SUCCESS)
    fullInfo = info;

  // Parse native data formats
  QSet<int> sampleRates;
  QSet<ma_format> formats;
  QSet<int> channelCounts = {1};
  for (ma_uint32 i = 0; i < fullInfo.nativeDataFormatCount; ++i) {
    const auto& f = fullInfo.nativeDataFormats[i];

    if (f.format == ma_format_unknown)
      formats.unite(defaultFormats);
    else
      formats.insert(f.format);

    if (f.channels == 0)
      channelCounts.unite(defaultChannels);
    else
      channelCounts.insert(static_cast<int>(f.channels));

    if (f.sampleRate == 0)
      sampleRates.unite(defaultSampleRates);
    else
      sampleRates.insert(static_cast<int>(f.sampleRate));
  }

  // Apply fallback defaults for empty sets
  if (formats.isEmpty())
    formats = defaultFormats;
  if (sampleRates.isEmpty())
    sampleRates = defaultSampleRates;
  if (channelCounts.isEmpty())
    channelCounts = defaultChannels;

  IO::Drivers::Audio::AudioDeviceInfo caps;
  caps.supportedFormats       = formats.values();
  caps.supportedSampleRates   = sampleRates.values();
  caps.supportedChannelCounts = channelCounts.values();

  // Sort for consistent UI display
  std::sort(caps.supportedFormats.begin(), caps.supportedFormats.end());
  std::sort(caps.supportedSampleRates.begin(), caps.supportedSampleRates.end());
  std::sort(caps.supportedChannelCounts.begin(), caps.supportedChannelCounts.end());

  return caps;
}

/**
 * @brief Checks for changes in the audio device list and updates it if necessary.
 */
static bool checkAndUpdateDeviceList(ma_context* context,
                                     ma_device_type type,
                                     QVector<ma_device_info>& currentList,
                                     const QVector<ma_device_info>& newList,
                                     int selectedIndex,
                                     bool isOpen,
                                     const std::function<void()>& settingsChanged,
                                     const std::function<void()>& configurationChanged,
                                     QVector<IO::Drivers::Audio::AudioDeviceInfo>& capabilities)
{
  // Safe to replace lists while no device is active
  if (!isOpen) {
    if (deviceListsDiffer(newList, currentList)) {
      currentList = newList;
      capabilities.clear();
      for (const auto& info : std::as_const(currentList))
        capabilities.append(extractCapabilities(context, info, type));

      settingsChanged();
      return true;
    }

    return false;
  }

  // Verify the active device is still present in the new list
  bool stillConnected = false;
  if (selectedIndex >= 0 && selectedIndex < currentList.size()) {
    const ma_device_id& currentId = currentList[selectedIndex].id;
    for (const auto& device : newList) {
      if (memcmp(&device.id, &currentId, sizeof(ma_device_id)) == 0) {
        stillConnected = true;
        break;
      }
    }
  }

  // Active device was unplugged, disconnect and refresh
  if (!stillConnected) {
    IO::ConnectionManager::instance().disconnectDevice();
    currentList = newList;
    capabilities.clear();
    for (const auto& info : std::as_const(currentList))
      capabilities.append(extractCapabilities(context, info, type));

    settingsChanged();
    configurationChanged();
    return true;
  }

  // Active device still present, no update needed
  return false;
}

//--------------------------------------------------------------------------------------------------
// Constructor, destructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the Audio driver and initializes the MiniAudio backend.
 */
IO::Drivers::Audio::Audio()
  : m_init(false)
  , m_isOpen(false)
  , m_selectedSampleRate(0)
  , m_selectedInputDevice(-1)
  , m_selectedInputSampleFormat(0)
  , m_selectedInputChannelConfiguration(0)
  , m_selectedOutputDevice(-1)
  , m_selectedOutputSampleFormat(0)
  , m_selectedOutputChannelConfiguration(0)
  , m_inputWorkerTimer(nullptr)
{
  // Manually select backend for each operating system
#if defined(Q_OS_WIN)
  ma_backend backend[] = {ma_backend_wasapi};
#elif defined(Q_OS_APPLE)
  ma_backend backend[] = {ma_backend_coreaudio};
#elif defined(Q_OS_LINUX)
  ma_backend backend[] = {ma_backend_alsa};
#else
#  error "Unsupported platform"
#endif

  // Initialize Mini Audio
  m_config = ma_device_config_init(ma_device_type_duplex);
  m_init   = ma_context_init(backend, 1, nullptr, &m_context) == MA_SUCCESS;
  if (!m_init)
    qWarning("Failed to initialize miniaudio context");

  // Configure the CSV ouput buffer
  m_csvData.reserve(96 * 1024);
  m_csvBuffer.setBuffer(&m_csvData);
  m_csvBuffer.open(QIODevice::WriteOnly);

  // Configure the CSV stream
  m_csvStream.setRealNumberPrecision(6);
  m_csvStream.setDevice(&m_csvBuffer);
  m_csvStream.setRealNumberNotation(QTextStream::FixedNotation);

  // Initialize maps to connect user friendly strings to actual values
  generateLists();
  refreshAudioDevices();

  // Sync UI selection with applied format
  syncInputParameters();
  syncOutputParameters();

  // Configure model
  configureInput();
  configureOutput();

  // Refresh devices every second
  connect(&Misc::TimerEvents::instance(),
          &Misc::TimerEvents::timeout1Hz,
          this,
          &Audio::refreshAudioDevices);

  // Update lists when language changes
  connect(&Misc::Translator::instance(),
          &Misc::Translator::languageChanged,
          this,
          &IO::Drivers::Audio::generateLists);
}

/**
 * @brief Closes the device and tears down the MiniAudio context.
 */
IO::Drivers::Audio::~Audio()
{
  closeDevice();
  if (m_init)
    ma_context_uninit(&m_context);
}

//--------------------------------------------------------------------------------------------------
// HAL driver function implementations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Closes the audio device and releases all associated resources.
 */
void IO::Drivers::Audio::closeDevice()
{
  if (!m_isOpen)
    return;

  // Stop and uninit MiniAudio device
  ma_device_uninit(&m_device);

  // Stop and delete timer in its own thread
  if (m_inputWorkerTimer) {
    QMetaObject::invokeMethod(m_inputWorkerTimer, "stop", Qt::QueuedConnection);
    QMetaObject::invokeMethod(m_inputWorkerTimer, "deleteLater", Qt::QueuedConnection);
    m_inputWorkerTimer = nullptr;
  }

  // Terminate input thread cleanly
  if (m_inputWorkerThread.isRunning()) {
    m_inputWorkerThread.quit();
    m_inputWorkerThread.wait();
  }

  // Clear internal input buffer
  {
    QMutexLocker locker(&m_inputBufferLock);
    m_rawInput.clear();
  }

  // Clear internal output queue
  {
    QMutexLocker locker(&m_outputBufferLock);
    m_outputQueue.clear();
  }

  m_isOpen = false;
}

/**
 * @brief Closes the audio device.
 */
void IO::Drivers::Audio::close()
{
  closeDevice();
}

/**
 * @brief Returns true when the audio device is currently open.
 */
bool IO::Drivers::Audio::isOpen() const noexcept
{
  return m_isOpen;
}

/**
 * @brief Determines if the current audio input configuration is valid and readable.
 */
bool IO::Drivers::Audio::isReadable() const noexcept
{
  if (!m_isOpen)
    return false;

  return m_selectedInputDevice >= 0 && m_selectedInputDevice < m_inputDevices.size()
      && m_config.capture.channels > 0;
}

/**
 * @brief Determines if the current audio output configuration is valid and writable.
 */
bool IO::Drivers::Audio::isWritable() const noexcept
{
  if (!m_isOpen)
    return false;

  return m_selectedOutputDevice >= 0 && m_selectedOutputDevice < m_outputDevices.size()
      && m_config.playback.channels > 0;
}

/**
 * @brief Validates the currently selected input device configuration.
 */
bool IO::Drivers::Audio::configurationOk() const noexcept
{
  if (!validateInput())
    return false;

  bool ok       = true;
  const auto& c = m_inputCapabilities[m_selectedInputDevice];
  ok &= m_selectedSampleRate >= 0;
  ok &= m_selectedInputSampleFormat >= 0;
  ok &= m_selectedInputChannelConfiguration >= 0;
  ok &= m_selectedInputSampleFormat < c.supportedFormats.size();
  ok &= m_selectedSampleRate < c.supportedSampleRates.size();
  ok &= m_selectedInputChannelConfiguration < c.supportedChannelCounts.size();

  return ok;
}

/**
 * @brief Writes a CSV-formatted audio frame into the internal output queue.
 */
qint64 IO::Drivers::Audio::write(const QByteArray& data)
{
  // Abort if output device is not ready
  if (!m_isOpen || m_config.playback.channels <= 0) {
    qWarning() << "Output device not available or misconfigured.";
    return 0;
  }

  // Verify CSV column count matches the playback channel count
  const int channels            = m_config.playback.channels;
  const ma_format format        = m_config.playback.format;
  const QList<QByteArray> parts = data.trimmed().split(',');
  if (parts.size() != channels) {
    qWarning() << "Channel mismatch: expected" << channels << "but got" << parts.size() << ":"
               << data;
    return 0;
  }

  // Pack each channel value into native sample bytes
  QVector<quint8> frame;
  for (int i = 0; i < channels; ++i) {
    bool ok = false;

    if (format == ma_format_u8) {
      int value = parts[i].toInt(&ok);
      if (!ok) {
        qWarning() << "Invalid Unigned 8-bit number:" << parts[i];
        return 0;
      }

      frame.append(static_cast<quint8>(qBound(0, value, 255)));
    }

    else if (format == ma_format_s16) {
      int value = parts[i].toInt(&ok);
      if (!ok) {
        qWarning() << "Invalid Signed 16-bit number:" << parts[i];
        return 0;
      }

      const qint16 sample = static_cast<qint16>(qBound(-32768, value, 32767));
      const quint8* bytes = reinterpret_cast<const quint8*>(&sample);
      frame.append(bytes[0]);
      frame.append(bytes[1]);
    }

    else if (format == ma_format_s24) {
      int value = parts[i].toInt(&ok);
      if (!ok) {
        qWarning() << "Invalid Signed 24-bit number:" << parts[i];
        return 0;
      }

      value = qBound(-8388608, value, 8388607);
      frame.append(static_cast<quint8>(value & 0xFF));
      frame.append(static_cast<quint8>((value >> 8) & 0xFF));
      frame.append(static_cast<quint8>((value >> 16) & 0xFF));
    }

    else if (format == ma_format_s32) {
      qint32 value = parts[i].toInt(&ok);
      if (!ok) {
        qWarning() << "Invalid Signed 32-bit number:" << parts[i];
        return 0;
      }

      value               = qBound(-2147483647, value, 2147483647);
      const quint8* bytes = reinterpret_cast<const quint8*>(&value);
      frame.append(bytes[0]);
      frame.append(bytes[1]);
      frame.append(bytes[2]);
      frame.append(bytes[3]);
    }

    else if (format == ma_format_f32) {
      float value = parts[i].toFloat(&ok);
      if (!ok) {
        qWarning() << "Invalid 32-bit Float number:" << parts[i];
        return 0;
      }

      value               = qBound(-1.0f, value, 1.0f);
      const quint8* bytes = reinterpret_cast<const quint8*>(&value);
      for (size_t b = 0; b < sizeof(float); ++b)
        frame.append(bytes[b]);
    }

    else {
      qWarning() << "Unsupported format:" << static_cast<int>(format);
      return 0;
    }
  }

  // Enqueue frame for playback
  QMutexLocker locker(&m_outputBufferLock);
  m_outputQueue.push_back(std::move(frame));

  return data.size();
}

/**
 * @brief Opens the audio I/O device with the current configuration.
 */
bool IO::Drivers::Audio::open(const QIODevice::OpenMode mode)
{
  if (!m_init || m_isOpen || !configurationOk())
    return false;

  // Reset config and wire in the audio callback
  m_config = ma_device_config_init(ma_device_type_duplex);

  // clang-format off
  m_config.pUserData = this;
  m_config.dataCallback = &Audio::callback;
  m_config.sampleRate = m_inputCapabilities[m_selectedInputDevice].supportedSampleRates[m_selectedSampleRate];
  // clang-format on

  // Disable clipping and denormal suppression, allow variable frame sizes
  m_config.noClip                    = MA_FALSE;
  m_config.noDisableDenormals        = MA_FALSE;
  m_config.noFixedSizedCallback      = MA_TRUE;
  m_config.noPreSilencedOutputBuffer = MA_FALSE;

  // macOS configuration
#ifdef Q_OS_MAC
  m_config.coreaudio.allowNominalSampleRateChange = MA_TRUE;
#endif

  // Windows configuration
#ifdef Q_OS_WIN
  m_config.wasapi.noAutoConvertSRC     = MA_FALSE;
  m_config.wasapi.noDefaultQualitySRC  = MA_FALSE;
  m_config.wasapi.noAutoStreamRouting  = MA_FALSE;
  m_config.wasapi.noHardwareOffloading = MA_TRUE;
#endif

  // Linux configuration
#ifdef Q_OS_LINUX
  m_config.alsa.noMMap         = MA_FALSE;
  m_config.alsa.noAutoFormat   = MA_FALSE;
  m_config.alsa.noAutoChannels = MA_FALSE;
  m_config.alsa.noAutoResample = MA_FALSE;
#endif

  // Configure capture from the selected input device
  if (mode & QIODevice::ReadOnly) {
    // clang-format off
    m_config.capture.pDeviceID = &m_inputDevices[m_selectedInputDevice].id;
    m_config.capture.format = m_inputCapabilities[m_selectedInputDevice].supportedFormats[m_selectedInputSampleFormat];
    m_config.capture.channels = m_inputCapabilities[m_selectedInputDevice].supportedChannelCounts[m_selectedInputChannelConfiguration];
    // clang-format on
  }

  // No capture requested, disable input channels
  else {
    m_config.capture.format   = ma_format_unknown;
    m_config.capture.channels = 0;
  }

  // Configure playback from the selected output device
  if (mode & QIODevice::WriteOnly) {
    if (m_selectedOutputDevice < 0 || m_selectedOutputDevice >= m_outputDevices.size()
        || m_selectedOutputDevice >= m_outputCapabilities.size()) {
      qWarning() << "Audio::open: output device index out of range";
      return false;
    }

    const auto& oc = m_outputCapabilities[m_selectedOutputDevice];
    if (m_selectedOutputSampleFormat < 0
        || m_selectedOutputSampleFormat >= oc.supportedFormats.size()
        || m_selectedOutputChannelConfiguration < 0
        || m_selectedOutputChannelConfiguration >= oc.supportedChannelCounts.size()) {
      qWarning() << "Audio::open: output format/channel index out of range";
      return false;
    }

    // clang-format off
    m_config.playback.pDeviceID = &m_outputDevices[m_selectedOutputDevice].id;
    m_config.playback.format = oc.supportedFormats[m_selectedOutputSampleFormat];
    m_config.playback.channels = oc.supportedChannelCounts[m_selectedOutputChannelConfiguration];
    // clang-format on
  }

  // No playback requested, disable output channels
  else {
    m_config.playback.format   = ma_format_unknown;
    m_config.playback.channels = 0;
  }

  // Try to initialize the duplex device
  std::memset(&m_device, 0, sizeof(m_device));
  if (ma_device_init(&m_context, &m_config, &m_device) != MA_SUCCESS) {
    qWarning() << "Failed to initialize miniaudio device.";
    return false;
  }

  // Start the audio device
  if (ma_device_start(&m_device) != MA_SUCCESS) {
    qWarning() << "Failed to start miniaudio device.";
    ma_device_uninit(&m_device);
    return false;
  }

  // Configure audio processing thread and timer
  if (!m_inputWorkerTimer) {
    m_inputWorkerTimer = new QTimer();
    m_inputWorkerTimer->setInterval(10);
    m_inputWorkerTimer->setTimerType(Qt::PreciseTimer);
    m_inputWorkerTimer->moveToThread(&m_inputWorkerThread);
    connect(&m_inputWorkerThread, &QThread::finished, m_inputWorkerTimer, &QObject::deleteLater);
    connect(m_inputWorkerTimer, &QTimer::timeout, this, &IO::Drivers::Audio::processInputBuffer);
  }

  // Start the worker thread
  if (!m_inputWorkerThread.isRunning()) {
    m_inputWorkerThread.start();
    m_inputWorkerThread.setPriority(QThread::HighestPriority);
  }

  // Start the read timer @ 100 Hz
  QMetaObject::invokeMethod(m_inputWorkerTimer, "start", Qt::QueuedConnection);

  m_isOpen = true;
  return true;
}

//--------------------------------------------------------------------------------------------------
// Sample rate selection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the index of the currently selected sample rate.
 */
int IO::Drivers::Audio::selectedSampleRate() const
{
  return m_selectedSampleRate;
}

/**
 * @brief Returns a list of supported input sample rates as strings.
 */
QStringList IO::Drivers::Audio::sampleRates() const
{
  QStringList list;
  if (!validateInput())
    return list;

  const auto& device = m_inputCapabilities[m_selectedInputDevice];
  for (int rate : device.supportedSampleRates)
    list.append(QString::number(rate));

  return list;
}

//--------------------------------------------------------------------------------------------------
// Input device parameters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the index of the currently selected input device.
 */
int IO::Drivers::Audio::selectedInputDevice() const
{
  return m_selectedInputDevice;
}

/**
 * @brief Returns the index of the currently selected input sample format.
 */
int IO::Drivers::Audio::selectedInputSampleFormat() const
{
  return m_selectedInputSampleFormat;
}

/**
 * @brief Returns the index of the currently selected input channel configuration.
 */
int IO::Drivers::Audio::selectedInputChannelConfiguration() const
{
  return m_selectedInputChannelConfiguration;
}

//--------------------------------------------------------------------------------------------------
// Output device parameters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the index of the currently selected output device.
 */
int IO::Drivers::Audio::selectedOutputDevice() const
{
  return m_selectedOutputDevice;
}

/**
 * @brief Returns the index of the currently selected output sample format.
 */
int IO::Drivers::Audio::selectedOutputSampleFormat() const
{
  return m_selectedOutputSampleFormat;
}

/**
 * @brief Returns the index of the currently selected output channel configuration.
 */
int IO::Drivers::Audio::selectedOutputChannelConfiguration() const
{
  return m_selectedOutputChannelConfiguration;
}

//--------------------------------------------------------------------------------------------------
// Input device parameter models
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a list of available input audio devices.
 */
QStringList IO::Drivers::Audio::inputDeviceList() const
{
  QStringList list;

  for (const auto& device : m_inputDevices)
    list.append(QString::fromUtf8(device.name));

  return list;
}

/**
 * @brief Returns the list of supported sample formats for the selected input device.
 */
QStringList IO::Drivers::Audio::inputSampleFormats() const
{
  QStringList list;
  if (!validateInput())
    return list;

  const auto& device = m_inputCapabilities[m_selectedInputDevice];
  for (ma_format format : device.supportedFormats)
    if (m_sampleFormats.contains(format))
      list.append(m_sampleFormats.value(format));

  return list;
}

/**
 * @brief Returns the list of supported input channel configurations.
 */
QStringList IO::Drivers::Audio::inputChannelConfigurations() const
{
  QStringList list;
  if (!validateInput())
    return list;

  const auto& device = m_inputCapabilities[m_selectedInputDevice];
  for (int channels : device.supportedChannelCounts)
    if (m_knownConfigs.contains(static_cast<ma_channel>(channels)))
      list.append(m_knownConfigs.value(static_cast<ma_channel>(channels)));
    else
      list.append(QString::number(channels) + " " + tr("channels"));

  return list;
}

//--------------------------------------------------------------------------------------------------
// Output device parameter models
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a list of available output audio devices.
 */
QStringList IO::Drivers::Audio::outputDeviceList() const
{
  QStringList list;

  for (const auto& device : m_outputDevices)
    list.append(QString::fromUtf8(device.name));

  return list;
}

/**
 * @brief Returns a list of supported sample formats for the selected output device.
 */
QStringList IO::Drivers::Audio::outputSampleFormats() const
{
  QStringList list;
  if (!validateOutput())
    return list;

  const auto& device = m_outputCapabilities[m_selectedOutputDevice];
  for (ma_format format : device.supportedFormats)
    if (m_sampleFormats.contains(format))
      list.append(m_sampleFormats.value(format));

  return list;
}

/**
 * @brief Returns a list of supported output channel configurations.
 */
QStringList IO::Drivers::Audio::outputChannelConfigurations() const
{
  QStringList list;
  if (!validateOutput())
    return list;

  const auto& device = m_outputCapabilities[m_selectedOutputDevice];
  for (int channels : device.supportedChannelCounts)
    if (m_knownConfigs.contains(static_cast<ma_channel>(channels)))
      list.append(m_knownConfigs.value(static_cast<ma_channel>(channels)));
    else
      list.append(QString::number(channels) + tr(" channels"));

  return list;
}

//--------------------------------------------------------------------------------------------------
// Sample rate configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the selected sample rate index for the input device.
 */
void IO::Drivers::Audio::setSelectedSampleRate(int index)
{
  if (isOpen())
    return;

  if (index < 0 || index >= sampleRates().size())
    index = 0;

  m_selectedSampleRate = index;
  configureInput();
}

//--------------------------------------------------------------------------------------------------
// Input device parameter setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the selected input device by index.
 */
void IO::Drivers::Audio::setSelectedInputDevice(int index)
{
  if (isOpen())
    return;

  if (index < 0 || index >= inputDeviceList().size())
    index = 0;

  m_selectedInputDevice               = index;
  m_selectedSampleRate                = -1;
  m_selectedInputSampleFormat         = -1;
  m_selectedInputChannelConfiguration = -1;

  syncInputParameters();
  configureInput();
}

/**
 * @brief Sets the selected sample format index for the input device.
 */
void IO::Drivers::Audio::setSelectedInputSampleFormat(int index)
{
  if (isOpen())
    return;

  if (index < 0 || index >= inputSampleFormats().size())
    index = 0;

  m_selectedInputSampleFormat = index;
  configureInput();
}

/**
 * @brief Sets the selected channel configuration for the input device.
 */
void IO::Drivers::Audio::setSelectedInputChannelConfiguration(int index)
{
  if (isOpen())
    return;

  if (index < 0 || index >= inputChannelConfigurations().size())
    index = 0;

  m_selectedInputChannelConfiguration = index;
  configureInput();
}

//--------------------------------------------------------------------------------------------------
// Output device parameter setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sets the selected output device by index.
 */
void IO::Drivers::Audio::setSelectedOutputDevice(int index)
{
  if (isOpen())
    return;

  if (index < 0 || index >= outputDeviceList().size())
    index = 0;

  m_selectedOutputDevice               = index;
  m_selectedOutputSampleFormat         = -1;
  m_selectedOutputChannelConfiguration = -1;

  syncOutputParameters();
  configureOutput();
}

/**
 * @brief Sets the selected sample format for the output device.
 */
void IO::Drivers::Audio::setSelectedOutputSampleFormat(int index)
{
  if (isOpen())
    return;

  if (index < 0 || index >= outputSampleFormats().size())
    index = 0;

  m_selectedOutputSampleFormat = index;
  configureOutput();
}

/**
 * @brief Sets the selected output channel configuration.
 */
void IO::Drivers::Audio::setSelectedOutputChannelConfiguration(int index)
{
  if (isOpen())
    return;

  if (index < 0 || index >= outputChannelConfigurations().size())
    index = 0;

  m_selectedOutputChannelConfiguration = index;
  configureOutput();
}

//--------------------------------------------------------------------------------------------------
// UI helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Populates maps that associate MiniAudio constants with user-friendly strings.
 */
void IO::Drivers::Audio::generateLists()
{
  m_sampleFormats = {
    { ma_format_u8, tr("Unsigned 8-bit")},
    {ma_format_s16,  tr("Signed 16-bit")},
    {ma_format_s24,  tr("Signed 24-bit")},
    {ma_format_s32,  tr("Signed 32-bit")},
    {ma_format_f32,   tr("Float 32-bit")}
  };
  m_knownConfigs = {
    {1,   tr("Mono")},
    {2, tr("Stereo")},
    {3,        "3.0"},
    {4,        "4.0"},
    {5,        "5.0"},
    {6,        "5.1"},
    {7,        "6.1"},
    {8,        "7.1"}
  };

  Q_EMIT inputSettingsChanged();
  Q_EMIT outputSettingsChanged();
}

//--------------------------------------------------------------------------------------------------
// Device configuration helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies the selected input device configuration.
 */
void IO::Drivers::Audio::configureInput()
{
  if (!validateInput())
    return;

  // Abort if capabilities are empty
  const AudioDeviceInfo& caps = m_inputCapabilities[m_selectedInputDevice];
  if (caps.supportedSampleRates.isEmpty() || caps.supportedFormats.isEmpty()
      || caps.supportedChannelCounts.isEmpty()) {
    qWarning() << "Input capabilities for selected device are not populated";
    return;
  }

  // Clamp indices to valid range
  // clang-format off
  m_selectedSampleRate = qBound(0, m_selectedSampleRate, caps.supportedSampleRates.size() - 1);
  m_selectedInputSampleFormat = qBound(0, m_selectedInputSampleFormat, caps.supportedFormats.size() - 1);
  m_selectedInputChannelConfiguration = qBound(0, m_selectedInputChannelConfiguration, caps.supportedChannelCounts.size() - 1);
  // clang-format on

  // Apply selected parameters to the MiniAudio config
  // clang-format off
  const int sampleRate = caps.supportedSampleRates[m_selectedSampleRate];
  const ma_format format = caps.supportedFormats[m_selectedInputSampleFormat];
  const int channels = caps.supportedChannelCounts[m_selectedInputChannelConfiguration];
  // clang-format on

  m_config.sampleRate       = sampleRate;
  m_config.capture.format   = format;
  m_config.capture.channels = channels;

  // Sync parameters and notify UI
  syncInputParameters();
  Q_EMIT configurationChanged();
}

/**
 * @brief Applies the selected output device configuration.
 */
void IO::Drivers::Audio::configureOutput()
{
  if (!validateOutput())
    return;

  // Abort if capabilities are empty
  const AudioDeviceInfo& caps = m_outputCapabilities[m_selectedOutputDevice];
  if (caps.supportedSampleRates.isEmpty() || caps.supportedFormats.isEmpty()
      || caps.supportedChannelCounts.isEmpty()) {
    qWarning() << "Output capabilities for selected device are not populated";
    return;
  }

  // Clamp indices to valid range
  // clang-format off
  m_selectedOutputSampleFormat = qBound(0, m_selectedOutputSampleFormat, caps.supportedFormats.size() - 1);
  m_selectedOutputChannelConfiguration = qBound(0, m_selectedOutputChannelConfiguration, caps.supportedChannelCounts.size() - 1);
  // clang-format on

  // Apply selected parameters to the MiniAudio config
  // clang-format off
  const ma_format format = caps.supportedFormats[m_selectedOutputSampleFormat];
  const int channels = caps.supportedChannelCounts[m_selectedOutputChannelConfiguration];
  // clang-format on

  m_config.playback.format   = format;
  m_config.playback.channels = channels;

  // Sync parameters and notify UI
  syncOutputParameters();
  Q_EMIT configurationChanged();
}

//--------------------------------------------------------------------------------------------------
// Audio input data parsing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Converts raw audio input into CSV-formatted text and emits it.
 */
void IO::Drivers::Audio::processInputBuffer()
{
  // Swap out the shared buffer to minimize lock duration
  QByteArray raw;
  {
    QMutexLocker locker(&m_inputBufferLock);
    if (m_rawInput.isEmpty())
      return;

    raw = std::move(m_rawInput);
    m_rawInput.clear();
  }

  // Validate frame alignment
  const int channels       = m_config.capture.channels;
  const ma_format format   = m_config.capture.format;
  const int bytesPerSample = ma_get_bytes_per_sample(format);
  const int frameSize      = bytesPerSample * channels;
  if (frameSize <= 0 || channels <= 0 || raw.size() % frameSize != 0)
    return;

  const int totalFrames = raw.size() / frameSize;

  // Rewind the reusable CSV buffer
  m_csvBuffer.seek(0);

  // Convert each audio frame to comma-separated channel values
  const char* ptr = raw.constData();
  for (int i = 0; i < totalFrames; ++i) {
    for (int ch = 0; ch < channels; ++ch) {
      switch (format) {
        case ma_format_u8: {
          const auto sample = static_cast<quint8>(*ptr);
          m_csvStream << static_cast<int>(sample);
          break;
        }
        case ma_format_s16: {
          const qint16 sample = qFromLittleEndian<qint16>(reinterpret_cast<const quint8*>(ptr));
          m_csvStream << sample;
          break;
        }
        case ma_format_s24: {
          const quint8* b  = reinterpret_cast<const quint8*>(ptr);
          const qint32 s24 = static_cast<qint32>(b[0]) | (static_cast<qint32>(b[1]) << 8)
                           | (static_cast<qint32>(b[2]) << 16);
          const qint32 sample = (s24 & 0x800000) ? (s24 | static_cast<qint32>(0xFF000000)) : s24;
          m_csvStream << sample;
          break;
        }
        case ma_format_s32: {
          const qint32 sample = qFromLittleEndian<qint32>(reinterpret_cast<const quint8*>(ptr));
          m_csvStream << sample;
          break;
        }
        case ma_format_f32: {
          float sample;
          std::memcpy(&sample, ptr, sizeof(float));
          m_csvStream << sample;
          break;
        }
        default:
          return;
      }

      ptr += bytesPerSample;
      if (ch < channels - 1)
        m_csvStream << ',';
    }

    m_csvStream << '\n';
  }

  // Flush and emit only the portion we actually wrote
  m_csvStream.flush();
  const auto length = m_csvBuffer.pos();
  const auto frameStep =
    std::max(std::chrono::nanoseconds(1),
             std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::duration<double>(1.0 / static_cast<double>(m_config.sampleRate))));
  const auto now       = IO::CapturedData::SteadyClock::now();
  const auto timestamp = now - (frameStep * std::max(0, totalFrames - 1));
  publishReceivedData(m_csvData.left(length), timestamp, frameStep, totalFrames);
}

//--------------------------------------------------------------------------------------------------
// Device discovery functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Refreshes the list of available audio input/output devices.
 */
void IO::Drivers::Audio::refreshAudioDevices()
{
  if (!m_init)
    return;

  // Query available devices from the MiniAudio backend
  ma_uint32 inputCount          = 0;
  ma_uint32 outputCount         = 0;
  ma_device_info* inputDevices  = nullptr;
  ma_device_info* outputDevices = nullptr;
  // clang-format off
  auto result = ma_context_get_devices(&m_context,
                                       &outputDevices,
                                       &outputCount,
                                       &inputDevices,
                                       &inputCount);

  if (result != MA_SUCCESS)
    return;
  // clang-format on

  // clang-format off
  const QVector<ma_device_info> newInputDevices(inputDevices, inputDevices + inputCount);
  const QVector<ma_device_info> newOutputDevices(outputDevices, outputDevices + outputCount);
  // clang-format on

  // Update list of input devices
  checkAndUpdateDeviceList(
    &m_context,
    ma_device_type_capture,
    m_inputDevices,
    newInputDevices,
    m_selectedInputDevice,
    m_isOpen,
    [this] { Q_EMIT inputSettingsChanged(); },
    [this] { Q_EMIT configurationChanged(); },
    m_inputCapabilities);

  // Update list of output devices
  checkAndUpdateDeviceList(
    &m_context,
    ma_device_type_playback,
    m_outputDevices,
    newOutputDevices,
    m_selectedOutputDevice,
    m_isOpen,
    [this] { Q_EMIT outputSettingsChanged(); },
    [this] { Q_EMIT configurationChanged(); },
    m_outputCapabilities);

  // Update fallback input device index if needed
  if (m_selectedInputDevice < 0 && newInputDevices.count() > 0) {
    m_selectedInputDevice = 0;
    Q_EMIT inputSettingsChanged();
  }

  // Update fallback output device index if needed
  if (m_selectedOutputDevice < 0 && newOutputDevices.count() > 0) {
    m_selectedOutputDevice = 0;
    Q_EMIT outputSettingsChanged();
  }
}

//--------------------------------------------------------------------------------------------------
// Model sync functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Synchronizes internal input parameters with the actual device config.
 */
void IO::Drivers::Audio::syncInputParameters()
{
  if (!validateInput())
    return;

  const auto& caps = m_inputCapabilities[m_selectedInputDevice];

  // Match current sample rate, fallback to 44.1 KHz (22 KHz on Windows)
  m_selectedSampleRate = caps.supportedSampleRates.indexOf(m_config.sampleRate);
  if (m_selectedSampleRate < 0) {
#ifdef Q_OS_WIN
    int fallback = caps.supportedSampleRates.indexOf(22050);
#else
    int fallback = caps.supportedSampleRates.indexOf(44100);
#endif
    m_selectedSampleRate = fallback >= 0 ? fallback : 0;
  }

  // clang-format off

  // Match current format, fallback to first available
  m_selectedInputSampleFormat = caps.supportedFormats.indexOf(m_config.capture.format);
  if (m_selectedInputSampleFormat < 0)
    m_selectedInputSampleFormat = 0;

  // Match current channel count, fallback to first available
  m_selectedInputChannelConfiguration = caps.supportedChannelCounts.indexOf(m_config.capture.channels);
  if (m_selectedInputChannelConfiguration < 0)
    m_selectedInputChannelConfiguration = 0;

  // clang-format on

  Q_EMIT inputSettingsChanged();
}

/**
 * @brief Synchronizes internal output parameters with the actual device config.
 */
void IO::Drivers::Audio::syncOutputParameters()
{
  if (!validateOutput())
    return;

  const auto& caps = m_outputCapabilities[m_selectedOutputDevice];

  // clang-format off

  // Match current format, fallback to first available
  m_selectedOutputSampleFormat = caps.supportedFormats.indexOf(m_config.playback.format);
  if (m_selectedOutputSampleFormat < 0)
    m_selectedOutputSampleFormat = 0;

  // Match current channel count, fallback to first available
  m_selectedOutputChannelConfiguration = caps.supportedChannelCounts.indexOf(m_config.playback.channels);
  if (m_selectedOutputChannelConfiguration < 0)
    m_selectedOutputChannelConfiguration = 0;

  // clang-format on

  Q_EMIT outputSettingsChanged();
}

//--------------------------------------------------------------------------------------------------
// Audio callback function
//--------------------------------------------------------------------------------------------------

/**
 * @brief Audio callback handler for processing input and output streams.
 */
void IO::Drivers::Audio::handleCallback(void* output, const void* input, ma_uint32 frameCount)
{
  // Capture parameters for byte arithmetic
  const ma_format format         = m_config.capture.format;
  const ma_uint32 channels       = m_config.capture.channels;
  const ma_uint32 bytesPerSample = ma_get_bytes_per_sample(format);
  const ma_uint32 bytesPerFrame  = bytesPerSample * channels;

  // Append raw input data to buffer for later CSV conversion
  if (input && channels > 0 && format != ma_format_unknown) {
    QMutexLocker locker(&m_inputBufferLock);
    const char* inputPtr = reinterpret_cast<const char*>(input);
    m_rawInput.append(inputPtr, frameCount * bytesPerFrame);
  }

  // Drain queued output frames, zero-fill if queue is empty
  if (output && m_config.playback.channels > 0 && m_config.playback.format != ma_format_unknown) {
    QMutexLocker locker(&m_outputBufferLock);

    // clang-format off
    char *out = reinterpret_cast<char *>(output);
    const ma_uint32 outBytesPerFrame = ma_get_bytes_per_sample(m_config.playback.format) * m_config.playback.channels;
    // clang-format on

    // Write one queued frame per output slot
    for (ma_uint32 i = 0; i < frameCount; ++i) {
      if (!m_outputQueue.isEmpty()) {
        // clang-format off
        const QVector<quint8> &frame = m_outputQueue.front();
        const int bytesToCopy = qMin(outBytesPerFrame, static_cast<ma_uint32>(frame.size()));
        std::memcpy(out, frame.constData(), bytesToCopy);
        out += outBytesPerFrame;
        m_outputQueue.pop_front();
        // clang-format on
      }

      else {
        std::memset(out, 0, outBytesPerFrame);
        out += outBytesPerFrame;
      }
    }
  }
}

/**
 * @brief Static callback function for MiniAudio device I/O.
 */
void IO::Drivers::Audio::callback(ma_device* device,
                                  void* output,
                                  const void* input,
                                  ma_uint32 frameCount)
{
  if (!device || !device->pUserData)
    return;

  auto* self = static_cast<IO::Drivers::Audio*>(device->pUserData);
  self->handleCallback(output, input, frameCount);
}

//--------------------------------------------------------------------------------------------------
// Driver property model
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the Audio configuration as a flat list of editable properties.
 */
QList<IO::DriverProperty> IO::Drivers::Audio::driverProperties() const
{
  QList<IO::DriverProperty> props;

  IO::DriverProperty inDev;
  inDev.key     = QStringLiteral("inputDevice");
  inDev.label   = tr("Input Device");
  inDev.type    = IO::DriverProperty::ComboBox;
  inDev.value   = m_selectedInputDevice;
  inDev.options = inputDeviceList();
  props.append(inDev);

  IO::DriverProperty rate;
  rate.key     = QStringLiteral("sampleRate");
  rate.label   = tr("Sample Rate");
  rate.type    = IO::DriverProperty::ComboBox;
  rate.value   = m_selectedSampleRate;
  rate.options = sampleRates();
  props.append(rate);

  IO::DriverProperty fmt;
  fmt.key     = QStringLiteral("inputFormat");
  fmt.label   = tr("Sample Format");
  fmt.type    = IO::DriverProperty::ComboBox;
  fmt.value   = m_selectedInputSampleFormat;
  fmt.options = inputSampleFormats();
  props.append(fmt);

  IO::DriverProperty ch;
  ch.key     = QStringLiteral("inputChannels");
  ch.label   = tr("Channels");
  ch.type    = IO::DriverProperty::ComboBox;
  ch.value   = m_selectedInputChannelConfiguration;
  ch.options = inputChannelConfigurations();
  props.append(ch);

  return props;
}

/**
 * @brief Applies a single Audio configuration change by key.
 */
void IO::Drivers::Audio::setDriverProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("inputDevice"))
    setSelectedInputDevice(value.toInt());

  else if (key == QLatin1String("sampleRate"))
    setSelectedSampleRate(value.toInt());

  else if (key == QLatin1String("inputFormat"))
    setSelectedInputSampleFormat(value.toInt());

  else if (key == QLatin1String("inputChannels"))
    setSelectedInputChannelConfiguration(value.toInt());
}

/**
 * @brief Returns a JSON identifier for the currently selected audio input device.
 */
QJsonObject IO::Drivers::Audio::deviceIdentifier() const
{
  QJsonObject id;

  if (m_selectedInputDevice < 0 || m_selectedInputDevice >= m_inputDevices.size())
    return id;

  // Device name
  id.insert(QStringLiteral("inputDeviceName"),
            QString::fromUtf8(m_inputDevices[m_selectedInputDevice].name));

  // Actual sample rate value
  const auto rates = sampleRates();
  if (m_selectedSampleRate >= 0 && m_selectedSampleRate < rates.size())
    id.insert(QStringLiteral("sampleRateValue"), rates.at(m_selectedSampleRate).toInt());

  // Format name string
  const auto fmts = inputSampleFormats();
  if (m_selectedInputSampleFormat >= 0 && m_selectedInputSampleFormat < fmts.size())
    id.insert(QStringLiteral("formatName"), fmts.at(m_selectedInputSampleFormat));

  // Channel count
  if (validateInput()) {
    const auto& caps = m_inputCapabilities[m_selectedInputDevice];
    if (m_selectedInputChannelConfiguration >= 0
        && m_selectedInputChannelConfiguration < caps.supportedChannelCounts.size())
      id.insert(QStringLiteral("channelCount"),
                caps.supportedChannelCounts.at(m_selectedInputChannelConfiguration));
  }

  return id;
}

/**
 * @brief Selects the input device and sub-settings matching a saved identifier.
 */
bool IO::Drivers::Audio::selectByIdentifier(const QJsonObject& id)
{
  if (id.isEmpty())
    return false;

  // Match device by name
  const auto saved_name = id.value(QStringLiteral("inputDeviceName")).toString();
  if (saved_name.isEmpty())
    return false;

  int device_idx = -1;
  for (int i = 0; i < m_inputDevices.size(); ++i) {
    if (QString::fromUtf8(m_inputDevices[i].name) == saved_name) {
      device_idx = i;
      break;
    }
  }

  if (device_idx < 0)
    return false;

  // Select the device (resets sub-settings to defaults)
  setSelectedInputDevice(device_idx);

  // Restore sample rate by value
  const int saved_rate = id.value(QStringLiteral("sampleRateValue")).toInt();
  if (saved_rate > 0) {
    const auto rates = sampleRates();
    for (int i = 0; i < rates.size(); ++i) {
      if (rates.at(i).toInt() == saved_rate) {
        setSelectedSampleRate(i);
        break;
      }
    }
  }

  // Restore format by name
  const auto saved_fmt = id.value(QStringLiteral("formatName")).toString();
  if (!saved_fmt.isEmpty()) {
    const auto fmts = inputSampleFormats();
    for (int i = 0; i < fmts.size(); ++i) {
      if (fmts.at(i) == saved_fmt) {
        setSelectedInputSampleFormat(i);
        break;
      }
    }
  }

  // Restore channel config by count
  const int saved_ch = id.value(QStringLiteral("channelCount")).toInt();
  if (saved_ch > 0 && validateInput()) {
    const auto& caps = m_inputCapabilities[m_selectedInputDevice];
    for (int i = 0; i < caps.supportedChannelCounts.size(); ++i) {
      if (caps.supportedChannelCounts.at(i) == saved_ch) {
        setSelectedInputChannelConfiguration(i);
        break;
      }
    }
  }

  return true;
}
