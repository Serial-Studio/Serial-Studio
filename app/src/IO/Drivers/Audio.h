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

//------------------------------------------------------------------------------
// Class declaration & Qt Libs
//------------------------------------------------------------------------------

#include <QMap>
#include <QMutex>
#include <QTimer>
#include <QThread>
#include <QVector>
#include <QObject>

#include <QBuffer>
#include <QTextStream>

#include "IO/HAL_Driver.h"
#include "ThirdParty/miniaudio.h"

namespace IO
{
namespace Drivers
{
class Audio : public HAL_Driver
{
  // clang-format off
  Q_OBJECT
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
  void outputSettingsChanged();

private:
  explicit Audio();
  Audio(Audio &&) = delete;
  Audio(const Audio &) = delete;
  Audio &operator=(Audio &&) = delete;
  Audio &operator=(const Audio &) = delete;

  ~Audio();

public:
  typedef struct
  {
    QList<int> supportedSampleRates;
    QList<ma_format> supportedFormats;
    QList<int> supportedChannelCounts;
  } AudioDeviceInfo;

  static Audio &instance();

  void closeDevice();
  void close() override;

  [[nodiscard]] bool isOpen() const override;
  [[nodiscard]] bool isReadable() const override;
  [[nodiscard]] bool isWritable() const override;
  [[nodiscard]] bool configurationOk() const override;
  [[nodiscard]] quint64 write(const QByteArray &data) override;
  [[nodiscard]] bool open(const QIODevice::OpenMode mode) override;

  [[nodiscard]] inline const ma_device_config &config() const
  {
    return m_config;
  }

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
  void processInputBuffer();
  void refreshAudioDevices();
  void syncInputParameters();
  void syncOutputParameters();

private:
  inline bool validateInput() const
  {
    return m_init && m_selectedInputDevice >= 0
           && m_selectedInputDevice < m_inputDevices.size()
           && m_selectedInputDevice < m_inputCapabilities.size();
  }

  inline bool validateOutput() const
  {
    return m_init && m_selectedOutputDevice >= 0
           && m_selectedOutputDevice < m_outputDevices.size()
           && m_selectedOutputDevice < m_outputCapabilities.size();
  }

  void handleCallback(void *output, const void *input, ma_uint32 frameCount);
  static void callback(ma_device *device, void *output, const void *input,
                       ma_uint32 frameCount);

private:
  bool m_init;
  bool m_isOpen;

  int m_selectedSampleRate;

  int m_selectedInputDevice;
  int m_selectedInputSampleFormat;
  int m_selectedInputChannelConfiguration;

  int m_selectedOutputDevice;
  int m_selectedOutputSampleFormat;
  int m_selectedOutputChannelConfiguration;

  QMap<ma_format, QString> m_sampleFormats;
  QMap<ma_channel, QString> m_knownConfigs;

  ma_device m_device;
  ma_context m_context;
  ma_device_config m_config;
  QVector<ma_device_info> m_inputDevices;
  QVector<ma_device_info> m_outputDevices;

  QMutex m_inputBufferLock;
  QByteArray m_rawInput;

  mutable QBuffer m_csvBuffer;
  mutable QByteArray m_csvData;
  mutable QTextStream m_csvStream;

  QMutex m_outputBufferLock;
  QVector<QVector<quint8>> m_outputQueue;

  QTimer *m_inputWorkerTimer;
  QThread m_inputWorkerThread;

  QVector<AudioDeviceInfo> m_inputCapabilities;
  QVector<AudioDeviceInfo> m_outputCapabilities;
};
} // namespace Drivers
} // namespace IO
