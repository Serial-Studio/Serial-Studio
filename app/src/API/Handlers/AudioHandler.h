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

#include "API/CommandProtocol.h"

namespace API {
namespace Handlers {
/**
 * @brief Registers API commands for IO::Drivers::Audio operations (Pro feature).
 */
class AudioHandler {
public:
  static void registerCommands();

private:
  static CommandResponse setInputDevice(const QString& id, const QJsonObject& params);
  static CommandResponse setOutputDevice(const QString& id, const QJsonObject& params);
  static CommandResponse setSampleRate(const QString& id, const QJsonObject& params);
  static CommandResponse setInputSampleFormat(const QString& id, const QJsonObject& params);
  static CommandResponse setInputChannelConfig(const QString& id, const QJsonObject& params);
  static CommandResponse setOutputSampleFormat(const QString& id, const QJsonObject& params);
  static CommandResponse setOutputChannelConfig(const QString& id, const QJsonObject& params);

  static CommandResponse getInputDevices(const QString& id, const QJsonObject& params);
  static CommandResponse getOutputDevices(const QString& id, const QJsonObject& params);
  static CommandResponse getSampleRates(const QString& id, const QJsonObject& params);
  static CommandResponse getInputFormats(const QString& id, const QJsonObject& params);
  static CommandResponse getOutputFormats(const QString& id, const QJsonObject& params);
  static CommandResponse getConfiguration(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API
