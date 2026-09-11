/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <functional>
#include <QByteArray>

#include "DataModel/Scripting/TransmitScriptEnvironment.h"

namespace Widgets {
namespace Output {

/**
 * @brief Where an output widget's payload goes and how often. Injected so one widget class serves
 *        a device or a preview without holding either: built with a capture target it owns no path
 *        to a connection, which is what stops previewing a relay script from actuating the relay.
 *        An empty @c deliver drops the payload; a zero @c minIntervalMs reports every interaction.
 */
struct TransmitTarget {
  int minIntervalMs = 0;
  std::function<void(const QByteArray&)> deliver;
  DataModel::TransmitScriptSurface surface = DataModel::TransmitScriptSurface::Live;
};

// Pacing a live control transmits at; a preview passes zero so every interaction reports
inline constexpr int kLiveSendIntervalMs = 50;

}  // namespace Output
}  // namespace Widgets
