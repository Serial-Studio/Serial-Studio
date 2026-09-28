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

#include <array>
#include <atomic>
#include <memory>
#include <QString>
#include <QtGlobal>
#include <vector>

#include "IO/Audio/WavDecoder.h"

namespace IO {
namespace Audio {

/**
 * @brief Fixed table of pre-decoded sounds at the playback rate as interleaved stereo float
 *        (spec 0087). The GUI thread loads and replaces slots; the callback re-acquires a slot's
 *        pointer every callback, and a replaced buffer waits in a graveyard until the callback
 *        generation (read live at retirement) has moved past its mark, or the reader is idle.
 */
class SoundBank {
public:
  static constexpr int kSlotCount = 48;
  static constexpr int kChannels  = 2;

  /**
   * @brief One device-rate stereo buffer; frames is samples.size() / kChannels.
   */
  struct Sound {
    qint64 frames;
    std::vector<float> samples;
  };

  explicit SoundBank(int deviceRate, const std::atomic<quint64>& generation);
  SoundBank(SoundBank&&)                 = delete;
  SoundBank(const SoundBank&)            = delete;
  SoundBank& operator=(SoundBank&&)      = delete;
  SoundBank& operator=(const SoundBank&) = delete;

  [[nodiscard]] int deviceRate() const noexcept;
  [[nodiscard]] bool loaded(int slot) const noexcept;
  [[nodiscard]] const Sound* acquire(int slot) const noexcept;

  [[nodiscard]] bool load(int slot, const DecodedSound& decoded, QString& reason);

  void clear(int slot);
  void clearAll();
  void setDeviceRate(int rate);
  void collectGarbage(bool readerIdle);

private:
  struct Retired {
    quint64 generation;
    std::unique_ptr<Sound> sound;
  };

  [[nodiscard]] static bool convert(const DecodedSound& decoded,
                                    int deviceRate,
                                    Sound& out,
                                    QString& reason);

  void publish(int slot, std::unique_ptr<Sound> sound);

  int m_deviceRate;
  const std::atomic<quint64>& m_generation;
  std::vector<Retired> m_graveyard;
  std::array<std::unique_ptr<Sound>, kSlotCount> m_owned;
  std::array<std::atomic<const Sound*>, kSlotCount> m_visible;
};

}  // namespace Audio
}  // namespace IO
