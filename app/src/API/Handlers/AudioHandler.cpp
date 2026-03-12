/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is part of Serial Studio Pro. All rights reserved.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "API/Handlers/AudioHandler.h"

#  include <QJsonArray>

#  include "API/CommandRegistry.h"
#  include "IO/ConnectionManager.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all Audio driver commands with the registry
 */
void API::Handlers::AudioHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(QStringLiteral("io.driver.audio.setInputDevice"),
                           QStringLiteral("Set input device (params: deviceIndex)"),
                           &setInputDevice);

  registry.registerCommand(QStringLiteral("io.driver.audio.setOutputDevice"),
                           QStringLiteral("Set output device (params: deviceIndex)"),
                           &setOutputDevice);

  registry.registerCommand(QStringLiteral("io.driver.audio.setSampleRate"),
                           QStringLiteral("Set sample rate (params: rateIndex)"),
                           &setSampleRate);

  registry.registerCommand(QStringLiteral("io.driver.audio.setInputSampleFormat"),
                           QStringLiteral("Set input sample format (params: formatIndex)"),
                           &setInputSampleFormat);

  registry.registerCommand(QStringLiteral("io.driver.audio.setInputChannelConfig"),
                           QStringLiteral("Set input channel config (params: channelIndex)"),
                           &setInputChannelConfig);

  registry.registerCommand(QStringLiteral("io.driver.audio.setOutputSampleFormat"),
                           QStringLiteral("Set output sample format (params: formatIndex)"),
                           &setOutputSampleFormat);

  registry.registerCommand(QStringLiteral("io.driver.audio.setOutputChannelConfig"),
                           QStringLiteral("Set output channel config (params: channelIndex)"),
                           &setOutputChannelConfig);

  registry.registerCommand(QStringLiteral("io.driver.audio.getInputDevices"),
                           QStringLiteral("List input devices"),
                           &getInputDevices);

  registry.registerCommand(QStringLiteral("io.driver.audio.getOutputDevices"),
                           QStringLiteral("List output devices"),
                           &getOutputDevices);

  registry.registerCommand(QStringLiteral("io.driver.audio.getSampleRates"),
                           QStringLiteral("List sample rates"),
                           &getSampleRates);

  registry.registerCommand(QStringLiteral("io.driver.audio.getInputFormats"),
                           QStringLiteral("List input sample formats"),
                           &getInputFormats);

  registry.registerCommand(QStringLiteral("io.driver.audio.getOutputFormats"),
                           QStringLiteral("List output sample formats"),
                           &getOutputFormats);

  registry.registerCommand(QStringLiteral("io.driver.audio.getConfiguration"),
                           QStringLiteral("Get complete audio configuration"),
                           &getConfiguration);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Set input device
 * @param params Requires "deviceIndex" (int)
 */
API::CommandResponse API::Handlers::AudioHandler::setInputDevice(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("deviceIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: deviceIndex"));
  }

  const int device_index = params.value(QStringLiteral("deviceIndex")).toInt();

  const auto& devices = IO::ConnectionManager::instance().audio()->inputDeviceList();
  if (device_index < 0 || device_index >= devices.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid deviceIndex: %1. Valid range: 0-%2"))
        .arg(device_index)
        .arg(devices.count() - 1));
  }

  IO::ConnectionManager::instance().audio()->setSelectedInputDevice(device_index);

  QJsonObject result;
  result[QStringLiteral("deviceIndex")] = device_index;
  result[QStringLiteral("deviceName")]  = devices.at(device_index);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set output device
 * @param params Requires "deviceIndex" (int)
 */
API::CommandResponse API::Handlers::AudioHandler::setOutputDevice(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("deviceIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: deviceIndex"));
  }

  const int device_index = params.value(QStringLiteral("deviceIndex")).toInt();

  const auto& devices = IO::ConnectionManager::instance().audio()->outputDeviceList();
  if (device_index < 0 || device_index >= devices.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid deviceIndex: %1. Valid range: 0-%2"))
        .arg(device_index)
        .arg(devices.count() - 1));
  }

  IO::ConnectionManager::instance().audio()->setSelectedOutputDevice(device_index);

  QJsonObject result;
  result[QStringLiteral("deviceIndex")] = device_index;
  result[QStringLiteral("deviceName")]  = devices.at(device_index);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set sample rate
 * @param params Requires "rateIndex" (int)
 */
API::CommandResponse API::Handlers::AudioHandler::setSampleRate(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("rateIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: rateIndex"));
  }

  const int rate_index = params.value(QStringLiteral("rateIndex")).toInt();

  const auto& rates = IO::ConnectionManager::instance().audio()->sampleRates();
  if (rate_index < 0 || rate_index >= rates.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid rateIndex: %1. Valid range: 0-%2"))
        .arg(rate_index)
        .arg(rates.count() - 1));
  }

  IO::ConnectionManager::instance().audio()->setSelectedSampleRate(rate_index);

  QJsonObject result;
  result[QStringLiteral("rateIndex")] = rate_index;
  result[QStringLiteral("rateName")]  = rates.at(rate_index);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set input sample format
 * @param params Requires "formatIndex" (int)
 */
API::CommandResponse API::Handlers::AudioHandler::setInputSampleFormat(const QString& id,
                                                                       const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("formatIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: formatIndex"));
  }

  const int format_index = params.value(QStringLiteral("formatIndex")).toInt();

  const auto& formats = IO::ConnectionManager::instance().audio()->inputSampleFormats();
  if (format_index < 0 || format_index >= formats.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid formatIndex: %1. Valid range: 0-%2"))
        .arg(format_index)
        .arg(formats.count() - 1));
  }

  IO::ConnectionManager::instance().audio()->setSelectedInputSampleFormat(format_index);

  QJsonObject result;
  result[QStringLiteral("formatIndex")] = format_index;
  result[QStringLiteral("formatName")]  = formats.at(format_index);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set input channel configuration
 * @param params Requires "channelIndex" (int)
 */
API::CommandResponse API::Handlers::AudioHandler::setInputChannelConfig(const QString& id,
                                                                        const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("channelIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: channelIndex"));
  }

  const int channel_index = params.value(QStringLiteral("channelIndex")).toInt();

  const auto& channels = IO::ConnectionManager::instance().audio()->inputChannelConfigurations();
  if (channel_index < 0 || channel_index >= channels.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid channelIndex: %1. Valid range: 0-%2"))
        .arg(channel_index)
        .arg(channels.count() - 1));
  }

  IO::ConnectionManager::instance().audio()->setSelectedInputChannelConfiguration(channel_index);

  QJsonObject result;
  result[QStringLiteral("channelIndex")] = channel_index;
  result[QStringLiteral("channelName")]  = channels.at(channel_index);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set output sample format
 * @param params Requires "formatIndex" (int)
 */
API::CommandResponse API::Handlers::AudioHandler::setOutputSampleFormat(const QString& id,
                                                                        const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("formatIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: formatIndex"));
  }

  const int format_index = params.value(QStringLiteral("formatIndex")).toInt();

  const auto& formats = IO::ConnectionManager::instance().audio()->outputSampleFormats();
  if (format_index < 0 || format_index >= formats.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid formatIndex: %1. Valid range: 0-%2"))
        .arg(format_index)
        .arg(formats.count() - 1));
  }

  IO::ConnectionManager::instance().audio()->setSelectedOutputSampleFormat(format_index);

  QJsonObject result;
  result[QStringLiteral("formatIndex")] = format_index;
  result[QStringLiteral("formatName")]  = formats.at(format_index);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set output channel configuration
 * @param params Requires "channelIndex" (int)
 */
API::CommandResponse API::Handlers::AudioHandler::setOutputChannelConfig(const QString& id,
                                                                         const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("channelIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: channelIndex"));
  }

  const int channel_index = params.value(QStringLiteral("channelIndex")).toInt();

  const auto& channels = IO::ConnectionManager::instance().audio()->outputChannelConfigurations();
  if (channel_index < 0 || channel_index >= channels.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid channelIndex: %1. Valid range: 0-%2"))
        .arg(channel_index)
        .arg(channels.count() - 1));
  }

  IO::ConnectionManager::instance().audio()->setSelectedOutputChannelConfiguration(channel_index);

  QJsonObject result;
  result[QStringLiteral("channelIndex")] = channel_index;
  result[QStringLiteral("channelName")]  = channels.at(channel_index);
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Get input devices list
 */
API::CommandResponse API::Handlers::AudioHandler::getInputDevices(const QString& id,
                                                                  const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& devices = IO::ConnectionManager::instance().audio()->inputDeviceList();

  QJsonArray devices_array;
  for (const auto& device : devices)
    devices_array.append(device);

  QJsonObject result;
  result[QStringLiteral("devices")] = devices_array;
  result[QStringLiteral("selectedIndex")] =
    IO::ConnectionManager::instance().audio()->selectedInputDevice();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get output devices list
 */
API::CommandResponse API::Handlers::AudioHandler::getOutputDevices(const QString& id,
                                                                   const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& devices = IO::ConnectionManager::instance().audio()->outputDeviceList();

  QJsonArray devices_array;
  for (const auto& device : devices)
    devices_array.append(device);

  QJsonObject result;
  result[QStringLiteral("devices")] = devices_array;
  result[QStringLiteral("selectedIndex")] =
    IO::ConnectionManager::instance().audio()->selectedOutputDevice();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get sample rates list
 */
API::CommandResponse API::Handlers::AudioHandler::getSampleRates(const QString& id,
                                                                 const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& rates = IO::ConnectionManager::instance().audio()->sampleRates();

  QJsonArray rates_array;
  for (const auto& rate : rates)
    rates_array.append(rate);

  QJsonObject result;
  result[QStringLiteral("sampleRates")] = rates_array;
  result[QStringLiteral("selectedIndex")] =
    IO::ConnectionManager::instance().audio()->selectedSampleRate();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get input sample formats list
 */
API::CommandResponse API::Handlers::AudioHandler::getInputFormats(const QString& id,
                                                                  const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& formats = IO::ConnectionManager::instance().audio()->inputSampleFormats();

  QJsonArray formats_array;
  for (const auto& format : formats)
    formats_array.append(format);

  QJsonObject result;
  result[QStringLiteral("formats")] = formats_array;
  result[QStringLiteral("selectedIndex")] =
    IO::ConnectionManager::instance().audio()->selectedInputSampleFormat();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get output sample formats list
 */
API::CommandResponse API::Handlers::AudioHandler::getOutputFormats(const QString& id,
                                                                   const QJsonObject& params)
{
  Q_UNUSED(params)

  const auto& formats = IO::ConnectionManager::instance().audio()->outputSampleFormats();

  QJsonArray formats_array;
  for (const auto& format : formats)
    formats_array.append(format);

  QJsonObject result;
  result[QStringLiteral("formats")] = formats_array;
  result[QStringLiteral("selectedIndex")] =
    IO::ConnectionManager::instance().audio()->selectedOutputSampleFormat();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get complete audio configuration
 */
API::CommandResponse API::Handlers::AudioHandler::getConfiguration(const QString& id,
                                                                   const QJsonObject& params)
{
  Q_UNUSED(params)

  auto* audio = IO::ConnectionManager::instance().audio();

  QJsonObject result;
  result[QStringLiteral("selectedInputDevice")]        = audio->selectedInputDevice();
  result[QStringLiteral("selectedOutputDevice")]       = audio->selectedOutputDevice();
  result[QStringLiteral("selectedSampleRate")]         = audio->selectedSampleRate();
  result[QStringLiteral("selectedInputSampleFormat")]  = audio->selectedInputSampleFormat();
  result[QStringLiteral("selectedInputChannelConfig")] = audio->selectedInputChannelConfiguration();
  result[QStringLiteral("selectedOutputSampleFormat")] = audio->selectedOutputSampleFormat();
  result[QStringLiteral("selectedOutputChannelConfig")] =
    audio->selectedOutputChannelConfiguration();

  return CommandResponse::makeSuccess(id, result);
}

#endif  // BUILD_COMMERCIAL
