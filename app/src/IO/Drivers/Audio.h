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

#include <QMap>
#include <QVector>
#include <QObject>
#include <QAudioFormat>
#include <QAudioDevice>
#include <QElapsedTimer>

#include "IO/HAL_Driver.h"

class QAudioSink;
class QAudioSource;

namespace IO
{
namespace Drivers
{
/**
 * @class IO::Drivers::Audio
 * @brief High-level audio I/O driver for input/output stream capture and
 *        playback.
 *
 * This class handles configuration, device enumeration, data format selection,
 * and streaming for both input (e.g., microphone) and output (e.g., speakers).
 *
 * It supports dynamic reconfiguration, format negotiation, and real-time audio
 * data acquisition using Qt Multimedia (QAudioSource/QAudioSink).
 *
 * Key features:
 * - Selectable input/output devices and formats
 * - Buffered audio stream handling
 * - CSV-formatted audio sample export for downstream processing
 * - Configurable timestamp injection for precise data alignment
 * - Safe fallback if output stream fails, allowing input-only capture
 *
 * Used primarily in Serial Studio to enable audio-based signal analysis or
 * visualization.
 */
class Audio : public HAL_Driver
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int bufferSizeIdx
             READ bufferSizeIdx
             WRITE setBufferSizeIdx
             NOTIFY bufferSizeChanged)
  Q_PROPERTY(QAudioFormat inputFormat
             READ inputFormat
             NOTIFY inputSettingsChanged)
  Q_PROPERTY(QAudioFormat outputFormat
             READ outputFormat
             NOTIFY outputSettingsChanged)
  Q_PROPERTY(int selectedInputDevice
             READ selectedInputDevice
             WRITE setSelectedInputDevice
             NOTIFY inputSettingsChanged)
  Q_PROPERTY(int selectedInputSampleRate
             READ selectedInputSampleRate
             WRITE setSelectedInputSampleRate
             NOTIFY inputSettingsChanged)
  Q_PROPERTY(int selectedInputSampleFormat
             READ selectedInputSampleFormat
             WRITE setSelectedInputSampleFormat
             NOTIFY inputSettingsChanged)
  Q_PROPERTY(int selectedInputChannelConfiguration
             READ selectedInputChannelConfiguration
             WRITE setSelectedInputChannelConfiguration
             NOTIFY inputSettingsChanged)
  Q_PROPERTY(int selectedOutputDevice
             READ selectedOutputDevice
             WRITE setSelectedOutputDevice
             NOTIFY outputSettingsChanged)
  Q_PROPERTY(int selectedOutputSampleRate
             READ selectedOutputSampleRate
             WRITE setSelectedOutputSampleRate
             NOTIFY outputSettingsChanged)
  Q_PROPERTY(int selectedOutputSampleFormat
             READ selectedOutputSampleFormat
             WRITE setSelectedOutputSampleFormat
             NOTIFY outputSettingsChanged)
  Q_PROPERTY(int selectedOutputChannelConfiguration
             READ selectedOutputChannelConfiguration
             WRITE setSelectedOutputChannelConfiguration
             NOTIFY outputSettingsChanged)
  Q_PROPERTY(QStringList inputDeviceList
             READ inputDeviceList
             NOTIFY inputSettingsChanged)
  Q_PROPERTY(QStringList inputSampleRates
             READ inputSampleRates
             NOTIFY inputSettingsChanged)
  Q_PROPERTY(QStringList inputSampleFormats
             READ inputSampleFormats
             NOTIFY inputSettingsChanged)
  Q_PROPERTY(QStringList inputChannelConfigurations
             READ inputChannelConfigurations
             NOTIFY inputSettingsChanged)
  Q_PROPERTY(QStringList outputDeviceList
             READ outputDeviceList
             NOTIFY outputSettingsChanged)
  Q_PROPERTY(QStringList outputSampleRates
             READ outputSampleRates
             NOTIFY outputSettingsChanged)
  Q_PROPERTY(QStringList outputSampleFormats
             READ outputSampleFormats
             NOTIFY outputSettingsChanged)
  Q_PROPERTY(QStringList outputChannelConfigurations
             READ outputChannelConfigurations
             NOTIFY outputSettingsChanged)
  Q_PROPERTY(QStringList bufferSizes
             READ bufferSizes
             CONSTANT)
  // clang-format on

signals:
  void bufferSizeChanged();
  void inputSettingsChanged();
  void outputSettingsChanged();

private:
  explicit Audio();
  Audio(Audio &&) = delete;
  Audio(const Audio &) = delete;
  Audio &operator=(Audio &&) = delete;
  Audio &operator=(const Audio &) = delete;

public:
  static Audio &instance();

  void close() override;

  [[nodiscard]] bool isOpen() const override;
  [[nodiscard]] bool isReadable() const override;
  [[nodiscard]] bool isWritable() const override;
  [[nodiscard]] bool configurationOk() const override;
  [[nodiscard]] quint64 write(const QByteArray &data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;

  [[nodiscard]] QAudioFormat inputFormat() const;
  [[nodiscard]] QAudioFormat outputFormat() const;

  [[nodiscard]] const QAudioDevice &inputDevice() const;
  [[nodiscard]] const QAudioDevice &outputDevice() const;

  [[nodiscard]] int selectedInputDevice() const;
  [[nodiscard]] int selectedInputSampleRate() const;
  [[nodiscard]] int selectedInputSampleFormat() const;
  [[nodiscard]] int selectedInputChannelConfiguration() const;

  [[nodiscard]] int selectedOutputDevice() const;
  [[nodiscard]] int selectedOutputSampleRate() const;
  [[nodiscard]] int selectedOutputSampleFormat() const;
  [[nodiscard]] int selectedOutputChannelConfiguration() const;

  [[nodiscard]] int bufferSize() const;
  [[nodiscard]] int bufferSizeIdx() const;
  [[nodiscard]] QStringList bufferSizes() const;

  [[nodiscard]] QStringList inputDeviceList() const;
  [[nodiscard]] QStringList inputSampleRates() const;
  [[nodiscard]] QStringList inputSampleFormats() const;
  [[nodiscard]] QStringList inputChannelConfigurations() const;

  [[nodiscard]] QStringList outputDeviceList() const;
  [[nodiscard]] QStringList outputSampleRates() const;
  [[nodiscard]] QStringList outputSampleFormats() const;
  [[nodiscard]] QStringList outputChannelConfigurations() const;

  [[nodiscard]] const QVector<QAudioDevice> &inputDevices() const;
  [[nodiscard]] const QVector<QAudioDevice> &outputDevices() const;

public slots:
  void setBufferSizeIdx(int index);

  void setSelectedInputDevice(int index);
  void setSelectedInputSampleRate(int index);
  void setSelectedInputSampleFormat(int index);
  void setSelectedInputChannelConfiguration(int index);

  void setSelectedOutputDevice(int index);
  void setSelectedOutputSampleRate(int index);
  void setSelectedOutputSampleFormat(int index);
  void setSelectedOutputChannelConfiguration(int index);

private slots:
  void generateLists();
  void configureInput();
  void configureOutput();
  void onInputReadyRead();
  void refreshAudioInputs();
  void refreshAudioOutputs();
  void syncInputParameters();
  void syncOutputParameters();

private:
  QStringList getSampleRates(const QAudioDevice &device) const;
  QStringList getSampleFormats(const QAudioDevice &device) const;
  QStringList getChannelConfigurations(const QAudioDevice &device) const;

private:
  bool m_isOpen;
  int m_bufferSizeIdx;

  int m_selectedInputDevice;
  int m_selectedInputSampleRate;
  int m_selectedInputSampleFormat;
  int m_selectedInputChannelConfiguration;

  int m_selectedOutputDevice;
  int m_selectedOutputSampleRate;
  int m_selectedOutputSampleFormat;
  int m_selectedOutputChannelConfiguration;

  QAudioFormat m_inputFormat;
  QAudioFormat m_outputFormat;

  QVector<QAudioDevice> m_inputDevices;
  QVector<QAudioDevice> m_outputDevices;

  QAudioSink *m_sink;
  QAudioSource *m_source;
  QIODevice *m_inputStream;
  QIODevice *m_outputStream;

  QMap<QAudioFormat::SampleFormat, QString> m_sampleFormats;
  QMap<QAudioFormat::ChannelConfig, QString> m_knownConfigs;
};
} // namespace Drivers
} // namespace IO
