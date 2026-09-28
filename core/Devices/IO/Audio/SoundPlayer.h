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
#include <QByteArray>
#include <QList>
#include <QString>
#include <QtGlobal>

#include "IO/Audio/SoundBank.h"
#include "ThirdParty/miniaudio.h"

namespace IO {
namespace Audio {

/**
 * @brief Two-lane WAV player on a raw miniaudio playback device (spec 0087). The GUI thread
 *        pushes POD commands into a fixed SPSC ring; the real-time callback drains it and mixes
 *        the alarm lane and the event lane (ducked while an alarm sounds) from the bank's
 *        pre-decoded buffers. The callback never allocates, locks, logs or touches Qt.
 */
class SoundPlayer {
public:
  enum Lane : int {
    AlarmLane = 0,
    EventLane = 1,
  };

  static constexpr int kPeriods      = 3;
  static constexpr int kPeriodMs     = 10;
  static constexpr int kChannels     = SoundBank::kChannels;
  static constexpr int kLaneCount    = 2;
  static constexpr int kSampleRate   = 48000;
  static constexpr int kCommandSlots = 32;
  static constexpr float kDuckGain   = 0.25f;

  /**
   * @brief One playback device as the backend reports it; id is the backend's opaque identifier.
   */
  struct OutputDevice {
    bool isDefault;
    QString name;
    QByteArray id;
  };

  SoundPlayer();
  ~SoundPlayer();
  SoundPlayer(SoundPlayer&&)                 = delete;
  SoundPlayer(const SoundPlayer&)            = delete;
  SoundPlayer& operator=(SoundPlayer&&)      = delete;
  SoundPlayer& operator=(const SoundPlayer&) = delete;

  [[nodiscard]] bool running() const noexcept;
  [[nodiscard]] bool available() const noexcept;
  [[nodiscard]] bool deviceLost() const noexcept;
  [[nodiscard]] bool usingRequestedDevice() const noexcept;
  [[nodiscard]] bool laneActive(Lane lane) const noexcept;
  [[nodiscard]] SoundBank& bank() noexcept;
  [[nodiscard]] const SoundBank& bank() const noexcept;
  [[nodiscard]] QList<OutputDevice> enumerateOutputs() const;

  [[nodiscard]] bool start(const QByteArray& deviceId);
  [[nodiscard]] bool play(Lane lane, int slot, float gain) noexcept;

  void stop();
  void collectGarbage();
  void stopLane(Lane lane) noexcept;
  void setMasterGain(float gain) noexcept;

private:
  enum class Op : quint8 {
    Play = 0,
    Stop = 1,
  };

  struct Command {
    Op op;
    quint8 lane;
    qint16 slot;
    float gain;
  };

  struct LaneState {
    int slot;
    float gain;
    qint64 cursor;
  };

  static void dataCallback(ma_device* device, void* out, const void* in, ma_uint32 frames);
  static void notificationCallback(const ma_device_notification* notification);

  [[nodiscard]] bool pushCommand(const Command& command) noexcept;
  [[nodiscard]] bool resolveDevice(const QByteArray& deviceId, ma_device_id& out) const;

  void drainCommands() noexcept;
  void render(float* out, ma_uint32 frames) noexcept;
  void mixLane(int lane, float* out, ma_uint32 frames, float duck) noexcept;

  bool m_deviceReady;
  bool m_contextReady;
  bool m_usingRequested;
  ma_device m_device;
  ma_context m_context;
  ma_device_id m_deviceId;
  QByteArray m_requestedId;
  alignas(64) std::atomic<bool> m_lost;
  alignas(64) std::atomic<bool> m_running;
  alignas(64) std::atomic<bool> m_stopArmed;
  alignas(64) std::atomic<float> m_masterGain;
  alignas(64) std::atomic<quint32> m_head;
  alignas(64) std::atomic<quint32> m_tail;
  alignas(64) std::atomic<quint64> m_generation;
  std::array<LaneState, kLaneCount> m_lanes;
  std::array<Command, kCommandSlots> m_commands;
  std::array<std::atomic<bool>, kLaneCount> m_laneActive;
  SoundBank m_bank;
};

}  // namespace Audio
}  // namespace IO
