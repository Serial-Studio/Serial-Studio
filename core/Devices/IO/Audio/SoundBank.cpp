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

#include "IO/Audio/SoundBank.h"

#include <algorithm>
#include <QObject>

#include "Core/SSAssert.h"
#include "ThirdParty/miniaudio.h"

//--------------------------------------------------------------------------------------------------
// Tunables
//--------------------------------------------------------------------------------------------------

static constexpr quint64 kGenerationsToRetire = 2;

//--------------------------------------------------------------------------------------------------
// Constructor & getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds an empty bank whose conversions target @p deviceRate.
 */
IO::Audio::SoundBank::SoundBank(int deviceRate, const std::atomic<quint64>& generation)
  : m_deviceRate(deviceRate), m_generation(generation)
{
  for (auto& slot : m_visible)
    slot.store(nullptr, std::memory_order_relaxed);
}

/**
 * @brief The playback rate every slot is converted to.
 */
int IO::Audio::SoundBank::deviceRate() const noexcept
{
  return m_deviceRate;
}

/**
 * @brief True when the slot holds a sound.
 */
bool IO::Audio::SoundBank::loaded(int slot) const noexcept
{
  return acquire(slot) != nullptr;
}

/**
 * @brief Reader side: the sound currently published in @p slot, or nullptr. The pointer stays
 *        valid for at least kGenerationsToRetire callbacks after a replacement.
 */
const IO::Audio::SoundBank::Sound* IO::Audio::SoundBank::acquire(int slot) const noexcept
{
  if (slot < 0 || slot >= kSlotCount)
    return nullptr;

  return m_visible[static_cast<std::size_t>(slot)].load(std::memory_order_acquire);
}

//--------------------------------------------------------------------------------------------------
// Mutation (GUI thread)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Converts @p decoded to the device rate and publishes it; the previous buffer, if any,
 *        is retired at the current callback generation.
 */
bool IO::Audio::SoundBank::load(int slot, const DecodedSound& decoded, QString& reason)
{
  SS_ASSERT(slot >= 0 && slot < kSlotCount, return false);
  SS_ASSERT(decoded.channels >= 1 && decoded.channels <= kChannels, return false);

  auto sound = std::make_unique<Sound>();
  if (!convert(decoded, m_deviceRate, *sound, reason))
    return false;

  publish(slot, std::move(sound));
  return true;
}

/**
 * @brief Empties one slot, retiring its buffer at the current callback generation.
 */
void IO::Audio::SoundBank::clear(int slot)
{
  SS_ASSERT(slot >= 0 && slot < kSlotCount, return);
  publish(slot, nullptr);
}

/**
 * @brief Empties every slot.
 */
void IO::Audio::SoundBank::clearAll()
{
  for (int slot = 0; slot < kSlotCount; ++slot)
    publish(slot, nullptr);
}

/**
 * @brief Changes the target rate; every slot is dropped because its samples no longer match,
 *        and the owner reloads what it needs.
 */
void IO::Audio::SoundBank::setDeviceRate(int rate)
{
  SS_ASSERT(rate > 0, return);
  if (rate == m_deviceRate)
    return;

  m_deviceRate = rate;
  clearAll();
}

/**
 * @brief Frees retired buffers the callback can no longer be reading: those retired at least
 *        kGenerationsToRetire callbacks ago (the counter is read live), or all when idle.
 */
void IO::Audio::SoundBank::collectGarbage(bool readerIdle)
{
  const quint64 generation = m_generation.load(std::memory_order_acquire);
  const auto dead          = [generation, readerIdle](const Retired& r) {
    return readerIdle || generation >= r.generation + kGenerationsToRetire;
  };

  m_graveyard.erase(std::remove_if(m_graveyard.begin(), m_graveyard.end(), dead),
                    m_graveyard.end());
}

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Publishes @p sound (or nothing) in @p slot and moves the old buffer to the graveyard.
 */
void IO::Audio::SoundBank::publish(int slot, std::unique_ptr<Sound> sound)
{
  SS_ASSERT(slot >= 0 && slot < kSlotCount, return);

  const auto index = static_cast<std::size_t>(slot);
  m_visible[index].store(sound.get(), std::memory_order_release);

  const quint64 generation = m_generation.load(std::memory_order_acquire);
  if (m_owned[index])
    m_graveyard.push_back(Retired{generation, std::move(m_owned[index])});

  m_owned[index] = std::move(sound);
}

/**
 * @brief Converts a decoded sound to device-rate interleaved stereo float through miniaudio's
 *        data converter; a same-rate stereo file is copied straight through.
 */
bool IO::Audio::SoundBank::convert(const DecodedSound& decoded,
                                   int deviceRate,
                                   Sound& out,
                                   QString& reason)
{
  SS_ASSERT(deviceRate > 0, return false);
  SS_ASSERT(decoded.frames() > 0, return false);

  if (decoded.sampleRate == deviceRate && decoded.channels == kChannels) {
    out.samples = decoded.samples;
    out.frames  = decoded.frames();
    return true;
  }

  ma_data_converter_config config =
    ma_data_converter_config_init(ma_format_f32,
                                  ma_format_f32,
                                  static_cast<ma_uint32>(decoded.channels),
                                  static_cast<ma_uint32>(kChannels),
                                  static_cast<ma_uint32>(decoded.sampleRate),
                                  static_cast<ma_uint32>(deviceRate));

  ma_data_converter converter;
  if (ma_data_converter_init(&config, nullptr, &converter) != MA_SUCCESS) {
    reason = QObject::tr("Sample-rate conversion is unavailable for this file");
    return false;
  }

  ma_uint64 inFrames  = static_cast<ma_uint64>(decoded.frames());
  ma_uint64 outFrames = 0;
  if (ma_data_converter_get_expected_output_frame_count(&converter, inFrames, &outFrames)
        != MA_SUCCESS
      || outFrames == 0) {
    ma_data_converter_uninit(&converter, nullptr);
    reason = QObject::tr("Sample-rate conversion failed for this file");
    return false;
  }

  out.samples.assign(static_cast<std::size_t>(outFrames) * kChannels, 0.0f);
  const ma_result result = ma_data_converter_process_pcm_frames(
    &converter, decoded.samples.data(), &inFrames, out.samples.data(), &outFrames);
  ma_data_converter_uninit(&converter, nullptr);

  if (result != MA_SUCCESS || outFrames == 0) {
    reason = QObject::tr("Sample-rate conversion failed for this file");
    return false;
  }

  out.samples.resize(static_cast<std::size_t>(outFrames) * kChannels);
  out.frames = static_cast<qint64>(outFrames);
  return true;
}
