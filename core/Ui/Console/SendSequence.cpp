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

#include "Console/SendSequence.h"

#include <utility>

#include "Core/Runtime.h"
#include "Core/SSAssert.h"
#include "DataModel/ActionBytes.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

static const QString kPayloadKey  = QStringLiteral("payload");
static const QString kHexKey      = QStringLiteral("hex");
static const QString kEolKey      = QStringLiteral("lineEnding");
static const QString kChecksumKey = QStringLiteral("checksumName");
static const QString kEncodingKey = QStringLiteral("encoding");
static const QString kEnabledKey  = QStringLiteral("enabled");
static const QString kDelayKey    = QStringLiteral("delayMs");

//--------------------------------------------------------------------------------------------------
// Transmission presets
//--------------------------------------------------------------------------------------------------

/**
 * @brief The bytes of a console line-ending ordinal: none, LF, CR or CR LF.
 */
QByteArray Console::lineEndingBytes(const int lineEnding)
{
  switch (lineEnding) {
    case 1:
      return QByteArrayLiteral("\n");
    case 2:
      return QByteArrayLiteral("\r");
    case 3:
      return QByteArrayLiteral("\r\n");
    default:
      return {};
  }
}

/**
 * @brief Reads a preset from a stored pin or rule; absent fields take the send bar's defaults.
 */
Console::TxPreset Console::txPresetFromMap(const QVariantMap& map)
{
  TxPreset preset;
  preset.payload    = map.value(kPayloadKey).toString();
  preset.checksum   = map.value(kChecksumKey).toString();
  preset.lineEnding = qBound(0, map.value(kEolKey, 0).toInt(), 3);
  preset.encoding   = map.value(kEncodingKey, 0).toInt();
  preset.hex        = map.value(kHexKey, false).toBool();
  return preset;
}

/**
 * @brief Stores a preset under the keys a pin uses, so a rule's reply and a pin share one shape.
 */
void Console::writeTxPreset(const TxPreset& preset, QVariantMap& map)
{
  map.insert(kPayloadKey, preset.payload);
  map.insert(kChecksumKey, preset.checksum);
  map.insert(kEolKey, qBound(0, preset.lineEnding, 3));
  map.insert(kEncodingKey, preset.encoding);
  map.insert(kHexKey, preset.hex);
}

/**
 * @brief Encodes a preset through the unified TX encoder the send bar and the Actions use.
 */
QByteArray Console::encodeTxPreset(const TxPreset& preset)
{
  DataModel::TxPayload spec;
  spec.hex      = preset.hex;
  spec.encoding = preset.encoding;
  spec.payload  = preset.payload;
  spec.checksum = preset.checksum;
  spec.eolBytes = lineEndingBytes(preset.lineEnding);
  return DataModel::encode_tx(spec);
}

//--------------------------------------------------------------------------------------------------
// Pin list reading
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether a pin takes part in a list run; a pin saved before lists existed is enabled.
 */
bool Console::pinEnabled(const QVariantMap& pin)
{
  return pin.value(kEnabledKey, true).toBool();
}

/**
 * @brief The delay a pin waits before it is sent, clamped; a pin saved before lists existed
 *        waits the default.
 */
int Console::pinDelayMs(const QVariantMap& pin)
{
  const int stored = pin.value(kDelayKey, kSequenceDelayDefault).toInt();
  return qBound(kSequenceDelayMin, stored, kSequenceDelayMax);
}

/**
 * @brief The first enabled pin with a payload after position @p after, or -1 when none is left.
 */
int Console::nextEnabledPin(const QVariantList& pins, const int after)
{
  SS_ASSERT(after >= -1, return -1);

  const int count = static_cast<int>(pins.count());
  for (int i = after + 1; i < count; ++i) {
    const auto pin = pins.at(i).toMap();
    if (pinEnabled(pin) && !pin.value(kPayloadKey).toString().isEmpty())
      return i;
  }

  return -1;
}

//--------------------------------------------------------------------------------------------------
// Command list runner
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores the link callbacks; the timer is single-shot because every step has its own
 *        delay.
 */
Console::SendSequencer::SendSequencer(SequenceLink link, QObject* parent)
  : QObject(parent), m_link(std::move(link)), m_timer(this), m_index(-1), m_loop(false)
{
  SS_ASSERT_LOG(m_link.connected && m_link.paused && m_link.send);

  m_timer.setSingleShot(true);
  m_timer.setTimerType(Qt::PreciseTimer);
  connect(&m_timer, &QTimer::timeout, this, &SendSequencer::onTick);
}

/**
 * @brief Whether a list is being run.
 */
bool Console::SendSequencer::running() const noexcept
{
  return m_index >= 0;
}

/**
 * @brief Whether the running list starts over after its last pin.
 */
bool Console::SendSequencer::looping() const noexcept
{
  return running() && m_loop;
}

/**
 * @brief Position of the pin that will be sent next, or -1 when no list is running.
 */
int Console::SendSequencer::currentIndex() const noexcept
{
  return m_index;
}

/**
 * @brief Starts a run over a copy of @p pins. Refused in a sealed session, while a run is
 *        already active, with the link closed, or when no enabled pin carries a payload.
 */
bool Console::SendSequencer::start(const QVariantList& pins, const bool loop)
{
  SS_ASSERT(m_link.connected && m_link.send, return false);
  if (Core::Runtime::sealed() || running() || !m_link.connected())
    return false;

  const int first = nextEnabledPin(pins, -1);
  if (first < 0)
    return false;

  m_pins = pins;
  m_loop = loop;
  schedule(first);
  return true;
}

/**
 * @brief Ends the run and forgets its copy of the list.
 */
void Console::SendSequencer::stop()
{
  if (!running())
    return;

  m_timer.stop();
  m_pins.clear();
  m_index = -1;
  m_loop  = false;
  Q_EMIT stateChanged();
}

/**
 * @brief Arms the timer for the pin at @p index. A delay of zero still goes through the event
 *        loop for one millisecond, so a loop of zero-delay pins cannot starve the interface.
 */
void Console::SendSequencer::schedule(const int index)
{
  SS_ASSERT(index >= 0 && index < m_pins.count(), return);

  m_index = index;
  m_timer.start(qMax(1, pinDelayMs(m_pins.at(index).toMap())));
  Q_EMIT stateChanged();
}

/**
 * @brief Sends the current pin and moves to the next enabled one. A closed link ends the run; a
 *        paused link keeps the position and looks again shortly, sending nothing.
 */
void Console::SendSequencer::onTick()
{
  SS_ASSERT(running(), return);
  SS_ASSERT(m_index < m_pins.count(), stop());
  if (!running())
    return;

  if (!m_link.connected()) {
    stop();
    return;
  }

  if (m_link.paused()) {
    m_timer.start(kSequencePausePollMs);
    return;
  }

  const auto preset = txPresetFromMap(m_pins.at(m_index).toMap());
  const auto bytes  = encodeTxPreset(preset);
  if (!bytes.isEmpty() && !(preset.hex && SerialStudio::hexToBytes(preset.payload).isEmpty()))
    m_link.send(bytes);

  int next = nextEnabledPin(m_pins, m_index);
  if (next < 0 && m_loop)
    next = nextEnabledPin(m_pins, -1);

  if (next < 0) {
    stop();
    return;
  }

  schedule(next);
}
