/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include <iterator>
#include <QtGlobal>

#include "ThirdParty/miniaudio.h"

namespace IO {
namespace Audio {

/**
 * @brief Initializes @p context on the platform backends in preference order, the one place both
 *        the capture driver and the alarm player get it from. Linux tries the sound server
 *        (PulseAudio, or PipeWire through pipewire-pulse) before raw ALSA: the AppImage's
 *        libasound cannot load the host's pipewire plugin, so ALSA's "default" PCM fails to open
 *        there, and the raw hw devices it falls back to bypass the system's DSP chain. A host
 *        without a sound server fails the PulseAudio connect and lands on ALSA as before.
 */
[[nodiscard]] inline bool initContext(ma_context& context)
{
#if defined(Q_OS_WIN)
  const ma_backend backends[] = {ma_backend_wasapi};
#elif defined(Q_OS_APPLE)
  const ma_backend backends[] = {ma_backend_coreaudio};
#elif defined(Q_OS_LINUX)
  const ma_backend backends[] = {ma_backend_pulseaudio, ma_backend_alsa};
#else
#  error "Unsupported platform"
#endif

  ma_context_config config      = ma_context_config_init();
  config.pulse.pApplicationName = "Serial Studio";

  const auto count = static_cast<ma_uint32>(std::size(backends));
  return ma_context_init(backends, count, &config, &context) == MA_SUCCESS;
}

}  // namespace Audio
}  // namespace IO
