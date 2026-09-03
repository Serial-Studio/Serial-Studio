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

#include "IO/Drivers/Audio/AudioDeviceCatalog.h"

#include <algorithm>
#include <cstring>
#include <QSet>

#include "SSAssert.h"

// Format preference under normalization, cheapest decode first (f32 is a straight passthrough)
static constexpr ma_format kNormalizedFormats[] = {
  ma_format_f32, ma_format_s16, ma_format_s32, ma_format_s24, ma_format_u8};

//--------------------------------------------------------------------------------------------------
// Backend enumeration helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Compares two device lists to determine if they differ.
 */
static bool deviceListsDiffer(const QVector<ma_device_info>& a, const QVector<ma_device_info>& b)
{
  if (a.size() != b.size())
    return true;

  for (int i = 0; i < a.size(); ++i) {
    if (memcmp(&a[i].id, &b[i].id, sizeof(ma_device_id)) != 0)
      return true;

    if (strcmp(a[i].name, b[i].name) != 0)
      return true;
  }

  return false;
}

/**
 * @brief Returns the index of the device with the given ID, or -1 if absent.
 */
static int deviceIndexById(const QVector<ma_device_info>& list, const ma_device_id& id)
{
  for (int i = 0; i < list.size(); ++i)
    if (memcmp(&list[i].id, &id, sizeof(ma_device_id)) == 0)
      return i;

  return -1;
}

/**
 * @brief Returns the index of the device with the given backend name, or -1 if absent.
 */
static int deviceIndexByName(const QVector<ma_device_info>& list, const QString& name)
{
  for (int i = 0; i < list.size(); ++i)
    if (QString::fromUtf8(list[i].name) == name)
      return i;

  return -1;
}

/**
 * @brief Extracts audio device capabilities using MiniAudio's backend context.
 */
static IO::Drivers::AudioDeviceCatalog::AudioDeviceInfo extractCapabilities(
  ma_context* context, const ma_device_info& info, ma_device_type type)
{
  const QSet<int> defaultChannels    = {1, 2};
  const QSet<int> defaultSampleRates = {
    8000, 11025, 16000, 22050, 44100, 48000, 88200, 96000, 176400, 192000, 352800, 384000};

  const QSet<ma_format> defaultFormats = {
    ma_format_u8, ma_format_s16, ma_format_s24, ma_format_s32, ma_format_f32};

  ma_device_info fullInfo = {};
  auto r                  = ma_context_get_device_info(context, type, &info.id, &fullInfo);
  if (r != MA_SUCCESS)
    fullInfo = info;

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

  if (formats.isEmpty())
    formats = defaultFormats;

  if (sampleRates.isEmpty())
    sampleRates = defaultSampleRates;

  if (channelCounts.isEmpty())
    channelCounts = defaultChannels;

  IO::Drivers::AudioDeviceCatalog::AudioDeviceInfo caps;
  caps.supportedFormats       = formats.values();
  caps.supportedSampleRates   = sampleRates.values();
  caps.supportedChannelCounts = channelCounts.values();

  std::sort(caps.supportedFormats.begin(), caps.supportedFormats.end());
  std::sort(caps.supportedSampleRates.begin(), caps.supportedSampleRates.end());
  std::sort(caps.supportedChannelCounts.begin(), caps.supportedChannelCounts.end());

  return caps;
}

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds an empty catalog bound to the driver's miniaudio context and backend-ready flag;
 *        both are the driver's, so a backend that failed to initialize answers nothing.
 */
IO::Drivers::AudioDeviceCatalog::AudioDeviceCatalog(ma_context* context, const bool& backendReady)
  : m_context(context)
  , m_backendReady(backendReady)
  , m_selectedSampleRate(0)
  , m_selectedInputDevice(-1)
  , m_selectedInputSampleFormat(0)
  , m_selectedInputChannelConfiguration(0)
  , m_selectedOutputDevice(-1)
  , m_selectedOutputSampleFormat(0)
  , m_selectedOutputChannelConfiguration(0)
{}

//--------------------------------------------------------------------------------------------------
// Labels
//--------------------------------------------------------------------------------------------------

/**
 * @brief Adopts the translated format and channel-configuration labels. The driver owns these
 *        strings so that every user-visible word keeps one translation context across a language
 *        change, which re-runs this with the new translations.
 */
void IO::Drivers::AudioDeviceCatalog::updateLabels(const QMap<ma_format, QString>& formats,
                                                   const QMap<ma_channel, QString>& channelConfigs,
                                                   const QString& inputChannelSuffix,
                                                   const QString& outputChannelSuffix)
{
  m_sampleFormats       = formats;
  m_knownConfigs        = channelConfigs;
  m_inputChannelSuffix  = inputChannelSuffix;
  m_outputChannelSuffix = outputChannelSuffix;
}

//--------------------------------------------------------------------------------------------------
// Selection validity
//--------------------------------------------------------------------------------------------------

/**
 * @brief True when the selected input device exists and its capabilities are populated.
 */
bool IO::Drivers::AudioDeviceCatalog::validateInput() const
{
  return m_backendReady && m_selectedInputDevice >= 0
      && m_selectedInputDevice < m_inputDevices.size()
      && m_selectedInputDevice < m_inputCapabilities.size();
}

/**
 * @brief True when the selected output device exists and its capabilities are populated.
 */
bool IO::Drivers::AudioDeviceCatalog::validateOutput() const
{
  return m_backendReady && m_selectedOutputDevice >= 0
      && m_selectedOutputDevice < m_outputDevices.size()
      && m_selectedOutputDevice < m_outputCapabilities.size();
}

//--------------------------------------------------------------------------------------------------
// Backend enumeration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Copies one backend enumeration pass into @p probe without adopting anything.
 */
bool IO::Drivers::AudioDeviceCatalog::enumerate(Enumeration& probe) const
{
  SS_ASSERT(m_context != nullptr, return false);

  ma_uint32 inputCount          = 0;
  ma_uint32 outputCount         = 0;
  ma_device_info* inputDevices  = nullptr;
  ma_device_info* outputDevices = nullptr;

  const auto result =
    ma_context_get_devices(m_context, &outputDevices, &outputCount, &inputDevices, &inputCount);

  if (result != MA_SUCCESS)
    return false;

  probe.inputs  = QVector<ma_device_info>(inputDevices, inputDevices + inputCount);
  probe.outputs = QVector<ma_device_info>(outputDevices, outputDevices + outputCount);
  return true;
}

/**
 * @brief True when the selected input device is still present in @p probe; an invalid selection
 *        reads as absent, which is what makes a live session drop instead of streaming from a
 *        device that is no longer there.
 */
bool IO::Drivers::AudioDeviceCatalog::inputSelectionPresent(const Enumeration& probe) const
{
  if (m_selectedInputDevice < 0 || m_selectedInputDevice >= m_inputDevices.size())
    return false;

  return deviceIndexById(probe.inputs, m_inputDevices[m_selectedInputDevice].id) >= 0;
}

/**
 * @brief True when the selected output device is still present in @p probe.
 */
bool IO::Drivers::AudioDeviceCatalog::outputSelectionPresent(const Enumeration& probe) const
{
  if (m_selectedOutputDevice < 0 || m_selectedOutputDevice >= m_outputDevices.size())
    return false;

  return deviceIndexById(probe.outputs, m_outputDevices[m_selectedOutputDevice].id) >= 0;
}

/**
 * @brief True when an output device is selected at all. Kept apart from outputSelectionPresent(),
 *        which answers false for BOTH "nothing selected" and "the selection vanished" -- one
 *        meaning a capture-only session, the other a device that really is gone.
 */
bool IO::Drivers::AudioDeviceCatalog::outputDeviceSelected() const
{
  return m_selectedOutputDevice >= 0 && m_selectedOutputDevice < m_outputDevices.size();
}

/**
 * @brief Adopts the enumerated input devices when they differ (or when @p force demands it);
 *        returns true when the list was replaced.
 */
bool IO::Drivers::AudioDeviceCatalog::replaceInputDevices(const Enumeration& probe,
                                                          const bool force)
{
  if (!force && !deviceListsDiffer(probe.inputs, m_inputDevices))
    return false;

  swapDeviceList(ma_device_type_capture,
                 probe.inputs,
                 m_inputDevices,
                 m_inputCapabilities,
                 m_selectedInputDevice);
  return true;
}

/**
 * @brief Adopts the enumerated output devices when they differ (or when @p force demands it);
 *        returns true when the list was replaced.
 */
bool IO::Drivers::AudioDeviceCatalog::replaceOutputDevices(const Enumeration& probe,
                                                           const bool force)
{
  if (!force && !deviceListsDiffer(probe.outputs, m_outputDevices))
    return false;

  swapDeviceList(ma_device_type_playback,
                 probe.outputs,
                 m_outputDevices,
                 m_outputCapabilities,
                 m_selectedOutputDevice);
  return true;
}

/**
 * @brief Replaces a device list and its capabilities, remapping the selected index by device ID
 *        so an insertion or removal earlier in the enumeration cannot silently retarget the
 *        selection; a selection whose device disappeared becomes -1.
 */
void IO::Drivers::AudioDeviceCatalog::swapDeviceList(const ma_device_type type,
                                                     const QVector<ma_device_info>& newList,
                                                     QVector<ma_device_info>& currentList,
                                                     QVector<AudioDeviceInfo>& capabilities,
                                                     int& selectedIndex) const
{
  ma_device_id selectedId = {};
  const bool hadSelection = selectedIndex >= 0 && selectedIndex < currentList.size();
  if (hadSelection)
    selectedId = currentList[selectedIndex].id;

  currentList = newList;
  capabilities.clear();
  for (const auto& info : std::as_const(currentList))
    capabilities.append(extractCapabilities(m_context, info, type));

  if (hadSelection)
    selectedIndex = deviceIndexById(currentList, selectedId);
}

//--------------------------------------------------------------------------------------------------
// Selection accessors
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the index of the currently selected sample rate.
 */
int IO::Drivers::AudioDeviceCatalog::selectedSampleRate() const
{
  return m_selectedSampleRate;
}

/**
 * @brief Returns the index of the currently selected input device.
 */
int IO::Drivers::AudioDeviceCatalog::selectedInputDevice() const
{
  return m_selectedInputDevice;
}

/**
 * @brief Returns the index of the currently selected output device.
 */
int IO::Drivers::AudioDeviceCatalog::selectedOutputDevice() const
{
  return m_selectedOutputDevice;
}

/**
 * @brief Returns the index of the currently selected input sample format.
 */
int IO::Drivers::AudioDeviceCatalog::selectedInputSampleFormat() const
{
  return m_selectedInputSampleFormat;
}

/**
 * @brief Returns the index of the currently selected output sample format.
 */
int IO::Drivers::AudioDeviceCatalog::selectedOutputSampleFormat() const
{
  return m_selectedOutputSampleFormat;
}

/**
 * @brief Returns the index of the currently selected input channel configuration.
 */
int IO::Drivers::AudioDeviceCatalog::selectedInputChannelConfiguration() const
{
  return m_selectedInputChannelConfiguration;
}

/**
 * @brief Returns the index of the currently selected output channel configuration.
 */
int IO::Drivers::AudioDeviceCatalog::selectedOutputChannelConfiguration() const
{
  return m_selectedOutputChannelConfiguration;
}

/**
 * @brief Sets the selected sample rate index.
 */
void IO::Drivers::AudioDeviceCatalog::setSelectedSampleRate(const int index)
{
  if (m_selectedSampleRate == index)
    return;

  m_selectedSampleRate = index;
}

/**
 * @brief Sets the selected input device index.
 */
void IO::Drivers::AudioDeviceCatalog::setSelectedInputDevice(const int index)
{
  if (m_selectedInputDevice == index)
    return;

  m_selectedInputDevice = index;
}

/**
 * @brief Sets the selected output device index.
 */
void IO::Drivers::AudioDeviceCatalog::setSelectedOutputDevice(const int index)
{
  if (m_selectedOutputDevice == index)
    return;

  m_selectedOutputDevice = index;
}

/**
 * @brief Sets the selected input sample format index.
 */
void IO::Drivers::AudioDeviceCatalog::setSelectedInputSampleFormat(const int index)
{
  if (m_selectedInputSampleFormat == index)
    return;

  m_selectedInputSampleFormat = index;
}

/**
 * @brief Sets the selected output sample format index.
 */
void IO::Drivers::AudioDeviceCatalog::setSelectedOutputSampleFormat(const int index)
{
  if (m_selectedOutputSampleFormat == index)
    return;

  m_selectedOutputSampleFormat = index;
}

/**
 * @brief Sets the selected input channel configuration index.
 */
void IO::Drivers::AudioDeviceCatalog::setSelectedInputChannelConfiguration(const int index)
{
  if (m_selectedInputChannelConfiguration == index)
    return;

  m_selectedInputChannelConfiguration = index;
}

/**
 * @brief Sets the selected output channel configuration index.
 */
void IO::Drivers::AudioDeviceCatalog::setSelectedOutputChannelConfiguration(const int index)
{
  if (m_selectedOutputChannelConfiguration == index)
    return;

  m_selectedOutputChannelConfiguration = index;
}

//--------------------------------------------------------------------------------------------------
// Device and capability access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the adopted input devices.
 */
const QVector<ma_device_info>& IO::Drivers::AudioDeviceCatalog::inputDevices() const
{
  return m_inputDevices;
}

/**
 * @brief Returns the adopted output devices.
 */
const QVector<ma_device_info>& IO::Drivers::AudioDeviceCatalog::outputDevices() const
{
  return m_outputDevices;
}

/**
 * @brief Returns the capabilities of every adopted input device.
 */
const QVector<IO::Drivers::AudioDeviceCatalog::AudioDeviceInfo>& IO::Drivers::AudioDeviceCatalog::
  inputCapabilities() const
{
  return m_inputCapabilities;
}

/**
 * @brief Returns the capabilities of every adopted output device.
 */
const QVector<IO::Drivers::AudioDeviceCatalog::AudioDeviceInfo>& IO::Drivers::AudioDeviceCatalog::
  outputCapabilities() const
{
  return m_outputCapabilities;
}

/**
 * @brief Returns the index of the input device with the given backend name, or -1.
 */
int IO::Drivers::AudioDeviceCatalog::indexOfInputDeviceNamed(const QString& name) const
{
  return deviceIndexByName(m_inputDevices, name);
}

/**
 * @brief Returns the index of the cheapest format to normalize from among those the device
 *        reports natively, or 0 when it offers none of the known ones.
 */
int IO::Drivers::AudioDeviceCatalog::bestNormalizedFormatIndex(const QList<ma_format>& formats)
{
  for (const auto format : kNormalizedFormats) {
    const int index = formats.indexOf(format);
    if (index >= 0)
      return index;
  }

  return 0;
}

//--------------------------------------------------------------------------------------------------
// Picker models
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a list of supported input sample rates as strings.
 */
QStringList IO::Drivers::AudioDeviceCatalog::sampleRates() const
{
  QStringList list;
  if (!validateInput())
    return list;

  const auto& device = m_inputCapabilities[m_selectedInputDevice];
  for (int rate : device.supportedSampleRates)
    list.append(QString::number(rate));

  return list;
}

/**
 * @brief Returns a list of available input audio devices.
 */
QStringList IO::Drivers::AudioDeviceCatalog::inputDeviceList() const
{
  QStringList list;

  for (const auto& device : m_inputDevices)
    list.append(QString::fromUtf8(device.name));

  return list;
}

/**
 * @brief Returns a list of available output audio devices.
 */
QStringList IO::Drivers::AudioDeviceCatalog::outputDeviceList() const
{
  QStringList list;

  for (const auto& device : m_outputDevices)
    list.append(QString::fromUtf8(device.name));

  return list;
}

/**
 * @brief Returns the list of supported sample formats for the selected input device.
 */
QStringList IO::Drivers::AudioDeviceCatalog::inputSampleFormats() const
{
  if (!validateInput())
    return {};

  return formatLabels(m_inputCapabilities[m_selectedInputDevice]);
}

/**
 * @brief Returns a list of supported sample formats for the selected output device.
 */
QStringList IO::Drivers::AudioDeviceCatalog::outputSampleFormats() const
{
  if (!validateOutput())
    return {};

  return formatLabels(m_outputCapabilities[m_selectedOutputDevice]);
}

/**
 * @brief Returns the list of supported input channel configurations.
 */
QStringList IO::Drivers::AudioDeviceCatalog::inputChannelConfigurations() const
{
  if (!validateInput())
    return {};

  return channelLabels(m_inputCapabilities[m_selectedInputDevice], m_inputChannelSuffix);
}

/**
 * @brief Returns a list of supported output channel configurations.
 */
QStringList IO::Drivers::AudioDeviceCatalog::outputChannelConfigurations() const
{
  if (!validateOutput())
    return {};

  return channelLabels(m_outputCapabilities[m_selectedOutputDevice], m_outputChannelSuffix);
}

/**
 * @brief Names every format @p caps reports that the driver knows a label for.
 */
QStringList IO::Drivers::AudioDeviceCatalog::formatLabels(const AudioDeviceInfo& caps) const
{
  QStringList list;
  for (ma_format format : caps.supportedFormats)
    if (m_sampleFormats.contains(format))
      list.append(m_sampleFormats.value(format));

  return list;
}

/**
 * @brief Names every channel count @p caps reports, falling back to "<n> <suffix>" for a layout
 *        with no common name.
 */
QStringList IO::Drivers::AudioDeviceCatalog::channelLabels(const AudioDeviceInfo& caps,
                                                           const QString& suffix) const
{
  QStringList list;
  for (int channels : caps.supportedChannelCounts)
    if (m_knownConfigs.contains(static_cast<ma_channel>(channels)))
      list.append(m_knownConfigs.value(static_cast<ma_channel>(channels)));
    else
      list.append(QString::number(channels) + suffix);

  return list;
}

//--------------------------------------------------------------------------------------------------
// Model synchronization
//--------------------------------------------------------------------------------------------------

/**
 * @brief Re-derives the input selection indices from the parameters the device was configured
 *        with, falling back to the platform's default rate when the device cannot honour it.
 */
void IO::Drivers::AudioDeviceCatalog::syncInputSelection(const int sampleRate,
                                                         const ma_format format,
                                                         const int channels)
{
  if (!validateInput())
    return;

  const auto& caps = m_inputCapabilities[m_selectedInputDevice];

  m_selectedSampleRate = caps.supportedSampleRates.indexOf(sampleRate);
  if (m_selectedSampleRate < 0) {
#ifdef Q_OS_WIN
    int fallback = caps.supportedSampleRates.indexOf(22050);
#else
    int fallback = caps.supportedSampleRates.indexOf(44100);
#endif
    m_selectedSampleRate = fallback >= 0 ? fallback : 0;
  }

  m_selectedInputSampleFormat = caps.supportedFormats.indexOf(format);
  if (m_selectedInputSampleFormat < 0)
    m_selectedInputSampleFormat = 0;

  m_selectedInputChannelConfiguration = caps.supportedChannelCounts.indexOf(channels);
  if (m_selectedInputChannelConfiguration < 0)
    m_selectedInputChannelConfiguration = 0;
}

/**
 * @brief Re-derives the output selection indices from the parameters the device was configured
 *        with.
 */
void IO::Drivers::AudioDeviceCatalog::syncOutputSelection(const ma_format format,
                                                          const int channels)
{
  if (!validateOutput())
    return;

  const auto& caps = m_outputCapabilities[m_selectedOutputDevice];

  m_selectedOutputSampleFormat = caps.supportedFormats.indexOf(format);
  if (m_selectedOutputSampleFormat < 0)
    m_selectedOutputSampleFormat = 0;

  m_selectedOutputChannelConfiguration = caps.supportedChannelCounts.indexOf(channels);
  if (m_selectedOutputChannelConfiguration < 0)
    m_selectedOutputChannelConfiguration = 0;
}

//--------------------------------------------------------------------------------------------------
// Identity & persistence
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a JSON identifier for the currently selected audio input device: names and
 *        values, never indices, so the selection survives a reordered enumeration.
 */
QJsonObject IO::Drivers::AudioDeviceCatalog::deviceIdentifier() const
{
  QJsonObject id;

  if (m_selectedInputDevice < 0 || m_selectedInputDevice >= m_inputDevices.size())
    return id;

  id.insert(QStringLiteral("inputDeviceName"),
            QString::fromUtf8(m_inputDevices[m_selectedInputDevice].name));

  const auto rates = sampleRates();
  if (m_selectedSampleRate >= 0 && m_selectedSampleRate < rates.size())
    id.insert(QStringLiteral("sampleRateValue"), rates.at(m_selectedSampleRate).toInt());

  const auto fmts = inputSampleFormats();
  if (m_selectedInputSampleFormat >= 0 && m_selectedInputSampleFormat < fmts.size())
    id.insert(QStringLiteral("formatName"), fmts.at(m_selectedInputSampleFormat));

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
 * @brief Writes the current selection to QSettings using stable identifiers.
 */
void IO::Drivers::AudioDeviceCatalog::persistSelection(QSettings& settings) const
{
  if (validateInput()) {
    const auto& caps = m_inputCapabilities[m_selectedInputDevice];
    settings.setValue(QStringLiteral("AudioDriver/inputDeviceName"),
                      QString::fromUtf8(m_inputDevices[m_selectedInputDevice].name));

    if (m_selectedSampleRate >= 0 && m_selectedSampleRate < caps.supportedSampleRates.size())
      settings.setValue(QStringLiteral("AudioDriver/sampleRate"),
                        caps.supportedSampleRates[m_selectedSampleRate]);

    const auto fmts = inputSampleFormats();
    if (m_selectedInputSampleFormat >= 0 && m_selectedInputSampleFormat < fmts.size())
      settings.setValue(QStringLiteral("AudioDriver/inputFormat"),
                        fmts.at(m_selectedInputSampleFormat));

    if (m_selectedInputChannelConfiguration >= 0
        && m_selectedInputChannelConfiguration < caps.supportedChannelCounts.size())
      settings.setValue(QStringLiteral("AudioDriver/inputChannels"),
                        caps.supportedChannelCounts[m_selectedInputChannelConfiguration]);
  }

  if (!validateOutput())
    return;

  const auto& caps = m_outputCapabilities[m_selectedOutputDevice];
  settings.setValue(QStringLiteral("AudioDriver/outputDeviceName"),
                    QString::fromUtf8(m_outputDevices[m_selectedOutputDevice].name));

  const auto fmts = outputSampleFormats();
  if (m_selectedOutputSampleFormat >= 0 && m_selectedOutputSampleFormat < fmts.size())
    settings.setValue(QStringLiteral("AudioDriver/outputFormat"),
                      fmts.at(m_selectedOutputSampleFormat));

  if (m_selectedOutputChannelConfiguration >= 0
      && m_selectedOutputChannelConfiguration < caps.supportedChannelCounts.size())
    settings.setValue(QStringLiteral("AudioDriver/outputChannels"),
                      caps.supportedChannelCounts[m_selectedOutputChannelConfiguration]);
}

/**
 * @brief Restores the last-used selection from QSettings, falling back to the first device.
 */
void IO::Drivers::AudioDeviceCatalog::restoreSelection(QSettings& settings)
{
  const auto inName = settings.value(QStringLiteral("AudioDriver/inputDeviceName")).toString();
  if (!inName.isEmpty()) {
    const int index = indexOfInputDeviceNamed(inName);
    if (index >= 0)
      m_selectedInputDevice = index;
  }

  if ((m_selectedInputDevice < 0 || m_selectedInputDevice >= m_inputDevices.size())
      && !m_inputDevices.isEmpty())
    m_selectedInputDevice = 0;

  if (validateInput()) {
    const auto& caps = m_inputCapabilities[m_selectedInputDevice];

    const int savedRate = settings.value(QStringLiteral("AudioDriver/sampleRate"), 0).toInt();
    if (savedRate > 0) {
      const int idx = caps.supportedSampleRates.indexOf(savedRate);
      if (idx >= 0)
        m_selectedSampleRate = idx;
    }

    const auto savedFmt = settings.value(QStringLiteral("AudioDriver/inputFormat")).toString();
    if (!savedFmt.isEmpty()) {
      const int idx = inputSampleFormats().indexOf(savedFmt);
      if (idx >= 0)
        m_selectedInputSampleFormat = idx;
    }

    const int savedCh = settings.value(QStringLiteral("AudioDriver/inputChannels"), 0).toInt();
    if (savedCh > 0) {
      const int idx = caps.supportedChannelCounts.indexOf(savedCh);
      if (idx >= 0)
        m_selectedInputChannelConfiguration = idx;
    }
  }

  const auto outName = settings.value(QStringLiteral("AudioDriver/outputDeviceName")).toString();
  if (!outName.isEmpty()) {
    const int index = deviceIndexByName(m_outputDevices, outName);
    if (index >= 0)
      m_selectedOutputDevice = index;
  }

  if ((m_selectedOutputDevice < 0 || m_selectedOutputDevice >= m_outputDevices.size())
      && !m_outputDevices.isEmpty())
    m_selectedOutputDevice = 0;

  if (!validateOutput())
    return;

  const auto& caps = m_outputCapabilities[m_selectedOutputDevice];

  const auto savedFmt = settings.value(QStringLiteral("AudioDriver/outputFormat")).toString();
  if (!savedFmt.isEmpty()) {
    const int idx = outputSampleFormats().indexOf(savedFmt);
    if (idx >= 0)
      m_selectedOutputSampleFormat = idx;
  }

  const int savedCh = settings.value(QStringLiteral("AudioDriver/outputChannels"), 0).toInt();
  if (savedCh > 0) {
    const int idx = caps.supportedChannelCounts.indexOf(savedCh);
    if (idx >= 0)
      m_selectedOutputChannelConfiguration = idx;
  }
}

/**
 * @brief Resolves a project's saved input selection: the device by name first and by index only
 *        as a fallback, then rate, format and channels by value before index. Returns false when
 *        no device could be resolved, in which case nothing was written and the caller must leave
 *        the live configuration alone.
 */
bool IO::Drivers::AudioDeviceCatalog::applySavedSelection(const QJsonObject& settings,
                                                          const QJsonObject& deviceId,
                                                          const bool normalization)
{
  const auto savedDevName = deviceId.value(QStringLiteral("inputDeviceName")).toString();
  const int savedDevIndex = settings.value(QStringLiteral("inputDevice")).toInt(-1);

  int deviceIndex = -1;
  if (!savedDevName.isEmpty())
    deviceIndex = indexOfInputDeviceNamed(savedDevName);

  if (deviceIndex < 0 && savedDevIndex >= 0 && savedDevIndex < m_inputDevices.size())
    deviceIndex = savedDevIndex;

  if (deviceIndex < 0 || deviceIndex >= m_inputCapabilities.size())
    return false;

  m_selectedInputDevice = deviceIndex;
  const auto& caps      = m_inputCapabilities[deviceIndex];

  const int savedRateHz    = deviceId.value(QStringLiteral("sampleRateValue")).toInt(0);
  const int savedRateIndex = settings.value(QStringLiteral("sampleRate")).toInt(-1);

  int rateIndex = -1;
  if (savedRateHz > 0)
    rateIndex = caps.supportedSampleRates.indexOf(savedRateHz);

  if (rateIndex < 0 && savedRateIndex >= 0 && savedRateIndex < caps.supportedSampleRates.size())
    rateIndex = savedRateIndex;

  if (rateIndex < 0) {
#ifdef Q_OS_WIN
    rateIndex = caps.supportedSampleRates.indexOf(22050);
#else
    rateIndex = caps.supportedSampleRates.indexOf(44100);
#endif
  }

  if (rateIndex < 0)
    rateIndex = 0;

  const auto savedFmtName = deviceId.value(QStringLiteral("formatName")).toString();
  const int savedFmtIndex = settings.value(QStringLiteral("inputFormat")).toInt(-1);
  const auto fmtNames     = inputSampleFormats();

  int fmtIndex = -1;
  if (!savedFmtName.isEmpty())
    fmtIndex = fmtNames.indexOf(savedFmtName);

  if (fmtIndex < 0 && savedFmtIndex >= 0 && savedFmtIndex < caps.supportedFormats.size())
    fmtIndex = savedFmtIndex;

  if (fmtIndex < 0)
    fmtIndex = 0;

  if (normalization)
    fmtIndex = bestNormalizedFormatIndex(caps.supportedFormats);

  const int savedChCount = deviceId.value(QStringLiteral("channelCount")).toInt(0);
  const int savedChIndex = settings.value(QStringLiteral("inputChannels")).toInt(-1);

  int chIndex = -1;
  if (savedChCount > 0)
    chIndex = caps.supportedChannelCounts.indexOf(savedChCount);

  if (chIndex < 0 && savedChIndex >= 0 && savedChIndex < caps.supportedChannelCounts.size())
    chIndex = savedChIndex;

  if (chIndex < 0)
    chIndex = 0;

  m_selectedSampleRate                = rateIndex;
  m_selectedInputSampleFormat         = fmtIndex;
  m_selectedInputChannelConfiguration = chIndex;
  return true;
}
