/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include "Console/SendLibrary.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "AppState.h"
#include "Console/Handler.h"
#include "Core/Bus/MessageBus.h"
#include "Core/Bus/Messages.h"
#include "Core/Checksum.h"
#include "Core/SSAssert.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"

static constexpr int kPinCap              = 100;
static constexpr int kHistoryCap          = 100;
static constexpr int kCyclicIntervalMin   = 1;
static constexpr int kCyclicIntervalMax   = 3600000;
static constexpr int kCyclicIntervalReset = 1000;

static const QString kPinsKey     = QStringLiteral("Console/SendPins");
static const QString kHistoryKey  = QStringLiteral("Console/SendHistory");
static const QString kIntervalKey = QStringLiteral("Console/CyclicIntervalMs");
static const QString kTitle       = QStringLiteral("title");
static const QString kPayload     = QStringLiteral("payload");
static const QString kHexFlag     = QStringLiteral("hex");
static const QString kEol         = QStringLiteral("lineEnding");
static const QString kCrcName     = QStringLiteral("checksumName");
static const QString kTextEnc     = QStringLiteral("encoding");

/**
 * @brief Constructs the library and seeds the handler's in-memory history from the persisted
 *        copy, recall cursor past the end exactly as addToHistory() leaves it (R1).
 */
Console::SendLibrary::SendLibrary(Handler& handler)
  : QObject(&handler), m_handler(handler), m_cyclicIntervalMs(kCyclicIntervalReset)
{
  auto items = m_settings.value(kHistoryKey).toStringList();
  while (items.count() > kHistoryCap)
    items.removeFirst();

  m_handler.m_historyItems = items;
  m_handler.m_historyItem  = items.count();

  loadPins();

  const int storedInterval = m_settings.value(kIntervalKey, kCyclicIntervalReset).toInt();
  m_cyclicIntervalMs       = qBound(kCyclicIntervalMin, storedInterval, kCyclicIntervalMax);

  m_cyclicTimer.setTimerType(Qt::PreciseTimer);
  connect(&m_cyclicTimer, &QTimer::timeout, this, &SendLibrary::onCyclicTick);
  connect(m_handler.m_connectionManager,
          &IO::ConnectionManager::connectedChanged,
          this,
          &SendLibrary::onConnectionChanged);

  m_opModeWatch = m_handler.m_bus.subscribe<Core::Bus::OperationModeChanged>(
    this, [this](const std::shared_ptr<const Core::Bus::OperationModeChanged>&) {
      Q_EMIT canPromoteChanged();
    });
}

/**
 * @brief The send history, oldest first; the popover reverses for most-recent-first display.
 */
const QStringList& Console::SendLibrary::history() const noexcept
{
  return m_handler.m_historyItems;
}

/**
 * @brief Writes the current history to app settings; called on manual-send mutation only,
 *        never from a cyclic tick.
 */
void Console::SendLibrary::persistHistory()
{
  m_settings.setValue(kHistoryKey, m_handler.m_historyItems);
  Q_EMIT historyChanged();
}

//--------------------------------------------------------------------------------------------------
// Pins
//--------------------------------------------------------------------------------------------------

/**
 * @brief The app-global pin list, each entry a map of title/payload/hex/lineEnding/
 *        checksumName/encoding.
 */
const QVariantList& Console::SendLibrary::pins() const noexcept
{
  return m_pins;
}

/**
 * @brief Applies the pin's framing state through the handler's setters (combos follow via
 *        their NOTIFY signals) and returns the payload for the send field.
 */
QString Console::SendLibrary::recallPin(int index)
{
  SS_ASSERT(index >= 0 && index < m_pins.count(), return {});

  const auto pin = m_pins.at(index).toMap();
  const int eol  = pin.value(kEol, 0).toInt();
  const int enc  = pin.value(kTextEnc, 0).toInt();
  const bool hex = pin.value(kHexFlag, false).toBool();
  const int crc  = qMax(0, IO::availableChecksums().indexOf(pin.value(kCrcName).toString()));

  m_handler.setEncoding(enc);
  m_handler.setChecksumMethod(crc);
  m_handler.setDataMode(hex ? Handler::DataMode::DataHexadecimal : Handler::DataMode::DataUTF8);
  m_handler.setLineEnding(static_cast<Handler::LineEnding>(qBound(0, eol, 3)));

  return pin.value(kPayload).toString();
}

/**
 * @brief Pins @a payload with the send bar's current framing state under @a title; the list is
 *        capped at 100 entries (Power of Ten fixed bound).
 */
void Console::SendLibrary::addPin(const QString& title, const QString& payload)
{
  if (payload.isEmpty() || m_pins.count() >= kPinCap)
    return;

  QVariantMap pin;
  pin.insert(kTitle, title.isEmpty() ? payload : title);
  pin.insert(kPayload, payload);
  pin.insert(kHexFlag, m_handler.dataMode() == Handler::DataMode::DataHexadecimal);
  pin.insert(kEol, static_cast<int>(m_handler.lineEnding()));
  pin.insert(kCrcName, IO::availableChecksums().value(m_handler.checksumMethod()));
  pin.insert(kTextEnc, m_handler.encoding());

  m_pins.append(pin);
  savePins();
}

/**
 * @brief Removes the pin at @a index.
 */
void Console::SendLibrary::removePin(int index)
{
  SS_ASSERT(index >= 0 && index < m_pins.count(), return);

  m_pins.removeAt(index);
  savePins();
}

/**
 * @brief Renames the pin at @a index; an empty title is refused.
 */
void Console::SendLibrary::renamePin(int index, const QString& title)
{
  SS_ASSERT(index >= 0 && index < m_pins.count(), return);
  if (title.isEmpty())
    return;

  auto pin = m_pins.at(index).toMap();
  pin.insert(kTitle, title);
  m_pins.replace(index, pin);
  savePins();
}

/**
 * @brief Moves the pin at @a from to position @a to.
 */
void Console::SendLibrary::movePin(int from, int to)
{
  SS_ASSERT(from >= 0 && from < m_pins.count(), return);
  SS_ASSERT(to >= 0 && to < m_pins.count(), return);
  if (from == to)
    return;

  m_pins.move(from, to);
  savePins();
}

/**
 * @brief Loads pins from app settings. Settings are user-editable state, not a program
 *        invariant, so malformed data is dropped with plain guards (no debug abort): a corrupt
 *        blob yields an empty list, a malformed entry is skipped, and the loop is bounded.
 */
void Console::SendLibrary::loadPins()
{
  const auto raw = m_settings.value(kPinsKey).toByteArray();
  if (raw.isEmpty())
    return;

  const auto doc = QJsonDocument::fromJson(raw);
  if (!doc.isArray())
    return;

  const auto array = doc.array();
  for (const auto& entry : array) {
    if (m_pins.count() >= kPinCap)
      break;

    if (entry.isObject() && entry.toObject().contains(kPayload))
      m_pins.append(entry.toObject().toVariantMap());
  }
}

/**
 * @brief Persists the pin list to app settings and notifies QML; CRUD-rate only.
 */
void Console::SendLibrary::savePins()
{
  const auto doc = QJsonDocument(QJsonArray::fromVariantList(m_pins));
  m_settings.setValue(kPinsKey, doc.toJson(QJsonDocument::Compact));
  Q_EMIT pinsChanged();
}

//--------------------------------------------------------------------------------------------------
// Cyclic re-send
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns true while the cyclic timer repeats the captured payload.
 */
bool Console::SendLibrary::cyclicArmed() const noexcept
{
  return m_cyclicTimer.isActive();
}

/**
 * @brief The cyclic re-send interval in milliseconds.
 */
int Console::SendLibrary::cyclicIntervalMs() const noexcept
{
  return m_cyclicIntervalMs;
}

/**
 * @brief Captures @a payload and starts repeating it at the configured interval; the payload
 *        is frozen at arm time, so editing the send field does not change what repeats.
 */
void Console::SendLibrary::armCyclic(const QString& payload)
{
  SS_ASSERT(m_handler.m_connectionManager != nullptr, return);
  if (!m_handler.m_connectionManager->isConnected())
    return;

  m_cyclicPayload = payload;
  m_cyclicTimer.start(m_cyclicIntervalMs);
  Q_EMIT cyclicArmedChanged();
}

/**
 * @brief Stops the cyclic timer and drops the captured payload.
 */
void Console::SendLibrary::disarmCyclic()
{
  if (!m_cyclicTimer.isActive())
    return;

  m_cyclicTimer.stop();
  m_cyclicPayload.clear();
  Q_EMIT cyclicArmedChanged();
}

/**
 * @brief Sets and persists the cyclic interval, clamped to [1, 3600000] ms (the 1 ms floor is
 *        a deliberate trust-the-user call, spec 0091 R4); a running timer re-arms at the new
 *        interval.
 */
void Console::SendLibrary::setCyclicIntervalMs(int intervalMs)
{
  const int clamped = qBound(kCyclicIntervalMin, intervalMs, kCyclicIntervalMax);
  if (m_cyclicIntervalMs == clamped)
    return;

  m_cyclicIntervalMs = clamped;
  m_settings.setValue(kIntervalKey, m_cyclicIntervalMs);
  if (m_cyclicTimer.isActive())
    m_cyclicTimer.start(m_cyclicIntervalMs);

  Q_EMIT cyclicIntervalChanged();
}

/**
 * @brief One cyclic repetition: skipped while paused, never recorded in history, disarmed if
 *        the link dropped under the timer.
 */
void Console::SendLibrary::onCyclicTick()
{
  SS_ASSERT(m_handler.m_connectionManager != nullptr, return);
  if (!m_handler.m_connectionManager->isConnected()) {
    disarmCyclic();
    return;
  }

  if (m_handler.m_connectionManager->paused())
    return;

  m_handler.sendPayload(m_cyclicPayload, false);
}

/**
 * @brief Disarms the cyclic timer when the connection closes (spec 0091 R4).
 */
void Console::SendLibrary::onConnectionChanged()
{
  SS_ASSERT(m_handler.m_connectionManager != nullptr, return);
  if (!m_handler.m_connectionManager->isConnected())
    disarmCyclic();
}

//--------------------------------------------------------------------------------------------------
// Promotion to project Actions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Maps a console LineEnding ordinal to the Action tier's escape-sequence EOL text,
 *        resolved only at encode time.
 */
static QString eolSequenceForLineEnding(int lineEnding)
{
  switch (lineEnding) {
    case 1:
      return QStringLiteral("\\n");
    case 2:
      return QStringLiteral("\\r");
    case 3:
      return QStringLiteral("\\r\\n");
    default:
      return {};
  }
}

/**
 * @brief True while a project is open (ProjectFile mode): the only state in which the durable
 *        Action tier exists to promote into.
 */
bool Console::SendLibrary::canPromote() const
{
  SS_ASSERT(m_handler.m_appState != nullptr, return false);
  return m_handler.m_appState->operationMode() == SerialStudio::ProjectFile;
}

/**
 * @brief Promotes the pin at @a index into a project Action through the one-undo-step
 *        compound mutator.
 */
void Console::SendLibrary::promotePin(int index)
{
  SS_ASSERT(index >= 0 && index < m_pins.count(), return);
  if (!canPromote())
    return;

  const auto pin = m_pins.at(index).toMap();

  DataModel::Action action;
  action.title       = pin.value(kTitle).toString();
  action.txData      = pin.value(kPayload).toString();
  action.binaryData  = pin.value(kHexFlag, false).toBool();
  action.txEncoding  = pin.value(kTextEnc, 0).toInt();
  action.checksum    = pin.value(kCrcName).toString();
  action.eolSequence = eolSequenceForLineEnding(pin.value(kEol, 0).toInt());

  m_handler.m_projectModel->addActionFromTemplate(action);
}

/**
 * @brief Promotes @a payload with the send bar's current framing state into a project Action.
 */
void Console::SendLibrary::promoteCurrent(const QString& payload)
{
  if (payload.isEmpty() || !canPromote())
    return;

  DataModel::Action action;
  action.title       = payload;
  action.txData      = payload;
  action.binaryData  = (m_handler.dataMode() == Handler::DataMode::DataHexadecimal);
  action.txEncoding  = m_handler.encoding();
  action.checksum    = IO::availableChecksums().value(m_handler.checksumMethod());
  action.eolSequence = eolSequenceForLineEnding(static_cast<int>(m_handler.lineEnding()));

  m_handler.m_projectModel->addActionFromTemplate(action);
}
