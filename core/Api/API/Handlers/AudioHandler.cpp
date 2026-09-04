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

#include "API/Handlers/AudioHandler.h"

#include <QJsonArray>

#include "API/CommandRegistry.h"
#include "API/SchemaBuilder.h"
#include "IO/ConnectionManager.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all Audio driver commands with the registry
 */
void API::Handlers::AudioHandler::registerCommands()
{
  registerSetterCommands();
  registerGetterCommands();
}

/**
 * @brief Registers the device-gated setters, each taking an index into an option list except
 * setNormalization, which takes a boolean.
 */
void API::Handlers::AudioHandler::registerSetterCommands()
{
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(QStringLiteral("io.audio.setInputDevice"),
                           QStringLiteral("Set input device (params: deviceIndex)"),
                           makeSchema({
                             {QStringLiteral("deviceIndex"),
                              QStringLiteral("integer"),
                              QStringLiteral("Input device index")}
  }),
                           &setInputDevice);

  registry.registerCommand(QStringLiteral("io.audio.setOutputDevice"),
                           QStringLiteral("Set output device (params: deviceIndex)"),
                           makeSchema({
                             {QStringLiteral("deviceIndex"),
                              QStringLiteral("integer"),
                              QStringLiteral("Output device index")}
  }),
                           &setOutputDevice);

  registry.registerCommand(QStringLiteral("io.audio.setSampleRate"),
                           QStringLiteral("Set sample rate (params: rateIndex)"),
                           makeSchema({
                             {QStringLiteral("rateIndex"),
                              QStringLiteral("integer"),
                              QStringLiteral("Sample rate index")}
  }),
                           &setSampleRate);

  registry.registerCommand(QStringLiteral("io.audio.setNormalization"),
                           QStringLiteral("Enable normalized -1..1 sampling (params: enabled)"),
                           makeSchema({
                             {QStringLiteral("enabled"),
                              QStringLiteral("boolean"),
                              QStringLiteral("Publish samples as floats in the -1..1 range")}
  }),
                           &setNormalization);

  registry.registerCommand(QStringLiteral("io.audio.setInputSampleFormat"),
                           QStringLiteral("Set input sample format (params: formatIndex)"),
                           makeSchema({
                             {QStringLiteral("formatIndex"),
                              QStringLiteral("integer"),
                              QStringLiteral("Input sample format index")}
  }),
                           &setInputSampleFormat);

  registry.registerCommand(QStringLiteral("io.audio.setInputChannelConfig"),
                           QStringLiteral("Set input channel config (params: channelIndex)"),
                           makeSchema({
                             {QStringLiteral("channelIndex"),
                              QStringLiteral("integer"),
                              QStringLiteral("Input channel configuration index")}
  }),
                           &setInputChannelConfig);

  registry.registerCommand(QStringLiteral("io.audio.setOutputSampleFormat"),
                           QStringLiteral("Set output sample format (params: formatIndex)"),
                           makeSchema({
                             {QStringLiteral("formatIndex"),
                              QStringLiteral("integer"),
                              QStringLiteral("Output sample format index")}
  }),
                           &setOutputSampleFormat);

  registry.registerCommand(QStringLiteral("io.audio.setOutputChannelConfig"),
                           QStringLiteral("Set output channel config (params: channelIndex)"),
                           makeSchema({
                             {QStringLiteral("channelIndex"),
                              QStringLiteral("integer"),
                              QStringLiteral("Output channel configuration index")}
  }),
                           &setOutputChannelConfig);
}

/**
 * @brief Registers the read-only listing and configuration getters.
 */
void API::Handlers::AudioHandler::registerGetterCommands()
{
  static auto& registry = CommandRegistry::instance();

  const auto empty = emptySchema();
  registry.registerCommand(QStringLiteral("io.audio.listInputDevices"),
                           QStringLiteral("List input devices"),
                           empty,
                           &getInputDevices);
  registry.registerCommand(QStringLiteral("io.audio.listOutputDevices"),
                           QStringLiteral("List output devices"),
                           empty,
                           &getOutputDevices);
  registry.registerCommand(QStringLiteral("io.audio.listSampleRates"),
                           QStringLiteral("List sample rates"),
                           empty,
                           &getSampleRates);
  registry.registerCommand(QStringLiteral("io.audio.listInputFormats"),
                           QStringLiteral("List input sample formats"),
                           empty,
                           &getInputFormats);
  registry.registerCommand(QStringLiteral("io.audio.listOutputFormats"),
                           QStringLiteral("List output sample formats"),
                           empty,
                           &getOutputFormats);
  registry.registerCommand(QStringLiteral("io.audio.getConfig"),
                           QStringLiteral("Get complete audio configuration"),
                           empty,
                           &getConfiguration);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Set input device
 */
API::CommandResponse API::Handlers::AudioHandler::setInputDevice(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("deviceIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: deviceIndex"));
  }

  const int device_index = params.value(QStringLiteral("deviceIndex")).toInt();

  static auto& connectionManager = IO::ConnectionManager::instance();
  const auto& devices            = connectionManager.audio()->inputDeviceList();
  if (device_index < 0 || device_index >= devices.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid deviceIndex: %1. Valid range: 0-%2"))
        .arg(device_index)
        .arg(devices.count() - 1));
  }

  connectionManager.audio()->setSelectedInputDevice(device_index);

  QJsonObject result;
  result[QStringLiteral("deviceIndex")] = device_index;
  result[QStringLiteral("deviceName")]  = devices.at(device_index);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set output device
 */
API::CommandResponse API::Handlers::AudioHandler::setOutputDevice(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("deviceIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: deviceIndex"));
  }

  const int device_index = params.value(QStringLiteral("deviceIndex")).toInt();

  static auto& connectionManager = IO::ConnectionManager::instance();
  const auto& devices            = connectionManager.audio()->outputDeviceList();
  if (device_index < 0 || device_index >= devices.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid deviceIndex: %1. Valid range: 0-%2"))
        .arg(device_index)
        .arg(devices.count() - 1));
  }

  connectionManager.audio()->setSelectedOutputDevice(device_index);

  QJsonObject result;
  result[QStringLiteral("deviceIndex")] = device_index;
  result[QStringLiteral("deviceName")]  = devices.at(device_index);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set sample rate
 */
API::CommandResponse API::Handlers::AudioHandler::setSampleRate(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("rateIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: rateIndex"));
  }

  const int rate_index = params.value(QStringLiteral("rateIndex")).toInt();

  static auto& connectionManager = IO::ConnectionManager::instance();
  const auto& rates              = connectionManager.audio()->sampleRates();
  if (rate_index < 0 || rate_index >= rates.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid rateIndex: %1. Valid range: 0-%2"))
        .arg(rate_index)
        .arg(rates.count() - 1));
  }

  connectionManager.audio()->setSelectedSampleRate(rate_index);

  QJsonObject result;
  result[QStringLiteral("rateIndex")] = rate_index;
  result[QStringLiteral("rateName")]  = rates.at(rate_index);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Enable or disable normalized sampling
 */
API::CommandResponse API::Handlers::AudioHandler::setNormalization(const QString& id,
                                                                   const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const bool enabled = params.value(QStringLiteral("enabled")).toBool();

  static auto& connectionManager = IO::ConnectionManager::instance();
  connectionManager.audio()->setNormalization(enabled);

  QJsonObject result;
  result[QStringLiteral("enabled")] = connectionManager.audio()->normalization();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set input sample format
 */
API::CommandResponse API::Handlers::AudioHandler::setInputSampleFormat(const QString& id,
                                                                       const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("formatIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: formatIndex"));
  }

  const int format_index = params.value(QStringLiteral("formatIndex")).toInt();

  static auto& connectionManager = IO::ConnectionManager::instance();
  const auto& formats            = connectionManager.audio()->inputSampleFormats();
  if (format_index < 0 || format_index >= formats.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid formatIndex: %1. Valid range: 0-%2"))
        .arg(format_index)
        .arg(formats.count() - 1));
  }

  connectionManager.audio()->setSelectedInputSampleFormat(format_index);

  QJsonObject result;
  result[QStringLiteral("formatIndex")] = format_index;
  result[QStringLiteral("formatName")]  = formats.at(format_index);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set input channel configuration
 */
API::CommandResponse API::Handlers::AudioHandler::setInputChannelConfig(const QString& id,
                                                                        const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("channelIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: channelIndex"));
  }

  const int channel_index = params.value(QStringLiteral("channelIndex")).toInt();

  static auto& connectionManager = IO::ConnectionManager::instance();
  const auto& channels           = connectionManager.audio()->inputChannelConfigurations();
  if (channel_index < 0 || channel_index >= channels.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid channelIndex: %1. Valid range: 0-%2"))
        .arg(channel_index)
        .arg(channels.count() - 1));
  }

  connectionManager.audio()->setSelectedInputChannelConfiguration(channel_index);

  QJsonObject result;
  result[QStringLiteral("channelIndex")] = channel_index;
  result[QStringLiteral("channelName")]  = channels.at(channel_index);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set output sample format
 */
API::CommandResponse API::Handlers::AudioHandler::setOutputSampleFormat(const QString& id,
                                                                        const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("formatIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: formatIndex"));
  }

  const int format_index = params.value(QStringLiteral("formatIndex")).toInt();

  static auto& connectionManager = IO::ConnectionManager::instance();
  const auto& formats            = connectionManager.audio()->outputSampleFormats();
  if (format_index < 0 || format_index >= formats.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid formatIndex: %1. Valid range: 0-%2"))
        .arg(format_index)
        .arg(formats.count() - 1));
  }

  connectionManager.audio()->setSelectedOutputSampleFormat(format_index);

  QJsonObject result;
  result[QStringLiteral("formatIndex")] = format_index;
  result[QStringLiteral("formatName")]  = formats.at(format_index);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set output channel configuration
 */
API::CommandResponse API::Handlers::AudioHandler::setOutputChannelConfig(const QString& id,
                                                                         const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("channelIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: channelIndex"));
  }

  const int channel_index = params.value(QStringLiteral("channelIndex")).toInt();

  static auto& connectionManager = IO::ConnectionManager::instance();
  const auto& channels           = connectionManager.audio()->outputChannelConfigurations();
  if (channel_index < 0 || channel_index >= channels.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid channelIndex: %1. Valid range: 0-%2"))
        .arg(channel_index)
        .arg(channels.count() - 1));
  }

  connectionManager.audio()->setSelectedOutputChannelConfiguration(channel_index);

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

  static auto& connectionManager = IO::ConnectionManager::instance();
  const auto& devices            = connectionManager.audio()->inputDeviceList();

  QJsonArray devices_array;
  for (const auto& device : devices)
    devices_array.append(device);

  QJsonObject result;
  result[QStringLiteral("devices")]       = devices_array;
  result[QStringLiteral("selectedIndex")] = connectionManager.audio()->selectedInputDevice();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get output devices list
 */
API::CommandResponse API::Handlers::AudioHandler::getOutputDevices(const QString& id,
                                                                   const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& connectionManager = IO::ConnectionManager::instance();
  const auto& devices            = connectionManager.audio()->outputDeviceList();

  QJsonArray devices_array;
  for (const auto& device : devices)
    devices_array.append(device);

  QJsonObject result;
  result[QStringLiteral("devices")]       = devices_array;
  result[QStringLiteral("selectedIndex")] = connectionManager.audio()->selectedOutputDevice();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get sample rates list
 */
API::CommandResponse API::Handlers::AudioHandler::getSampleRates(const QString& id,
                                                                 const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& connectionManager = IO::ConnectionManager::instance();
  const auto& rates              = connectionManager.audio()->sampleRates();

  QJsonArray rates_array;
  for (const auto& rate : rates)
    rates_array.append(rate);

  QJsonObject result;
  result[QStringLiteral("sampleRates")]   = rates_array;
  result[QStringLiteral("selectedIndex")] = connectionManager.audio()->selectedSampleRate();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get input sample formats list
 */
API::CommandResponse API::Handlers::AudioHandler::getInputFormats(const QString& id,
                                                                  const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& connectionManager = IO::ConnectionManager::instance();
  const auto& formats            = connectionManager.audio()->inputSampleFormats();

  QJsonArray formats_array;
  for (const auto& format : formats)
    formats_array.append(format);

  QJsonObject result;
  result[QStringLiteral("formats")]       = formats_array;
  result[QStringLiteral("selectedIndex")] = connectionManager.audio()->selectedInputSampleFormat();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get output sample formats list
 */
API::CommandResponse API::Handlers::AudioHandler::getOutputFormats(const QString& id,
                                                                   const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& connectionManager = IO::ConnectionManager::instance();
  const auto& formats            = connectionManager.audio()->outputSampleFormats();

  QJsonArray formats_array;
  for (const auto& format : formats)
    formats_array.append(format);

  QJsonObject result;
  result[QStringLiteral("formats")]       = formats_array;
  result[QStringLiteral("selectedIndex")] = connectionManager.audio()->selectedOutputSampleFormat();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get complete audio configuration
 */
API::CommandResponse API::Handlers::AudioHandler::getConfiguration(const QString& id,
                                                                   const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& connectionManager = IO::ConnectionManager::instance();
  auto* audio                    = connectionManager.audio();

  QJsonObject result;
  result[QStringLiteral("normalization")]              = audio->normalization();
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
