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
#include <QByteArray>
#include <QHash>
#include <QJsonObject>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <vector>

#include "UI/Alarms/AnnunciatorSequence.h"

namespace IO {
namespace Audio {
class SoundPlayer;
}  // namespace Audio
}  // namespace IO

namespace UI {
namespace Alarms {

/**
 * @brief The theme's sound slots: four alarm signals and ten application events (spec 0087
 *        R8, R16). The ordinal doubles as the bank slot index.
 */
enum class Slot : int {
  Warning          = 0,
  Caution          = 1,
  Advisory         = 2,
  Ringback         = 3,
  Connected        = 4,
  Disconnected     = 5,
  LinkLost         = 6,
  Reconnected      = 7,
  ExportFinished   = 8,
  RecordingStarted = 9,
  RecordingStopped = 10,
  Error            = 11,
  Button           = 12,
  Toggle           = 13,
  Count            = 14,
};

/**
 * @brief A sound file that could not be used and why; surfaced through the Problem Center.
 */
struct SoundIssue {
  QString path;
  QString reason;
};

/**
 * @brief The QSettings-backed slot table, the project overrides and the bank loader (spec 0087
 *        R13, R14). Resolution order for an alarm point: band override, project channel map,
 *        the theme slot's user file, the bundled file. A user file that fails to decode falls
 *        back to the bundled sound and is reported, never silently dropped (R12).
 */
class SoundTheme {
public:
  static constexpr int kSlotCount            = static_cast<int>(Slot::Count);
  static constexpr int kDefaultVolume        = 80;
  static constexpr int kWarningIntervalMinMs = 2500;
  static constexpr int kWarningIntervalMaxMs = 15000;
  static constexpr int kCautionIntervalMinMs = 2500;
  static constexpr int kCautionIntervalMaxMs = 30000;
  static constexpr int kWarningIntervalDefMs = 5000;
  static constexpr int kCautionIntervalDefMs = 10000;

  SoundTheme();
  SoundTheme(SoundTheme&&)                 = delete;
  SoundTheme(const SoundTheme&)            = delete;
  SoundTheme& operator=(SoundTheme&&)      = delete;
  SoundTheme& operator=(const SoundTheme&) = delete;

  [[nodiscard]] static bool isEventSlot(Slot slot) noexcept;
  [[nodiscard]] static QString slotName(Slot slot);
  [[nodiscard]] static QString bundledPath(Slot slot);
  [[nodiscard]] static QStringList slotNames();
  [[nodiscard]] static Slot slotForPriority(Priority priority) noexcept;
  [[nodiscard]] static Slot slotFromName(const QString& name, bool& ok);
  [[nodiscard]] static QString letterFor(Sequence sequence);
  [[nodiscard]] static Sequence sequenceFromLetter(const QString& letter, bool& ok);

  [[nodiscard]] bool muted() const noexcept;
  [[nodiscard]] bool enabled() const noexcept;
  [[nodiscard]] int volume() const noexcept;
  [[nodiscard]] Sequence sequence() const noexcept;
  [[nodiscard]] Sequence effectiveSequence() const noexcept;
  [[nodiscard]] QByteArray outputDeviceId() const;
  [[nodiscard]] QString outputDeviceName() const;
  [[nodiscard]] QString slotFile(Slot slot) const;
  [[nodiscard]] QString effectiveFile(Slot slot) const;
  [[nodiscard]] bool slotEnabled(Slot slot) const noexcept;
  [[nodiscard]] int intervalMs(Priority priority) const noexcept;
  [[nodiscard]] QString channelFile(const QString& channel, Priority priority) const;
  [[nodiscard]] QString resolveProjectPath(const QString& path) const;
  [[nodiscard]] const std::vector<SoundIssue>& issues() const noexcept;

  [[nodiscard]] bool setSlotFile(Slot slot, const QString& path, QString& reason);
  [[nodiscard]] int overrideSlotFor(const QString& path, IO::Audio::SoundPlayer& player);

  void setMuted(bool muted);
  void setEnabled(bool enabled);
  void setVolume(int volume);
  void setSequence(Sequence sequence);
  void setSlotEnabled(Slot slot, bool enabled);
  void setIntervalMs(Priority priority, int ms);
  void setOutputDevice(const QByteArray& id, const QString& name);
  void resetToDefaults();

  void applyProject(const QJsonObject& sounds, const QString& projectDir);
  void loadBank(IO::Audio::SoundPlayer& player);
  void clearOverrides(IO::Audio::SoundPlayer& player);

private:
  [[nodiscard]] static QString settingsKey(const QString& leaf);
  [[nodiscard]] static QString slotKey(Slot slot, const QString& leaf);
  [[nodiscard]] static int clampInterval(Priority priority, int ms) noexcept;

  void restore();
  void loadSlot(Slot slot, IO::Audio::SoundPlayer& player);
  void addIssue(const QString& path, const QString& reason);
  void removeIssue(const QString& path);
  [[nodiscard]] int loadOverride(const QString& path, IO::Audio::SoundPlayer& player);

  bool m_muted;
  bool m_enabled;
  int m_volume;
  int m_nextOverrideSlot;
  bool m_projectSequenceSet;
  Sequence m_sequence;
  Sequence m_projectSequence;
  QString m_deviceName;
  QString m_projectDir;
  QByteArray m_deviceId;
  QSettings m_settings;
  QJsonObject m_project;
  QHash<QString, int> m_overrideSlots;
  std::vector<SoundIssue> m_issues;
  std::array<int, 3> m_intervals;
  std::array<bool, kSlotCount> m_slotEnabled;
  std::array<QString, kSlotCount> m_files;
};

}  // namespace Alarms
}  // namespace UI
