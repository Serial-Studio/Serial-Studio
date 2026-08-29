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

#pragma once

#include <atomic>
#include <QBuffer>
#include <QObject>
#include <QSettings>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <QVector>

#include "IO/Drivers/Audio/AudioDeviceCatalog.h"
#include "IO/HAL_Driver.h"
#include "ThirdParty/miniaudio.h"
#include "ThirdParty/readerwriterqueue.h"

namespace IO {
namespace Drivers {
/**
 * @brief HAL driver that captures and plays audio via miniaudio.
 */
class Audio : public HAL_Driver {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool normalization
             READ normalization
             WRITE setNormalization
             NOTIFY normalizationChanged)
  Q_PROPERTY(int selectedInputDevice
             READ selectedInputDevice
             WRITE setSelectedInputDevice
             NOTIFY inputSettingsChanged)
  Q_PROPERTY(int selectedSampleRate
             READ selectedSampleRate
             WRITE setSelectedSampleRate
             NOTIFY inputSettingsChanged)
  Q_PROPERTY(QStringList sampleRates
             READ sampleRates
             NOTIFY inputSettingsChanged)
  Q_PROPERTY(int selectedInputSampleFormat
             READ selectedInputSampleFormat
             WRITE setSelectedInputSampleFormat
             NOTIFY inputSettingsChanged)
  Q_PROPERTY(int selectedInputChannelConfiguration
             READ selectedInputChannelConfiguration
             WRITE setSelectedInputChannelConfiguration
             NOTIFY inputSettingsChanged)
  Q_PROPERTY(QStringList inputDeviceList
             READ inputDeviceList
             NOTIFY inputSettingsChanged)
  Q_PROPERTY(QStringList inputSampleFormats
             READ inputSampleFormats
             NOTIFY inputSettingsChanged)
  Q_PROPERTY(QStringList inputChannelConfigurations
             READ inputChannelConfigurations
             NOTIFY inputSettingsChanged)
  Q_PROPERTY(int selectedOutputDevice
             READ selectedOutputDevice
             WRITE setSelectedOutputDevice
             NOTIFY outputSettingsChanged)
  Q_PROPERTY(int selectedOutputSampleFormat
             READ selectedOutputSampleFormat
             WRITE setSelectedOutputSampleFormat
             NOTIFY outputSettingsChanged)
  Q_PROPERTY(int selectedOutputChannelConfiguration
             READ selectedOutputChannelConfiguration
             WRITE setSelectedOutputChannelConfiguration
             NOTIFY outputSettingsChanged)
  Q_PROPERTY(QStringList outputDeviceList
             READ outputDeviceList
             NOTIFY outputSettingsChanged)
  Q_PROPERTY(QStringList outputSampleFormats
             READ outputSampleFormats
             NOTIFY outputSettingsChanged)
  Q_PROPERTY(QStringList outputChannelConfigurations
             READ outputChannelConfigurations
             NOTIFY outputSettingsChanged)
  // clang-format on

signals:
  void inputSettingsChanged();
  void normalizationChanged();
  void outputSettingsChanged();

public:
  explicit Audio();
  ~Audio();

  Audio(Audio&&)                 = delete;
  Audio(const Audio&)            = delete;
  Audio& operator=(Audio&&)      = delete;
  Audio& operator=(const Audio&) = delete;

public:
  void closeDevice();
  void close() override;

  [[nodiscard]] bool normalization() const noexcept;
  [[nodiscard]] bool isOpen() const noexcept override;
  [[nodiscard]] bool isStreamCapable() const noexcept override;
  [[nodiscard]] bool streamLaneActive() const noexcept;
  void setStreamLaneActive(bool active) noexcept;
  [[nodiscard]] bool isReadable() const noexcept override;
  [[nodiscard]] bool isWritable() const noexcept override;
  [[nodiscard]] bool configurationOk() const noexcept override;
  [[nodiscard]] qint64 write(const QByteArray& data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;
  bool selectByIdentifier(const QJsonObject& id) override;
  void applyConnectionSettings(const QJsonObject& settings) override;
  [[nodiscard]] QJsonObject deviceIdentifier() const override;
  [[nodiscard]] QList<IO::DriverProperty> driverProperties() const override;

  [[nodiscard]] inline const ma_device_config& config() const { return m_config; }

  [[nodiscard]] inline bool backendReady() const noexcept { return m_init; }

  [[nodiscard]] int selectedSampleRate() const;
  [[nodiscard]] QStringList sampleRates() const;

  [[nodiscard]] int selectedInputDevice() const;
  [[nodiscard]] int selectedInputSampleFormat() const;
  [[nodiscard]] int selectedInputChannelConfiguration() const;

  [[nodiscard]] int selectedOutputDevice() const;
  [[nodiscard]] int selectedOutputSampleFormat() const;
  [[nodiscard]] int selectedOutputChannelConfiguration() const;

  [[nodiscard]] QStringList inputDeviceList() const;
  [[nodiscard]] QStringList inputSampleFormats() const;
  [[nodiscard]] QStringList inputChannelConfigurations() const;

  [[nodiscard]] QStringList outputDeviceList() const;
  [[nodiscard]] QStringList outputSampleFormats() const;
  [[nodiscard]] QStringList outputChannelConfigurations() const;

public slots:
  void setDriverProperty(const QString& key, const QVariant& value) override;
  void setNormalization(bool enabled);
  void setDiscoveryPaused(const bool paused);
  void setSelectedSampleRate(int index);

  void setSelectedInputDevice(int index);
  void setSelectedInputSampleFormat(int index);
  void setSelectedInputChannelConfiguration(int index);

  void setSelectedOutputDevice(int index);
  void setSelectedOutputSampleFormat(int index);
  void setSelectedOutputChannelConfiguration(int index);

private slots:
  void generateLists();
  void configureInput();
  void configureOutput();
  void onBackendStopped();
  void processInputBuffer();
  void refreshAudioDevices();
  void syncInputParameters();
  void syncOutputParameters();

private:
  [[nodiscard]] inline bool validateInput() const { return m_catalog.validateInput(); }

  [[nodiscard]] inline bool validateOutput() const { return m_catalog.validateOutput(); }

  void dropSessionForLostDevice();

  [[nodiscard]] bool updateInputDevices(const AudioDeviceCatalog::Enumeration& probe);
  [[nodiscard]] bool updateOutputDevices(const AudioDeviceCatalog::Enumeration& probe);

  void handleCallback(void* output, const void* input, ma_uint32 frameCount);
  static void callback(ma_device* device, void* output, const void* input, ma_uint32 frameCount);
  static void notificationCallback(const ma_device_notification* notification);

  void applyPlatformAudioConfig();
  void renderCsv(const QByteArray& raw, int channels, ma_format format, int totalFrames);
  void renderNormalizedCsv(const QByteArray& raw, int channels, ma_format fmt, int totalFrames);
  void publishTypedBlock(const QByteArray& raw,
                         int channels,
                         ma_format format,
                         int totalFrames,
                         CapturedData::SteadyTimePoint timestamp,
                         std::chrono::nanoseconds frameStep);
  void configureCaptureFormat(QIODevice::OpenMode mode);
  [[nodiscard]] bool configurePlaybackFormat(QIODevice::OpenMode mode);
  void startInputWorker();

  void persistSettings();
  void restoreSettings();

private:
  bool m_init;
  bool m_isOpen;
  bool m_normalization;
  bool m_discoveryPaused;

  ma_device m_device;
  ma_context m_context;
  ma_device_config m_config;

  AudioDeviceCatalog m_catalog;

  QSettings m_settings;
  moodycamel::ReaderWriterQueue<QByteArray> m_inputQueue;

  mutable QBuffer m_csvBuffer;
  mutable QByteArray m_csvData;
  mutable QTextStream m_csvStream;

  moodycamel::ReaderWriterQueue<QVector<quint8>> m_outputQueue;

  QTimer* m_inputWorkerTimer;
  QThread m_inputWorkerThread;

  bool m_sampleClockValid;
  CapturedData::SteadyTimePoint m_nextSampleTime;

  std::atomic<bool> m_stopNotifyArmed;

  // code-verify off
  // Written at connect/config time only (no steady-state cross-core writes); sharing the line
  // with the stop-notify flag is harmless.
  std::atomic<bool> m_streamLaneActive;
  // code-verify on

  // code-verify off
  // Written once in open() before the RT thread starts, then read-only; there are no concurrent
  // cross-core writes, so sharing one cache line is intended (a single transfer to the RT core).
  std::atomic<ma_format> m_rtCaptureFormat;
  std::atomic<ma_format> m_rtPlaybackFormat;
  std::atomic<ma_uint32> m_rtCaptureChannels;
  std::atomic<ma_uint32> m_rtPlaybackChannels;
  // code-verify on
};
}  // namespace Drivers
}  // namespace IO
