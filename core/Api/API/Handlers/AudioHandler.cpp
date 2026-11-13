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
#include "API/HandlerContext.h"
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
 * @brief Range-checks the index parameter against an option list, applies it and echoes the pick.
 */
template<typename Apply>
[[nodiscard]] static API::CommandResponse applyListIndex(const QString& id,
                                                         const QJsonObject& params,
                                                         const char* key,
                                                         const char* name_key,
                                                         const QStringList& list,
                                                         Apply apply)
{
  const int index = params.value(QLatin1String(key)).toInt();
  if (index < 0 || index >= list.count()) {
    return API::CommandResponse::makeError(id,
                                           API::ErrorCode::InvalidParam,
                                           QStringLiteral("Invalid %1: %2. Valid range: 0-%3")
                                             .arg(QLatin1String(key))
                                             .arg(index)
                                             .arg(list.count() - 1));
  }

  apply(index);

  QJsonObject result;
  result[QLatin1String(key)]      = index;
  result[QLatin1String(name_key)] = list.at(index);
  return API::CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set input device
 */
API::CommandResponse API::Handlers::AudioHandler::setInputDevice(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("deviceIndex")))
    return CommandResponse::makeMissingParam(id, "deviceIndex");

  auto* audio = API::handlerContext().connectionManager.audio();
  return applyListIndex(
    id, params, "deviceIndex", "deviceName", audio->inputDeviceList(), [audio](int index) {
      audio->setSelectedInputDevice(index);
    });
}

/**
 * @brief Set output device
 */
API::CommandResponse API::Handlers::AudioHandler::setOutputDevice(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("deviceIndex")))
    return CommandResponse::makeMissingParam(id, "deviceIndex");

  auto* audio = API::handlerContext().connectionManager.audio();
  return applyListIndex(
    id, params, "deviceIndex", "deviceName", audio->outputDeviceList(), [audio](int index) {
      audio->setSelectedOutputDevice(index);
    });
}

/**
 * @brief Set sample rate
 */
API::CommandResponse API::Handlers::AudioHandler::setSampleRate(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("rateIndex")))
    return CommandResponse::makeMissingParam(id, "rateIndex");

  auto* audio = API::handlerContext().connectionManager.audio();
  return applyListIndex(
    id, params, "rateIndex", "rateName", audio->sampleRates(), [audio](int index) {
      audio->setSelectedSampleRate(index);
    });
}

/**
 * @brief Enable or disable normalized sampling
 */
API::CommandResponse API::Handlers::AudioHandler::setNormalization(const QString& id,
                                                                   const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled")))
    return CommandResponse::makeMissingParam(id, "enabled");

  const bool enabled = params.value(QStringLiteral("enabled")).toBool();

  auto& connectionManager = API::handlerContext().connectionManager;
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
  if (!params.contains(QStringLiteral("formatIndex")))
    return CommandResponse::makeMissingParam(id, "formatIndex");

  auto* audio = API::handlerContext().connectionManager.audio();
  return applyListIndex(
    id, params, "formatIndex", "formatName", audio->inputSampleFormats(), [audio](int index) {
      audio->setSelectedInputSampleFormat(index);
    });
}

/**
 * @brief Set input channel configuration
 */
API::CommandResponse API::Handlers::AudioHandler::setInputChannelConfig(const QString& id,
                                                                        const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("channelIndex")))
    return CommandResponse::makeMissingParam(id, "channelIndex");

  auto* audio = API::handlerContext().connectionManager.audio();
  return applyListIndex(id,
                        params,
                        "channelIndex",
                        "channelName",
                        audio->inputChannelConfigurations(),
                        [audio](int index) { audio->setSelectedInputChannelConfiguration(index); });
}

/**
 * @brief Set output sample format
 */
API::CommandResponse API::Handlers::AudioHandler::setOutputSampleFormat(const QString& id,
                                                                        const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("formatIndex")))
    return CommandResponse::makeMissingParam(id, "formatIndex");

  auto* audio = API::handlerContext().connectionManager.audio();
  return applyListIndex(
    id, params, "formatIndex", "formatName", audio->outputSampleFormats(), [audio](int index) {
      audio->setSelectedOutputSampleFormat(index);
    });
}

/**
 * @brief Set output channel configuration
 */
API::CommandResponse API::Handlers::AudioHandler::setOutputChannelConfig(const QString& id,
                                                                         const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("channelIndex")))
    return CommandResponse::makeMissingParam(id, "channelIndex");

  auto* audio = API::handlerContext().connectionManager.audio();
  return applyListIndex(
    id,
    params,
    "channelIndex",
    "channelName",
    audio->outputChannelConfigurations(),
    [audio](int index) { audio->setSelectedOutputChannelConfiguration(index); });
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

  auto& connectionManager = API::handlerContext().connectionManager;
  const auto& devices     = connectionManager.audio()->inputDeviceList();

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

  auto& connectionManager = API::handlerContext().connectionManager;
  const auto& devices     = connectionManager.audio()->outputDeviceList();

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

  auto& connectionManager = API::handlerContext().connectionManager;
  const auto& rates       = connectionManager.audio()->sampleRates();

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

  auto& connectionManager = API::handlerContext().connectionManager;
  const auto& formats     = connectionManager.audio()->inputSampleFormats();

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

  auto& connectionManager = API::handlerContext().connectionManager;
  const auto& formats     = connectionManager.audio()->outputSampleFormats();

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

  auto& connectionManager = API::handlerContext().connectionManager;
  auto* audio             = connectionManager.audio();

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
