/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QLatin1String>
#include <QString>
#include <QVariantMap>
#include <vector>

#include "Core/IO/HAL_Driver.h"

namespace IO {

/**
 * @brief Resolves the effective stream lane for a source: the per-source project override wins,
 *        otherwise the driver decides (stream-capable drivers stream by default, spec 0051 R6).
 */
[[nodiscard]] inline bool streamLaneOn(const HAL_Driver* driver, const QString& lane)
{
  if (lane == QLatin1String("on"))
    return true;

  if (lane == QLatin1String("off"))
    return false;

  return driver && driver->isStreamCapable();
}

/**
 * @brief One dataset bound to a stream channel: dashboard identity, channel index, display
 *        reductions to run, and the optional per-dataset transform.
 */
struct StreamChannelConfig {
  int uniqueId          = -1;
  int channel           = 0;
  bool plot             = false;
  bool fft              = false;
  int fftSamples        = 0;
  int transformLanguage = 0;
  QString transformCode;
  QVariantMap transformParams;
  QString title;
  QString alias;
};

/**
 * @brief Immutable per-source stream configuration handed to a worker at creation.
 */
struct StreamConfig {
  int sourceId      = 0;
  int channels      = 1;
  double sampleRate = 0.0;
  bool luaFastMode  = false;
  QString transformLibrary;
  QString transformLibraryJs;
  std::vector<StreamChannelConfig> datasets;
};

/**
 * @brief One dense-lane source the connection layer hands the ingest binder (spec 0077): the
 *        device id its workers publish under, the live driver that feeds them and the derived
 *        configuration. A source whose lane is off or binds no channel is not listed.
 */
struct StreamAttachment {
  int deviceId       = 0;
  HAL_Driver* driver = nullptr;
  StreamConfig config;
};

}  // namespace IO
