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

#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>

#include "Core/Bus/Subscription.h"
#include "IO/Audio/SoundPlayer.h"
#include "UI/Alarms/AnnunciatorSequence.h"
#include "UI/Alarms/AppEventSounds.h"
#include "UI/Alarms/SoundTheme.h"

namespace Core::Bus {
class MessageBus;
struct NotificationPosted;
}  // namespace Core::Bus

namespace Misc {
class ProblemCenter;
class TimerEvents;
}  // namespace Misc

namespace DataModel {
class ProjectModel;
class NotificationCenter;
}  // namespace DataModel

namespace IO {
class ConnectionManager;
}  // namespace IO

namespace Console {
class Export;
}  // namespace Console

namespace UI {
class Dashboard;
class AlarmMonitor;

namespace Alarms {

/**
 * @brief The aural alert facade (spec 0087): owns the ISA-18.1 point table, the sound theme,
 *        the app-event hooks and the playback device, and is what QML (Cpp_UI_Alarms), the
 *        palette commands and the alarms.* API talk to. Root-owned, never a singleton.
 */
class AlarmAnnunciator : public QObject {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int highestPriority
             READ highestPriority
             NOTIFY stateChanged)
  Q_PROPERTY(int unacknowledgedCount
             READ unacknowledgedCount
             NOTIFY stateChanged)
  Q_PROPERTY(bool alerting
             READ alerting
             NOTIFY stateChanged)
  Q_PROPERTY(bool sounding
             READ sounding
             NOTIFY stateChanged)
  Q_PROPERTY(int soundingPriority
             READ soundingPriority
             NOTIFY stateChanged)
  Q_PROPERTY(bool ringbackPending
             READ ringbackPending
             NOTIFY stateChanged)
  Q_PROPERTY(QString effectiveSequence
             READ effectiveSequence
             NOTIFY stateChanged)
  Q_PROPERTY(QVariantList points
             READ points
             NOTIFY stateChanged)
  Q_PROPERTY(bool testing
             READ testing
             NOTIFY testingChanged)
  Q_PROPERTY(int testPriority
             READ testPriority
             NOTIFY testingChanged)
  Q_PROPERTY(bool deviceLost
             READ deviceLost
             NOTIFY deviceStateChanged)
  Q_PROPERTY(bool muted
             READ muted
             WRITE setMuted
             NOTIFY themeChanged)
  Q_PROPERTY(bool enabled
             READ enabled
             WRITE setEnabled
             NOTIFY themeChanged)
  Q_PROPERTY(int volume
             READ volume
             WRITE setVolume
             NOTIFY themeChanged)
  Q_PROPERTY(QString sequence
             READ sequence
             WRITE setSequence
             NOTIFY themeChanged)
  Q_PROPERTY(int warningIntervalMs
             READ warningIntervalMs
             NOTIFY themeChanged)
  Q_PROPERTY(int cautionIntervalMs
             READ cautionIntervalMs
             NOTIFY themeChanged)
  Q_PROPERTY(int outputDeviceIndex
             READ outputDeviceIndex
             WRITE setOutputDeviceIndex
             NOTIFY themeChanged)
  Q_PROPERTY(QStringList outputDevices
             READ outputDevices
             NOTIFY outputDevicesChanged)
  Q_PROPERTY(QStringList slotNames
             READ slotNames
             CONSTANT)
  // clang-format on

signals:
  void stateChanged();
  void themeChanged();
  void testingChanged();
  void deviceStateChanged();
  void outputDevicesChanged();

public:
  /**
   * @brief The modules the composition root injects; none is reached through instance().
   */
  struct Modules {
    Core::Bus::MessageBus& bus;
    UI::Dashboard& dashboard;
    Misc::TimerEvents& timers;
    Misc::ProblemCenter* problems;
    DataModel::ProjectModel& project;
    DataModel::NotificationCenter& notifications;
    IO::ConnectionManager& connectionManager;
  };

  explicit AlarmAnnunciator(const Modules& modules, QObject* parent = nullptr);
  ~AlarmAnnunciator() override;
  AlarmAnnunciator(AlarmAnnunciator&&)                 = delete;
  AlarmAnnunciator(const AlarmAnnunciator&)            = delete;
  AlarmAnnunciator& operator=(AlarmAnnunciator&&)      = delete;
  AlarmAnnunciator& operator=(const AlarmAnnunciator&) = delete;

  void arm();
  void stopAudio();
  void setupExternalConnections(bool headless,
                                UI::AlarmMonitor* monitor,
                                Console::Export* consoleExport);

  [[nodiscard]] bool muted() const noexcept;
  [[nodiscard]] bool enabled() const noexcept;
  [[nodiscard]] bool testing() const noexcept;
  [[nodiscard]] bool alerting() const noexcept;
  [[nodiscard]] bool sounding() const noexcept;
  [[nodiscard]] bool deviceLost() const noexcept;
  [[nodiscard]] bool ringbackPending() const noexcept;
  [[nodiscard]] int volume() const noexcept;
  [[nodiscard]] int testPriority() const noexcept;
  [[nodiscard]] int highestPriority() const noexcept;
  [[nodiscard]] int soundingPriority() const noexcept;
  [[nodiscard]] int outputDeviceIndex() const noexcept;
  [[nodiscard]] int warningIntervalMs() const noexcept;
  [[nodiscard]] int cautionIntervalMs() const noexcept;
  [[nodiscard]] int unacknowledgedCount() const noexcept;
  [[nodiscard]] QString resolveProjectPath(const QString& path) const;
  [[nodiscard]] QString sequence() const;
  [[nodiscard]] QString effectiveSequence() const;
  [[nodiscard]] QStringList slotNames() const;
  [[nodiscard]] QStringList outputDevices() const;
  [[nodiscard]] QVariantList points() const;
  [[nodiscard]] QVariantMap stateSnapshot() const;
  [[nodiscard]] QVariantList validateProjectSounds(const QJsonObject& sounds) const;

  Q_INVOKABLE [[nodiscard]] bool slotEnabled(const QString& name) const;
  Q_INVOKABLE [[nodiscard]] int intervalMs(const QString& priority) const;
  Q_INVOKABLE [[nodiscard]] QString slotFile(const QString& name) const;
  Q_INVOKABLE [[nodiscard]] QString bundledFile(const QString& name) const;
  Q_INVOKABLE [[nodiscard]] QString setSlotFile(const QString& name, const QString& path);

public slots:
  void test();
  void clear();
  void reset();
  void silence();
  void acknowledge();
  void resetToDefaults();
  void refreshOutputDevices();
  void setMuted(bool muted);
  void setEnabled(bool enabled);
  void setVolume(int volume);
  void setSequence(const QString& letter);
  void setOutputDeviceIndex(int index);
  void playSlot(const QString& name);
  void playEvent(const QString& name);
  void setSlotEnabled(const QString& name, bool enabled);
  void setIntervalMs(const QString& priority, int ms);

private slots:
  void onTestStep();
  void onDataReset();
  void onHealthTick();
  void onLinkClosed();
  void onRepeatDue();
  void onTrackersRebuilt();
  void onProjectSoundsChanged();
  void onEventRequested(int slot);
  void onBandTransition(int uniqueId,
                        int severity,
                        const QString& title,
                        const QString& label,
                        const QString& sound,
                        double value);

private:
  [[nodiscard]] static qint64 nowMs() noexcept;
  [[nodiscard]] static int notificationId(const QString& channel, const QString& title) noexcept;
  [[nodiscard]] QVariantMap pointToVariant(const Point& point) const;
  [[nodiscard]] int widgetForDataset(int uniqueId, int& groupId) const;
  [[nodiscard]] static Priority priorityFromName(const QString& name) noexcept;

  [[nodiscard]] bool audioWanted() const noexcept;
  [[nodiscard]] int slotForPoint(const Point& point);
  [[nodiscard]] Slot slotFromName(const QString& name, bool& ok) const;

  void onNotificationPosted(const Core::Bus::NotificationPosted& event);
  void raisePoint(const PointKey& key,
                  Priority priority,
                  const QString& title,
                  const QString& channel,
                  const QString& label,
                  const QString& sound);
  void clearPoint(const PointKey& key);
  void dropAllPoints();
  void updateAudible(bool restart);
  void startBurst(Priority priority, int slot, const QString& file);
  void stopAudible();
  void scheduleRepeat();
  void applyProjectOverrides();
  void applyPlayerState();
  void reloadBank();
  void scheduleStateChanged();
  void restoreOverrides();
  void resumeBurstAfterRestart();
  [[nodiscard]] bool tryRecoverDevice();
  void registerChecker();

  bool m_armed;
  bool m_headless;
  bool m_deviceLost;
  bool m_ringbackSounding;
  int m_testStep;
  int m_burstCount;
  int m_soundingSlot;
  int m_outputDeviceIndex;
  int m_retryDelayTicks;
  int m_ticksUntilRetry;
  qint64 m_burstStartedMs;
  Priority m_soundingPriority;
  QString m_soundingFile;
  Modules m_modules;
  QTimer m_testTimer;
  QTimer m_stateTimer;
  QTimer m_repeatTimer;
  SoundTheme m_theme;
  AppEventSounds m_events;
  AnnunciatorSequence m_sequence;
  IO::Audio::SoundPlayer m_player;
  Core::Bus::Subscription m_notifications;
  QList<IO::Audio::SoundPlayer::OutputDevice> m_devices;
};

}  // namespace Alarms
}  // namespace UI
