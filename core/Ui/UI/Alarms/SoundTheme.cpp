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

#include "UI/Alarms/SoundTheme.h"

#include <algorithm>
#include <QDir>
#include <QFileInfo>
#include <QObject>

#include "Core/DataModel/FrameKeys.h"
#include "Core/SSAssert.h"
#include "IO/Audio/SoundBank.h"
#include "IO/Audio/SoundPlayer.h"
#include "IO/Audio/WavDecoder.h"

//--------------------------------------------------------------------------------------------------
// Slot catalog
//--------------------------------------------------------------------------------------------------

static constexpr const char* kSlotNames[] = {
  "warning",
  "caution",
  "advisory",
  "ringback",
  "connected",
  "disconnected",
  "link-lost",
  "reconnected",
  "export-finished",
  "recording-started",
  "recording-stopped",
  "error",
  "button",
  "toggle",
};

static_assert(sizeof(kSlotNames) / sizeof(kSlotNames[0]) == UI::Alarms::SoundTheme::kSlotCount,
              "slot name table out of step with the Slot enum");

static constexpr const char* kSettingsGroup = "AlarmSounds";

//--------------------------------------------------------------------------------------------------
// Static catalog helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Event slots have an enable switch; the four alarm signals are always on.
 */
bool UI::Alarms::SoundTheme::isEventSlot(Slot slot) noexcept
{
  return static_cast<int>(slot) >= static_cast<int>(Slot::Connected)
      && static_cast<int>(slot) < kSlotCount;
}

/**
 * @brief Stable slot name used for settings keys, bundled file names and the API.
 */
QString UI::Alarms::SoundTheme::slotName(Slot slot)
{
  const int index = static_cast<int>(slot);
  SS_ASSERT(index >= 0 && (index < kSlotCount), return QString());
  return QString::fromLatin1(kSlotNames[index]);
}

/**
 * @brief The bundled resource for a slot.
 */
QString UI::Alarms::SoundTheme::bundledPath(Slot slot)
{
  return QStringLiteral(":/sounds/%1.wav").arg(slotName(slot));
}

/**
 * @brief Every slot name in enum order.
 */
QStringList UI::Alarms::SoundTheme::slotNames()
{
  QStringList names;
  names.reserve(kSlotCount);
  for (int i = 0; i < kSlotCount; ++i)
    names.append(QString::fromLatin1(kSlotNames[i]));

  return names;
}

/**
 * @brief The alarm signal slot for a priority.
 */
UI::Alarms::Slot UI::Alarms::SoundTheme::slotForPriority(Priority priority) noexcept
{
  switch (priority) {
    case Priority::Warning:
      return Slot::Warning;
    case Priority::Caution:
      return Slot::Caution;
    default:
      return Slot::Advisory;
  }
}

/**
 * @brief Reverse lookup of slotName(); ok is false for an unknown name.
 */
UI::Alarms::Slot UI::Alarms::SoundTheme::slotFromName(const QString& name, bool& ok)
{
  for (int i = 0; i < kSlotCount; ++i) {
    if (name == QLatin1String(kSlotNames[i])) {
      ok = true;
      return static_cast<Slot>(i);
    }
  }

  ok = false;
  return Slot::Warning;
}

/**
 * @brief The ISA-18.1 sequence code: the letter, with a "-4" suffix for the no-lock-in variant.
 */
QString UI::Alarms::SoundTheme::letterFor(Sequence sequence)
{
  switch (sequence) {
    case Sequence::A:
      return QStringLiteral("A");
    case Sequence::M:
      return QStringLiteral("M");
    case Sequence::R:
      return QStringLiteral("R");
    case Sequence::M4:
      return QStringLiteral("M-4");
    case Sequence::R4:
      return QStringLiteral("R-4");
    default:
      return QStringLiteral("A-4");
  }
}

/**
 * @brief Parses a sequence code (A, M, R, A-4, M-4, R-4, case and blanks ignored); ok is
 *        false for anything else and the default A-4 is returned.
 */
UI::Alarms::Sequence UI::Alarms::SoundTheme::sequenceFromLetter(const QString& letter, bool& ok)
{
  static constexpr Sequence kAll[] = {
    Sequence::A, Sequence::M, Sequence::R, Sequence::A4, Sequence::M4, Sequence::R4};
  const QString upper = letter.trimmed().toUpper();
  ok                  = true;
  for (const Sequence candidate : kAll)
    if (upper == letterFor(candidate))
      return candidate;

  ok = false;
  return Sequence::A4;
}

//--------------------------------------------------------------------------------------------------
// Constructor & preference getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Restores the theme from QSettings.
 */
UI::Alarms::SoundTheme::SoundTheme()
  : m_muted(false)
  , m_enabled(false)
  , m_volume(kDefaultVolume)
  , m_nextOverrideSlot(kSlotCount)
  , m_projectSequenceSet(false)
  , m_sequence(Sequence::A4)
  , m_projectSequence(Sequence::A4)
  , m_intervals{0, kCautionIntervalDefMs, kWarningIntervalDefMs}
{
  restore();
}

/**
 * @brief Master mute state.
 */
bool UI::Alarms::SoundTheme::muted() const noexcept
{
  return m_muted;
}

/**
 * @brief Master enable state.
 */
bool UI::Alarms::SoundTheme::enabled() const noexcept
{
  return m_enabled;
}

/**
 * @brief Master volume, 0 to 100.
 */
int UI::Alarms::SoundTheme::volume() const noexcept
{
  return m_volume;
}

/**
 * @brief The app-preference sequence.
 */
UI::Alarms::Sequence UI::Alarms::SoundTheme::sequence() const noexcept
{
  return m_sequence;
}

/**
 * @brief The project's sequence override when it has one, else the app preference.
 */
UI::Alarms::Sequence UI::Alarms::SoundTheme::effectiveSequence() const noexcept
{
  return m_projectSequenceSet ? m_projectSequence : m_sequence;
}

/**
 * @brief Opaque backend id of the chosen output device; empty means the system default.
 */
QByteArray UI::Alarms::SoundTheme::outputDeviceId() const
{
  return m_deviceId;
}

/**
 * @brief Display name of the chosen output device.
 */
QString UI::Alarms::SoundTheme::outputDeviceName() const
{
  return m_deviceName;
}

/**
 * @brief The user's file for a slot; empty means the bundled sound.
 */
QString UI::Alarms::SoundTheme::slotFile(Slot slot) const
{
  const int index = static_cast<int>(slot);
  SS_ASSERT(index >= 0 && (index < kSlotCount), return QString());
  return m_files[static_cast<std::size_t>(index)];
}

/**
 * @brief The file a slot plays: the user's pick or the bundled resource.
 */
QString UI::Alarms::SoundTheme::effectiveFile(Slot slot) const
{
  const QString user = slotFile(slot);
  return user.isEmpty() ? bundledPath(slot) : user;
}

/**
 * @brief Whether an event slot plays; alarm slots always report true.
 */
bool UI::Alarms::SoundTheme::slotEnabled(Slot slot) const noexcept
{
  const int index = static_cast<int>(slot);
  SS_ASSERT(index >= 0 && (index < kSlotCount), return false);
  return m_slotEnabled[static_cast<std::size_t>(index)];
}

/**
 * @brief The repeat interval for a priority; Advisory never repeats and reports zero.
 */
int UI::Alarms::SoundTheme::intervalMs(Priority priority) const noexcept
{
  const int index = static_cast<int>(priority);
  SS_ASSERT(index >= 0 && index <= static_cast<int>(Priority::Warning), return 0);
  return m_intervals[static_cast<std::size_t>(index)];
}

/**
 * @brief The project channel map's file for @p channel at @p priority, resolved against the
 *        project directory; empty when the map has no entry.
 */
QString UI::Alarms::SoundTheme::channelFile(const QString& channel, Priority priority) const
{
  const QJsonObject channels = m_project.value(Keys::Channels).toObject();
  const QJsonObject entry    = channels.value(channel).toObject();
  if (entry.isEmpty())
    return QString();

  QString key;
  switch (priority) {
    case Priority::Warning:
      key = Keys::SoundWarning;
      break;
    case Priority::Caution:
      key = Keys::SoundCaution;
      break;
    default:
      key = Keys::SoundAdvisory;
      break;
  }

  return resolveProjectPath(entry.value(key).toString().trimmed());
}

/**
 * @brief Resolves a project-relative path against the project directory; absolute paths and
 *        resource paths pass through, empty stays empty, and a relative path that climbs out of
 *        the project folder resolves to nothing.
 */
QString UI::Alarms::SoundTheme::resolveProjectPath(const QString& path) const
{
  if (path.isEmpty() || path.startsWith(QLatin1Char(':')) || QDir::isAbsolutePath(path))
    return path;

  if (m_projectDir.isEmpty())
    return path;

  const QStringList parts = QDir::fromNativeSeparators(path).split(QLatin1Char('/'));
  if (parts.contains(QLatin1String("..")))
    return QString();

  return QDir(m_projectDir).absoluteFilePath(path);
}

/**
 * @brief Files that failed to load since the last bank load or override reset.
 */
const std::vector<UI::Alarms::SoundIssue>& UI::Alarms::SoundTheme::issues() const noexcept
{
  return m_issues;
}

//--------------------------------------------------------------------------------------------------
// Preference setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Persists the master mute.
 */
void UI::Alarms::SoundTheme::setMuted(bool muted)
{
  m_muted = muted;
  m_settings.setValue(settingsKey(QStringLiteral("muted")), muted);
}

/**
 * @brief Persists the master enable.
 */
void UI::Alarms::SoundTheme::setEnabled(bool enabled)
{
  m_enabled = enabled;
  m_settings.setValue(settingsKey(QStringLiteral("enabled")), enabled);
}

/**
 * @brief Persists the master volume, clamped to 0 to 100.
 */
void UI::Alarms::SoundTheme::setVolume(int volume)
{
  m_volume = std::clamp(volume, 0, 100);
  m_settings.setValue(settingsKey(QStringLiteral("volume")), m_volume);
}

/**
 * @brief Persists the app-preference sequence.
 */
void UI::Alarms::SoundTheme::setSequence(Sequence sequence)
{
  m_sequence = sequence;
  m_settings.setValue(settingsKey(QStringLiteral("sequence")), letterFor(sequence));
}

/**
 * @brief Persists an event slot enable switch.
 */
void UI::Alarms::SoundTheme::setSlotEnabled(Slot slot, bool enabled)
{
  const int index = static_cast<int>(slot);
  SS_ASSERT(index >= 0 && (index < kSlotCount), return);

  m_slotEnabled[static_cast<std::size_t>(index)] = enabled;
  m_settings.setValue(slotKey(slot, QStringLiteral("enabled")), enabled);
}

/**
 * @brief Persists a repeat interval, clamped to the IEC range.
 */
void UI::Alarms::SoundTheme::setIntervalMs(Priority priority, int ms)
{
  const int index = static_cast<int>(priority);
  SS_ASSERT(index >= 1 && index <= 2, return);

  m_intervals[static_cast<std::size_t>(index)] = clampInterval(priority, ms);
  const QString leaf = priority == Priority::Warning ? QStringLiteral("interval/warning")
                                                     : QStringLiteral("interval/caution");
  m_settings.setValue(settingsKey(leaf), m_intervals[static_cast<std::size_t>(index)]);
}

/**
 * @brief Remembers the chosen output device by opaque id plus display name (the name is what
 *        a lost-device warning shows).
 */
void UI::Alarms::SoundTheme::setOutputDevice(const QByteArray& id, const QString& name)
{
  m_deviceId   = id;
  m_deviceName = name;
  m_settings.setValue(settingsKey(QStringLiteral("outputDeviceId")), id);
  m_settings.setValue(settingsKey(QStringLiteral("outputDeviceName")), name);
}

/**
 * @brief Validates and stores a user file for a slot; an empty path restores the bundled sound.
 *        A refused file leaves the slot untouched and returns the decoder's reason (R10).
 */
bool UI::Alarms::SoundTheme::setSlotFile(Slot slot, const QString& path, QString& reason)
{
  const int index = static_cast<int>(slot);
  SS_ASSERT(index >= 0 && (index < kSlotCount), return false);

  const QString trimmed = path.trimmed();
  if (!trimmed.isEmpty() && !IO::Audio::WavDecoder::probe(trimmed, reason))
    return false;

  m_files[static_cast<std::size_t>(index)] = trimmed;
  m_settings.setValue(slotKey(slot, QStringLiteral("file")), trimmed);
  return true;
}

/**
 * @brief Clears every preference back to the bundled theme.
 */
void UI::Alarms::SoundTheme::resetToDefaults()
{
  m_settings.remove(QString::fromLatin1(kSettingsGroup));
  restore();
}

//--------------------------------------------------------------------------------------------------
// Project overrides
//--------------------------------------------------------------------------------------------------

/**
 * @brief Adopts a project's sounds object; the sequence letter and the channel map are read
 *        lazily by the getters, so an absent key means no override.
 */
void UI::Alarms::SoundTheme::applyProject(const QJsonObject& sounds, const QString& projectDir)
{
  m_project    = sounds;
  m_projectDir = projectDir;

  bool ok              = false;
  const QString letter = sounds.value(Keys::Sequence).toString();
  m_projectSequence    = sequenceFromLetter(letter, ok);
  m_projectSequenceSet = ok;
}

//--------------------------------------------------------------------------------------------------
// Bank loading
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads every theme slot into the player's bank and refreshes the issue list.
 */
void UI::Alarms::SoundTheme::loadBank(IO::Audio::SoundPlayer& player)
{
  m_issues.clear();
  for (int i = 0; i < kSlotCount; ++i)
    loadSlot(static_cast<Slot>(i), player);
}

/**
 * @brief Returns the bank slot holding @p path, loading it into a free override slot on first
 *        use; -1 when the file is unusable or the override slots are exhausted (both reported).
 */
int UI::Alarms::SoundTheme::overrideSlotFor(const QString& path, IO::Audio::SoundPlayer& player)
{
  if (path.isEmpty())
    return -1;

  const auto it = m_overrideSlots.constFind(path);
  if (it != m_overrideSlots.cend())
    return it.value();

  const int slot = loadOverride(path, player);
  m_overrideSlots.insert(path, slot);
  return slot;
}

/**
 * @brief Decodes one override into the next free slot; -1 (recorded as an issue) when the file
 *        is unusable or the slots are exhausted. The result is cached either way, so a broken
 *        file is probed once per bank generation, not once per burst.
 */
int UI::Alarms::SoundTheme::loadOverride(const QString& path, IO::Audio::SoundPlayer& player)
{
  if (m_nextOverrideSlot >= IO::Audio::SoundBank::kSlotCount) {
    addIssue(path, QObject::tr("Too many distinct override sounds in this project"));
    return -1;
  }

  QString reason;
  IO::Audio::DecodedSound decoded;
  if (!IO::Audio::WavDecoder::decode(path, decoded, reason)
      || !player.bank().load(m_nextOverrideSlot, decoded, reason)) {
    addIssue(path, reason);
    return -1;
  }

  removeIssue(path);
  return m_nextOverrideSlot++;
}

/**
 * @brief Drops every override slot (project change) and the issues they raised.
 */
void UI::Alarms::SoundTheme::clearOverrides(IO::Audio::SoundPlayer& player)
{
  for (int slot = kSlotCount; slot < m_nextOverrideSlot; ++slot)
    player.bank().clear(slot);

  m_overrideSlots.clear();
  m_nextOverrideSlot = kSlotCount;
  m_issues.clear();
}

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Full QSettings key under the theme group.
 */
QString UI::Alarms::SoundTheme::settingsKey(const QString& leaf)
{
  return QStringLiteral("%1/%2").arg(QString::fromLatin1(kSettingsGroup), leaf);
}

/**
 * @brief Full QSettings key for one slot property.
 */
QString UI::Alarms::SoundTheme::slotKey(Slot slot, const QString& leaf)
{
  return settingsKey(QStringLiteral("slot/%1/%2").arg(slotName(slot), leaf));
}

/**
 * @brief Clamps a repeat interval to the IEC 60601-1-8 range for its priority.
 */
int UI::Alarms::SoundTheme::clampInterval(Priority priority, int ms) noexcept
{
  if (priority == Priority::Warning)
    return std::clamp(ms, kWarningIntervalMinMs, kWarningIntervalMaxMs);

  return std::clamp(ms, kCautionIntervalMinMs, kCautionIntervalMaxMs);
}

/**
 * @brief Reads every preference; the master enable and UI feedback slots default off,
 *        everything else on.
 */
void UI::Alarms::SoundTheme::restore()
{
  bool ok   = false;
  m_muted   = m_settings.value(settingsKey(QStringLiteral("muted")), false).toBool();
  m_enabled = m_settings.value(settingsKey(QStringLiteral("enabled")), false).toBool();
  m_volume  = std::clamp(
    m_settings.value(settingsKey(QStringLiteral("volume")), kDefaultVolume).toInt(), 0, 100);
  m_sequence = sequenceFromLetter(
    m_settings.value(settingsKey(QStringLiteral("sequence")), QStringLiteral("A-4")).toString(),
    ok);
  m_deviceId   = m_settings.value(settingsKey(QStringLiteral("outputDeviceId"))).toByteArray();
  m_deviceName = m_settings.value(settingsKey(QStringLiteral("outputDeviceName"))).toString();

  m_intervals[2] = clampInterval(
    Priority::Warning,
    m_settings.value(settingsKey(QStringLiteral("interval/warning")), kWarningIntervalDefMs)
      .toInt());
  m_intervals[1] = clampInterval(
    Priority::Caution,
    m_settings.value(settingsKey(QStringLiteral("interval/caution")), kCautionIntervalDefMs)
      .toInt());

  for (int i = 0; i < kSlotCount; ++i) {
    const auto slot      = static_cast<Slot>(i);
    const bool defaultOn = slot != Slot::Button && slot != Slot::Toggle;
    const auto index     = static_cast<std::size_t>(i);
    m_files[index]       = m_settings.value(slotKey(slot, QStringLiteral("file"))).toString();
    m_slotEnabled[index] =
      m_settings.value(slotKey(slot, QStringLiteral("enabled")), defaultOn).toBool();
  }
}

/**
 * @brief Loads one theme slot: the user's file when it decodes, else the bundled sound with the
 *        failure recorded.
 */
void UI::Alarms::SoundTheme::loadSlot(Slot slot, IO::Audio::SoundPlayer& player)
{
  const int index = static_cast<int>(slot);
  QString reason;
  IO::Audio::DecodedSound decoded;

  const QString user = slotFile(slot);
  if (!user.isEmpty()) {
    if (IO::Audio::WavDecoder::decode(user, decoded, reason)
        && player.bank().load(index, decoded, reason)) {
      removeIssue(user);
      return;
    }

    addIssue(user, reason);
  }

  if (IO::Audio::WavDecoder::decode(bundledPath(slot), decoded, reason)
      && player.bank().load(index, decoded, reason))
    return;

  addIssue(bundledPath(slot), reason);
}

/**
 * @brief Forgets a problem once its path loads again.
 */
void UI::Alarms::SoundTheme::removeIssue(const QString& path)
{
  m_issues.erase(std::remove_if(m_issues.begin(),
                                m_issues.end(),
                                [&path](const SoundIssue& i) { return i.path == path; }),
                 m_issues.end());
}

/**
 * @brief Records a problem once per path.
 */
void UI::Alarms::SoundTheme::addIssue(const QString& path, const QString& reason)
{
  for (const auto& issue : m_issues)
    if (issue.path == path)
      return;

  m_issues.push_back(SoundIssue{path, reason});
}
