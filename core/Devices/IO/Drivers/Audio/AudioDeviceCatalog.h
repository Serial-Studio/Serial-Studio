/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
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

#include <QJsonObject>
#include <QMap>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVector>

#include "ThirdParty/miniaudio.h"

namespace IO {
namespace Drivers {

/**
 * @brief The audio driver's device model: what the backend offers (devices and the sample rates,
 *        formats and channel counts each reports) and which of those the user picked, persisted by
 *        stable identifier rather than index so an unplugged interface cannot retarget a session.
 *        It owns no device, starts no stream, resolves no singleton and never emits.
 */
class AudioDeviceCatalog {
public:
  /**
   * @brief What one device reports it can do, sorted ascending so an index into any of the three
   *        lists is a stable selection.
   */
  struct AudioDeviceInfo {
    QList<int> supportedSampleRates;
    QList<ma_format> supportedFormats;
    QList<int> supportedChannelCounts;
  };

  /**
   * @brief One backend enumeration pass. Kept separate from the adopted lists so the driver can
   *        decide what a vanished device means (it may have to tear a live session down first)
   *        before the catalog replaces anything.
   */
  struct Enumeration {
    QVector<ma_device_info> inputs;
    QVector<ma_device_info> outputs;
  };

public:
  explicit AudioDeviceCatalog(ma_context* context, const bool& backendReady);

  AudioDeviceCatalog(AudioDeviceCatalog&&)                 = delete;
  AudioDeviceCatalog(const AudioDeviceCatalog&)            = delete;
  AudioDeviceCatalog& operator=(AudioDeviceCatalog&&)      = delete;
  AudioDeviceCatalog& operator=(const AudioDeviceCatalog&) = delete;

  void restoreSelection(QSettings& settings);
  void setSelectedSampleRate(const int index);
  void setSelectedInputDevice(const int index);
  void setSelectedOutputDevice(const int index);
  void persistSelection(QSettings& settings) const;
  void setSelectedInputSampleFormat(const int index);
  void setSelectedOutputSampleFormat(const int index);
  void setSelectedInputChannelConfiguration(const int index);
  void setSelectedOutputChannelConfiguration(const int index);
  void syncOutputSelection(const ma_format format, const int channels);
  void syncInputSelection(const int sampleRate, const ma_format format, const int channels);
  void updateLabels(const QMap<ma_format, QString>& formats,
                    const QMap<ma_channel, QString>& channelConfigs,
                    const QString& inputChannelSuffix,
                    const QString& outputChannelSuffix);

  [[nodiscard]] bool validateInput() const;
  [[nodiscard]] bool validateOutput() const;
  [[nodiscard]] bool enumerate(Enumeration& probe) const;
  [[nodiscard]] bool inputSelectionPresent(const Enumeration& probe) const;
  [[nodiscard]] bool outputSelectionPresent(const Enumeration& probe) const;
  [[nodiscard]] bool outputDeviceSelected() const;
  [[nodiscard]] bool replaceInputDevices(const Enumeration& probe, const bool force);
  [[nodiscard]] bool replaceOutputDevices(const Enumeration& probe, const bool force);
  [[nodiscard]] bool applySavedSelection(const QJsonObject& settings,
                                         const QJsonObject& deviceId,
                                         const bool normalization);

  [[nodiscard]] int selectedSampleRate() const;
  [[nodiscard]] int selectedInputDevice() const;
  [[nodiscard]] int selectedOutputDevice() const;
  [[nodiscard]] int selectedInputSampleFormat() const;
  [[nodiscard]] int selectedOutputSampleFormat() const;
  [[nodiscard]] int selectedInputChannelConfiguration() const;
  [[nodiscard]] int selectedOutputChannelConfiguration() const;
  [[nodiscard]] int indexOfInputDeviceNamed(const QString& name) const;
  [[nodiscard]] static int bestNormalizedFormatIndex(const QList<ma_format>& formats);

  [[nodiscard]] QStringList sampleRates() const;
  [[nodiscard]] QStringList inputDeviceList() const;
  [[nodiscard]] QStringList outputDeviceList() const;
  [[nodiscard]] QStringList inputSampleFormats() const;
  [[nodiscard]] QStringList outputSampleFormats() const;
  [[nodiscard]] QStringList inputChannelConfigurations() const;
  [[nodiscard]] QStringList outputChannelConfigurations() const;

  [[nodiscard]] QJsonObject deviceIdentifier() const;

  [[nodiscard]] const QVector<ma_device_info>& inputDevices() const;
  [[nodiscard]] const QVector<ma_device_info>& outputDevices() const;
  [[nodiscard]] const QVector<AudioDeviceInfo>& inputCapabilities() const;
  [[nodiscard]] const QVector<AudioDeviceInfo>& outputCapabilities() const;

private:
  [[nodiscard]] QStringList channelLabels(const AudioDeviceInfo& caps, const QString& suffix) const;
  [[nodiscard]] QStringList formatLabels(const AudioDeviceInfo& caps) const;

  void swapDeviceList(const ma_device_type type,
                      const QVector<ma_device_info>& newList,
                      QVector<ma_device_info>& currentList,
                      QVector<AudioDeviceInfo>& capabilities,
                      int& selectedIndex) const;

private:
  ma_context* const m_context;
  const bool& m_backendReady;

  QMap<ma_format, QString> m_sampleFormats;
  QMap<ma_channel, QString> m_knownConfigs;
  QString m_inputChannelSuffix;
  QString m_outputChannelSuffix;

  int m_selectedSampleRate;

  int m_selectedInputDevice;
  int m_selectedInputSampleFormat;
  int m_selectedInputChannelConfiguration;

  int m_selectedOutputDevice;
  int m_selectedOutputSampleFormat;
  int m_selectedOutputChannelConfiguration;

  QVector<ma_device_info> m_inputDevices;
  QVector<ma_device_info> m_outputDevices;
  QVector<AudioDeviceInfo> m_inputCapabilities;
  QVector<AudioDeviceInfo> m_outputCapabilities;
};

}  // namespace Drivers
}  // namespace IO
