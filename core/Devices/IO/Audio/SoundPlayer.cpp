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

#include "IO/Audio/SoundPlayer.h"

#include <algorithm>
#include <cstring>

#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Constructor & destructor
//--------------------------------------------------------------------------------------------------

/**
 * @brief Initializes the platform backend context; a failure leaves the player unavailable and
 *        every later call a no-op, so a machine without audio still runs the annunciator.
 */
IO::Audio::SoundPlayer::SoundPlayer()
  : m_deviceReady(false)
  , m_contextReady(false)
  , m_usingRequested(false)
  , m_lost(false)
  , m_running(false)
  , m_stopArmed(false)
  , m_masterGain(1.0f)
  , m_head(0)
  , m_tail(0)
  , m_generation(0)
  , m_bank(kSampleRate, m_generation)
{
#if defined(Q_OS_WIN)
  ma_backend backend[] = {ma_backend_wasapi};
#elif defined(Q_OS_APPLE)
  ma_backend backend[] = {ma_backend_coreaudio};
#elif defined(Q_OS_LINUX)
  ma_backend backend[] = {ma_backend_alsa};
#else
#  error "Unsupported platform"
#endif

  std::memset(&m_device, 0, sizeof(m_device));
  std::memset(&m_deviceId, 0, sizeof(m_deviceId));
  for (auto& lane : m_lanes)
    lane = LaneState{-1, 1.0f, 0};

  for (auto& active : m_laneActive)
    active.store(false, std::memory_order_relaxed);

  m_contextReady = ma_context_init(backend, 1, nullptr, &m_context) == MA_SUCCESS;
}

/**
 * @brief Stops the device before the bank it reads from is destroyed.
 */
IO::Audio::SoundPlayer::~SoundPlayer()
{
  stop();
  if (m_contextReady)
    ma_context_uninit(&m_context);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief True while the playback device is started and its callback runs.
 */
bool IO::Audio::SoundPlayer::running() const noexcept
{
  return m_running.load(std::memory_order_acquire);
}

/**
 * @brief True when the backend context initialized.
 */
bool IO::Audio::SoundPlayer::available() const noexcept
{
  return m_contextReady;
}

/**
 * @brief True after the backend stopped the device on its own (unplugged, stolen, fatal xrun).
 */
bool IO::Audio::SoundPlayer::deviceLost() const noexcept
{
  return m_lost.load(std::memory_order_acquire);
}

/**
 * @brief True when the running device is the one start() was asked for, false on a fallback.
 */
bool IO::Audio::SoundPlayer::usingRequestedDevice() const noexcept
{
  return m_usingRequested;
}

/**
 * @brief True while @p lane still has samples to play.
 */
bool IO::Audio::SoundPlayer::laneActive(Lane lane) const noexcept
{
  SS_ASSERT(lane >= 0 && (lane < kLaneCount), return false);
  return m_laneActive[static_cast<std::size_t>(lane)].load(std::memory_order_acquire);
}

/**
 * @brief The slot table this player mixes from.
 */
IO::Audio::SoundBank& IO::Audio::SoundPlayer::bank() noexcept
{
  return m_bank;
}

/**
 * @brief Read-only view of the slot table.
 */
const IO::Audio::SoundBank& IO::Audio::SoundPlayer::bank() const noexcept
{
  return m_bank;
}

/**
 * @brief Enumerates playback devices on demand; never called from a callback or a tick.
 */
QList<IO::Audio::SoundPlayer::OutputDevice> IO::Audio::SoundPlayer::enumerateOutputs() const
{
  QList<OutputDevice> list;
  if (!m_contextReady)
    return list;

  ma_device_info* playback = nullptr;
  ma_uint32 count          = 0;
  auto* context            = const_cast<ma_context*>(&m_context);
  if (ma_context_get_devices(context, &playback, &count, nullptr, nullptr) != MA_SUCCESS)
    return list;

  for (ma_uint32 i = 0; i < count; ++i) {
    OutputDevice device;
    device.isDefault = playback[i].isDefault != MA_FALSE;
    device.name      = QString::fromUtf8(playback[i].name);
    device.id        = QByteArray(sizeof(ma_device_id), '\0');
    std::memcpy(device.id.data(), &playback[i].id, sizeof(ma_device_id));
    list.append(device);
  }

  return list;
}

//--------------------------------------------------------------------------------------------------
// Device lifecycle
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts playback on @p deviceId (empty = system default), falling back to the default
 *        when the requested device is not present; the fallback is reported, never hidden.
 */
bool IO::Audio::SoundPlayer::start(const QByteArray& deviceId)
{
  if (!m_contextReady)
    return false;

  stop();
  m_requestedId    = deviceId;
  m_usingRequested = deviceId.isEmpty() || resolveDevice(deviceId, m_deviceId);

  ma_device_config config         = ma_device_config_init(ma_device_type_playback);
  config.pUserData                = this;
  config.dataCallback             = &SoundPlayer::dataCallback;
  config.notificationCallback     = &SoundPlayer::notificationCallback;
  config.sampleRate               = static_cast<ma_uint32>(kSampleRate);
  config.periods                  = static_cast<ma_uint32>(kPeriods);
  config.periodSizeInMilliseconds = static_cast<ma_uint32>(kPeriodMs);
  config.playback.format          = ma_format_f32;
  config.playback.channels        = static_cast<ma_uint32>(kChannels);
  config.playback.pDeviceID       = m_usingRequested && !deviceId.isEmpty() ? &m_deviceId : nullptr;
  config.noPreSilencedOutputBuffer = MA_FALSE;

  std::memset(&m_device, 0, sizeof(m_device));
  if (ma_device_init(&m_context, &config, &m_device) != MA_SUCCESS)
    return false;

  m_deviceReady = true;
  m_head.store(0, std::memory_order_relaxed);
  m_tail.store(0, std::memory_order_relaxed);
  m_lost.store(false, std::memory_order_release);
  m_bank.setDeviceRate(static_cast<int>(m_device.sampleRate));

  if (ma_device_start(&m_device) != MA_SUCCESS) {
    stop();
    return false;
  }

  m_stopArmed.store(true, std::memory_order_release);
  m_running.store(true, std::memory_order_release);
  return true;
}

/**
 * @brief Stops and releases the device; a teardown stop is disarmed first so the backend's
 *        stop notification is not mistaken for a lost device.
 */
void IO::Audio::SoundPlayer::stop()
{
  if (!m_deviceReady)
    return;

  m_stopArmed.store(false, std::memory_order_release);
  m_running.store(false, std::memory_order_release);
  ma_device_uninit(&m_device);
  std::memset(&m_device, 0, sizeof(m_device));
  m_deviceReady = false;

  for (auto& lane : m_lanes)
    lane = LaneState{-1, 1.0f, 0};

  for (auto& active : m_laneActive)
    active.store(false, std::memory_order_release);
}

/**
 * @brief Frees retired bank buffers; safe on the GUI thread at any rate.
 */
void IO::Audio::SoundPlayer::collectGarbage()
{
  m_bank.collectGarbage(!running());
}

//--------------------------------------------------------------------------------------------------
// Commands (GUI thread side)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts @p slot on @p lane at @p gain from its first sample, replacing whatever the lane
 *        was playing; false when the device is down, the slot is empty or the ring is full. The
 *        active flag is raised before the command is published so the callback's own clear of
 *        it is always the later write.
 */
bool IO::Audio::SoundPlayer::play(Lane lane, int slot, float gain) noexcept
{
  SS_ASSERT(lane >= 0 && (lane < kLaneCount), return false);
  SS_ASSERT(slot >= 0 && (slot < SoundBank::kSlotCount), return false);

  if (!running() || !m_bank.loaded(slot))
    return false;

  Command command;
  command.op   = Op::Play;
  command.lane = static_cast<quint8>(lane);
  command.slot = static_cast<qint16>(slot);
  command.gain = std::clamp(gain, 0.0f, 1.0f);
  m_laneActive[static_cast<std::size_t>(lane)].store(true, std::memory_order_release);
  if (pushCommand(command))
    return true;

  m_laneActive[static_cast<std::size_t>(lane)].store(false, std::memory_order_release);
  return false;
}

/**
 * @brief Silences @p lane at the next callback.
 */
void IO::Audio::SoundPlayer::stopLane(Lane lane) noexcept
{
  SS_ASSERT(lane >= 0 && (lane < kLaneCount), return);
  if (!running())
    return;

  Command command;
  command.op   = Op::Stop;
  command.lane = static_cast<quint8>(lane);
  command.slot = -1;
  command.gain = 0.0f;
  (void)pushCommand(command);
}

/**
 * @brief Sets the master gain applied to both lanes (0 to 1).
 */
void IO::Audio::SoundPlayer::setMasterGain(float gain) noexcept
{
  m_masterGain.store(std::clamp(gain, 0.0f, 1.0f), std::memory_order_release);
}

/**
 * @brief Producer side of the SPSC ring; refuses the command when the ring is full.
 */
bool IO::Audio::SoundPlayer::pushCommand(const Command& command) noexcept
{
  const quint32 head = m_head.load(std::memory_order_relaxed);
  const quint32 tail = m_tail.load(std::memory_order_acquire);
  if (head - tail >= static_cast<quint32>(kCommandSlots))
    return false;

  m_commands[head % static_cast<quint32>(kCommandSlots)] = command;
  m_head.store(head + 1, std::memory_order_release);
  return true;
}

/**
 * @brief Looks the requested device up in the current enumeration by its opaque id.
 */
bool IO::Audio::SoundPlayer::resolveDevice(const QByteArray& deviceId, ma_device_id& out) const
{
  SS_ASSERT(m_contextReady, return false);
  if (deviceId.size() != static_cast<qsizetype>(sizeof(ma_device_id)))
    return false;

  const auto devices = enumerateOutputs();
  for (const auto& device : devices) {
    if (device.id != deviceId)
      continue;

    std::memcpy(&out, deviceId.constData(), sizeof(ma_device_id));
    return true;
  }

  return false;
}

//--------------------------------------------------------------------------------------------------
// Real-time callback side
//--------------------------------------------------------------------------------------------------

/**
 * @brief miniaudio data callback; the output buffer arrives pre-silenced.
 */
void IO::Audio::SoundPlayer::dataCallback(ma_device* device,
                                          void* out,
                                          const void* in,
                                          ma_uint32 frames)
{
  (void)in;
  if (!device || !out || frames == 0)
    return;

  auto* self = static_cast<SoundPlayer*>(device->pUserData);
  if (!self)
    return;

  self->render(static_cast<float*>(out), frames);
}

/**
 * @brief Marks the device lost when the backend stops it on its own; a teardown stop is
 *        disarmed beforehand and ignored here.
 */
void IO::Audio::SoundPlayer::notificationCallback(const ma_device_notification* notification)
{
  if (!notification || !notification->pDevice)
    return;

  if (notification->type != ma_device_notification_type_stopped)
    return;

  auto* self = static_cast<SoundPlayer*>(notification->pDevice->pUserData);
  if (!self || !self->m_stopArmed.exchange(false, std::memory_order_acq_rel))
    return;

  self->m_running.store(false, std::memory_order_release);
  self->m_lost.store(true, std::memory_order_release);
}

/**
 * @brief Consumer side of the ring: applies every pending command to the lane states. A lane
 *        remembers its slot, never a buffer pointer, so a bank replacement can never leave it
 *        reading a retired buffer.
 */
void IO::Audio::SoundPlayer::drainCommands() noexcept
{
  quint32 tail       = m_tail.load(std::memory_order_relaxed);
  const quint32 head = m_head.load(std::memory_order_acquire);
  for (int i = 0; i < kCommandSlots && tail != head; ++i, ++tail) {
    const Command& command = m_commands[tail % static_cast<quint32>(kCommandSlots)];
    const auto index       = static_cast<std::size_t>(command.lane % kLaneCount);
    LaneState& lane        = m_lanes[index];
    lane.cursor            = 0;
    lane.gain              = command.gain;
    lane.slot              = command.op == Op::Play ? command.slot : -1;
    m_laneActive[index].store(lane.slot >= 0, std::memory_order_release);
  }

  m_tail.store(tail, std::memory_order_release);
}

/**
 * @brief Mixes both lanes into the pre-silenced output and advances the generation counter.
 */
void IO::Audio::SoundPlayer::render(float* out, ma_uint32 frames) noexcept
{
  drainCommands();

  const bool alarmActive = m_lanes[AlarmLane].slot >= 0;
  mixLane(AlarmLane, out, frames, 1.0f);
  mixLane(EventLane, out, frames, alarmActive ? kDuckGain : 1.0f);
  m_generation.fetch_add(1, std::memory_order_release);
}

/**
 * @brief Adds one lane's samples to @p out and releases the lane when its sound ends. The slot
 *        pointer is re-acquired every callback: it stays valid for this callback by the bank's
 *        retirement rule, and a slot emptied or replaced mid-burst ends or continues the lane
 *        from the current cursor instead of dereferencing freed memory.
 */
void IO::Audio::SoundPlayer::mixLane(int lane, float* out, ma_uint32 frames, float duck) noexcept
{
  LaneState& state = m_lanes[static_cast<std::size_t>(lane)];
  if (state.slot < 0)
    return;

  const SoundBank::Sound* sound = m_bank.acquire(state.slot);
  const qint64 total            = sound ? sound->frames : 0;
  if (!sound || state.cursor >= total) {
    state.slot = -1;
    m_laneActive[static_cast<std::size_t>(lane)].store(false, std::memory_order_release);
    return;
  }

  const float gain     = state.gain * duck * m_masterGain.load(std::memory_order_acquire);
  const float* samples = sound->samples.data();
  const qint64 first   = state.cursor;
  const qint64 last    = (std::min)(total, first + static_cast<qint64>(frames));
  for (qint64 frame = first; frame < last; ++frame) {
    const auto src  = static_cast<std::size_t>(frame) * kChannels;
    const auto dst  = static_cast<std::size_t>(frame - first) * kChannels;
    out[dst]       += samples[src] * gain;
    out[dst + 1]   += samples[src + 1] * gain;
  }

  state.cursor = last;
  if (last < total)
    return;

  state.slot = -1;
  m_laneActive[static_cast<std::size_t>(lane)].store(false, std::memory_order_release);
}
