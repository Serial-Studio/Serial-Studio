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

#include "Core/Bus/MessageBus.h"
#include "Core/Bus/Messages.h"
#include "Core/Services.h"
#include "Core/SSAssert.h"
#include "Core/TimerEvents.h"
#include "Core/Translator.h"
#include "IO/ConnectionManager.h"
#include "IO/Drivers/Audio/AudioDeviceCatalog.h"
#include "IO/Drivers/Audio/AudioPcm.h"

using namespace IO::Drivers::AudioPcm;

// SPSC queue depth for audio in/out buffers; sized for ~24Hz drain vs ~10ms produce
static constexpr std::size_t kAudioQueueCapacity = 1024;

// Playback ring: a quarter second of 48 kHz stereo float32, so a burst of written samples survives
static constexpr qsizetype kPlaybackRingBytes = 48000 * 2 * 4 / 4;

// Pre-sized capture slots handed to the real-time callback, so it never allocates
static constexpr int kInputPoolSlots      = 32;
static constexpr qsizetype kInputSlotSize = 16384;

// Continuous-clock resync bound: jitter under this is absorbed, drift over it snaps to wall time
static constexpr std::chrono::milliseconds kAudioClockResync{50};

//--------------------------------------------------------------------------------------------------
// Constructor, destructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the Audio driver and initializes the MiniAudio backend;
 * configureInput/configureOutput sync m_config at the end, so no separate
 * sync call is needed here.
 */
IO::Drivers::Audio::Audio()
  : m_init(false)
  , m_isOpen(false)
  , m_normalization(true)
  , m_discoveryPaused(false)
  , m_catalog(&m_context, m_init)
  , m_inputQueue(kAudioQueueCapacity)
  , m_playbackRing(kPlaybackRingBytes)
  , m_inputPool(kAudioQueueCapacity)
  , m_inputDrops(0)
  , m_inputWorkerTimer(nullptr)
  , m_sampleClockValid(false)
  , m_stopNotifyArmed(false)
  , m_streamLaneActive(false)
  , m_rtCaptureFormat(ma_format_unknown)
  , m_rtPlaybackFormat(ma_format_unknown)
  , m_rtCaptureChannels(0)
  , m_rtPlaybackChannels(0)
{
#if defined(Q_OS_WIN)
  ma_backend backend[] = {ma_backend_wasapi};
#elif defined(Q_OS_APPLE)
  ma_backend backend[] = {ma_backend_coreaudio};
#elif defined(Q_OS_LINUX)
  ma_backend backend[] = {ma_backend_alsa};
#else
#  error "Unsupported platform"
#endif

  m_config = ma_device_config_init(ma_device_type_duplex);
  m_init   = ma_context_init(backend, 1, nullptr, &m_context) == MA_SUCCESS;
  if (!m_init)
    qWarning("Failed to initialize miniaudio context");

  m_csvData.reserve(96 * 1024);
  m_csvBuffer.setBuffer(&m_csvData);
  m_csvBuffer.open(QIODevice::WriteOnly);

  m_csvStream.setRealNumberPrecision(6);
  m_csvStream.setDevice(&m_csvBuffer);
  m_csvStream.setRealNumberNotation(QTextStream::FixedNotation);

  generateLists();
  refreshAudioDevices();

  restoreSettings();
  configureInput();
  configureOutput();

  auto& timerEvents = Core::services().timerEvents;
  connect(&timerEvents, &Misc::TimerEvents::timeout1Hz, this, &Audio::refreshAudioDevices);

  auto& translator = Core::services().translator;
  connect(
    &translator, &Misc::Translator::languageChanged, this, &IO::Drivers::Audio::generateLists);
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
 * @brief Closes the audio device and releases all associated resources; draining the SPSC queues
 *        is safe because ma_device_uninit already joined the audio thread. The worker timer's
 *        deleteLater is posted before quit() so the thread's exit-time DeferredDelete sweep runs
 *        ~QTimer on its own thread (a plain delete here kills the timer id from the GUI thread).
 */
void IO::Drivers::Audio::closeDevice()
{
  if (!m_isOpen)
    return;

  m_stopNotifyArmed.store(false, std::memory_order_release);
  ma_device_uninit(&m_device);

  if (m_inputWorkerTimer && m_inputWorkerThread.isRunning())
    m_inputWorkerTimer->deleteLater();

  else
    delete m_inputWorkerTimer;

  if (m_inputWorkerThread.isRunning()) {
    m_inputWorkerThread.quit();
    m_inputWorkerThread.wait();
  }

  m_inputWorkerTimer = nullptr;

  {
    QByteArray dropped;
    while (m_inputQueue.try_dequeue(dropped)) {
    }

    while (m_inputPool.try_dequeue(dropped)) {
    }
  }

  m_playbackRing.reset();

  m_sampleClockValid = false;

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

  const int index = m_catalog.selectedInputDevice();
  return index >= 0 && index < m_catalog.inputDevices().size() && m_config.capture.channels > 0;
}

/**
 * @brief Determines if the current audio output configuration is valid and writable.
 */
bool IO::Drivers::Audio::isWritable() const noexcept
{
  if (!m_isOpen)
    return false;

  const int index = m_catalog.selectedOutputDevice();
  return index >= 0 && index < m_catalog.outputDevices().size() && m_config.playback.channels > 0;
}

/**
 * @brief Validates the currently selected input device configuration.
 */
bool IO::Drivers::Audio::configurationOk() const noexcept
{
  if (!validateInput())
    return false;

  bool ok        = true;
  const auto& c  = m_catalog.inputCapabilities()[m_catalog.selectedInputDevice()];
  ok            &= m_catalog.selectedSampleRate() >= 0;
  ok            &= m_catalog.selectedInputSampleFormat() >= 0;
  ok            &= m_catalog.selectedInputChannelConfiguration() >= 0;
  ok            &= m_catalog.selectedInputSampleFormat() < c.supportedFormats.size();
  ok            &= m_catalog.selectedSampleRate() < c.supportedSampleRates.size();
  ok            &= m_catalog.selectedInputChannelConfiguration() < c.supportedChannelCounts.size();

  return ok;
}

/**
 * @brief Writes CSV-formatted audio frames into the playback ring; values are normalized -1..1
 *        floats or raw per-format magnitudes, matching whatever capture publishes. One call may
 *        carry many newline-separated frames, which is what makes a continuous tone possible: a
 *        single frame per call could only ever be one sample of an output period.
 */
qint64 IO::Drivers::Audio::write(const QByteArray& data)
{
  if (!m_isOpen || m_config.playback.channels <= 0) {
    logDriverError(tr("Audio output unavailable"),
                   tr("No output device is configured for this session."));
    return 0;
  }

  const int channels     = m_config.playback.channels;
  const ma_format format = m_config.playback.format;

  const auto lines = data.split('\n');
  for (const auto& line : lines) {
    if (line.trimmed().isEmpty())
      continue;

    if (!packPlaybackFrame(line, channels, format))
      return 0;
  }

  return data.size();
}

/**
 * @brief Packs one CSV frame and hands it to the ring, reporting whether it landed. The scratch
 *        buffer is a member so a steady tone does not allocate once per sample frame.
 */
bool IO::Drivers::Audio::packPlaybackFrame(const QByteArray& line, int channels, ma_format format)
{
  const QList<QByteArray> parts = line.trimmed().split(',');
  if (parts.size() != channels) {
    logDriverError(tr("Audio channel mismatch"),
                   tr("Expected %1 value(s) per frame, got %2.")
                     .arg(QString::number(channels), QString::number(parts.size())));
    return false;
  }

  m_playbackScratch.clear();
  for (int i = 0; i < channels; ++i) {
    const bool packed = m_normalization ? packNormalizedSample(format, parts[i], m_playbackScratch)
                                        : packCsvSample(format, parts[i], m_playbackScratch);
    if (!packed)
      return false;
  }

  return m_playbackRing.write(reinterpret_cast<const char*>(m_playbackScratch.constData()),
                              m_playbackScratch.size());
}

/**
 * @brief How many capture blocks the real-time callback had to drop for want of a free slot;
 *        pulled, never pushed (spec 0033).
 */
quint64 IO::Drivers::Audio::inputDrops() const noexcept
{
  return m_inputDrops.load(std::memory_order_relaxed);
}

/**
 * @brief How many output callbacks ran out of buffered samples and played silence instead.
 */
quint64 IO::Drivers::Audio::playbackUnderruns() const noexcept
{
  return m_playbackRing.underruns();
}

/**
 * @brief Opens the audio I/O device with the current configuration.
 */
bool IO::Drivers::Audio::open(const QIODevice::OpenMode mode)
{
  if (!m_init || m_isOpen || !configurationOk())
    return false;

  m_config = ma_device_config_init(ma_device_type_duplex);

  const auto& inputCaps = m_catalog.inputCapabilities()[m_catalog.selectedInputDevice()];

  // clang-format off
  m_config.pUserData = this;
  m_config.dataCallback = &Audio::callback;
  m_config.notificationCallback = &Audio::notificationCallback;
  m_config.sampleRate = inputCaps.supportedSampleRates[m_catalog.selectedSampleRate()];
  // clang-format on

  m_config.noClip                    = MA_FALSE;
  m_config.noDisableDenormals        = MA_FALSE;
  m_config.noFixedSizedCallback      = MA_TRUE;
  m_config.noPreSilencedOutputBuffer = MA_FALSE;

  applyPlatformAudioConfig();
  configureCaptureFormat(mode);
  if (!configurePlaybackFormat(mode))
    return false;

  m_rtCaptureFormat.store(m_config.capture.format, std::memory_order_relaxed);
  m_rtCaptureChannels.store(m_config.capture.channels, std::memory_order_relaxed);
  m_rtPlaybackFormat.store(m_config.playback.format, std::memory_order_relaxed);
  m_rtPlaybackChannels.store(m_config.playback.channels, std::memory_order_release);
  publishCaptureFormat();

  m_playbackRing.reset();
  seedInputPool(kInputSlotSize);

  std::memset(&m_device, 0, sizeof(m_device));
  if (ma_device_init(&m_context, &m_config, &m_device) != MA_SUCCESS) {
    logDriverError(tr("Audio device error"),
                   tr("The selected audio device could not be initialized."));
    return false;
  }

  if (ma_device_start(&m_device) != MA_SUCCESS) {
    logDriverError(tr("Audio device error"), tr("The selected audio device could not be started."));
    ma_device_uninit(&m_device);
    return false;
  }

  startInputWorker();

  m_stopNotifyArmed.store(true, std::memory_order_release);
  m_isOpen = true;
  return true;
}

/**
 * @brief Fills the capture pool with pre-sized slots so the real-time callback never allocates:
 *        it dequeues a slot, fills it and hands it on, and the worker returns it here.
 */
void IO::Drivers::Audio::seedInputPool(qsizetype slotBytes)
{
  SS_ASSERT(slotBytes > 0, return);

  QByteArray dropped;
  while (m_inputPool.try_dequeue(dropped)) {
  }

  for (int i = 0; i < kInputPoolSlots; ++i) {
    QByteArray slot;
    slot.reserve(slotBytes);
    (void)m_inputPool.try_enqueue(std::move(slot));
  }

  m_inputDrops.store(0, std::memory_order_relaxed);
}

/**
 * @brief Marshals a backend-initiated stop (device yanked, exclusive-mode steal, fatal xrun)
 *        onto the main thread; a teardown-initiated stop is disarmed before ma_device_uninit
 *        so only the backend's own stops reach onBackendStopped().
 */
void IO::Drivers::Audio::notificationCallback(const ma_device_notification* notification)
{
  SS_ASSERT(notification != nullptr, return);
  SS_ASSERT(notification->pDevice != nullptr, return);

  if (notification->type != ma_device_notification_type_stopped)
    return;

  auto* self = static_cast<Audio*>(notification->pDevice->pUserData);
  if (!self || !self->m_stopNotifyArmed.exchange(false, std::memory_order_acq_rel))
    return;

  QMetaObject::invokeMethod(self, "onBackendStopped", Qt::QueuedConnection);
}

/**
 * @brief Tears the session down after the audio backend stopped the stream on its own; the box
 *        is queued after the teardown so the UI never shows a dead stream as connected.
 */
void IO::Drivers::Audio::onBackendStopped()
{
  if (!m_isOpen)
    return;

  static auto& connectionManager = IO::ConnectionManager::instance();
  connectionManager.disconnectDevice(this);

  logDriverError(tr("Audio Device Stopped"),
                 tr("The audio backend stopped the stream. The device may have been "
                    "unplugged or claimed by another application."));
}

/**
 * @brief Applies platform-specific miniaudio backend tweaks to m_config.
 */
void IO::Drivers::Audio::applyPlatformAudioConfig()
{
#ifdef Q_OS_MAC
  m_config.coreaudio.allowNominalSampleRateChange = MA_TRUE;
#endif

#ifdef Q_OS_WIN
  m_config.wasapi.noAutoConvertSRC     = MA_FALSE;
  m_config.wasapi.noDefaultQualitySRC  = MA_FALSE;
  m_config.wasapi.noAutoStreamRouting  = MA_FALSE;
  m_config.wasapi.noHardwareOffloading = MA_TRUE;
#endif

#ifdef Q_OS_LINUX
  m_config.alsa.noMMap         = MA_FALSE;
  m_config.alsa.noAutoFormat   = MA_FALSE;
  m_config.alsa.noAutoChannels = MA_FALSE;
  m_config.alsa.noAutoResample = MA_FALSE;
#endif
}

/**
 * @brief Sets capture format/channels on m_config based on the requested mode.
 */
void IO::Drivers::Audio::configureCaptureFormat(QIODevice::OpenMode mode)
{
  if (mode & QIODevice::ReadOnly) {
    const int device = m_catalog.selectedInputDevice();
    const auto& caps = m_catalog.inputCapabilities()[device];

    m_config.capture.pDeviceID = &m_catalog.inputDevices()[device].id;
    m_config.capture.format    = caps.supportedFormats[m_catalog.selectedInputSampleFormat()];
    m_config.capture.channels =
      caps.supportedChannelCounts[m_catalog.selectedInputChannelConfiguration()];
    return;
  }

  m_config.capture.format   = ma_format_unknown;
  m_config.capture.channels = 0;
}

/**
 * @brief Validates and sets playback format/channels on m_config; false on error.
 */
bool IO::Drivers::Audio::configurePlaybackFormat(QIODevice::OpenMode mode)
{
  if (!(mode & QIODevice::WriteOnly)) {
    m_config.playback.format   = ma_format_unknown;
    m_config.playback.channels = 0;
    return true;
  }

  const int device = m_catalog.selectedOutputDevice();
  if (device < 0 || device >= m_catalog.outputDevices().size()
      || device >= m_catalog.outputCapabilities().size()) {
    qWarning() << "Audio::open: output device index out of range";
    return false;
  }

  const int format  = m_catalog.selectedOutputSampleFormat();
  const int channel = m_catalog.selectedOutputChannelConfiguration();
  const auto& oc    = m_catalog.outputCapabilities()[device];
  if (format < 0 || format >= oc.supportedFormats.size() || channel < 0
      || channel >= oc.supportedChannelCounts.size()) {
    qWarning() << "Audio::open: output format/channel index out of range";
    return false;
  }

  m_config.playback.pDeviceID = &m_catalog.outputDevices()[device].id;
  m_config.playback.format    = oc.supportedFormats[format];
  m_config.playback.channels  = oc.supportedChannelCounts[channel];
  return true;
}

/**
 * @brief Lazily creates the input worker timer/thread and starts the read tick. The timeout hop
 *        is DirectConnection so processInputBuffer executes on the worker thread (the timer's
 *        thread), never the GUI thread (spec 0051 T22); the timer is deleted exactly once, by
 *        closeDevice()'s queued deleteLater (the old finished->deleteLater double-delete is gone).
 */
void IO::Drivers::Audio::startInputWorker()
{
  if (!m_inputWorkerTimer) {
    m_inputWorkerTimer = new QTimer();
    m_inputWorkerTimer->setInterval(10);
    m_inputWorkerTimer->setTimerType(Qt::PreciseTimer);
    m_inputWorkerTimer->moveToThread(&m_inputWorkerThread);
    connect(m_inputWorkerTimer,
            &QTimer::timeout,
            this,
            &IO::Drivers::Audio::processInputBuffer,
            Qt::DirectConnection);
  }

  if (!m_inputWorkerThread.isRunning()) {
    m_inputWorkerThread.start();
    m_inputWorkerThread.setPriority(QThread::HighestPriority);
  }

  QMetaObject::invokeMethod(m_inputWorkerTimer, "start", Qt::QueuedConnection);
}

//--------------------------------------------------------------------------------------------------
// Normalization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether samples are published in the normalized -1..1 float range.
 */
bool IO::Drivers::Audio::normalization() const noexcept
{
  return m_normalization;
}

/**
 * @brief Enables or disables normalized sampling. While enabled the driver overrides the manual
 * sample-format selection with the cheapest format the device offers natively, because the
 * amplitude range no longer depends on it; the setting only moves while the device is closed,
 * which is what lets the input worker read it without an atomic.
 */
void IO::Drivers::Audio::setNormalization(bool enabled)
{
  if (isOpen() || m_normalization == enabled)
    return;

  m_normalization = enabled;
  configureInput();
  configureOutput();
  persistSettings();

  Q_EMIT normalizationChanged();
}

//--------------------------------------------------------------------------------------------------
// Sample rate selection
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the index of the currently selected sample rate.
 */
int IO::Drivers::Audio::selectedSampleRate() const
{
  return m_catalog.selectedSampleRate();
}

/**
 * @brief Returns a list of supported input sample rates as strings.
 */
QStringList IO::Drivers::Audio::sampleRates() const
{
  return m_catalog.sampleRates();
}

//--------------------------------------------------------------------------------------------------
// Input device parameters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the index of the currently selected input device.
 */
int IO::Drivers::Audio::selectedInputDevice() const
{
  return m_catalog.selectedInputDevice();
}

/**
 * @brief Returns the index of the currently selected input sample format.
 */
int IO::Drivers::Audio::selectedInputSampleFormat() const
{
  return m_catalog.selectedInputSampleFormat();
}

/**
 * @brief Returns the index of the currently selected input channel configuration.
 */
int IO::Drivers::Audio::selectedInputChannelConfiguration() const
{
  return m_catalog.selectedInputChannelConfiguration();
}

//--------------------------------------------------------------------------------------------------
// Output device parameters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the index of the currently selected output device.
 */
int IO::Drivers::Audio::selectedOutputDevice() const
{
  return m_catalog.selectedOutputDevice();
}

/**
 * @brief Returns the index of the currently selected output sample format.
 */
int IO::Drivers::Audio::selectedOutputSampleFormat() const
{
  return m_catalog.selectedOutputSampleFormat();
}

/**
 * @brief Returns the index of the currently selected output channel configuration.
 */
int IO::Drivers::Audio::selectedOutputChannelConfiguration() const
{
  return m_catalog.selectedOutputChannelConfiguration();
}

//--------------------------------------------------------------------------------------------------
// Input device parameter models
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a list of available input audio devices.
 */
QStringList IO::Drivers::Audio::inputDeviceList() const
{
  return m_catalog.inputDeviceList();
}

/**
 * @brief Returns the list of supported sample formats for the selected input device.
 */
QStringList IO::Drivers::Audio::inputSampleFormats() const
{
  return m_catalog.inputSampleFormats();
}

/**
 * @brief Returns the list of supported input channel configurations.
 */
QStringList IO::Drivers::Audio::inputChannelConfigurations() const
{
  return m_catalog.inputChannelConfigurations();
}

//--------------------------------------------------------------------------------------------------
// Output device parameter models
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a list of available output audio devices.
 */
QStringList IO::Drivers::Audio::outputDeviceList() const
{
  return m_catalog.outputDeviceList();
}

/**
 * @brief Returns a list of supported sample formats for the selected output device.
 */
QStringList IO::Drivers::Audio::outputSampleFormats() const
{
  return m_catalog.outputSampleFormats();
}

/**
 * @brief Returns a list of supported output channel configurations.
 */
QStringList IO::Drivers::Audio::outputChannelConfigurations() const
{
  return m_catalog.outputChannelConfigurations();
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

  m_catalog.setSelectedSampleRate(index);
  configureInput();
  persistSettings();
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

  m_catalog.setSelectedInputDevice(index);
  m_catalog.setSelectedSampleRate(-1);
  m_catalog.setSelectedInputSampleFormat(-1);
  m_catalog.setSelectedInputChannelConfiguration(-1);

  syncInputParameters();
  configureInput();
  persistSettings();
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

  m_catalog.setSelectedInputSampleFormat(index);
  configureInput();
  persistSettings();
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

  m_catalog.setSelectedInputChannelConfiguration(index);
  configureInput();
  persistSettings();
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

  m_catalog.setSelectedOutputDevice(index);
  m_catalog.setSelectedOutputSampleFormat(-1);
  m_catalog.setSelectedOutputChannelConfiguration(-1);

  syncOutputParameters();
  configureOutput();
  persistSettings();
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

  m_catalog.setSelectedOutputSampleFormat(index);
  configureOutput();
  persistSettings();
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

  m_catalog.setSelectedOutputChannelConfiguration(index);
  configureOutput();
  persistSettings();
}

//--------------------------------------------------------------------------------------------------
// UI helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Populates maps that associate MiniAudio constants with user-friendly strings.
 */
void IO::Drivers::Audio::generateLists()
{
  const QMap<ma_format, QString> formats = {
    { ma_format_u8, tr("Unsigned 8-bit")},
    {ma_format_s16,  tr("Signed 16-bit")},
    {ma_format_s24,  tr("Signed 24-bit")},
    {ma_format_s32,  tr("Signed 32-bit")},
    {ma_format_f32,   tr("Float 32-bit")}
  };
  const QMap<ma_channel, QString> configs = {
    {1,   tr("Mono")},
    {2, tr("Stereo")},
    {3,        "3.0"},
    {4,        "4.0"},
    {5,        "5.0"},
    {6,        "5.1"},
    {7,        "6.1"},
    {8,        "7.1"}
  };

  m_catalog.updateLabels(formats, configs, QStringLiteral(" ") + tr("channels"), tr(" channels"));

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

  const auto& caps = m_catalog.inputCapabilities()[m_catalog.selectedInputDevice()];
  if (caps.supportedSampleRates.isEmpty() || caps.supportedFormats.isEmpty()
      || caps.supportedChannelCounts.isEmpty()) {
    qWarning() << "Input capabilities for selected device are not populated";
    return;
  }

  if (m_normalization)
    m_catalog.setSelectedInputSampleFormat(
      AudioDeviceCatalog::bestNormalizedFormatIndex(caps.supportedFormats));

  const int rateMax    = caps.supportedSampleRates.size() - 1;
  const int formatMax  = caps.supportedFormats.size() - 1;
  const int channelMax = caps.supportedChannelCounts.size() - 1;

  m_catalog.setSelectedSampleRate(qBound(0, m_catalog.selectedSampleRate(), rateMax));
  m_catalog.setSelectedInputSampleFormat(
    qBound(0, m_catalog.selectedInputSampleFormat(), formatMax));
  m_catalog.setSelectedInputChannelConfiguration(
    qBound(0, m_catalog.selectedInputChannelConfiguration(), channelMax));

  const int sampleRate   = caps.supportedSampleRates[m_catalog.selectedSampleRate()];
  const ma_format format = caps.supportedFormats[m_catalog.selectedInputSampleFormat()];
  const int channels = caps.supportedChannelCounts[m_catalog.selectedInputChannelConfiguration()];

  m_config.sampleRate       = sampleRate;
  m_config.capture.format   = format;
  m_config.capture.channels = channels;

  syncInputParameters();
  publishCaptureFormat();
  Q_EMIT configurationChanged();
}

/**
 * @brief Retains the capture format, sample rate and normalization flag the QuickPlot builder
 *        sizes its audio lane from (spec 0077); the format is the miniaudio ma_format ordinal.
 */
void IO::Drivers::Audio::publishCaptureFormat()
{
  auto* bus = messageBus();
  if (!bus)
    return;

  bus->publishState<Core::Bus::AudioCaptureFormat>(static_cast<int>(m_config.capture.format),
                                                   static_cast<int>(m_config.sampleRate),
                                                   m_normalization);
}

/**
 * @brief Applies the selected output device configuration.
 */
void IO::Drivers::Audio::configureOutput()
{
  if (!validateOutput())
    return;

  const auto& caps = m_catalog.outputCapabilities()[m_catalog.selectedOutputDevice()];
  if (caps.supportedSampleRates.isEmpty() || caps.supportedFormats.isEmpty()
      || caps.supportedChannelCounts.isEmpty()) {
    qWarning() << "Output capabilities for selected device are not populated";
    return;
  }

  if (m_normalization)
    m_catalog.setSelectedOutputSampleFormat(
      AudioDeviceCatalog::bestNormalizedFormatIndex(caps.supportedFormats));

  const int formatMax  = caps.supportedFormats.size() - 1;
  const int channelMax = caps.supportedChannelCounts.size() - 1;

  m_catalog.setSelectedOutputSampleFormat(
    qBound(0, m_catalog.selectedOutputSampleFormat(), formatMax));
  m_catalog.setSelectedOutputChannelConfiguration(
    qBound(0, m_catalog.selectedOutputChannelConfiguration(), channelMax));

  const ma_format format = caps.supportedFormats[m_catalog.selectedOutputSampleFormat()];
  const int channels = caps.supportedChannelCounts[m_catalog.selectedOutputChannelConfiguration()];

  m_config.playback.format   = format;
  m_config.playback.channels = channels;

  syncOutputParameters();
  Q_EMIT configurationChanged();
}

//--------------------------------------------------------------------------------------------------
// Audio input data parsing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Drains captured PCM on the input worker thread and publishes it: a typed SampleBlock when
 *        the stream lane is active (spec 0051), the CSV text otherwise. The lane still renders that
 *        CSV for the terminal alone, so a stream source reads there as it did before the lane. The
 *        sample-clock resync is shared by both, so input jitter never shifts the sample timeline.
 */
void IO::Drivers::Audio::processInputBuffer()
{
  m_inputScratch.resize(0);

  QByteArray chunk;
  while (m_inputQueue.try_dequeue(chunk)) {
    m_inputScratch.append(chunk);
    chunk.resize(0);
    (void)m_inputPool.try_enqueue(std::move(chunk));
  }

  const QByteArray& raw = m_inputScratch;
  if (raw.isEmpty())
    return;

  const int channels       = m_config.capture.channels;
  const ma_format format   = m_config.capture.format;
  const int bytesPerSample = ma_get_bytes_per_sample(format);
  const int frameSize      = bytesPerSample * channels;
  if (frameSize <= 0 || channels <= 0 || raw.size() % frameSize != 0)
    return;

  const int totalFrames = raw.size() / frameSize;

  const auto frameStep =
    std::max(std::chrono::nanoseconds(1),
             std::chrono::duration_cast<std::chrono::nanoseconds>(
               std::chrono::duration<double>(1.0 / static_cast<double>(m_config.sampleRate))));

  const auto now       = IO::CapturedData::SteadyClock::now();
  const auto wallFirst = now - (frameStep * std::max(0, totalFrames - 1));
  if (!m_sampleClockValid || std::chrono::abs(m_nextSampleTime - wallFirst) > kAudioClockResync) {
    m_nextSampleTime   = wallFirst;
    m_sampleClockValid = true;
  }

  const auto timestamp  = m_nextSampleTime;
  m_nextSampleTime     += frameStep * totalFrames;

  const bool streamLane = m_streamLaneActive.load(std::memory_order_relaxed);
  if (streamLane)
    publishTypedBlock(raw, channels, format, totalFrames, timestamp, frameStep);

  m_csvBuffer.seek(0);

  if (m_normalization)
    renderNormalizedCsv(raw, channels, format, totalFrames);
  else
    renderCsv(raw, channels, format, totalFrames);

  m_csvStream.flush();
  const auto length = m_csvBuffer.pos();
  if (streamLane)
    publishConsoleData(m_csvData.left(length), timestamp, frameStep, totalFrames);
  else
    publishReceivedData(m_csvData.left(length), timestamp, frameStep, totalFrames);
}

/**
 * @brief Renders raw PCM into the CSV buffer as native-range values: integers for the integer
 *        formats, so a 32-bit sample keeps every bit that a float round trip would drop.
 */
void IO::Drivers::Audio::renderCsv(const QByteArray& raw,
                                   int channels,
                                   ma_format format,
                                   int totalFrames)
{
  const int bytesPerSample = ma_get_bytes_per_sample(format);
  const char* ptr          = raw.constData();
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
}

/**
 * @brief Renders raw PCM into the CSV buffer as normalized -1..1 floats, using the same affine
 *        terms as the typed lane so a stream source and a text source read identically.
 */
void IO::Drivers::Audio::renderNormalizedCsv(const QByteArray& raw,
                                             int channels,
                                             ma_format fmt,
                                             int totalFrames)
{
  float offset = 0.0f;
  float scale  = 1.0f;
  normalizationTerms(fmt, true, offset, scale);

  const int bytesPerSample = ma_get_bytes_per_sample(fmt);
  const char* ptr          = raw.constData();
  for (int i = 0; i < totalFrames; ++i) {
    for (int ch = 0; ch < channels; ++ch) {
      m_csvStream << (decodeSample(fmt, ptr) + offset) * scale;
      ptr += bytesPerSample;
      if (ch < channels - 1)
        m_csvStream << ',';
    }

    m_csvStream << '\n';
  }
}

/**
 * @brief Decodes raw PCM into a typed SampleBlock (interleaved float32, carrying the same numeric
 *        magnitudes the CSV lane emits under the current normalization setting, so the two lanes
 *        never disagree) and publishes it for the stream worker (spec 0051 R6).
 */
void IO::Drivers::Audio::publishTypedBlock(const QByteArray& raw,
                                           int channels,
                                           ma_format format,
                                           int totalFrames,
                                           CapturedData::SteadyTimePoint timestamp,
                                           std::chrono::nanoseconds frameStep)
{
  SS_ASSERT(channels > 0, return);
  SS_ASSERT(totalFrames > 0, return);

  auto block      = std::make_shared<SampleBlock>();
  block->channels = channels;
  block->frames   = totalFrames;
  block->t0       = timestamp;
  block->dt       = frameStep;
  block->samples.resize(static_cast<std::size_t>(totalFrames) * static_cast<std::size_t>(channels));

  float offset = 0.0f;
  float scale  = 1.0f;
  normalizationTerms(format, m_normalization, offset, scale);

  const int bytesPerSample = ma_get_bytes_per_sample(format);
  const char* ptr          = raw.constData();
  float* out               = block->samples.data();
  const qsizetype count    = block->frames * channels;

  for (qsizetype i = 0; i < count; ++i, ptr += bytesPerSample)
    out[i] = (decodeSample(format, ptr) + offset) * scale;

  publishSampleBlock(block);
}

/**
 * @brief Audio captures native typed samples, so it is a stream source by default (spec 0051 R6).
 */
bool IO::Drivers::Audio::isStreamCapable() const noexcept
{
  return true;
}

/**
 * @brief Returns whether the stream lane currently consumes this driver's blocks.
 */
bool IO::Drivers::Audio::streamLaneActive() const noexcept
{
  return m_streamLaneActive.load(std::memory_order_relaxed);
}

/**
 * @brief Selects the publish lane: typed SampleBlocks (true) or the legacy CSV text path.
 *        Written by ConnectionManager at connect/config time, read by the input worker.
 */
void IO::Drivers::Audio::setStreamLaneActive(bool active) noexcept
{
  m_streamLaneActive.store(active, std::memory_order_relaxed);
}

//--------------------------------------------------------------------------------------------------
// Device discovery functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Suspends device discovery while a session is live: ma_context_get_devices() is a
 *        syscall-heavy enumeration on the GUI thread, and the pickers it feeds are locked for the
 *        duration anyway. Resuming refreshes once so the lists are current when they unlock.
 */
void IO::Drivers::Audio::setDiscoveryPaused(const bool paused)
{
  if (m_discoveryPaused == paused)
    return;

  m_discoveryPaused = paused;
  if (!paused)
    refreshAudioDevices();
}

/**
 * @brief Refreshes the list of available audio input/output devices. A device that vanished while
 *        a session runs tears the session down BEFORE its list is replaced, so the connection
 *        manager still sees the selection it was streaming from.
 */
void IO::Drivers::Audio::refreshAudioDevices()
{
  if (!m_init || m_discoveryPaused)
    return;

  AudioDeviceCatalog::Enumeration probe;
  if (!m_catalog.enumerate(probe))
    return;

  if (updateInputDevices(probe)) {
    Q_EMIT inputSettingsChanged();
    Q_EMIT configurationChanged();
  }

  if (updateOutputDevices(probe)) {
    Q_EMIT outputSettingsChanged();
    Q_EMIT configurationChanged();
  }

  if (m_catalog.selectedInputDevice() < 0 && !probe.inputs.isEmpty()) {
    m_catalog.setSelectedInputDevice(0);
    m_catalog.setSelectedSampleRate(-1);
    m_catalog.setSelectedInputSampleFormat(-1);
    m_catalog.setSelectedInputChannelConfiguration(-1);
    syncInputParameters();
    configureInput();
  }

  if (m_catalog.selectedOutputDevice() < 0 && !probe.outputs.isEmpty()) {
    m_catalog.setSelectedOutputDevice(0);
    m_catalog.setSelectedOutputSampleFormat(-1);
    m_catalog.setSelectedOutputChannelConfiguration(-1);
    syncOutputParameters();
    configureOutput();
  }
}

/**
 * @brief Adopts the enumerated input devices, dropping a live session first when the device it
 *        streams from is gone; true when the list changed.
 */
bool IO::Drivers::Audio::updateInputDevices(const AudioDeviceCatalog::Enumeration& probe)
{
  if (!m_isOpen)
    return m_catalog.replaceInputDevices(probe, false);

  if (m_catalog.inputSelectionPresent(probe))
    return false;

  dropSessionForLostDevice();
  return m_catalog.replaceInputDevices(probe, true);
}

/**
 * @brief Adopts the enumerated output devices. A capture-only session is NOT dropped when the
 *        output list changes: an absent output was read as "the session's device vanished", so
 *        unplugging a headset killed a live microphone capture, and a machine with no output at
 *        all dropped every session a second after it opened.
 */
bool IO::Drivers::Audio::updateOutputDevices(const AudioDeviceCatalog::Enumeration& probe)
{
  const bool playbackInUse =
    m_isOpen && m_config.playback.channels > 0 && m_catalog.outputDeviceSelected();
  if (!playbackInUse)
    return m_catalog.replaceOutputDevices(probe, false);

  if (m_catalog.outputSelectionPresent(probe))
    return false;

  dropSessionForLostDevice();
  return m_catalog.replaceOutputDevices(probe, true);
}

/**
 * @brief Tears the live session down because the device it streams from is gone. The single
 *        connection-manager resolution of the discovery path lives here.
 */
void IO::Drivers::Audio::dropSessionForLostDevice()
{
  static auto& connectionManager = IO::ConnectionManager::instance();
  connectionManager.disconnectDevice(this);
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

  m_catalog.syncInputSelection(
    m_config.sampleRate, m_config.capture.format, m_config.capture.channels);

  Q_EMIT inputSettingsChanged();
}

/**
 * @brief Synchronizes internal output parameters with the actual device config.
 */
void IO::Drivers::Audio::syncOutputParameters()
{
  if (!validateOutput())
    return;

  m_catalog.syncOutputSelection(m_config.playback.format, m_config.playback.channels);

  Q_EMIT outputSettingsChanged();
}

//--------------------------------------------------------------------------------------------------
// Audio callback function
//--------------------------------------------------------------------------------------------------

/**
 * @brief Audio callback handler for processing input and output streams; runs
 * on the realtime audio thread, so input is handed to the main-thread consumer
 * via the SPSC queue and dropped on full rather than blocking, to avoid
 * underrun.
 */
void IO::Drivers::Audio::handleCallback(void* output, const void* input, ma_uint32 frameCount)
{
  const ma_format format         = m_rtCaptureFormat.load(std::memory_order_acquire);
  const ma_uint32 channels       = m_rtCaptureChannels.load(std::memory_order_relaxed);
  const ma_format playbackFormat = m_rtPlaybackFormat.load(std::memory_order_relaxed);
  const ma_uint32 playbackChans  = m_rtPlaybackChannels.load(std::memory_order_relaxed);
  const ma_uint32 bytesPerSample = ma_get_bytes_per_sample(format);
  const ma_uint32 bytesPerFrame  = bytesPerSample * channels;

  if (input && channels > 0 && format != ma_format_unknown) {
    const auto bytes = static_cast<qsizetype>(frameCount * bytesPerFrame);

    QByteArray slot;
    if (!m_inputPool.try_dequeue(slot)) [[unlikely]] {
      m_inputDrops.fetch_add(1, std::memory_order_relaxed);
    }

    else {
      slot.resize(bytes);
      std::memcpy(slot.data(), input, static_cast<std::size_t>(bytes));
      if (!m_inputQueue.try_enqueue(std::move(slot))) [[unlikely]]
        m_inputDrops.fetch_add(1, std::memory_order_relaxed);
    }
  }

  if (output && playbackChans > 0 && playbackFormat != ma_format_unknown) {
    char* out                        = reinterpret_cast<char*>(output);
    const ma_uint32 outBytesPerFrame = ma_get_bytes_per_sample(playbackFormat) * playbackChans;
    (void)m_playbackRing.read(out, static_cast<qsizetype>(frameCount * outBytesPerFrame));
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
  inDev.value   = m_catalog.selectedInputDevice();
  inDev.options = inputDeviceList();
  props.append(inDev);

  IO::DriverProperty rate;
  rate.key     = QStringLiteral("sampleRate");
  rate.label   = tr("Sample Rate");
  rate.type    = IO::DriverProperty::ComboBox;
  rate.value   = m_catalog.selectedSampleRate();
  rate.options = sampleRates();
  props.append(rate);

  IO::DriverProperty norm;
  norm.key         = QStringLiteral("normalization");
  norm.label       = tr("Normalization");
  norm.description = tr("Publish samples as floats in the -1.0 to 1.0 range");
  norm.type        = IO::DriverProperty::CheckBox;
  norm.value       = m_normalization;
  props.append(norm);

  if (!m_normalization) {
    IO::DriverProperty fmt;
    fmt.key     = QStringLiteral("inputFormat");
    fmt.label   = tr("Sample Format");
    fmt.type    = IO::DriverProperty::ComboBox;
    fmt.value   = m_catalog.selectedInputSampleFormat();
    fmt.options = inputSampleFormats();
    props.append(fmt);
  }

  IO::DriverProperty ch;
  ch.key     = QStringLiteral("inputChannels");
  ch.label   = tr("Channels");
  ch.type    = IO::DriverProperty::ComboBox;
  ch.value   = m_catalog.selectedInputChannelConfiguration();
  ch.options = inputChannelConfigurations();
  props.append(ch);

  return props;
}

/**
 * @brief Applies a single Audio configuration change by key.
 */
void IO::Drivers::Audio::setDriverProperty(const QString& key, const QVariant& value)
{
  if (key == QLatin1String("inputDevice")) {
    setSelectedInputDevice(value.toInt());
    return;
  }

  if (key == QLatin1String("sampleRate")) {
    setSelectedSampleRate(value.toInt());
    return;
  }

  if (key == QLatin1String("normalization")) {
    setNormalization(value.toBool());
    return;
  }

  if (key == QLatin1String("inputFormat")) {
    setSelectedInputSampleFormat(value.toInt());
    return;
  }

  if (key == QLatin1String("inputChannels"))
    setSelectedInputChannelConfiguration(value.toInt());
}

/**
 * @brief Returns a JSON identifier for the currently selected audio input device.
 */
QJsonObject IO::Drivers::Audio::deviceIdentifier() const
{
  return m_catalog.deviceIdentifier();
}

/**
 * @brief Selects the input device and sub-settings matching a saved identifier.
 */
bool IO::Drivers::Audio::selectByIdentifier(const QJsonObject& id)
{
  if (id.isEmpty())
    return false;

  const auto saved_name = id.value(QStringLiteral("inputDeviceName")).toString();
  if (saved_name.isEmpty())
    return false;

  const int device_idx = m_catalog.indexOfInputDeviceNamed(saved_name);
  if (device_idx < 0)
    return false;

  setSelectedInputDevice(device_idx);

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

  const int saved_ch = id.value(QStringLiteral("channelCount")).toInt();
  if (saved_ch > 0 && validateInput()) {
    const auto& caps = m_catalog.inputCapabilities()[m_catalog.selectedInputDevice()];
    for (int i = 0; i < caps.supportedChannelCounts.size(); ++i) {
      if (caps.supportedChannelCounts.at(i) == saved_ch) {
        setSelectedInputChannelConfiguration(i);
        break;
      }
    }
  }

  return true;
}

/**
 * @brief Atomically restores Audio settings, resolving sub-settings by value. Every index is
 *        resolved before the live configuration is touched, so a settings blob naming a device
 *        this machine does not have leaves the current capture format alone.
 */
void IO::Drivers::Audio::applyConnectionSettings(const QJsonObject& settings)
{
  if (settings.isEmpty() || isOpen())
    return;

  if (settings.contains(QStringLiteral("normalization")))
    setNormalization(settings.value(QStringLiteral("normalization")).toBool());

  const auto deviceId = settings.value(QStringLiteral("deviceId")).toObject();
  if (!m_catalog.applySavedSelection(settings, deviceId, m_normalization))
    return;

  const auto& caps = m_catalog.inputCapabilities()[m_catalog.selectedInputDevice()];

  m_config.sampleRate     = caps.supportedSampleRates[m_catalog.selectedSampleRate()];
  m_config.capture.format = caps.supportedFormats[m_catalog.selectedInputSampleFormat()];
  m_config.capture.channels =
    caps.supportedChannelCounts[m_catalog.selectedInputChannelConfiguration()];

  Q_EMIT inputSettingsChanged();
  Q_EMIT configurationChanged();
}

/**
 * @brief Writes the current selection to QSettings using stable identifiers.
 */
void IO::Drivers::Audio::persistSettings()
{
  m_settings.setValue(QStringLiteral("AudioDriver/normalization"), m_normalization);
  m_catalog.persistSelection(m_settings);
}

/**
 * @brief Restores the last-used selection from QSettings, falling back to defaults.
 */
void IO::Drivers::Audio::restoreSettings()
{
  m_normalization = m_settings.value(QStringLiteral("AudioDriver/normalization"), true).toBool();
  m_catalog.restoreSelection(m_settings);
}
