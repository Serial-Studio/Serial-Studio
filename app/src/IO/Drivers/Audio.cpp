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

#include "Audio.h"
#include "Misc/TimerEvents.h"

#include <QDebug>
#include <QtEndian>
#include <QAudioSink>
#include <QAudioSource>
#include <QMediaDevices>

//------------------------------------------------------------------------------
// Constructor & singleton access functions
//------------------------------------------------------------------------------

/**
 * @brief Constructs the Audio driver instance.
 *
 * Initializes internal state and configuration maps, refreshes audio input
 * and output devices, sets default audio formats, synchronizes selected
 * parameters with device preferred formats, and applies initial configurations.
 *
 * Also connects a periodic 1 Hz timer to refresh the list of available audio
 * devices dynamically at runtime.
 */
IO::Drivers::Audio::Audio()
  : m_isOpen(false)
  , m_bufferSizeIdx(0)
  , m_selectedInputDevice(0)
  , m_selectedInputSampleRate(0)
  , m_selectedInputSampleFormat(0)
  , m_selectedInputChannelConfiguration(0)
  , m_selectedOutputDevice(0)
  , m_selectedOutputSampleRate(0)
  , m_selectedOutputSampleFormat(0)
  , m_selectedOutputChannelConfiguration(0)
  , m_sink(nullptr)
  , m_source(nullptr)
  , m_inputStream(nullptr)
  , m_outputStream(nullptr)
{
  // Initialize maps to connect user friendly strings to actual values
  m_sampleFormats = {{QAudioFormat::UInt8, tr("Unsigned 8-bit")},
                     {QAudioFormat::Int16, tr("Signed 16-bit")},
                     {QAudioFormat::Int32, tr("Signed 32-bit")},
                     {QAudioFormat::Float, tr("Float 32-bit")}};
  m_knownConfigs = {{QAudioFormat::ChannelConfigMono, tr("Mono")},
                    {QAudioFormat::ChannelConfigStereo, tr("Stereo")},
                    {QAudioFormat::ChannelConfig2Dot1, "2.1"},
                    {QAudioFormat::ChannelConfig3Dot0, "3.0"},
                    {QAudioFormat::ChannelConfig3Dot1, "3.1"},
                    {QAudioFormat::ChannelConfigSurround5Dot0, "5.0"},
                    {QAudioFormat::ChannelConfigSurround5Dot1, "5.1"},
                    {QAudioFormat::ChannelConfigSurround7Dot0, "7.0"},
                    {QAudioFormat::ChannelConfigSurround7Dot1, "7.1"}};

  // Get list of devices
  refreshAudioInputs();
  refreshAudioOutputs();

  // Load default audio formats
  m_inputFormat = inputDevice().preferredFormat();
  m_outputFormat = outputDevice().preferredFormat();

  // Sync UI selection with applied format
  syncInputParameters();
  syncOutputParameters();

  // Configure model
  configureInput();
  configureOutput();

  // Refresh devices every second
  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::timeout1Hz, this,
          &Audio::refreshAudioInputs);
  connect(&Misc::TimerEvents::instance(), &Misc::TimerEvents::timeout1Hz, this,
          &Audio::refreshAudioOutputs);
}

/**
 * @brief Returns the singleton instance of the Audio driver.
 *
 * Used to access the global audio input/output handler. Ensures a single,
 * globally accessible object for managing audio streams and configuration.
 *
 * @return Reference to the Audio singleton.
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
 * @brief Closes all open audio streams and resets internal state.
 *
 * Stops and deletes the active QAudioSource and QAudioSink instances,
 * disconnects signal handlers, and clears internal stream pointers.
 * Sets the open state to false.
 */
void IO::Drivers::Audio::close()
{
  if (!m_isOpen)
    return;

  if (m_source)
  {
    if (m_inputStream)
      disconnect(m_inputStream, &QIODevice::readyRead, this,
                 &Audio::onInputReadyRead);

    m_source->stop();
    m_source->deleteLater();
    m_source = nullptr;
    m_inputStream = nullptr;
  }

  if (m_sink)
  {
    m_sink->stop();
    m_sink->deleteLater();
    m_sink = nullptr;
    m_outputStream = nullptr;
  }

  m_isOpen = false;
}

/**
 * @brief Checks if the audio driver is currently open.
 *
 * A device is considered open if a QAudioSource was successfully started.
 *
 * @return True if the input stream is active, false otherwise.
 */
bool IO::Drivers::Audio::isOpen() const
{
  return m_isOpen && m_inputStream != nullptr;
}

/**
 * @brief Indicates whether input streaming is available and active.
 *
 * Returns true if the input device is valid and an input stream is running.
 *
 * @return True if readable, false otherwise.
 */
bool IO::Drivers::Audio::isReadable() const
{
  if (isOpen())
    return !inputDevice().isNull() && m_inputStream != nullptr;

  return false;
}

/**
 * @brief Indicates whether output streaming is available and active.
 *
 * Returns true if the output device is valid and an output stream is active.
 *
 * @return True if writable, false otherwise.
 */
bool IO::Drivers::Audio::isWritable() const
{
  if (isOpen())
    return !outputDevice().isNull() && m_outputStream != nullptr;

  return false;
}

/**
 * @brief Validates the audio driver configuration.
 *
 * Ensures input/output devices are not null and the selected input format
 * is supported by the input device. Output format support is *not* checked
 * here to allow partial functionality (e.g., input-only use cases).
 *
 * @return True if the configuration is valid, false otherwise.
 */
bool IO::Drivers::Audio::configurationOk() const
{
  bool ok = true;
  ok &= !inputDevice().isNull();
  ok &= !outputDevice().isNull();
  ok &= inputDevice().isFormatSupported(inputFormat());
  return ok;
}

/**
 * @brief Writes raw data to the output audio stream.
 *
 * If the driver is open and an output stream is available, the given
 * QByteArray is written to the stream buffer.
 *
 * @param data Byte buffer to write to the output device.
 * @return Number of bytes written, or 0 if not writable.
 */
quint64 IO::Drivers::Audio::write(const QByteArray &data)
{
  if (!m_isOpen || !m_outputStream)
    return 0;

  return m_outputStream->write(data);
}

/**
 * @brief Opens the audio device for input/output streaming.
 *
 * Initializes the audio input/output streams based on the provided mode.
 * Applies the configured audio format and selected devices. If opening
 * the input stream fails, the method logs the error and returns false.
 * Output stream failures are logged but do not prevent partial operation
 * in input-only mode.
 *
 * @param mode Open mode: QIODevice::ReadOnly, WriteOnly, or ReadWrite.
 * @return True if at least one stream is opened successfully,
 *         false if input fails.
 */
bool IO::Drivers::Audio::open(const QIODevice::OpenMode mode)
{
  if (m_isOpen || !configurationOk())
    return false;

  if (mode & QIODevice::ReadOnly)
  {
    m_source = new QAudioSource(inputDevice(), m_inputFormat, this);
    m_source->setBufferSize(bufferSizes().at(m_bufferSizeIdx).toInt());
    m_inputStream = m_source->start();
    if (!m_inputStream)
    {
      qWarning() << "[Audio] Failed to start input stream on device:"
                 << m_source->error();

      m_source->stop();
      m_source->deleteLater();
      m_source = nullptr;
      m_inputStream = nullptr;
      return false;
    }

    connect(m_inputStream, &QIODevice::readyRead, this,
            &Audio::onInputReadyRead, Qt::QueuedConnection);
  }

  if (mode & QIODevice::WriteOnly)
  {
    m_sink = new QAudioSink(outputDevice(), m_outputFormat, this);
    m_outputStream = m_sink->start();
    if (!m_outputStream)
    {
      qWarning() << "[Audio] Failed to start output stream on device:"
                 << m_sink->error();

      m_sink->stop();
      m_sink->deleteLater();
      m_sink = nullptr;
      m_outputStream = nullptr;
    }
  }

  m_isOpen = true;
  return true;
}

//------------------------------------------------------------------------------
// Audio format access members
//------------------------------------------------------------------------------

/**
 * @brief Returns the currently configured input audio format.
 *
 * This includes sample rate, channel configuration, and sample format
 * as configured by the UI or automatically loaded from the preferred
 * input device format.
 *
 * @return The active QAudioFormat for input.
 */
QAudioFormat IO::Drivers::Audio::inputFormat() const
{
  return m_inputFormat;
}

/**
 * @brief Returns the currently configured output audio format.
 *
 * This format is used when initializing QAudioSink, and includes
 * sample rate, format type, and channel configuration.
 *
 * @return The active QAudioFormat for output.
 */
QAudioFormat IO::Drivers::Audio::outputFormat() const
{
  return m_outputFormat;
}

//------------------------------------------------------------------------------
// Audio device access members
//------------------------------------------------------------------------------

/**
 * @brief Returns the currently selected input audio device.
 *
 * The device is selected via `setSelectedInputDevice()` or initialized
 * from the system's preferred input device on startup.
 *
 * @return The selected QAudioDevice used for input.
 */
const QAudioDevice &IO::Drivers::Audio::inputDevice() const
{
  return m_inputDevices[m_selectedInputDevice];
}

/**
 * @brief Returns the currently selected output audio device.
 *
 * The device is selected via `setSelectedOutputDevice()` or initialized
 * from the system's preferred output device on startup.
 *
 * @return The selected QAudioDevice used for output.
 */
const QAudioDevice &IO::Drivers::Audio::outputDevice() const
{
  return m_outputDevices[m_selectedOutputDevice];
}

//------------------------------------------------------------------------------
// Input device parameters
//------------------------------------------------------------------------------

/**
 * @brief Returns the index of the currently selected input audio device.
 *
 * This index corresponds to the position of the device in the list
 * returned by `inputDeviceList()`.
 *
 * @return Index of the selected input device.
 */
int IO::Drivers::Audio::selectedInputDevice() const
{
  return m_selectedInputDevice;
}

/**
 * @brief Returns the index of the currently selected input sample rate.
 *
 * The index maps to an entry in the list from `inputSampleRates()`.
 *
 * @return Index of the selected input sample rate.
 */
int IO::Drivers::Audio::selectedInputSampleRate() const
{
  return m_selectedInputSampleRate;
}

/**
 * @brief Returns the index of the currently selected input sample format.
 *
 * This corresponds to an entry in the list returned by
 * `inputSampleFormats()`, which maps human-readable names to
 * `QAudioFormat::SampleFormat` values.
 *
 * @return Index of the selected input sample format.
 */
int IO::Drivers::Audio::selectedInputSampleFormat() const
{
  return m_selectedInputSampleFormat;
}

/**
 * @brief Returns the index of the selected input channel configuration.
 *
 * This index maps to values in `inputChannelConfigurations()`, which
 * describes supported audio layouts like Mono, Stereo, etc.
 *
 * @return Index of the selected input channel configuration.
 */
int IO::Drivers::Audio::selectedInputChannelConfiguration() const
{
  return m_selectedInputChannelConfiguration;
}

//------------------------------------------------------------------------------
// Output device parameters
//------------------------------------------------------------------------------

/**
 * @brief Returns the index of the currently selected output audio device.
 *
 * This index corresponds to the position of the device in the list
 * returned by `outputDeviceList()`.
 *
 * @return Index of the selected output device.
 */
int IO::Drivers::Audio::selectedOutputDevice() const
{
  return m_selectedOutputDevice;
}

/**
 * @brief Returns the index of the currently selected output sample rate.
 *
 * The index maps to an entry in the list returned by `outputSampleRates()`.
 *
 * @return Index of the selected output sample rate.
 */
int IO::Drivers::Audio::selectedOutputSampleRate() const
{
  return m_selectedOutputSampleRate;
}

/**
 * @brief Returns the index of the currently selected output sample format.
 *
 * This corresponds to an entry in the list returned by
 * `outputSampleFormats()`, which maps readable names to
 * `QAudioFormat::SampleFormat` values.
 *
 * @return Index of the selected output sample format.
 */
int IO::Drivers::Audio::selectedOutputSampleFormat() const
{
  return m_selectedOutputSampleFormat;
}

/**
 * @brief Returns the index of the selected output channel configuration.
 *
 * This index maps to values in `outputChannelConfigurations()`, which
 * describes supported audio layouts like Mono, Stereo, Surround, etc.
 *
 * @return Index of the selected output channel configuration.
 */
int IO::Drivers::Audio::selectedOutputChannelConfiguration() const
{
  return m_selectedOutputChannelConfiguration;
}

//------------------------------------------------------------------------------
// Streaming models
//------------------------------------------------------------------------------

/**
 * @brief Returns the index of the currently selected audio buffer size.
 *
 * This index corresponds to an entry in the list returned by `bufferSizes()`,
 * which represents the available buffer sizes in bytes.
 *
 * @return Index of the selected buffer size.
 */
int IO::Drivers::Audio::bufferSize() const
{
  return m_bufferSizeIdx;
}

/**
 * @brief Returns a list of commonly used audio buffer sizes.
 *
 * Each entry in the list is a string representing the buffer size in bytes.
 * These values are used to configure the internal buffer for audio streaming,
 * affecting latency and performance.
 *
 * @return List of buffer sizes in bytes (as strings).
 */
QStringList IO::Drivers::Audio::bufferSizes() const
{
  // clang-format off
  const QStringList list  = {"128", "256", "512", "1024", "2048", "4096",
                             "8192", "16384", "32768"};
  return list;
  // clang-format on
}

//------------------------------------------------------------------------------
// Input device parameter models
//------------------------------------------------------------------------------

/**
 * @brief Returns a list of available audio input devices.
 *
 * Each entry in the list is a user-friendly description of an input device
 * (e.g., "Built-in Microphone", "USB Audio Interface").
 *
 * @return List of input device descriptions.
 */
QStringList IO::Drivers::Audio::inputDeviceList() const
{
  QStringList list;
  for (const auto &device : m_inputDevices)
  {
    if (!device.isNull())
      list.append(device.description());
  }

  return list;
}

/**
 * @brief Returns the supported sample rates for the selected input device.
 *
 * The list is filtered to only include valid and supported rates, based on the
 * preferred format of the device.
 *
 * @return List of supported input sample rates (as strings).
 */
QStringList IO::Drivers::Audio::inputSampleRates() const
{
  return getSampleRates(inputDevice());
}

/**
 * @brief Returns the supported sample formats for the selected input device.
 *
 * The list is returned using human-readable descriptions such as
 * "Signed 16-bit", "Float 32-bit", etc.
 *
 * @return List of supported input sample format descriptions.
 */
QStringList IO::Drivers::Audio::inputSampleFormats() const
{
  return getSampleFormats(inputDevice());
}

/**
 * @brief Returns the supported channel configurations for the input device.
 *
 * Returns configurations like "Mono", "Stereo", "5.1", etc., based on the
 * capabilities of the device.
 *
 * @return List of supported input channel configuration descriptions.
 */
QStringList IO::Drivers::Audio::inputChannelConfigurations() const
{
  return getChannelConfigurations(inputDevice());
}

//------------------------------------------------------------------------------
// Output device parameter models
//------------------------------------------------------------------------------

/**
 * @brief Returns a list of available audio output devices.
 *
 * Each entry in the list is a user-readable description, such as
 * "Built-in Output", "HDMI", or "External USB DAC".
 *
 * @return List of output device descriptions.
 */
QStringList IO::Drivers::Audio::outputDeviceList() const
{
  QStringList list;
  for (const auto &device : m_outputDevices)
  {
    if (!device.isNull())
      list.append(device.description());
  }

  return list;
}

/**
 * @brief Returns the supported sample rates for the selected output device.
 *
 * The result includes only the sample rates that are valid for the selected
 * device and format.
 *
 * @return List of supported output sample rates (as strings).
 */
QStringList IO::Drivers::Audio::outputSampleRates() const
{
  return getSampleRates(outputDevice());
}

/**
 * @brief Returns the supported sample formats for the selected output device.
 *
 * The list is returned using human-readable descriptions such as
 * "Signed 16-bit", "Float 32-bit", etc.
 *
 * @return List of supported output sample format descriptions.
 */
QStringList IO::Drivers::Audio::outputSampleFormats() const
{
  return getSampleFormats(outputDevice());
}

/**
 * @brief Returns the supported channel configurations for the output device.
 *
 * Values include "Mono", "Stereo", "5.1", "7.1", etc., depending on the device.
 *
 * @return List of supported output channel configuration descriptions.
 */
QStringList IO::Drivers::Audio::outputChannelConfigurations() const
{
  return getChannelConfigurations(outputDevice());
}

//------------------------------------------------------------------------------
// I/O devices access functions
//------------------------------------------------------------------------------

/**
 * @brief Returns the list of available input audio devices.
 *
 * This gives access to the full QAudioDevice entries representing
 * all available input (recording) devices on the system.
 *
 * @return Reference to internal vector of QAudioDevice input devices.
 */
const QVector<QAudioDevice> &IO::Drivers::Audio::inputDevices() const
{
  return m_inputDevices;
}

/**
 * @brief Returns the list of available output audio devices.
 *
 * This gives access to the full QAudioDevice entries representing
 * all available output (playback) devices on the system.
 *
 * @return Reference to internal vector of QAudioDevice output devices.
 */
const QVector<QAudioDevice> &IO::Drivers::Audio::outputDevices() const
{
  return m_outputDevices;
}

//------------------------------------------------------------------------------
// Data format configuration setters
//------------------------------------------------------------------------------

/**
 * @brief Sets the audio input buffer size by selecting a predefined option.
 *
 * This function updates the internal buffer size index, which is later used
 * when configuring the QAudioSource. A signal is emitted if the index changes.
 *
 * @param index Index to the new buffer size in the list set by bufferSizes().
 */
void IO::Drivers::Audio::setBufferSize(const int index)
{
  if (m_bufferSizeIdx != index)
  {
    m_bufferSizeIdx = index;
    Q_EMIT bufferSizeChanged();
  }
}

//------------------------------------------------------------------------------
// Input device parameter setters
//------------------------------------------------------------------------------

/**
 * @brief Sets the selected audio input device.
 *
 * This updates the current input device index, reloads the preferred format
 * for the new device, synchronizes UI selection indexes, and applies the new
 * format.
 *
 * This call is ignored if the device is already open or if the index has not
 * changed.
 *
 * @param index Index of the input device in inputDeviceList().
 */
void IO::Drivers::Audio::setSelectedInputDevice(int index)
{
  Q_ASSERT(index >= 0 && index < inputDeviceList().size());
  if (isOpen() || index == m_selectedInputDevice)
    return;

  // Modify device index
  m_selectedInputDevice = index;

  // Load preferred format
  m_inputFormat = inputDevice().preferredFormat();
  syncInputParameters();

  // Configure the device
  configureInput();
}

/**
 * @brief Sets the selected input sample rate.
 *
 * Updates the input sample rate index and reconfigures the input format.
 * Ignored if the device is open or the index hasn't changed.
 *
 * @param index Index in the list returned by inputSampleRates().
 */
void IO::Drivers::Audio::setSelectedInputSampleRate(int index)
{
  Q_ASSERT(index >= 0 && index < inputSampleRates().size());
  if (isOpen() || index == m_selectedInputSampleRate)
    return;

  m_selectedInputSampleRate = index;
  configureInput();
}

/**
 * @brief Sets the selected input sample format.
 *
 * Updates the input sample format index and reconfigures the input format.
 * Ignored if the device is open or the index hasn't changed.
 *
 * @param index Index in the list returned by inputSampleFormats().
 */
void IO::Drivers::Audio::setSelectedInputSampleFormat(int index)
{
  Q_ASSERT(index >= 0 && index < inputSampleFormats().size());
  if (isOpen() || index == m_selectedInputSampleFormat)
    return;

  m_selectedInputSampleFormat = index;
  configureInput();
}

/**
 * @brief Sets the selected input channel configuration.
 *
 * Updates the input channel config index and reconfigures the input format.
 * Ignored if the device is open or the index hasn't changed.
 *
 * @param index Index in the list returned by inputChannelConfigurations().
 */
void IO::Drivers::Audio::setSelectedInputChannelConfiguration(int index)
{
  Q_ASSERT(index >= 0 && index < inputChannelConfigurations().size());
  if (isOpen() || index == m_selectedInputChannelConfiguration)
    return;

  m_selectedInputChannelConfiguration = index;
  configureInput();
}

//------------------------------------------------------------------------------
// Output device parameter setters
//------------------------------------------------------------------------------

/**
 * @brief Sets the selected audio output device.
 *
 * Updates the output device index, loads its preferred format,
 * synchronizes parameter indexes, and applies the new format.
 *
 * This call is ignored if the device is already open or if the index hasn't
 * changed.
 *
 * @param index Index of the output device in outputDeviceList().
 */
void IO::Drivers::Audio::setSelectedOutputDevice(int index)
{
  Q_ASSERT(index >= 0 && index < outputDeviceList().size());
  if (isOpen() || index == m_selectedOutputDevice)
    return;

  // Modify device index
  m_selectedOutputDevice = index;

  // Load preferred format
  m_outputFormat = outputDevice().preferredFormat();
  syncOutputParameters();

  // Configure the device
  configureOutput();
}

/**
 * @brief Sets the selected output sample rate.
 *
 * Updates the output sample rate index and reconfigures the output format.
 * Ignored if the device is open or the index hasn't changed.
 *
 * @param index Index in the list returned by outputSampleRates().
 */
void IO::Drivers::Audio::setSelectedOutputSampleRate(int index)
{
  Q_ASSERT(index >= 0 && index < outputSampleRates().size());
  if (isOpen() || index == m_selectedOutputSampleRate)
    return;

  m_selectedOutputSampleRate = index;
  configureOutput();
}

/**
 * @brief Sets the selected output sample format.
 *
 * Updates the output sample format index and reconfigures the output format.
 * Ignored if the device is open or the index hasn't changed.
 *
 * @param index Index in the list returned by outputSampleFormats().
 */
void IO::Drivers::Audio::setSelectedOutputSampleFormat(int index)
{
  Q_ASSERT(index >= 0 && index < outputSampleFormats().size());
  if (isOpen() || index == m_selectedOutputSampleFormat)
    return;

  m_selectedOutputSampleFormat = index;
  configureOutput();
}

/**
 * @brief Sets the selected output channel configuration.
 *
 * Updates the output channel config index and reconfigures the output format.
 * Ignored if the device is open or the index hasn't changed.
 *
 * @param index Index in the list returned by outputChannelConfigurations().
 */
void IO::Drivers::Audio::setSelectedOutputChannelConfiguration(int index)
{
  Q_ASSERT(index >= 0 && index < outputChannelConfigurations().size());
  if (isOpen() || index == m_selectedOutputChannelConfiguration)
    return;

  m_selectedOutputChannelConfiguration = index;
  configureOutput();
}

//------------------------------------------------------------------------------
// Device configuration helpers
//------------------------------------------------------------------------------

/**
 * @brief Configures the audio input format based on current UI selections.
 *
 * This method constructs a QAudioFormat using:
 * - The preferred format from the selected input device
 * - User-selected sample rate, sample format, and channel configuration
 *
 * If the final format is not supported by the device, it falls back to the
 * preferred format. After applying the format, it updates the internal
 * parameter indexes to match and emits `configurationChanged()`.
 */
void IO::Drivers::Audio::configureInput()
{
  // Obtain models
  const QStringList sampleRates = inputSampleRates();
  const QStringList sampleFormats = inputSampleFormats();
  const QStringList channelConfigs = inputChannelConfigurations();

  // Load default format
  m_inputFormat = inputDevice().preferredFormat();

  // Modify sample rate
  if (!sampleRates.isEmpty())
  {
    const auto rate = sampleRates[m_selectedInputSampleRate].toInt();
    m_inputFormat.setSampleRate(rate);
  }

  // Modify sample format
  if (!sampleFormats.isEmpty())
  {
    auto fmt = m_inputFormat.sampleFormat();
    const auto sampleFormat = sampleFormats[m_selectedInputSampleFormat];
    for (auto it = m_sampleFormats.begin(); it != m_sampleFormats.end(); ++it)
    {
      if (it.value() == sampleFormat)
      {
        fmt = it.key();
        break;
      }
    }

    m_inputFormat.setSampleFormat(fmt);
  }

  // Modify channel configuration
  if (!channelConfigs.isEmpty())
  {
    auto cfg = m_inputFormat.channelConfig();
    const auto config = channelConfigs[m_selectedInputChannelConfiguration];
    for (auto it = m_knownConfigs.begin(); it != m_knownConfigs.end(); ++it)
    {
      if (it.value() == config)
      {
        cfg = it.key();
        break;
      }
    }

    m_inputFormat.setChannelConfig(cfg);
  }

  // Load default format if needed
  if (!inputDevice().isFormatSupported(m_inputFormat))
    m_inputFormat = inputDevice().preferredFormat();

  // Sync parameters with loaded format
  syncInputParameters();

  // Update user interface
  Q_EMIT configurationChanged();
}

/**
 * @brief Configures the audio output format based on current UI selections.
 *
 * This method builds a QAudioFormat using:
 * - The preferred format from the selected output device
 * - User-selected sample rate, sample format, and channel configuration
 *
 * If the selected combination is unsupported, the device's preferred format
 * is restored. After setting the format, UI indexes are re-synced via
 * `syncOutputParameters()` and `configurationChanged()` is emitted.
 */
void IO::Drivers::Audio::configureOutput()
{
  // Obtain models
  const QStringList sampleRates = outputSampleRates();
  const QStringList sampleFormats = outputSampleFormats();
  const QStringList channelConfigs = outputChannelConfigurations();

  // Load default format
  m_outputFormat = outputDevice().preferredFormat();

  // Modify sample rate
  if (!sampleRates.isEmpty())
  {
    const auto rate = sampleRates[m_selectedOutputSampleRate].toInt();
    m_outputFormat.setSampleRate(rate);
  }

  // Modify sample format
  if (!sampleFormats.isEmpty())
  {
    auto fmt = m_outputFormat.sampleFormat();
    const auto sampleFormat = sampleFormats[m_selectedOutputSampleFormat];
    for (auto it = m_sampleFormats.begin(); it != m_sampleFormats.end(); ++it)
    {
      if (it.value() == sampleFormat)
      {
        fmt = it.key();
        break;
      }
    }

    m_outputFormat.setSampleFormat(fmt);
  }

  // Modify channel configuration
  if (!channelConfigs.isEmpty())
  {
    auto cfg = m_outputFormat.channelConfig();
    const auto config = channelConfigs[m_selectedOutputChannelConfiguration];
    for (auto it = m_knownConfigs.begin(); it != m_knownConfigs.end(); ++it)
    {
      if (it.value() == config)
      {
        cfg = it.key();
        break;
      }
    }

    m_outputFormat.setChannelConfig(cfg);
  }

  // Load default format if needed
  if (!outputDevice().isFormatSupported(m_outputFormat))
    m_outputFormat = outputDevice().preferredFormat();

  // Sync parameters with loaded format
  syncOutputParameters();

  // Update user interface
  Q_EMIT configurationChanged();
}

//------------------------------------------------------------------------------
// Audio input data parsing
//------------------------------------------------------------------------------

/**
 * @brief Slot triggered when the input device has audio data available.
 *
 * This function reads and parses raw audio frames from the input stream,
 * converts each sample to a CSV-format string, and sends the result to
 * `processData()`.
 *
 * For each frame:
 * - Splits samples by channel
 * - Converts the binary sample to a numeric string
 * - Appends values comma-separated, one line per frame
 *
 * Supported sample formats:
 * - QAudioFormat::UInt8
 * - QAudioFormat::Int16
 * - QAudioFormat::Int32
 * - QAudioFormat::Float (32-bit IEEE)
 *
 * Early exits if:
 * - Stream is null
 * - Input format is invalid (e.g., no channels)
 * - Buffer is empty
 * - Frame size or channel count is invalid
 *
 * Note: This method assumes the device buffer size has been tuned for
 * low latency and frequent `readyRead()` triggers to preserve temporal
 * accuracy.
 */
void IO::Drivers::Audio::onInputReadyRead()
{
  if (!m_inputStream || m_inputFormat.channelCount() == 0)
    return;

  const QByteArray raw = m_inputStream->readAll();
  if (raw.isEmpty())
    return;

  const int frameSize = m_inputFormat.bytesPerFrame();
  const int channels = m_inputFormat.channelCount();

  if (frameSize <= 0 || channels <= 0)
    return;

  const int bytesPerSample = frameSize / channels;
  const int totalSamples = raw.size() / frameSize;

  QByteArray csv;
  const char *ptr = raw.constData();
  const auto format = m_inputFormat.sampleFormat();

  for (int i = 0; i < totalSamples; ++i)
  {
    for (int ch = 0; ch < channels; ++ch)
    {
      switch (format)
      {
        case QAudioFormat::UInt8: {
          const quint8 sample = qFromLittleEndian<quint8>(
              reinterpret_cast<const quint8 *>(ptr));
          csv += QByteArray::number(sample);
          break;
        }
        case QAudioFormat::Int16: {
          const qint16 sample = qFromLittleEndian<qint16>(
              reinterpret_cast<const quint8 *>(ptr));
          csv += QByteArray::number(sample);
          break;
        }
        case QAudioFormat::Int32: {
          const qint32 sample = qFromLittleEndian<qint32>(
              reinterpret_cast<const quint8 *>(ptr));
          csv += QByteArray::number(sample);
          break;
        }
        case QAudioFormat::Float: {
          float sample;
          std::memcpy(&sample, ptr, sizeof(float));
          csv += QByteArray::number(sample, 'f', 6);
          break;
        }
        default:
          return;
      }

      ptr += bytesPerSample;

      if (ch < channels - 1)
        csv += ',';
    }

    csv += '\n';
  }

  processData(csv);
}

//------------------------------------------------------------------------------
// Device discovery functions
//------------------------------------------------------------------------------

/**
 * @brief Refreshes the list of available audio input devices.
 *
 * Queries the system for available input devices via QMediaDevices. If the
 * list has changed and no stream is currently open, updates the internal
 * device list and emits `inputSettingsChanged()`.
 *
 * This is called periodically to handle dynamic hardware changes.
 */
void IO::Drivers::Audio::refreshAudioInputs()
{
  auto audioInputs = QMediaDevices::audioInputs();
  if (m_inputDevices != audioInputs && !m_isOpen)
  {
    m_inputDevices = audioInputs;
    Q_EMIT inputSettingsChanged();
  }
}

/**
 * @brief Refreshes the list of available audio output devices.
 *
 * Similar to refreshAudioInputs(), but for output devices.
 * Updates internal state only if the device list changed and audio is not open.
 *
 * Emits `outputSettingsChanged()` when changes are detected.
 */
void IO::Drivers::Audio::refreshAudioOutputs()
{
  auto audioOutputs = QMediaDevices::audioOutputs();
  if (m_outputDevices != audioOutputs && !m_isOpen)
  {
    m_outputDevices = audioOutputs;
    Q_EMIT outputSettingsChanged();
  }
}

//------------------------------------------------------------------------------
// Model sync functions
//------------------------------------------------------------------------------

/**
 * @brief Synchronizes selected input parameters with the current input format.
 *
 * Matches the current input audio format's sample rate, sample format,
 * and channel configuration against the available options.
 * Updates internal indexes and emits `inputSettingsChanged()` to refresh UI.
 */
void IO::Drivers::Audio::syncInputParameters()
{
  // Get current input format
  auto format = inputFormat();

  // Get current model data
  const QStringList sampleRates = inputSampleRates();
  const QStringList sampleFormats = inputSampleFormats();
  const QStringList channelConfigs = inputChannelConfigurations();

  // Sync selections
  // clang-format off
  m_selectedInputSampleRate = sampleRates.indexOf(QString::number(format.sampleRate()));
  m_selectedInputSampleFormat = sampleFormats.indexOf(m_sampleFormats.value(format.sampleFormat()));
  m_selectedInputChannelConfiguration = channelConfigs.indexOf(m_knownConfigs.value(format.channelConfig()));
  // clang-format on

  // Bound selections to valid values
  // clang-format off
  m_selectedInputSampleRate = qBound(0, m_selectedInputSampleRate, sampleRates.count() - 1);
  m_selectedInputSampleFormat = qBound(0, m_selectedInputSampleFormat, sampleFormats.count() - 1);
  m_selectedInputChannelConfiguration = qBound(0, m_selectedInputChannelConfiguration, channelConfigs.count() - 1);
  // clang-format on

  // Update UI
  Q_EMIT inputSettingsChanged();
}

/**
 * @brief Synchronizes selected output parameters with the output format.
 *
 * Matches the current output audio format's sample rate, sample format,
 * and channel configuration against the available options.
 * Updates internal indexes and emits `outputSettingsChanged()` to refresh UI.
 */
void IO::Drivers::Audio::syncOutputParameters()
{
  // Get current output format
  auto format = outputFormat();

  // Get current model data
  const QStringList sampleRates = outputSampleRates();
  const QStringList sampleFormats = outputSampleFormats();
  const QStringList channelConfigs = outputChannelConfigurations();

  // Sync selections
  // clang-format off
  m_selectedOutputSampleRate = sampleRates.indexOf(QString::number(format.sampleRate()));
  m_selectedOutputSampleFormat = sampleFormats.indexOf(m_sampleFormats.value(format.sampleFormat()));
  m_selectedOutputChannelConfiguration = channelConfigs.indexOf(m_knownConfigs.value(format.channelConfig()));
  // clang-format on

  // Bound selections to valid values
  // clang-format off
  m_selectedOutputSampleRate = qBound(0, m_selectedOutputSampleRate, sampleRates.count() - 1);
  m_selectedOutputSampleFormat = qBound(0, m_selectedOutputSampleFormat, sampleFormats.count() - 1);
  m_selectedOutputChannelConfiguration = qBound(0, m_selectedOutputChannelConfiguration, channelConfigs.count() - 1);
  // clang-format on

  // Update UI
  Q_EMIT outputSettingsChanged();
}

//------------------------------------------------------------------------------
// Utility functions
//------------------------------------------------------------------------------

/**
 * @brief Returns a list of supported sample rates for a given audio device.
 *
 * Checks the device's support against a list of common sample rates.
 * Only includes those explicitly supported by the device.
 *
 * @param device The audio device to query.
 * @return A list of supported sample rates as strings (e.g., "44100").
 */
QStringList IO::Drivers::Audio::getSampleRates(const QAudioDevice &device) const
{
  QStringList list;
  if (device.isNull())
    return list;

  static const QList<int> commonRates
      = {8000, 11025, 16000, 22050, 32000, 44100, 48000, 88200, 96000, 192000};

  QAudioFormat fmt = device.preferredFormat();
  for (int rate : commonRates)
  {
    fmt.setSampleRate(rate);
    if (device.isFormatSupported(fmt))
      list.append(QString::number(rate));
  }

  return list;
}

/**
 * @brief Returns a list of supported sample formats for a given audio device.
 *
 * Converts internal enum values to human-readable strings, filtered by device
 * capabilities.
 *
 * @param device The audio device to query.
 * @return A list of supported sample format descriptions (e.g., "Signed
 * 16-bit").
 */
QStringList
IO::Drivers::Audio::getSampleFormats(const QAudioDevice &device) const
{
  QStringList list;
  if (device.isNull())
    return list;

  const auto formats = device.supportedSampleFormats();
  for (const auto format : formats)
  {
    if (m_sampleFormats.contains(format))
      list.append(m_sampleFormats.value(format));
  }

  return list;
}

/**
 * @brief Returns a list of supported channel setups for an audio device.
 *
 * Iterates over known configurations and checks which are supported.
 *
 * @param device The audio device to query.
 * @return A list of supported channel configuration names (e.g., "Mono",
 * "Stereo").
 */
QStringList
IO::Drivers::Audio::getChannelConfigurations(const QAudioDevice &device) const
{
  QStringList list;
  if (device.isNull())
    return list;

  QAudioFormat fmt = device.preferredFormat();
  for (auto it = m_knownConfigs.cbegin(); it != m_knownConfigs.cend(); ++it)
  {
    fmt.setChannelConfig(it.key());
    if (device.isFormatSupported(fmt))
      list.append(it.value());
  }

  return list;
}
