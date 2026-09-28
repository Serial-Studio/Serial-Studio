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

#include "UI/Alarms/AlarmAnnunciator.h"

#include <algorithm>
#include <QDateTime>
#include <QFileInfo>
#include <QHash>
#include <QUrl>

#include "API/HandlerContext.h"
#include "Console/Export.h"
#include "Core/Bus/MessageBus.h"
#include "Core/Bus/Messages.h"
#include "Core/DataModel/FrameKeys.h"
#include "Core/SerialStudio.h"
#include "Core/SSAssert.h"
#include "Core/TimerEvents.h"
#include "CSV/Export.h"
#include "DataModel/NotificationCenter.h"
#include "DataModel/ProjectModel.h"
#include "IO/ConnectionManager.h"
#include "MDF4/Export.h"
#include "Misc/ProblemCenter.h"
#include "UI/AlarmMonitor.h"
#include "UI/Dashboard.h"

#ifdef BUILD_COMMERCIAL
#  include "Sessions/Export.h"
#endif

//--------------------------------------------------------------------------------------------------
// Tunables
//--------------------------------------------------------------------------------------------------

static constexpr int kTestStepMs                             = 1500;
static constexpr int kTestStepCount                          = 4;
static constexpr float kVolumeScale                          = 0.01f;
static constexpr int kMaxRetryTicks                          = 30;
static constexpr qint64 kReflashMinMs                        = 300;
static const QString kResolvedPrefix                         = QStringLiteral("Resolved: ");
static const QString kProblemsChannel                        = QStringLiteral("Problems");
static const QString kSystemChannel                          = QStringLiteral("System");
static const QString kCheckerId                              = QStringLiteral("alarms.sounds");
static constexpr UI::Alarms::Slot kTestOrder[kTestStepCount] = {
  UI::Alarms::Slot::Advisory,
  UI::Alarms::Slot::Caution,
  UI::Alarms::Slot::Warning,
  UI::Alarms::Slot::Ringback,
};

//--------------------------------------------------------------------------------------------------
// Constructor & wiring
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the facade around injected modules; nothing is wired or started here.
 */
UI::Alarms::AlarmAnnunciator::AlarmAnnunciator(const Modules& modules, QObject* parent)
  : QObject(parent)
  , m_armed(false)
  , m_headless(true)
  , m_deviceLost(false)
  , m_ringbackSounding(false)
  , m_testStep(-1)
  , m_burstCount(0)
  , m_soundingSlot(-1)
  , m_outputDeviceIndex(0)
  , m_retryDelayTicks(1)
  , m_ticksUntilRetry(0)
  , m_burstStartedMs(0)
  , m_soundingPriority(Priority::None)
  , m_modules(modules)
  , m_events(modules.bus, modules.connectionManager, this)
  , m_sequence(Sequence::A4)
{
  m_testTimer.setSingleShot(true);
  m_repeatTimer.setSingleShot(true);
  m_testTimer.setTimerType(Qt::PreciseTimer);
  m_repeatTimer.setTimerType(Qt::PreciseTimer);
  m_stateTimer.setSingleShot(true);
  m_stateTimer.setInterval(0);
  connect(&m_stateTimer, &QTimer::timeout, this, &AlarmAnnunciator::stateChanged);
}

/**
 * @brief Stops the device before the members it reads are torn down.
 */
UI::Alarms::AlarmAnnunciator::~AlarmAnnunciator()
{
  m_player.stop();
}

/**
 * @brief Quits the audio side while QML may still be bound to this object: joins the device
 *        thread and stops the timers; the object itself lives until the root releases it after
 *        the QML engine is gone, so no binding ever sees a null Cpp_UI_Alarms.
 */
void UI::Alarms::AlarmAnnunciator::stopAudio()
{
  m_testTimer.stop();
  m_repeatTimer.stop();
  m_player.stop();
}

/**
 * @brief Coalesces stateChanged into one emission per event-loop turn: a burst of band
 *        transitions on one display tick otherwise re-evaluates every QML binding (and rebuilds
 *        the points list) once per point.
 */
void UI::Alarms::AlarmAnnunciator::scheduleStateChanged()
{
  if (!m_stateTimer.isActive())
    m_stateTimer.start();
}

/**
 * @brief Marks project restore complete: from here on a connect is an operator event.
 */
void UI::Alarms::AlarmAnnunciator::arm()
{
  m_armed = true;
  m_events.arm();
}

/**
 * @brief Wires every input on the GUI thread (direct connections, command rate), registers the
 *        Problem Center checker, adopts the loaded project's overrides and starts the player. A
 *        headless root keeps the state machine and the API, never opens a device, and passes no
 *        monitor because nothing evaluates bands there.
 */
void UI::Alarms::AlarmAnnunciator::setupExternalConnections(bool headless,
                                                            UI::AlarmMonitor* monitor,
                                                            Console::Export* consoleExport)
{
  SS_ASSERT(!m_notifications.isActive(), return);

  m_headless = headless;
  if (monitor) {
    connect(monitor, &UI::AlarmMonitor::bandTransition, this, &AlarmAnnunciator::onBandTransition);
    connect(
      monitor, &UI::AlarmMonitor::trackersRebuilt, this, &AlarmAnnunciator::onTrackersRebuilt);
  }

  connect(&m_modules.dashboard, &UI::Dashboard::dataReset, this, &AlarmAnnunciator::onDataReset);
  connect(&m_modules.timers, &Misc::TimerEvents::timeout1Hz, this, &AlarmAnnunciator::onHealthTick);
  connect(&m_testTimer, &QTimer::timeout, this, &AlarmAnnunciator::onTestStep);
  connect(&m_repeatTimer, &QTimer::timeout, this, &AlarmAnnunciator::onRepeatDue);
  connect(&m_events, &AppEventSounds::linkClosed, this, &AlarmAnnunciator::onLinkClosed);
  connect(&m_events, &AppEventSounds::eventRequested, this, &AlarmAnnunciator::onEventRequested);

  auto& project = m_modules.project;
  connect(&project,
          &DataModel::ProjectModel::soundsChanged,
          this,
          &AlarmAnnunciator::onProjectSoundsChanged);
  connect(&project,
          &DataModel::ProjectModel::jsonFileChanged,
          this,
          &AlarmAnnunciator::onProjectSoundsChanged);

  m_notifications = m_modules.bus.subscribe<Core::Bus::NotificationPosted>(
    this, [this](const std::shared_ptr<const Core::Bus::NotificationPosted>& event) {
      onNotificationPosted(*event);
    });

  auto& context = API::handlerContext();
  m_events.watchRecordingSink(&context.csvExport, &CSV::Export::openChanged, [&context] {
    return context.csvExport.isOpen();
  });
  m_events.watchRecordingSink(&context.mdf4Export, &MDF4::Export::openChanged, [&context] {
    return context.mdf4Export.isOpen();
  });
  if (consoleExport)
    m_events.watchRecordingSink(consoleExport, &Console::Export::openChanged, [consoleExport] {
      return consoleExport->isOpen();
    });
#ifdef BUILD_COMMERCIAL
  m_events.watchRecordingSink(&context.sessionsExport, &Sessions::Export::openChanged, [&context] {
    return context.sessionsExport.isOpen();
  });
#endif
  m_events.setupExternalConnections();

  registerChecker();
  applyProjectOverrides();
  applyPlayerState();
}

//--------------------------------------------------------------------------------------------------
// State getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Master mute.
 */
bool UI::Alarms::AlarmAnnunciator::muted() const noexcept
{
  return m_theme.muted();
}

/**
 * @brief Master enable.
 */
bool UI::Alarms::AlarmAnnunciator::enabled() const noexcept
{
  return m_theme.enabled();
}

/**
 * @brief True while the Test sequence runs.
 */
bool UI::Alarms::AlarmAnnunciator::testing() const noexcept
{
  return m_testStep >= 0;
}

/**
 * @brief True while any point is unacknowledged; the master annunciator flashes on it.
 */
bool UI::Alarms::AlarmAnnunciator::alerting() const noexcept
{
  return m_sequence.anyAlert();
}

/**
 * @brief True while the alarm lane has a burst scheduled or playing (ringback included).
 */
bool UI::Alarms::AlarmAnnunciator::sounding() const noexcept
{
  return m_soundingPriority != Priority::None || m_ringbackSounding;
}

/**
 * @brief True while playback fell back from the chosen device to the system default.
 */
bool UI::Alarms::AlarmAnnunciator::deviceLost() const noexcept
{
  return m_deviceLost;
}

/**
 * @brief True when sequence R holds a Return-to-normal point whose ringback still sounds.
 */
bool UI::Alarms::AlarmAnnunciator::ringbackPending() const noexcept
{
  return m_sequence.ringbackPending();
}

/**
 * @brief Master volume, 0 to 100.
 */
int UI::Alarms::AlarmAnnunciator::volume() const noexcept
{
  return m_theme.volume();
}

/**
 * @brief The priority of the Test step that is sounding (m_testStep already points at the next
 *        step), -1 when idle or during the ringback step.
 */
int UI::Alarms::AlarmAnnunciator::testPriority() const noexcept
{
  const int sounding = m_testStep - 1;
  if (m_testStep < 0 || sounding < 0 || sounding >= kTestStepCount)
    return -1;

  switch (kTestOrder[sounding]) {
    case Slot::Advisory:
      return static_cast<int>(Priority::Advisory);
    case Slot::Caution:
      return static_cast<int>(Priority::Caution);
    case Slot::Warning:
      return static_cast<int>(Priority::Warning);
    default:
      return -1;
  }
}

/**
 * @brief Highest priority among active points, -1 when none.
 */
int UI::Alarms::AlarmAnnunciator::highestPriority() const noexcept
{
  return static_cast<int>(m_sequence.highestActivePriority());
}

/**
 * @brief Priority of the burst on the alarm lane, -1 when quiet or ringing back.
 */
int UI::Alarms::AlarmAnnunciator::soundingPriority() const noexcept
{
  return static_cast<int>(m_soundingPriority);
}

/**
 * @brief Index into outputDevices(); 0 is always the system default entry.
 */
int UI::Alarms::AlarmAnnunciator::outputDeviceIndex() const noexcept
{
  return m_outputDeviceIndex;
}

/**
 * @brief Points in Alert.
 */
int UI::Alarms::AlarmAnnunciator::unacknowledgedCount() const noexcept
{
  return m_sequence.unacknowledgedCount();
}

/**
 * @brief Warning repeat interval, QML-bindable.
 */
int UI::Alarms::AlarmAnnunciator::warningIntervalMs() const noexcept
{
  return m_theme.intervalMs(Priority::Warning);
}

/**
 * @brief Caution repeat interval, QML-bindable.
 */
int UI::Alarms::AlarmAnnunciator::cautionIntervalMs() const noexcept
{
  return m_theme.intervalMs(Priority::Caution);
}

/**
 * @brief Resolves a project sound path the way the theme does (the API handler's policy check
 *        needs the same absolute path the decoder will open).
 */
QString UI::Alarms::AlarmAnnunciator::resolveProjectPath(const QString& path) const
{
  return m_theme.resolveProjectPath(path);
}

/**
 * @brief The app-preference sequence letter.
 */
QString UI::Alarms::AlarmAnnunciator::sequence() const
{
  return SoundTheme::letterFor(m_theme.sequence());
}

/**
 * @brief The sequence in force (project override or preference).
 */
QString UI::Alarms::AlarmAnnunciator::effectiveSequence() const
{
  return SoundTheme::letterFor(m_theme.effectiveSequence());
}

/**
 * @brief Every theme slot name in enum order.
 */
QStringList UI::Alarms::AlarmAnnunciator::slotNames() const
{
  return SoundTheme::slotNames();
}

/**
 * @brief Display names of the output devices as last enumerated; entry 0 is the system default.
 */
QStringList UI::Alarms::AlarmAnnunciator::outputDevices() const
{
  QStringList names;
  names.append(tr("System default"));
  for (const auto& device : m_devices)
    names.append(device.name);

  return names;
}

/**
 * @brief Every active point, most recently raised first, for the annunciator panel and the API.
 */
QVariantList UI::Alarms::AlarmAnnunciator::points() const
{
  std::vector<const Point*> sorted;
  sorted.reserve(m_sequence.points().size());
  for (const auto& point : m_sequence.points())
    sorted.push_back(&point);

  std::stable_sort(sorted.begin(), sorted.end(), [](const Point* a, const Point* b) {
    return a->sinceMs > b->sinceMs;
  });

  QVariantList list;
  list.reserve(static_cast<qsizetype>(sorted.size()));
  for (const auto* point : sorted)
    list.append(pointToVariant(*point));

  return list;
}

/**
 * @brief The full state the alarms.state verb returns.
 */
QVariantMap UI::Alarms::AlarmAnnunciator::stateSnapshot() const
{
  QVariantMap m;
  m.insert(QStringLiteral("sequence"), effectiveSequence());
  m.insert(QStringLiteral("muted"), muted());
  m.insert(QStringLiteral("enabled"), enabled());
  m.insert(QStringLiteral("alerting"), alerting());
  m.insert(QStringLiteral("deviceLost"), m_deviceLost);
  m.insert(QStringLiteral("audioRunning"), m_player.running());
  m.insert(QStringLiteral("highestPriority"), highestPriority());
  m.insert(QStringLiteral("unacknowledgedCount"), unacknowledgedCount());
  m.insert(QStringLiteral("ringbackPending"), ringbackPending());
  m.insert(QStringLiteral("points"), points());

  if (!sounding()) {
    m.insert(QStringLiteral("sounding"), QVariant());
    return m;
  }

  QVariantMap s;
  s.insert(QStringLiteral("priority"), m_ringbackSounding ? -1 : soundingPriority());
  s.insert(QStringLiteral("ringback"), m_ringbackSounding);
  s.insert(QStringLiteral("slot"), m_soundingSlot);
  s.insert(QStringLiteral("file"), m_soundingFile);
  s.insert(QStringLiteral("burstStartedMs"), m_burstStartedMs);
  s.insert(QStringLiteral("burstCount"), m_burstCount);
  m.insert(QStringLiteral("sounding"), s);
  return m;
}

/**
 * @brief Probes every file a project sounds object names; returns the rejected ones.
 */
QVariantList UI::Alarms::AlarmAnnunciator::validateProjectSounds(const QJsonObject& sounds) const
{
  QVariantList rejected;
  const QJsonObject channels = sounds.value(Keys::Channels).toObject();
  const QStringList keys{Keys::SoundWarning, Keys::SoundCaution, Keys::SoundAdvisory};
  for (auto it = channels.constBegin(); it != channels.constEnd(); ++it) {
    const QJsonObject entry = it.value().toObject();
    for (const auto& key : keys) {
      const QString raw  = entry.value(key).toString().trimmed();
      const QString path = m_theme.resolveProjectPath(raw);
      QString reason     = tr("Path climbs out of the project folder");
      if (raw.isEmpty() || (!path.isEmpty() && IO::Audio::WavDecoder::probe(path, reason)))
        continue;

      QVariantMap r;
      r.insert(QStringLiteral("channel"), it.key());
      r.insert(QStringLiteral("priority"), key);
      r.insert(QStringLiteral("path"), path);
      r.insert(QStringLiteral("reason"), reason);
      rejected.append(r);
    }
  }

  return rejected;
}

//--------------------------------------------------------------------------------------------------
// Theme surface (QML + API)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether an event slot plays.
 */
bool UI::Alarms::AlarmAnnunciator::slotEnabled(const QString& name) const
{
  bool ok         = false;
  const Slot slot = slotFromName(name, ok);
  return ok && m_theme.slotEnabled(slot);
}

/**
 * @brief Repeat interval for a priority name.
 */
int UI::Alarms::AlarmAnnunciator::intervalMs(const QString& priority) const
{
  const Priority p = priorityFromName(priority);
  return p == Priority::None ? 0 : m_theme.intervalMs(p);
}

/**
 * @brief The user file for a slot; empty means bundled.
 */
QString UI::Alarms::AlarmAnnunciator::slotFile(const QString& name) const
{
  bool ok         = false;
  const Slot slot = slotFromName(name, ok);
  return ok ? m_theme.slotFile(slot) : QString();
}

/**
 * @brief The bundled resource path for a slot.
 */
QString UI::Alarms::AlarmAnnunciator::bundledFile(const QString& name) const
{
  bool ok         = false;
  const Slot slot = slotFromName(name, ok);
  return ok ? SoundTheme::bundledPath(slot) : QString();
}

/**
 * @brief Stores a user file for a slot and reloads the bank; returns the refusal reason, or an
 *        empty string on success (an empty path restores the bundled sound).
 */
QString UI::Alarms::AlarmAnnunciator::setSlotFile(const QString& name, const QString& path)
{
  bool ok         = false;
  const Slot slot = slotFromName(name, ok);
  if (!ok)
    return tr("Unknown sound slot '%1'").arg(name);

  QString reason;
  const QString local = path.startsWith(QLatin1String("file:")) ? QUrl(path).toLocalFile() : path;
  if (!m_theme.setSlotFile(slot, local, reason))
    return reason;

  reloadBank();
  Q_EMIT themeChanged();
  return QString();
}

/**
 * @brief Master mute: stops the device without touching the point table.
 */
void UI::Alarms::AlarmAnnunciator::setMuted(bool muted)
{
  if (m_theme.muted() == muted)
    return;

  m_theme.setMuted(muted);
  applyPlayerState();
  Q_EMIT themeChanged();
  scheduleStateChanged();
}

/**
 * @brief Master enable: starts or stops the device.
 */
void UI::Alarms::AlarmAnnunciator::setEnabled(bool enabled)
{
  if (m_theme.enabled() == enabled)
    return;

  m_theme.setEnabled(enabled);
  applyPlayerState();
  Q_EMIT themeChanged();
  scheduleStateChanged();
}

/**
 * @brief Master volume, applied to both lanes immediately.
 */
void UI::Alarms::AlarmAnnunciator::setVolume(int volume)
{
  if (m_theme.volume() == volume)
    return;

  m_theme.setVolume(volume);
  m_player.setMasterGain(static_cast<float>(m_theme.volume()) * kVolumeScale);
  Q_EMIT themeChanged();
}

/**
 * @brief Sets the app-preference sequence; the project override, when present, still wins.
 */
void UI::Alarms::AlarmAnnunciator::setSequence(const QString& letter)
{
  bool ok            = false;
  const Sequence seq = SoundTheme::sequenceFromLetter(letter, ok);
  if (!ok || seq == m_theme.sequence())
    return;

  m_theme.setSequence(seq);
  m_sequence.setSequence(m_theme.effectiveSequence());
  updateAudible(false);
  Q_EMIT themeChanged();
  scheduleStateChanged();
}

/**
 * @brief Picks an output device from the last enumeration (0 = system default) and restarts
 *        playback on it.
 */
void UI::Alarms::AlarmAnnunciator::setOutputDeviceIndex(int index)
{
  const bool storedIsDefault = m_theme.outputDeviceId().isEmpty();
  if (index < 0 || index > m_devices.size())
    return;

  if (index == m_outputDeviceIndex && (index > 0 || storedIsDefault))
    return;

  m_outputDeviceIndex = index;
  m_retryDelayTicks   = 1;
  m_ticksUntilRetry   = 0;
  if (index == 0)
    m_theme.setOutputDevice(QByteArray(), QString());
  else
    m_theme.setOutputDevice(m_devices[index - 1].id, m_devices[index - 1].name);

  m_player.stop();
  applyPlayerState();
  Q_EMIT themeChanged();
}

/**
 * @brief Previews a slot on the event lane (the Sounds page's play buttons).
 */
void UI::Alarms::AlarmAnnunciator::playSlot(const QString& name)
{
  bool ok         = false;
  const Slot slot = slotFromName(name, ok);
  if (!ok || !audioWanted())
    return;

  (void)m_player.play(IO::Audio::SoundPlayer::EventLane, static_cast<int>(slot), 1.0f);
}

/**
 * @brief Plays an event slot when its switch is on (QML feedback hooks and the event sub-object).
 */
void UI::Alarms::AlarmAnnunciator::playEvent(const QString& name)
{
  bool ok         = false;
  const Slot slot = slotFromName(name, ok);
  if (!ok || !SoundTheme::isEventSlot(slot))
    return;

  onEventRequested(static_cast<int>(slot));
}

/**
 * @brief Toggles an event slot.
 */
void UI::Alarms::AlarmAnnunciator::setSlotEnabled(const QString& name, bool enabled)
{
  bool ok         = false;
  const Slot slot = slotFromName(name, ok);
  if (!ok || !SoundTheme::isEventSlot(slot))
    return;

  m_theme.setSlotEnabled(slot, enabled);
  Q_EMIT themeChanged();
}

/**
 * @brief Sets a repeat interval and re-arms the timer.
 */
void UI::Alarms::AlarmAnnunciator::setIntervalMs(const QString& priority, int ms)
{
  const Priority p = priorityFromName(priority);
  if (p != Priority::Warning && p != Priority::Caution)
    return;

  m_theme.setIntervalMs(p, ms);
  scheduleRepeat();
  Q_EMIT themeChanged();
}

/**
 * @brief Restores the bundled theme and the system default device.
 */
void UI::Alarms::AlarmAnnunciator::resetToDefaults()
{
  m_theme.resetToDefaults();
  m_outputDeviceIndex = 0;
  m_sequence.setSequence(m_theme.effectiveSequence());
  m_player.stop();
  applyPlayerState();
  Q_EMIT themeChanged();
  scheduleStateChanged();
}

/**
 * @brief Re-enumerates output devices (the Sounds page opening); never on a tick.
 */
void UI::Alarms::AlarmAnnunciator::refreshOutputDevices()
{
  m_devices           = m_player.enumerateOutputs();
  m_outputDeviceIndex = 0;
  const QByteArray id = m_theme.outputDeviceId();
  for (int i = 0; i < m_devices.size(); ++i)
    if (!id.isEmpty() && m_devices[i].id == id)
      m_outputDeviceIndex = i + 1;

  Q_EMIT outputDevicesChanged();
  Q_EMIT themeChanged();
}

//--------------------------------------------------------------------------------------------------
// Operator actions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Acknowledge: every Alert point settles, the audible stops.
 */
void UI::Alarms::AlarmAnnunciator::acknowledge()
{
  (void)m_sequence.acknowledge();
  updateAudible(false);
  scheduleStateChanged();
}

/**
 * @brief Silence: audible off, points stay in Alert; a new raise re-sounds.
 */
void UI::Alarms::AlarmAnnunciator::silence()
{
  (void)m_sequence.silence();
  updateAudible(false);
  scheduleStateChanged();
}

/**
 * @brief Reset: Return-to-normal points go Normal (sequences M and R).
 */
void UI::Alarms::AlarmAnnunciator::reset()
{
  (void)m_sequence.reset();
  updateAudible(false);
  scheduleStateChanged();
}

/**
 * @brief Clear: the operator dismisses the whole point table, acknowledged and alerting points
 *        included; a point still in its band returns on the next transition.
 */
void UI::Alarms::AlarmAnnunciator::clear()
{
  dropAllPoints();
}

/**
 * @brief Plays Advisory, Caution, Warning and Ringback in turn on the event lane and walks the
 *        master annunciator through each priority; no point is created.
 */
void UI::Alarms::AlarmAnnunciator::test()
{
  if (testing())
    return;

  m_testStep = 0;
  onTestStep();
}

//--------------------------------------------------------------------------------------------------
// Inputs
//--------------------------------------------------------------------------------------------------

/**
 * @brief A dataset changed band: severity 2 or 3 raises the point, anything else clears it.
 */
void UI::Alarms::AlarmAnnunciator::onBandTransition(int uniqueId,
                                                    int severity,
                                                    const QString& title,
                                                    const QString& label,
                                                    const QString& sound,
                                                    double value)
{
  (void)value;
  const PointKey key{PointKind::Band, uniqueId};
  if (severity < 2) {
    clearPoint(key);
    return;
  }

  const Priority priority = severity >= 3 ? Priority::Warning : Priority::Caution;
  raisePoint(key, priority, title, tr("Alarms"), label, m_theme.resolveProjectPath(sound));
}

/**
 * @brief Notification points: Critical raises Warning, Warning raises Caution, a "Resolved: X"
 *        Info clears X, any other Info is an Advisory one-shot. The Problems and System channels
 *        are the app's own diagnostics and the Alarms channel is the band monitor's own
 *        notification of a point that already exists, so none of them become points.
 */
void UI::Alarms::AlarmAnnunciator::onNotificationPosted(const Core::Bus::NotificationPosted& event)
{
  if (event.channel == kProblemsChannel || event.channel == kSystemChannel
      || event.channel == UI::AlarmMonitor::tr("Alarms"))
    return;

  if (event.severity >= Core::Bus::kSeverityWarning) {
    const Priority priority =
      event.severity >= Core::Bus::kSeverityCritical ? Priority::Warning : Priority::Caution;
    const PointKey key{PointKind::Notification, notificationId(event.channel, event.title)};
    raisePoint(key,
               priority,
               event.title,
               event.channel,
               event.text,
               m_theme.channelFile(event.channel, priority));
    return;
  }

  if (event.title.startsWith(kResolvedPrefix)) {
    const QString title = event.title.mid(kResolvedPrefix.size());
    clearPoint(PointKey{PointKind::Notification, notificationId(event.channel, title)});
    return;
  }

  if (sounding() || !audioWanted())
    return;

  const QString file = m_theme.channelFile(event.channel, Priority::Advisory);
  const int slot =
    file.isEmpty() ? static_cast<int>(Slot::Advisory) : m_theme.overrideSlotFor(file, m_player);
  startBurst(Priority::Advisory, slot < 0 ? static_cast<int>(Slot::Advisory) : slot, file);
  scheduleStateChanged();
}

/**
 * @brief Band trackers were rebuilt (project or layout change): drop the points whose dataset
 *        no longer exists or lost its bands; the survivors are re-seeded by the monitor's next
 *        baseline emission, so an acknowledged point stays acknowledged across the rebuild.
 */
void UI::Alarms::AlarmAnnunciator::onTrackersRebuilt()
{
  const auto& datasets = m_modules.dashboard.datasets();
  std::vector<PointKey> stale;
  for (const auto& point : m_sequence.points()) {
    if (point.key.kind != PointKind::Band)
      continue;

    const auto it = datasets.constFind(point.key.id);
    if (it == datasets.cend() || it.value().alarmBands.empty())
      stale.push_back(point.key);
  }

  for (const auto& key : stale)
    (void)m_sequence.clear(key, nowMs());

  updateAudible(false);
  scheduleStateChanged();
}

/**
 * @brief Dashboard data reset: every point is stale.
 */
void UI::Alarms::AlarmAnnunciator::onDataReset()
{
  dropAllPoints();
}

/**
 * @brief The link closed: every point returns to normal without ringback (spec R19).
 */
void UI::Alarms::AlarmAnnunciator::onLinkClosed()
{
  dropAllPoints();
}

/**
 * @brief An application event: plays its slot on the event lane when the switch is on.
 */
void UI::Alarms::AlarmAnnunciator::onEventRequested(int slot)
{
  SS_ASSERT(slot >= 0 && (slot < SoundTheme::kSlotCount), return);
  if (!audioWanted() || !m_theme.slotEnabled(static_cast<Slot>(slot)))
    return;

  (void)m_player.play(IO::Audio::SoundPlayer::EventLane, slot, 1.0f);
}

/**
 * @brief The project's sounds object or path changed: re-adopt overrides and the sequence.
 */
void UI::Alarms::AlarmAnnunciator::onProjectSoundsChanged()
{
  applyProjectOverrides();
  updateAudible(false);
  scheduleStateChanged();
}

/**
 * @brief Repeat timer: replays the current burst when the same priority is still sounding.
 */
void UI::Alarms::AlarmAnnunciator::onRepeatDue()
{
  if (!sounding() || m_soundingPriority == Priority::Advisory)
    return;

  startBurst(m_soundingPriority, m_soundingSlot, m_soundingFile);
  scheduleStateChanged();
}

/**
 * @brief Test sequence step: one slot per step, then back to idle.
 */
void UI::Alarms::AlarmAnnunciator::onTestStep()
{
  if (m_testStep < 0)
    return;

  if (m_testStep >= kTestStepCount) {
    m_testStep = -1;
    Q_EMIT testingChanged();
    return;
  }

  if (audioWanted())
    (void)m_player.play(
      IO::Audio::SoundPlayer::EventLane, static_cast<int>(kTestOrder[m_testStep]), 1.0f);

  ++m_testStep;
  m_testTimer.start(kTestStepMs);
  Q_EMIT testingChanged();
}

/**
 * @brief 1 Hz housekeeping: frees retired buffers, ends a finished Advisory one-shot, and
 *        handles a lost device by falling back to the default and rebinding when it returns.
 */
void UI::Alarms::AlarmAnnunciator::onHealthTick()
{
  m_player.collectGarbage();

  const bool advisoryDone = m_soundingPriority == Priority::Advisory
                         && !m_player.laneActive(IO::Audio::SoundPlayer::AlarmLane);
  if (advisoryDone) {
    m_soundingPriority = Priority::None;
    m_soundingSlot     = -1;
    m_soundingFile.clear();
    scheduleStateChanged();
  }

  if (!audioWanted() || !m_player.available())
    return;

  const bool lostNow = m_player.deviceLost() || (!m_player.running() && !m_deviceLost);
  if (lostNow && !m_deviceLost) {
    m_deviceLost = true;
    m_player.stop();
    if (m_player.start(QByteArray())) {
      reloadBank();
      resumeBurstAfterRestart();
    }

    if (m_modules.problems)
      m_modules.problems->runNow();

    Q_EMIT deviceStateChanged();
    return;
  }

  if (!m_deviceLost)
    return;

  if (m_ticksUntilRetry > 0) {
    --m_ticksUntilRetry;
    return;
  }

  m_ticksUntilRetry = m_retryDelayTicks;
  m_retryDelayTicks = (std::min)(m_retryDelayTicks * 2, kMaxRetryTicks);
  if (!tryRecoverDevice())
    return;

  m_deviceLost      = false;
  m_retryDelayTicks = 1;
  reloadBank();
  resumeBurstAfterRestart();
  if (m_modules.problems)
    m_modules.problems->runNow();

  Q_EMIT deviceStateChanged();
}

/**
 * @brief One recovery attempt: the chosen device when it is present again, else the system
 *        default when nothing is running at all. Enumerates at most once per attempt.
 */
bool UI::Alarms::AlarmAnnunciator::tryRecoverDevice()
{
  const QByteArray wanted = m_theme.outputDeviceId();
  if (wanted.isEmpty())
    return !m_player.running() && m_player.start(QByteArray());

  const auto devices = m_player.enumerateOutputs();
  const bool present = std::any_of(
    devices.cbegin(), devices.cend(), [&wanted](const auto& d) { return d.id == wanted; });
  if (!present)
    return false;

  m_player.stop();
  if (m_player.start(wanted) && m_player.usingRequestedDevice())
    return true;

  if (!m_player.running())
    (void)m_player.start(QByteArray());

  return false;
}

//--------------------------------------------------------------------------------------------------
// Point table helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Raises a point and restarts the audible when the sequence says so; a re-raise within
 *        kReflashMinMs of the current burst's start does not restart it, so a value chattering
 *        on a band edge at display-tick rate cannot stutter the burst.
 */
void UI::Alarms::AlarmAnnunciator::raisePoint(const PointKey& key,
                                              Priority priority,
                                              const QString& title,
                                              const QString& channel,
                                              const QString& label,
                                              const QString& sound)
{
  const qint64 now     = nowMs();
  const bool restart   = m_sequence.raise(key, priority, title, channel, label, sound, now);
  const bool justBegan = priority == m_soundingPriority && now - m_burstStartedMs < kReflashMinMs;
  updateAudible(restart && !justBegan);
  scheduleStateChanged();
}

/**
 * @brief Clears a point; a ringback starts when sequence R asks for one.
 */
void UI::Alarms::AlarmAnnunciator::clearPoint(const PointKey& key)
{
  if (!m_sequence.find(key))
    return;

  (void)m_sequence.clear(key, nowMs());
  updateAudible(false);
  scheduleStateChanged();
}

/**
 * @brief Every point to Normal, audible off, no ringback.
 */
void UI::Alarms::AlarmAnnunciator::dropAllPoints()
{
  m_sequence.clearAll();
  stopAudible();
  scheduleStateChanged();
}

/**
 * @brief Arbitration (spec R6): the highest unsilenced Alert priority sounds; on a change of
 *        priority, or when @p restart is set, the burst starts now; ringback only when no point
 *        is in Alert; nothing when the lane should be quiet.
 */
void UI::Alarms::AlarmAnnunciator::updateAudible(bool restart)
{
  const Priority wanted = m_sequence.soundingPriority();
  if (wanted == Priority::None) {
    if (m_sequence.ringbackPending()) {
      if (!m_ringbackSounding)
        startBurst(Priority::None,
                   static_cast<int>(Slot::Ringback),
                   SoundTheme::bundledPath(Slot::Ringback));

      return;
    }

    if (m_soundingPriority != Priority::Advisory)
      stopAudible();

    return;
  }

  if (wanted == m_soundingPriority && !restart)
    return;

  const Point* point = m_sequence.latestAlert(wanted);
  SS_ASSERT(point != nullptr, return);
  startBurst(wanted, slotForPoint(*point), point->sound);
}

/**
 * @brief Starts one burst on the alarm lane, records the sounding state for the API and arms
 *        the repeat timer; Priority::None means ringback.
 */
void UI::Alarms::AlarmAnnunciator::startBurst(Priority priority, int slot, const QString& file)
{
  const bool samePriority =
    priority == m_soundingPriority && (priority != Priority::None || m_ringbackSounding);
  m_burstCount       = samePriority ? m_burstCount + 1 : 1;
  m_burstStartedMs   = nowMs();
  m_soundingPriority = priority;
  m_ringbackSounding = priority == Priority::None;
  m_soundingSlot     = slot;
  m_soundingFile     = file.isEmpty() && slot >= 0 && slot < SoundTheme::kSlotCount
                       ? m_theme.effectiveFile(static_cast<Slot>(slot))
                       : file;

  if (audioWanted())
    (void)m_player.play(IO::Audio::SoundPlayer::AlarmLane, slot, 1.0f);

  scheduleRepeat();
}

/**
 * @brief Quiets the alarm lane and forgets the sounding state.
 */
void UI::Alarms::AlarmAnnunciator::stopAudible()
{
  m_repeatTimer.stop();
  m_player.stopLane(IO::Audio::SoundPlayer::AlarmLane);
  m_soundingPriority = Priority::None;
  m_ringbackSounding = false;
  m_soundingSlot     = -1;
  m_burstCount       = 0;
  m_soundingFile.clear();
}

/**
 * @brief Arms the single repeat timer for the sounding priority (ringback repeats at the
 *        Caution interval, Advisory never).
 */
void UI::Alarms::AlarmAnnunciator::scheduleRepeat()
{
  m_repeatTimer.stop();
  if (!sounding() || m_soundingPriority == Priority::Advisory)
    return;

  const Priority p = m_ringbackSounding ? Priority::Caution : m_soundingPriority;
  m_repeatTimer.start(m_theme.intervalMs(p));
}

/**
 * @brief The bank slot for a point: its own override file when it decodes, else its priority's
 *        theme slot.
 */
int UI::Alarms::AlarmAnnunciator::slotForPoint(const Point& point)
{
  const int fallback = static_cast<int>(SoundTheme::slotForPriority(point.priority));
  if (point.sound.isEmpty())
    return fallback;

  const int slot = m_theme.overrideSlotFor(point.sound, m_player);
  return slot < 0 ? fallback : slot;
}

//--------------------------------------------------------------------------------------------------
// Theme, player and checker helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief True when sounds should reach the device: enabled, not muted, not headless.
 */
bool UI::Alarms::AlarmAnnunciator::audioWanted() const noexcept
{
  return !m_headless && m_theme.enabled() && !m_theme.muted();
}

/**
 * @brief Case-insensitive slot lookup for the QML and API surfaces.
 */
UI::Alarms::Slot UI::Alarms::AlarmAnnunciator::slotFromName(const QString& name, bool& ok) const
{
  return SoundTheme::slotFromName(name.trimmed().toLower(), ok);
}

/**
 * @brief Adopts the project's sounds object, drops old override slots, applies the sequence.
 */
void UI::Alarms::AlarmAnnunciator::applyProjectOverrides()
{
  const QString path = m_modules.project.jsonFilePath();
  const QString dir  = path.isEmpty() ? QString() : QFileInfo(path).absolutePath();
  m_theme.clearOverrides(m_player);
  m_theme.applyProject(m_modules.project.sounds(), dir);
  m_sequence.setSequence(m_theme.effectiveSequence());
  restoreOverrides();
}

/**
 * @brief Re-adds the override slots the active points and the sounding burst still need after
 *        a bank rebuild, and re-resolves the sounding slot from its file so a repeat never plays
 *        a stale slot number.
 */
void UI::Alarms::AlarmAnnunciator::restoreOverrides()
{
  if (!m_player.running())
    return;

  for (const auto& point : m_sequence.points())
    if (!point.sound.isEmpty())
      (void)m_theme.overrideSlotFor(point.sound, m_player);

  if (!sounding() || m_ringbackSounding || m_soundingPriority == Priority::Advisory)
    return;

  const Point* point = m_sequence.latestAlert(m_soundingPriority);
  if (point)
    m_soundingSlot = slotForPoint(*point);

  if (m_modules.problems)
    m_modules.problems->runNow();
}

/**
 * @brief Starts or stops the device to match enabled/muted, loading the bank after a start
 *        (the bank is keyed to the device's rate).
 */
void UI::Alarms::AlarmAnnunciator::applyPlayerState()
{
  m_player.setMasterGain(static_cast<float>(m_theme.volume()) * kVolumeScale);
  if (!audioWanted()) {
    m_player.stop();
    return;
  }

  if (m_player.running())
    return;

  if (!m_player.start(m_theme.outputDeviceId()))
    return;

  m_deviceLost = !m_player.usingRequestedDevice();
  reloadBank();
  Q_EMIT deviceStateChanged();
}

/**
 * @brief A device restart silences both lanes; re-issue the burst the state machine still
 *        considers sounding so a fallback never chops an active alarm (Advisory is one-shot and
 *        ends).
 */
void UI::Alarms::AlarmAnnunciator::resumeBurstAfterRestart()
{
  if (!sounding() || m_soundingPriority == Priority::Advisory || m_soundingSlot < 0)
    return;

  if (audioWanted())
    (void)m_player.play(IO::Audio::SoundPlayer::AlarmLane, m_soundingSlot, 1.0f);
}

/**
 * @brief Reloads every theme slot and every override the active points still need.
 */
void UI::Alarms::AlarmAnnunciator::reloadBank()
{
  if (!m_player.running())
    return;

  m_theme.clearOverrides(m_player);
  m_theme.loadBank(m_player);
  restoreOverrides();
}

/**
 * @brief Registers the pull-only checker that reports unusable sound files and a lost device.
 */
void UI::Alarms::AlarmAnnunciator::registerChecker()
{
  using Finding = Misc::ProblemCenter::Finding;
  const auto triggers =
    static_cast<quint8>(Misc::ProblemCenter::LinkSample | Misc::ProblemCenter::ProjectChanged);
  if (!m_modules.problems)
    return;

  const QPointer<AlarmAnnunciator> self(this);
  m_modules.problems->registerChecker(kCheckerId, triggers, [self](QList<Finding>& out) {
    if (!self)
      return;

    for (const auto& issue : self->m_theme.issues()) {
      Finding f;
      f.severity    = Misc::ProblemCenter::Warning;
      f.code        = QStringLiteral("alarms.sound-file");
      f.title       = AlarmAnnunciator::tr("Alarm sound file unavailable: %1").arg(issue.path);
      f.explanation = issue.reason;
      f.remedy = tr("Pick a valid PCM WAV file, or clear the override to use the bundled sound.");
      out.append(f);
    }

    if (!self->m_deviceLost)
      return;

    Finding f;
    f.severity = Misc::ProblemCenter::Warning;
    if (self->m_theme.outputDeviceId().isEmpty() || !self->m_player.running()) {
      f.code  = QStringLiteral("alarms.no-output-device");
      f.title = AlarmAnnunciator::tr("No audio output device is available");
      f.explanation =
        AlarmAnnunciator::tr("Alarm sounds cannot play until an output device is present.");
      f.remedy =
        AlarmAnnunciator::tr("Connect an audio output, or disable sounds in Preferences > Sounds.");
      out.append(f);
      return;
    }

    f.code  = QStringLiteral("alarms.output-device");
    f.title = AlarmAnnunciator::tr("Alarm sound device '%1' not found")
                .arg(self->m_theme.outputDeviceName());
    f.explanation =
      AlarmAnnunciator::tr("Alarm sounds are playing on the system default output instead.");
    f.remedy =
      AlarmAnnunciator::tr("Reconnect the device, or pick another one in Preferences > Sounds.");
    out.append(f);
  });
}

//--------------------------------------------------------------------------------------------------
// Static helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Wall-clock milliseconds for point timestamps and burst starts.
 */
qint64 UI::Alarms::AlarmAnnunciator::nowMs() noexcept
{
  return QDateTime::currentMSecsSinceEpoch();
}

/**
 * @brief Stable id for a notification point: channel and title hashed together.
 */
int UI::Alarms::AlarmAnnunciator::notificationId(const QString& channel,
                                                 const QString& title) noexcept
{
  const QString key = channel + QLatin1Char('\0') + title;
  return static_cast<int>(qHash(key) & 0x7FFFFFFFu);
}

/**
 * @brief One point as the API and the panel see it; a band point also carries the dashboard
 *        widget index and group that display its dataset, so the panel can jump to it.
 */
QVariantMap UI::Alarms::AlarmAnnunciator::pointToVariant(const Point& point) const
{
  static const char* kStates[] = {"normal", "alert", "acknowledged", "returnToNormal"};
  int groupId                  = -1;
  const bool band              = point.key.kind == PointKind::Band;
  const int widgetIndex        = band ? widgetForDataset(point.key.id, groupId) : -1;

  QVariantMap m;
  m.insert(QStringLiteral("kind"), band ? QStringLiteral("band") : QStringLiteral("notification"));
  m.insert(QStringLiteral("id"), point.key.id);
  m.insert(QStringLiteral("title"), point.title);
  m.insert(QStringLiteral("channel"), point.channel);
  m.insert(QStringLiteral("label"), point.label);
  m.insert(QStringLiteral("sound"), point.sound);
  m.insert(QStringLiteral("priority"), static_cast<int>(point.priority));
  m.insert(QStringLiteral("state"), QString::fromLatin1(kStates[static_cast<int>(point.state)]));
  m.insert(QStringLiteral("active"), point.active);
  m.insert(QStringLiteral("silenced"), point.silenced);
  m.insert(QStringLiteral("sinceMs"), point.sinceMs);
  m.insert(QStringLiteral("widgetIndex"), widgetIndex);
  m.insert(QStringLiteral("groupId"), groupId);
  return m;
}

/**
 * @brief The dashboard widget index that shows dataset @p uniqueId (a dataset widget first, else
 *        the group widget that contains it) and that widget's group, or -1 when none is built.
 */
int UI::Alarms::AlarmAnnunciator::widgetForDataset(int uniqueId, int& groupId) const
{
  const auto& map = m_modules.dashboard.widgetMap();
  int groupHit    = -1;
  for (auto it = map.cbegin(); it != map.cend(); ++it) {
    const auto type    = it.value().first;
    const int relative = it.value().second;
    if (SerialStudio::isDatasetWidget(type)) {
      const auto& dataset = m_modules.dashboard.getDatasetWidget(type, relative);
      if (dataset.uniqueId != uniqueId)
        continue;

      groupId = dataset.groupId;
      return it.key();
    }

    if (groupHit >= 0 || !SerialStudio::isGroupWidget(type))
      continue;

    const auto& group = m_modules.dashboard.getGroupWidget(type, relative);
    for (const auto& dataset : group.datasets) {
      if (dataset.uniqueId != uniqueId)
        continue;

      groupId  = group.groupId;
      groupHit = it.key();
      break;
    }
  }

  return groupHit;
}

/**
 * @brief Parses a priority name or ordinal; None for anything else.
 */
UI::Alarms::Priority UI::Alarms::AlarmAnnunciator::priorityFromName(const QString& name) noexcept
{
  const QString n = name.trimmed().toLower();
  if (n == QLatin1String("warning") || n == QLatin1String("2"))
    return Priority::Warning;

  if (n == QLatin1String("caution") || n == QLatin1String("1"))
    return Priority::Caution;

  if (n == QLatin1String("advisory") || n == QLatin1String("0"))
    return Priority::Advisory;

  return Priority::None;
}
