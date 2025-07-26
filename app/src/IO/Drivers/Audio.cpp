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

#include "IO/Manager.h"
#include "Misc/Translator.h"
#include "Misc/TimerEvents.h"

#include <QtEndian>

//------------------------------------------------------------------------------
// Utility functions
//------------------------------------------------------------------------------

/**
 * @brief Compares two device lists to determine if they differ.
 *
 * This function checks whether two lists of `ma_device_info` entries are
 * different by comparing their sizes, device IDs, and device names.
 *
 * @param a First list of devices to compare.
 * @param b Second list of devices to compare.
 * @return true if the lists differ (in size, ID, or name); false otherwise.
 */
static bool deviceListsDiffer(const QVector<ma_device_info> &a,
                              const QVector<ma_device_info> &b)
{
  // Size of lists is different -> lists are different
  if (a.size() != b.size())
    return true;

  // Compare each item of the list to see if there is any difference
  for (int i = 0; i < a.size(); ++i)
  {
    if (memcmp(&a[i].id, &b[i].id, sizeof(ma_device_id)) != 0)
      return true;

    if (strcmp(a[i].name, b[i].name) != 0)
      return true;
  }

  // Size is the same and IDs/names are the same -> lists are equal
  return false;
}

/**
 * @brief Extracts audio device capabilities using MiniAudio's backend context.
 *
 * This function queries the backend via `ma_context_get_device_info()` to
 * retrieve a fully populated `ma_device_info` structure for the specified
 * device. It parses the `nativeDataFormats` array to extract supported sample
 * formats, sample rates, and channel counts. Wildcard entries (e.g., 0 sample
 * rate or channel count) are expanded using common defaults.
 *
 * If the backend fails to provide detailed format information, this function
 * falls back to a conservative set of standard sample formats, sample rates,
 * and channels.
 *
 * @param context The MiniAudio context used to query the device.
 * @param info    The basic `ma_device_info` from enumeration.
 * @param type    The device type (capture, playback or duplex)
 *
 * @return A populated `AudioDeviceInfo` structure listing supported formats,
 *         sample rates, and channel counts.
 */
static IO::Drivers::Audio::AudioDeviceInfo
extractCapabilities(ma_context *context, const ma_device_info &info,
                    ma_device_type type)
{
  // Set default number of channels
  const QSet<int> defaultChannels = {1, 2};

  // Set default sample rates
  const QSet<int> defaultSampleRates
      = {8000,  11025, 16000,  22050,  44100,  48000,
         88200, 96000, 176400, 192000, 352800, 384000};

  // Set default formats
  const QSet<ma_format> defaultFormats = {
      ma_format_u8, ma_format_s16, ma_format_s24, ma_format_s32, ma_format_f32};

  // Extract device info
  ma_device_info fullInfo = {};
  auto r = ma_context_get_device_info(context, type, &info.id, &fullInfo);
  if (r != MA_SUCCESS)
    fullInfo = info;

  // Extract native supported formats
  QSet<int> sampleRates;
  QSet<ma_format> formats;
  QSet<int> channelCounts = {1};
  for (ma_uint32 i = 0; i < fullInfo.nativeDataFormatCount; ++i)
  {
    const auto &f = fullInfo.nativeDataFormats[i];

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

  // Fallback values
  if (formats.isEmpty())
    formats = defaultFormats;
  if (sampleRates.isEmpty())
    sampleRates = defaultSampleRates;
  if (defaultChannels.isEmpty())
    channelCounts = defaultChannels;

  // Generate device capabilities structure
  IO::Drivers::Audio::AudioDeviceInfo caps;
  caps.supportedFormats = formats.values();
  caps.supportedSampleRates = sampleRates.values();
  caps.supportedChannelCounts = channelCounts.values();

  // Final UI polish
  std::sort(caps.supportedFormats.begin(), caps.supportedFormats.end());
  std::sort(caps.supportedSampleRates.begin(), caps.supportedSampleRates.end());
  std::sort(caps.supportedChannelCounts.begin(),
            caps.supportedChannelCounts.end());

  // Return result
  return caps;
}

/**
 * @brief Checks for changes in the audio device list and updates it if
 * necessary.
 *
 * This function compares the current device list with a newly detected list.
 * If no device is open, it updates the list and associated capabilities
 * immediately. If a device is open, it checks whether the currently selected
 * device is still available. If not, it triggers a disconnection and updates
 * internal state accordingly.
 *
 * @param currentList The current list of available devices (mutable).
 * @param newList The newly detected list of devices.
 * @param selectedIndex The index of the currently selected/active device.
 * @param isOpen Whether a device is currently open or in use.
 * @param settingsChanged Callback invoked if the settings/UI need to be
 *                        refreshed.
 * @param configurationChanged Callback invoked if the internal configuration
 *                             changed.
 * @param capabilities Mutable list of parsed capabilities for each device in
 *                     the list.
 *
 * @return @c true if the device list or configuration changed, @c false
 * otherwise.
 */
static bool checkAndUpdateDeviceList(
    ma_context *context, ma_device_type type,
    QVector<ma_device_info> &currentList,
    const QVector<ma_device_info> &newList, int selectedIndex, bool isOpen,
    const std::function<void()> &settingsChanged,
    const std::function<void()> &configurationChanged,
    QVector<IO::Drivers::Audio::AudioDeviceInfo> &capabilities)
{
  // Device is not open, we are free to update the lists
  if (!isOpen)
  {
    if (deviceListsDiffer(newList, currentList))
    {
      currentList = newList;
      capabilities.clear();
      for (const auto &info : std::as_const(currentList))
        capabilities.append(extractCapabilities(context, info, type));

      settingsChanged();
      return true;
    }

    return false;
  }

  // A device is connected, check if it's still available
  bool stillConnected = false;
  if (selectedIndex >= 0 && selectedIndex < currentList.size())
  {
    const ma_device_id &currentId = currentList[selectedIndex].id;
    for (const auto &device : newList)
    {
      if (memcmp(&device.id, &currentId, sizeof(ma_device_id)) == 0)
      {
        stillConnected = true;
        break;
      }
    }
  }

  // Device was disconnected, update IO::Manager & close the connection
  if (!stillConnected)
  {
    IO::Manager::instance().disconnectDevice();
    currentList = newList;
    capabilities.clear();
    for (const auto &info : std::as_const(currentList))
      capabilities.append(extractCapabilities(context, info, type));

    settingsChanged();
    configurationChanged();
    return true;
  }

  // Device is still connected, nothing changed
  return false;
}

//------------------------------------------------------------------------------
// Constructor, destructor & singleton access functions
//------------------------------------------------------------------------------

/**
 * @brief Constructs and initializes the audio driver interface.
 *
 * This constructor sets up the MiniAudio backend depending on the host OS
 * (WASAPI on Windows, CoreAudio on macOS, ALSA on Linux). It initializes the
 * MiniAudio context and duplex device configuration, builds internal mappings
 * for UI-friendly labels to native audio parameters, and syncs audio settings
 * to match the configured selections.
 *
 * It also sets up recurring tasks such as:
 * - Refreshing the audio device list every second.
 * - Updating internal translation-sensitive lists when the application
 *   language changes.
 *
 * If MiniAudio fails to initialize, the system logs a warning and audio
 * functionality will be disabled until fixed.
 */
IO::Drivers::Audio::Audio()
  : m_isOpen(false)
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
  m_init = ma_context_init(backend, 1, nullptr, &m_context) == MA_SUCCESS;
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
  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::timeout1Hz, this,
          &Audio::refreshAudioDevices);

  // Update lists when language changes
  connect(&Misc::Translator::instance(), &Misc::Translator::languageChanged,
          this, &IO::Drivers::Audio::generateLists);
}

/**
 * @brief Destructor for the Audio driver.
 *
 * Shuts down any active audio device and deinitializes the MiniAudio context
 * if it was successfully initialized.
 */
IO::Drivers::Audio::~Audio()
{
  closeDevice();
  if (m_init)
    ma_context_uninit(&m_context);
}

/**
 * @brief Returns the singleton instance of the Audio driver.
 *
 * This ensures a single shared instance of the audio system throughout
 * the application lifetime.
 *
 * @return Reference to the singleton Audio object.
 */
IO::Drivers::Audio &IO::Drivers::Audio::instance()
{
  static Audio instance;
  return instance;
}

//------------------------------------------------------------------------------
// HAL driver function implementations
//------------------------------------------------------------------------------

/**
 * @brief Closes the audio device and releases all associated resources.
 *
 * This function:
 * - Uninitializes the MiniAudio device
 * - Stops and deletes the worker timer used for input processing
 * - Gracefully shuts down the input worker thread
 * - Clears both the input buffer and output queue (thread-safe)
 * - Updates the internal state to reflect that the device is closed
 *
 * It ensures proper teardown of all audio-related resources to avoid memory
 * leaks or thread contention issues.
 */
void IO::Drivers::Audio::closeDevice()
{
  // Device is not open, nothing to do
  if (!m_isOpen)
    return;

  // Stop and uninit MiniAudio device
  ma_device_uninit(&m_device);

  // Stop and delete timer in its own thread
  if (m_inputWorkerTimer)
  {
    QMetaObject::invokeMethod(m_inputWorkerTimer, "stop", Qt::QueuedConnection);
    QMetaObject::invokeMethod(m_inputWorkerTimer, "deleteLater",
                              Qt::QueuedConnection);
    m_inputWorkerTimer = nullptr;
  }

  // Terminate input thread cleanly
  if (m_inputWorkerThread.isRunning())
  {
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

  // Set open flag to false
  m_isOpen = false;
}

/**
 * @brief Closes the audio subsystem.
 *
 * Delegates to closeDevice() to clean up device and associated resources.
 */
void IO::Drivers::Audio::close()
{
  closeDevice();
}

/**
 * @brief Checks whether the audio device is currently open.
 * @return True if the device is open, false otherwise.
 */
bool IO::Drivers::Audio::isOpen() const
{
  return m_isOpen;
}

/**
 * @brief Determines if the current audio input configuration is valid and
 *        readable.
 *
 * @return True if input is configured and active, false otherwise.
 */
bool IO::Drivers::Audio::isReadable() const
{
  if (!m_isOpen)
    return false;

  return m_selectedInputDevice >= 0
         && m_selectedInputDevice < m_inputDevices.size()
         && m_config.capture.channels > 0;
}

/**
 * @brief Determines if the current audio output configuration is valid and
 *        writable.
 *
 * @return True if output is configured and active, false otherwise.
 */
bool IO::Drivers::Audio::isWritable() const
{
  if (!m_isOpen)
    return false;

  return m_selectedOutputDevice >= 0
         && m_selectedOutputDevice < m_outputDevices.size()
         && m_config.playback.channels > 0;
}

/**
 * @brief Validates the currently selected input device configuration.
 * @return True if the selected parameters are within the supported capability
 *              ranges.
 */
bool IO::Drivers::Audio::configurationOk() const
{
  if (!validateInput())
    return false;

  bool ok = true;
  const auto &c = m_inputCapabilities[m_selectedInputDevice];
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
 *
 * @param data A comma-separated list of channel values (1 per channel).
 * @return Number of bytes written, or 0 if invalid.
 */
quint64 IO::Drivers::Audio::write(const QByteArray &data)
{
  // Check that we can actually write something
  if (!m_isOpen || m_config.playback.channels <= 0)
  {
    qWarning() << "Output device not available or misconfigured.";
    return 0;
  }

  // Validate that the user sent a valid CSV-like frame
  const int channels = m_config.playback.channels;
  const ma_format format = m_config.playback.format;
  const QList<QByteArray> parts = data.trimmed().split(',');
  if (parts.size() != channels)
  {
    qWarning() << "Channel mismatch: expected" << channels << "but got"
               << parts.size() << ":" << data;
    return 0;
  }

  // Initialize a vector with outputs for each channel
  QVector<quint8> frame;
  for (int i = 0; i < channels; ++i)
  {
    bool ok = false;

    // Convert to uint8_t
    if (format == ma_format_u8)
    {
      int value = parts[i].toInt(&ok);
      if (!ok)
      {
        qWarning() << "Invalid Unigned 8-bit number:" << parts[i];
        return 0;
      }

      frame.append(static_cast<quint8>(qBound(0, value, 255)));
      break;
    }

    // Convert to int16_t
    else if (format == ma_format_s16)
    {
      int value = parts[i].toInt(&ok);
      if (!ok)
      {
        qWarning() << "Invalid Signed 16-bit number:" << parts[i];
        return 0;
      }

      const qint16 sample = static_cast<qint16>(qBound(-32768, value, 32767));
      const quint8 *bytes = reinterpret_cast<const quint8 *>(&sample);
      frame.append(bytes[0]);
      frame.append(bytes[1]);
      break;
    }

    // Convert to 24-bit signed integer (S24LE, packed 3 bytes)
    else if (format == ma_format_s24)
    {
      int value = parts[i].toInt(&ok);
      if (!ok)
      {
        qWarning() << "Invalid Signed 24-bit number:" << parts[i];
        return 0;
      }

      value = qBound(-8388608, value, 8388607);
      frame.append(static_cast<quint8>(value & 0xFF));
      frame.append(static_cast<quint8>((value >> 8) & 0xFF));
      frame.append(static_cast<quint8>((value >> 16) & 0xFF));
      break;
    }

    // Convert to 32-bit signed integer
    else if (format == ma_format_s32)
    {
      qint32 value = parts[i].toInt(&ok);
      if (!ok)
      {
        qWarning() << "Invalid Signed 32-bit number:" << parts[i];
        return 0;
      }

      value = qBound(-2147483647, value, 2147483647);
      const quint8 *bytes = reinterpret_cast<const quint8 *>(&value);
      frame.append(bytes[0]);
      frame.append(bytes[1]);
      frame.append(bytes[2]);
      frame.append(bytes[3]);
      break;
    }

    // Convert to 32-bit float
    else if (format == ma_format_f32)
    {
      float value = parts[i].toFloat(&ok);
      if (!ok)
      {
        qWarning() << "Invalid 32-bit Float number:" << parts[i];
        return 0;
      }

      value = qBound(-1.0f, value, 1.0f);
      const quint8 *bytes = reinterpret_cast<const quint8 *>(&value);
      for (size_t b = 0; b < sizeof(float); ++b)
        frame.append(bytes[b]);

      break;
    }

    // Unsupported format
    else
    {
      qWarning() << "Unsupported format:" << static_cast<int>(format);
      return 0;
    }
  }

  // Write data to output queue
  QMutexLocker locker(&m_outputBufferLock);
  m_outputQueue.push_back(std::move(frame));

  // Return the number of bytes that have been written
  return data.size();
}

/**
 * @brief Opens the audio I/O device with the current configuration.
 *
 * Initializes and starts a full-duplex MiniAudio device using selected input
 * and output parameters. If the mode includes `ReadOnly` or `WriteOnly`,
 * the capture/playback sections of the device are configured accordingly.
 *
 * This function also starts a dedicated thread and timer to process audio
 * input every 10ms (100 Hz).
 *
 * @param mode QIODevice::OpenMode flags indicating read/write access.
 * @return true if the device was opened successfully, false otherwise.
 */
bool IO::Drivers::Audio::open(const QIODevice::OpenMode mode)
{
  if (!m_init || m_isOpen || !configurationOk())
    return false;

  // Initialize duplex config with callback function and set sample rate
  m_config = ma_device_config_init(ma_device_type_duplex);

  // clang-format off
  m_config.pUserData = this;
  m_config.dataCallback = &Audio::callback;
  m_config.sampleRate = m_inputCapabilities[m_selectedInputDevice].supportedSampleRates[m_selectedSampleRate];
  // clang-format on

  // Set common flags
  m_config.noClip = MA_FALSE;
  m_config.noDisableDenormals = MA_FALSE;
  m_config.noFixedSizedCallback = MA_TRUE;
  m_config.noPreSilencedOutputBuffer = MA_FALSE;

  // macOS configuration
#ifdef Q_OS_MAC
  m_config.coreaudio.allowNominalSampleRateChange = MA_TRUE;
#endif

  // Windows configuration
#ifdef Q_OS_WIN
  m_config.wasapi.noAutoConvertSRC = MA_FALSE;
  m_config.wasapi.noDefaultQualitySRC = MA_FALSE;
  m_config.wasapi.noAutoStreamRouting = MA_FALSE;
  m_config.wasapi.noHardwareOffloading = MA_TRUE;
#endif

  // Linux configuration
#ifdef Q_OS_LINUX
  m_config.alsa.noMMap = MA_FALSE;
  m_config.alsa.noAutoFormat = MA_FALSE;
  m_config.alsa.noAutoChannels = MA_FALSE;
  m_config.alsa.noAutoResample = MA_FALSE;
#endif

  // Initialize capture device
  if (mode & QIODevice::ReadOnly)
  {
    // clang-format off
    m_config.capture.pDeviceID = &m_inputDevices[m_selectedInputDevice].id;
    m_config.capture.format = m_inputCapabilities[m_selectedInputDevice].supportedFormats[m_selectedInputSampleFormat];
    m_config.capture.channels = m_inputCapabilities[m_selectedInputDevice].supportedChannelCounts[m_selectedInputChannelConfiguration];
    // clang-format on
  }

  // Edge-case for when user wants to open without read permissions
  else
  {
    m_config.capture.format = ma_format_unknown;
    m_config.capture.channels = 0;
  }

  // Initialize output device
  if (mode & QIODevice::WriteOnly)
  {
    // clang-format off
    m_config.playback.pDeviceID = &m_outputDevices[m_selectedOutputDevice].id;
    m_config.playback.format = m_outputCapabilities[m_selectedOutputDevice].supportedFormats[m_selectedOutputSampleFormat];
    m_config.playback.channels = m_outputCapabilities[m_selectedOutputDevice].supportedChannelCounts[m_selectedOutputChannelConfiguration];
    // clang-format on
  }

  // Edge-case for when user wants to open without write permissions
  else
  {
    m_config.playback.format = ma_format_unknown;
    m_config.playback.channels = 0;
  }

  // Try to initialize the duplex device
  std::memset(&m_device, 0, sizeof(m_device));
  if (ma_device_init(&m_context, &m_config, &m_device) != MA_SUCCESS)
  {
    qWarning() << "Failed to initialize miniaudio device.";
    return false;
  }

  // Failed to initialize duplex device
  if (ma_device_start(&m_device) != MA_SUCCESS)
  {
    qWarning() << "Failed to start miniaudio device.";
    ma_device_uninit(&m_device);
    return false;
  }

  // Configure audio processing thread and timer
  if (!m_inputWorkerTimer)
  {
    m_inputWorkerTimer = new QTimer();
    m_inputWorkerTimer->setInterval(10);
    m_inputWorkerTimer->setTimerType(Qt::PreciseTimer);
    m_inputWorkerTimer->moveToThread(&m_inputWorkerThread);
    connect(&m_inputWorkerThread, &QThread::finished, m_inputWorkerTimer,
            &QObject::deleteLater);
    connect(m_inputWorkerTimer, &QTimer::timeout, this,
            &IO::Drivers::Audio::processInputBuffer);
  }

  // Start the worker thread
  if (!m_inputWorkerThread.isRunning())
  {
    m_inputWorkerThread.start();
    m_inputWorkerThread.setPriority(QThread::HighestPriority);
  }

  // Start the read timer @ 100 Hz
  QMetaObject::invokeMethod(m_inputWorkerTimer, "start", Qt::QueuedConnection);

  // Set open flag to true
  m_isOpen = true;
  return true;
}

//------------------------------------------------------------------------------
// Sample rate selection
//------------------------------------------------------------------------------

/**
 * @brief Returns the currently selected sample rate index.
 *
 * This function retrieves the index of the sample rate currently selected
 * for the input audio device. This is used internally to look up the actual
 * numeric sample rate value from the capabilities list.
 *
 * @return The index of the selected sample rate.
 */
int IO::Drivers::Audio::selectedSampleRate() const
{
  return m_selectedSampleRate;
}

/**
 * @brief Returns a list of supported input sample rates as strings.
 *
 * This function queries the capabilities of the currently selected input
 * device and returns a list of supported sample rates in human-readable
 * string format. If the input device is not valid, an empty list is returned.
 *
 * @return A QStringList containing the supported sample rates.
 */
QStringList IO::Drivers::Audio::sampleRates() const
{
  QStringList list;
  if (!validateInput())
    return list;

  const auto &device = m_inputCapabilities[m_selectedInputDevice];
  for (int rate : device.supportedSampleRates)
    list.append(QString::number(rate));

  return list;
}

//------------------------------------------------------------------------------
// Input device parameters
//------------------------------------------------------------------------------

/**
 * @brief Returns the index of the currently selected input device.
 *
 * This function provides access to the internal index representing the
 * selected input audio device from the available list.
 *
 * @return Index of the selected input device.
 */
int IO::Drivers::Audio::selectedInputDevice() const
{
  return m_selectedInputDevice;
}

/**
 * @brief Returns the index of the selected input sample format.
 *
 * The sample format refers to the bit-depth/representation of audio samples
 * (e.g., 16-bit signed, 32-bit float). This index maps into the list of
 * supported formats for the selected input device.
 *
 * @return Index of the selected sample format for the input device.
 */
int IO::Drivers::Audio::selectedInputSampleFormat() const
{
  return m_selectedInputSampleFormat;
}

/**
 * @brief Returns the index of the selected input channel configuration.
 *
 * This refers to how many channels the selected input device is using
 * (e.g., mono, stereo). The returned index corresponds to the supported
 * channel counts list.
 *
 * @return Index of the selected channel configuration for the input device.
 */
int IO::Drivers::Audio::selectedInputChannelConfiguration() const
{
  return m_selectedInputChannelConfiguration;
}

//------------------------------------------------------------------------------
// Output device parameters
//------------------------------------------------------------------------------

/**
 * @brief Returns the index of the currently selected output device.
 *
 * This function retrieves the internal index representing the selected
 * output audio device from the list of available devices.
 *
 * @return Index of the selected output device.
 */
int IO::Drivers::Audio::selectedOutputDevice() const
{
  return m_selectedOutputDevice;
}

/**
 * @brief Returns the index of the selected output sample format.
 *
 * The sample format determines the audio data representation used by the
 * output device (e.g., unsigned 8-bit, signed 16-bit, float 32-bit).
 * The index corresponds to the supported formats for the output device.
 *
 * @return Index of the selected sample format for the output device.
 */
int IO::Drivers::Audio::selectedOutputSampleFormat() const
{
  return m_selectedOutputSampleFormat;
}

/**
 * @brief Returns the index of the selected output channel configuration.
 *
 * This value determines how many channels (e.g., mono, stereo) are used
 * by the output device. The index maps into the list of supported channel
 * configurations for the device.
 *
 * @return Index of the selected output channel configuration.
 */
int IO::Drivers::Audio::selectedOutputChannelConfiguration() const
{
  return m_selectedOutputChannelConfiguration;
}

//------------------------------------------------------------------------------
// Input device parameter models
//------------------------------------------------------------------------------

/**
 * @brief Returns a list of available input audio devices.
 *
 * Iterates over the internal list of detected input devices and returns
 * their names as a QStringList.
 *
 * @return List of input device names in UTF-8 format.
 */
QStringList IO::Drivers::Audio::inputDeviceList() const
{
  QStringList list;

  for (const auto &device : m_inputDevices)
    list.append(QString::fromUtf8(device.name));

  return list;
}

/**
 * @brief Returns the list of supported sample formats for the selected input
 * device.
 *
 * Formats such as 8-bit unsigned, 16-bit signed, or 32-bit float are included
 * depending on what the selected device supports. Invalid or unknown formats
 * are filtered out.
 *
 * @return Human-readable list of supported sample formats.
 */
QStringList IO::Drivers::Audio::inputSampleFormats() const
{
  QStringList list;
  if (!validateInput())
    return list;

  const auto &device = m_inputCapabilities[m_selectedInputDevice];
  for (ma_format format : device.supportedFormats)
  {
    if (m_sampleFormats.contains(format))
      list.append(m_sampleFormats.value(format));
  }

  return list;
}

/**
 * @brief Returns the list of supported input channel configurations.
 *
 * Common channel counts like mono or stereo are translated into user-friendly
 * labels. Unknown configurations are labeled as "<N> channels".
 *
 * @return List of supported input channel configurations.
 */
QStringList IO::Drivers::Audio::inputChannelConfigurations() const
{
  QStringList list;
  if (!validateInput())
    return list;

  const auto &device = m_inputCapabilities[m_selectedInputDevice];
  for (int channels : device.supportedChannelCounts)
  {
    if (m_knownConfigs.contains(static_cast<ma_channel>(channels)))
      list.append(m_knownConfigs.value(static_cast<ma_channel>(channels)));
    else
      list.append(QString::number(channels) + " " + tr("channels"));
  }

  return list;
}

//------------------------------------------------------------------------------
// Output device parameter models
//------------------------------------------------------------------------------

/**
 * @brief Returns a list of available output audio devices.
 *
 * Iterates over the internal list of detected output devices and returns
 * their names in UTF-8 encoding.
 *
 * @return List of output device names.
 */
QStringList IO::Drivers::Audio::outputDeviceList() const
{
  QStringList list;

  for (const auto &device : m_outputDevices)
    list.append(QString::fromUtf8(device.name));

  return list;
}

/**
 * @brief Returns a list of supported sample formats for the selected output
 * device.
 *
 * Converts device-specific format enums into user-friendly format names, like
 * "8-bit Unsigned", "16-bit Signed", or "32-bit Float".
 *
 * @return List of supported output sample formats.
 */
QStringList IO::Drivers::Audio::outputSampleFormats() const
{
  QStringList list;
  if (!validateOutput())
    return list;

  const auto &device = m_outputCapabilities[m_selectedOutputDevice];
  for (ma_format format : device.supportedFormats)
  {
    if (m_sampleFormats.contains(format))
      list.append(m_sampleFormats.value(format));
  }

  return list;
}

/**
 * @brief Returns a list of supported output channel configurations.
 *
 * Converts known channel counts into human-readable descriptions (e.g., Mono,
 * Stereo). For unknown configurations, displays "<N> channels".
 *
 * @return List of supported output channel configurations.
 */
QStringList IO::Drivers::Audio::outputChannelConfigurations() const
{
  QStringList list;
  if (!validateOutput())
    return list;

  const auto &device = m_outputCapabilities[m_selectedOutputDevice];
  for (int channels : device.supportedChannelCounts)
  {
    if (m_knownConfigs.contains(static_cast<ma_channel>(channels)))
      list.append(m_knownConfigs.value(static_cast<ma_channel>(channels)));
    else
      list.append(QString::number(channels) + tr(" channels"));
  }

  return list;
}

//------------------------------------------------------------------------------
// Sample rate configuration
//------------------------------------------------------------------------------

/**
 * @brief Sets the selected sample rate index for the input device.
 *
 * This function updates the internal selection index used to determine
 * which sample rate to apply when configuring the input device.
 * If the device is currently open, the change is ignored.
 *
 * @param index Index of the desired sample rate in the list.
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

//------------------------------------------------------------------------------
// Input device parameter setters
//------------------------------------------------------------------------------

/**
 * @brief Sets the selected input device by index.
 *
 * If the device is open, the change is ignored. This also resets
 * the input sample rate, format, and channel configuration to defaults.
 *
 * @param index Index of the desired input device in the available list.
 */
void IO::Drivers::Audio::setSelectedInputDevice(int index)
{
  if (isOpen())
    return;

  if (index < 0 || index >= inputDeviceList().size())
    index = 0;

  m_selectedInputDevice = index;
  m_selectedSampleRate = -1;
  m_selectedInputSampleFormat = -1;
  m_selectedInputChannelConfiguration = -1;

  syncInputParameters();
  configureInput();
}

/**
 * @brief Sets the selected sample format index for the input device.
 *
 * Ignores the call if the device is currently open.
 *
 * @param index Index of the desired sample format in the list.
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
 *
 * Ignores the call if the device is currently open.
 *
 * @param index Index of the desired channel configuration in the list.
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

//------------------------------------------------------------------------------
// Output device parameter setters
//------------------------------------------------------------------------------

/**
 * @brief Sets the selected output device by index.
 *
 * If the audio device is currently open, the function does nothing.
 * Resets output format and channel configuration to defaults.
 *
 * @param index Index of the output device in the device list.
 */
void IO::Drivers::Audio::setSelectedOutputDevice(int index)
{
  if (isOpen())
    return;

  if (index < 0 || index >= outputDeviceList().size())
    index = 0;

  m_selectedOutputDevice = index;
  m_selectedOutputSampleFormat = -1;
  m_selectedOutputChannelConfiguration = -1;

  syncOutputParameters();
  configureOutput();
}

/**
 * @brief Sets the selected sample format for the output device.
 *
 * Does nothing if the audio device is currently open.
 *
 * @param index Index of the desired format in the output sample formats list.
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
 *
 * Skips update if device is open or index is invalid.
 *
 * @param index Index in the list of available output channel configurations.
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

//------------------------------------------------------------------------------
// UI helpers
//------------------------------------------------------------------------------

/**
 * @brief Populates maps that associate MiniAudio constants with user-friendly
 *        strings.
 *
 * This includes supported sample formats and known channel configurations,
 * used throughout the UI for display purposes.
 *
 * Emits signals to update input and output settings in the UI.
 */
void IO::Drivers::Audio::generateLists()
{
  m_sampleFormats = {{ma_format_u8, tr("Unsigned 8-bit")},
                     {ma_format_s16, tr("Signed 16-bit")},
                     {ma_format_s24, tr("Signed 24-bit")},
                     {ma_format_s32, tr("Signed 32-bit")},
                     {ma_format_f32, tr("Float 32-bit")}};
  m_knownConfigs = {{1, tr("Mono")}, {2, tr("Stereo")}, {3, "3.0"}, {4, "4.0"},
                    {5, "5.0"},      {6, "5.1"},        {7, "6.1"}, {8, "7.1"}};

  Q_EMIT inputSettingsChanged();
  Q_EMIT outputSettingsChanged();
}

//------------------------------------------------------------------------------
// Device configuration helpers
//------------------------------------------------------------------------------

/**
 * @brief Applies the selected input device configuration.
 *
 * This method checks the validity of the selected input device and its
 * capabilities.
 *
 * It then clamps the selected configuration indices to valid ranges and
 * updates the MiniAudio device config accordingly.
 *
 * Emits the @c configurationChanged() signal after applying the new settings.
 *
 * If capabilities are missing, the method will issue a warning and abort.
 */
void IO::Drivers::Audio::configureInput()
{
  // Check that input is valid
  if (!validateInput())
    return;

  // Obtain current capabilities
  const AudioDeviceInfo &caps = m_inputCapabilities[m_selectedInputDevice];
  if (caps.supportedSampleRates.isEmpty() || caps.supportedFormats.isEmpty()
      || caps.supportedChannelCounts.isEmpty())
  {
    qWarning() << "Input capabilities for selected device are not populated";
    return;
  }

  // Clamp user-set indices
  // clang-format off
  m_selectedSampleRate = qBound(0, m_selectedSampleRate, caps.supportedSampleRates.size() - 1);
  m_selectedInputSampleFormat = qBound(0, m_selectedInputSampleFormat, caps.supportedFormats.size() - 1);
  m_selectedInputChannelConfiguration = qBound(0, m_selectedInputChannelConfiguration, caps.supportedChannelCounts.size() - 1);
  // clang-format on

  // Obtain current configuration
  // clang-format off
  const int sampleRate = caps.supportedSampleRates[m_selectedSampleRate];
  const ma_format format = caps.supportedFormats[m_selectedInputSampleFormat];
  const int channels = caps.supportedChannelCounts[m_selectedInputChannelConfiguration];
  // clang-format on

  // Set input configuration
  m_config.sampleRate = sampleRate;
  m_config.capture.format = format;
  m_config.capture.channels = channels;

  // Sync parameters & update UI
  syncInputParameters();
  Q_EMIT configurationChanged();
}

/**
 * @brief Applies the selected output device configuration.
 *
 * This method validates the selected output device and ensures its
 * capabilities are populated.
 *
 * It then clamps the selected sample format and channel configuration to
 * valid ranges, and applies them to the MiniAudio device configuration.
 *
 * Emits the @c configurationChanged() signal after updating the settings.
 *
 * If the capabilities for the selected output device are not available,
 * the function aborts with a warning.
 */
void IO::Drivers::Audio::configureOutput()
{
  // Check that output device is valid
  if (!validateOutput())
    return;

  // Obtain current capabilities
  const AudioDeviceInfo &caps = m_outputCapabilities[m_selectedOutputDevice];
  if (caps.supportedSampleRates.isEmpty() || caps.supportedFormats.isEmpty()
      || caps.supportedChannelCounts.isEmpty())
  {
    qWarning() << "Output capabilities for selected device are not populated";
    return;
  }

  // Clamp user-set indices
  // clang-format off
  m_selectedOutputSampleFormat = qBound(0, m_selectedOutputSampleFormat, caps.supportedFormats.size() - 1);
  m_selectedOutputChannelConfiguration = qBound(0, m_selectedOutputChannelConfiguration, caps.supportedChannelCounts.size() - 1);
  // clang-format on

  // Obtain current configuration
  // clang-format off
  const ma_format format = caps.supportedFormats[m_selectedOutputSampleFormat];
  const int channels = caps.supportedChannelCounts[m_selectedOutputChannelConfiguration];
  // clang-format on

  // Set output configuration
  m_config.playback.format = format;
  m_config.playback.channels = channels;

  // Sync parameters & update UI
  syncOutputParameters();
  Q_EMIT configurationChanged();
}

//------------------------------------------------------------------------------
// Audio input data parsing
//------------------------------------------------------------------------------

/**
 * @brief Converts raw audio input into CSV-formatted text and emits it.
 *
 * This function is periodically called by a high-resolution timer running
 * in a worker thread. It processes the raw audio buffer filled by the
 * MiniAudio callback, converts each frame to a CSV-like string, and emits
 * the result via the `dataReceived` signal.
 *
 * The function performs the following steps:
 * - Locks and swaps out the internal raw audio buffer (`m_rawInput`)
 *   to avoid contention.
 * - Validates audio configuration and buffer integrity.
 * - Parses each frame based on sample format (`u8`, `s16`, `s32`, or `f32`)
 *   and channel count.
 * - Writes parsed values into a reusable `QBuffer`-backed `QTextStream`
 *   (`m_csvStream`), avoiding dynamic allocations.
 * - Flushes the stream to ensure the data is written into `m_csvData`.
 * - Emits only the valid portion of the buffer via `dataReceived()`.
 *
 * @note This function is optimized for performance and minimal allocations.
 *       It assumes exclusive ownership of `m_rawInput` and reuses
 *       `m_csvData` to avoid repeated memory churn.
 */
void IO::Drivers::Audio::processInputBuffer()
{
  // Copy audio data
  QByteArray raw;
  {
    QMutexLocker locker(&m_inputBufferLock);
    if (m_rawInput.isEmpty())
      return;

    raw = std::move(m_rawInput);
    m_rawInput.clear();
  }

  // Device config
  const int channels = m_config.capture.channels;
  const ma_format format = m_config.capture.format;
  const int bytesPerSample = ma_get_bytes_per_sample(format);
  const int frameSize = bytesPerSample * channels;
  if (frameSize <= 0 || channels <= 0 || raw.size() % frameSize != 0)
    return;

  // Calculate the number of audio frames
  const int totalFrames = raw.size() / frameSize;

  // Reset the CSV output buffer to start position
  m_csvBuffer.seek(0);

  // Convert raw audio data to CSV-like format
  const char *ptr = raw.constData();
  for (int i = 0; i < totalFrames; ++i)
  {
    for (int ch = 0; ch < channels; ++ch)
    {
      switch (format)
      {
        case ma_format_u8: {
          const auto sample = static_cast<quint8>(*ptr);
          m_csvStream << static_cast<int>(sample);
          break;
        }
        case ma_format_s16: {
          const qint16 sample = qFromLittleEndian<qint16>(
              reinterpret_cast<const quint8 *>(ptr));
          m_csvStream << sample;
          break;
        }
        case ma_format_s24: {
          const quint8 *b = reinterpret_cast<const quint8 *>(ptr);
          qint32 sample = static_cast<qint32>(b[0])
                          | (static_cast<qint32>(b[1]) << 8)
                          | (static_cast<qint32>(b[2]) << 16);

          if (sample & 0x800000)
            sample |= 0xFF000000;

          m_csvStream << sample;
          break;
        }
        case ma_format_s32: {
          const qint32 sample = qFromLittleEndian<qint32>(
              reinterpret_cast<const quint8 *>(ptr));
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

  // Force any data in the stream to be written in the CSV buffer
  m_csvStream.flush();

  // Report only the valid chunk of CSV data that we wrote
  const auto length = m_csvBuffer.pos();
  Q_EMIT dataReceived(m_csvData.left(length));
}

//------------------------------------------------------------------------------
// Device discovery functions
//------------------------------------------------------------------------------

/**
 * @brief Refreshes the list of available audio input/output devices.
 *
 * Queries the MiniAudio context for the current list of input and output
 * audio devices. If the device list has changed or a previously selected
 * device is no longer present, the internal device lists and capabilities
 * are updated accordingly.
 *
 * Also emits @c inputSettingsChanged(), @c outputSettingsChanged(),
 * and @c configurationChanged() signals if any updates are made.
 *
 * This function is typically called periodically (e.g., every second) to
 * detect hot-plugged or removed audio devices.
 */
void IO::Drivers::Audio::refreshAudioDevices()
{
  // Failed to initialize library, stop
  if (!m_init)
    return;

  // Set initial values
  ma_uint32 inputCount = 0;
  ma_uint32 outputCount = 0;
  ma_device_info *inputDevices = nullptr;
  ma_device_info *outputDevices = nullptr;

  // Obtain list of available devices
  // clang-format off
  auto result = ma_context_get_devices(&m_context,
                                       &outputDevices,
                                       &outputCount,
                                       &inputDevices,
                                       &inputCount);

  if (result != MA_SUCCESS)
    return;
  // clang-format on

  // Convert device pointers to vectors
  // clang-format off
  const QVector<ma_device_info> newInputDevices(inputDevices, inputDevices + inputCount);
  const QVector<ma_device_info> newOutputDevices(outputDevices, outputDevices + outputCount);
  // clang-format on

  // Update list of input devices
  checkAndUpdateDeviceList(
      &m_context, ma_device_type_capture, m_inputDevices, newInputDevices,
      m_selectedInputDevice, m_isOpen,
      [this] { Q_EMIT inputSettingsChanged(); },
      [this] { Q_EMIT configurationChanged(); }, m_inputCapabilities);

  // Update list of output devices
  checkAndUpdateDeviceList(
      &m_context, ma_device_type_playback, m_outputDevices, newOutputDevices,
      m_selectedOutputDevice, m_isOpen,
      [this] { Q_EMIT outputSettingsChanged(); },
      [this] { Q_EMIT configurationChanged(); }, m_outputCapabilities);

  // Update fallback input device index if needed
  if (m_selectedInputDevice < 0 && newInputDevices.count() > 0)
  {
    m_selectedInputDevice = 0;
    Q_EMIT inputSettingsChanged();
  }

  // Update fallback output device index if needed
  if (m_selectedOutputDevice < 0 && newOutputDevices.count() > 0)
  {
    m_selectedOutputDevice = 0;
    Q_EMIT outputSettingsChanged();
  }
}

//------------------------------------------------------------------------------
// Model sync functions
//------------------------------------------------------------------------------

/**
 * @brief Synchronizes internal input parameters with the actual device config.
 *
 * This method updates internal indexes for input sample rate, sample format,
 * and channel configuration based on the current `ma_device_config`.
 *
 * If the current config is not found in the list of supported capabilities,
 * it falls back to 44.1 KHz or the first available option.
 *
 * Emits @c inputSettingsChanged() to update any dependent UI elements.
 */
void IO::Drivers::Audio::syncInputParameters()
{
  // Validate that input device is valid
  if (!validateInput())
    return;

  // Get capabilities
  const auto &caps = m_inputCapabilities[m_selectedInputDevice];

  // Load sample rate index, fallback to 44.1 KHz
  m_selectedSampleRate = caps.supportedSampleRates.indexOf(m_config.sampleRate);
  if (m_selectedSampleRate < 0)
  {
#ifdef Q_OS_WIN
    int fallback = caps.supportedSampleRates.indexOf(22050);
#else
    int fallback = caps.supportedSampleRates.indexOf(44100);
#endif
    m_selectedSampleRate = fallback >= 0 ? fallback : 0;
  }

  // clang-format off

  // Load sample format
  m_selectedInputSampleFormat = caps.supportedFormats.indexOf(m_config.capture.format);
  if (m_selectedInputSampleFormat < 0)
    m_selectedInputSampleFormat = 0;

  // Load channel configuration
  m_selectedInputChannelConfiguration = caps.supportedChannelCounts.indexOf(m_config.capture.channels);
  if (m_selectedInputChannelConfiguration < 0)
    m_selectedInputChannelConfiguration = 0;

  // clang-format on

  // Update user interface
  Q_EMIT inputSettingsChanged();
}

/**
 * @brief Synchronizes internal output parameters with the actual device config.
 *
 * Updates internal indexes for output sample format and channel configuration
 * using the current `ma_device_config`. Falls back to the first available
 * configuration if the currently selected format or channel count is not found.
 *
 * Emits @c outputSettingsChanged() to refresh the UI.
 */
void IO::Drivers::Audio::syncOutputParameters()
{
  // Validate that output device is valid
  if (!validateOutput())
    return;

  // Get capabilities
  const auto &caps = m_outputCapabilities[m_selectedOutputDevice];

  // clang-format off

  // Load sample format
  m_selectedOutputSampleFormat = caps.supportedFormats.indexOf(m_config.playback.format);
  if (m_selectedOutputSampleFormat < 0)
    m_selectedOutputSampleFormat = 0;

  // Load channel configuration
  m_selectedOutputChannelConfiguration = caps.supportedChannelCounts.indexOf(m_config.playback.channels);
  if (m_selectedOutputChannelConfiguration < 0)
    m_selectedOutputChannelConfiguration = 0;

  // clang-format on

  // Update user interface
  Q_EMIT outputSettingsChanged();
}

//------------------------------------------------------------------------------
// Audio callback function
//------------------------------------------------------------------------------

/**
 * @brief Audio callback handler for processing input and output streams.
 *
 * This method is invoked from the MiniAudio real-time thread. It processes
 * audio input by pushing raw bytes into a buffer and handles audio output
 * by streaming queued frames or zero-filling if empty.
 *
 * @param output Pointer to the output buffer (nullable).
 * @param input Pointer to the input buffer (nullable).
 * @param frameCount Number of audio frames to process.
 */
void IO::Drivers::Audio::handleCallback(void *output, const void *input,
                                        ma_uint32 frameCount)
{
  // Obtain capture format & number of channels
  const ma_format format = m_config.capture.format;
  const ma_uint32 channels = m_config.capture.channels;

  // Get number of bytes per sample & number of bytes per frame
  const ma_uint32 bytesPerSample = ma_get_bytes_per_sample(format);
  const ma_uint32 bytesPerFrame = bytesPerSample * channels;

  // Push raw input data to buffer, no formatting here
  if (input && channels > 0 && format != ma_format_unknown)
  {
    QMutexLocker locker(&m_inputBufferLock);
    const char *inputPtr = reinterpret_cast<const char *>(input);
    m_rawInput.append(inputPtr, frameCount * bytesPerFrame);
  }

  // Output → fast write, fallback to zero-fill
  if (output && m_config.playback.channels > 0
      && m_config.playback.format != ma_format_unknown)
  {
    QMutexLocker locker(&m_outputBufferLock);

    // Get number of bytes per frame for output
    // clang-format off
    char *out = reinterpret_cast<char *>(output);
    const ma_uint32 outBytesPerFrame = ma_get_bytes_per_sample(m_config.playback.format) * m_config.playback.channels;
    // clang-format on

    // Write output frames
    for (ma_uint32 i = 0; i < frameCount; ++i)
    {
      if (!m_outputQueue.isEmpty())
      {
        // clang-format off
        const QVector<quint8> &frame = m_outputQueue.front();
        const int bytesToCopy = qMin(outBytesPerFrame, static_cast<ma_uint32>(frame.size()));
        std::memcpy(out, frame.constData(), bytesToCopy);
        out += outBytesPerFrame;
        m_outputQueue.pop_front();
        // clang-format on
      }

      else
      {
        std::memset(out, 0, outBytesPerFrame);
        out += outBytesPerFrame;
      }
    }
  }
}

/**
 * @brief Static callback function for MiniAudio device I/O.
 *
 * This function is called by MiniAudio whenever audio input/output occurs.
 * It delegates the processing to the instance method @ref handleCallback().
 *
 * @param device Pointer to the MiniAudio device structure.
 * @param output Pointer to the output buffer for playback (can be null).
 * @param input Pointer to the input buffer for recording (can be null).
 * @param frameCount Number of audio frames in the input/output buffers.
 */
void IO::Drivers::Audio::callback(ma_device *device, void *output,
                                  const void *input, ma_uint32 frameCount)
{
  // Validate arguments
  if (!device || !device->pUserData)
    return;

  // Get pointer to origin class & call callback function
  auto *self = static_cast<IO::Drivers::Audio *>(device->pUserData);
  self->handleCallback(output, input, frameCount);
}
