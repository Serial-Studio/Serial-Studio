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

#pragma once

#ifdef BUILD_COMMERCIAL

#  include "API/CommandProtocol.h"

namespace API {
namespace Handlers {
/**
 * @class AudioHandler
 * @brief Registers API commands for IO::Drivers::Audio operations (Pro feature)
 *
 * Provides commands for audio driver configuration:
 *
 * Configuration:
 * - io.driver.audio.setInputDevice - Select input device
 * - io.driver.audio.setOutputDevice - Select output device
 * - io.driver.audio.setSampleRate - Set sample rate
 * - io.driver.audio.setInputSampleFormat - Set input format
 * - io.driver.audio.setInputChannelConfig - Set input channels
 * - io.driver.audio.setOutputSampleFormat - Set output format
 * - io.driver.audio.setOutputChannelConfig - Set output channels
 *
 * Queries:
 * - io.driver.audio.getInputDevices - List input devices
 * - io.driver.audio.getOutputDevices - List output devices
 * - io.driver.audio.getSampleRates - List sample rates
 * - io.driver.audio.getInputFormats - List input formats
 * - io.driver.audio.getOutputFormats - List output formats
 * - io.driver.audio.getConfiguration - Get complete config
 */
class AudioHandler {
public:
  /**
   * @brief Register all Audio driver commands with the CommandRegistry
   */
  static void registerCommands();

private:
  // Configuration commands
  static CommandResponse setInputDevice(const QString& id, const QJsonObject& params);
  static CommandResponse setOutputDevice(const QString& id, const QJsonObject& params);
  static CommandResponse setSampleRate(const QString& id, const QJsonObject& params);
  static CommandResponse setInputSampleFormat(const QString& id, const QJsonObject& params);
  static CommandResponse setInputChannelConfig(const QString& id, const QJsonObject& params);
  static CommandResponse setOutputSampleFormat(const QString& id, const QJsonObject& params);
  static CommandResponse setOutputChannelConfig(const QString& id, const QJsonObject& params);

  // Query commands
  static CommandResponse getInputDevices(const QString& id, const QJsonObject& params);
  static CommandResponse getOutputDevices(const QString& id, const QJsonObject& params);
  static CommandResponse getSampleRates(const QString& id, const QJsonObject& params);
  static CommandResponse getInputFormats(const QString& id, const QJsonObject& params);
  static CommandResponse getOutputFormats(const QString& id, const QJsonObject& params);
  static CommandResponse getConfiguration(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API

#endif  // BUILD_COMMERCIAL
