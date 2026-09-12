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

#pragma once

#include <atomic>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QVector>
#include <vector>

#include "Core/DataModel/Frame.h"
#include "Core/IO/FrameConfig.h"

/**
 * @file Messages.h
 * @brief The cross-library message vocabulary carried by Core::Bus::MessageBus.
 *
 * This file is the "DBC" of the in-process bus: the complete set of topics one library may
 * announce to another, in the only form a library below Ui can speak. Every topic is a plain
 * aggregate of Qt Core value types, so a subscriber needs nothing but Core to read it, and every
 * published object is immutable and shared by pointer.
 *
 * The vocabulary grows only through a spec (0076 and its follow-ups). A topic added ad hoc is a
 * cross-library coupling nobody reviewed, which is the coupling the bus exists to remove; a topic
 * whose fields change is a wire break for every library that reads it.
 *
 * Reviewer checklist for any change to this file (spec 0077):
 * - Every field is a Core or Qt Core value type; no enum owned above Core, no pointer to a module.
 * - A field added, renamed, retyped or removed is a wire break: every reader is updated in the
 *   same change, and the topic keeps its retained/request kind.
 * - A retained topic (publishState) carries a fact; a request topic (publish) carries an ask and
 *   never a reply slot: its publisher never waits, the owner answers with its own state topic.
 * - Exactly one library publishes each topic; the publisher never names a subscriber.
 * - No topic runs at frame, chunk or block rate; the hotpath keeps the pooled SPSC lane.
 * - The topic is named in the spec that adds it; the bus census fails on a topic nobody
 *   publishes or nobody subscribes to.
 */

namespace Core::Bus {
/**
 * @brief A data source finished connecting, dropped, had its stream paused, or is dialing.
 *        busType carries the SerialStudio::BusType ordinal of the active source.
 */
struct ConnectionStateChanged final {
  int sourceId;
  bool connected;
  bool paused;
  bool connecting;
  int busType;
};

/**
 * @brief A module reported something the user must see, without reaching the notification center.
 *        channel groups related events; key deduplicates repeats inside the center's window.
 */
struct NotificationRaised final {
  int severity;
  QString channel;
  QString key;
  QString title;
  QString text;
};

/**
 * @brief The severity ordinals a NotificationRaised carries, mirroring the
 *        DataModel::NotificationCenter::Level enum the center owns above Core: a publisher below
 *        the center names one of these instead of redeclaring the ordinal in its own file, which
 *        is a redefinition once two such files share a unity translation unit.
 */
inline constexpr int kSeverityInfo     = 0;
inline constexpr int kSeverityWarning  = 1;
inline constexpr int kSeverityCritical = 2;

/**
 * @brief The notification center accepted an event (after dedup and clamping): what the history
 *        holds, for sinks that forward notifications (the MQTT publisher).
 */
struct NotificationPosted final {
  qint64 timestampMs;
  int severity;
  QString channel;
  QString title;
  QString text;
};

/**
 * @brief The dashboard rebuilt its widget structure; the generation counter identifies the build.
 */
struct DashboardStructureChanged final {
  int generation;
};

/**
 * @brief The dashboard finished a display tick: its frame surface holds that tick's values.
 */
struct DashboardUpdated final {
  int reserved;
};

/**
 * @brief The dashboard dropped its data and layout (a session reset).
 */
struct DashboardDataReset final {
  int reserved;
};

/**
 * @brief A mirror viewer attached to, or detached from, a remote dashboard.
 */
struct MirrorAttachedChanged final {
  bool attached;
};

/**
 * @brief The licensing state settled on a real token-validity transition; tier is the
 *        Licensing::FeatureTier ordinal (0 when no token is installed).
 */
struct LicenseStateChanged final {
  bool activated;
  int tier;
  bool trialExpired;
};

/**
 * @brief The operation mode (SerialStudio::OperationMode ordinal) the application runs in.
 */
struct OperationModeChanged final {
  int mode;
};

/**
 * @brief The framing source 0 runs with, derived from the operation mode and the project;
 *        published right after OperationModeChanged and on every project load.
 */
struct FrameConfigChanged final {
  IO::FrameConfig config;
};

/**
 * @brief One replay player (0 CSV, 1 MDF4, 2 Historian) opened or closed a file.
 */
struct ReplayPlayerStateChanged final {
  int playerId;
  bool open;
};

/**
 * @brief The audio capture format the QuickPlot builder sizes its stream lane from.
 */
struct AudioCaptureFormat final {
  int format;
  int sampleRate;
  bool normalized;
};

/**
 * @brief One installed widget-extension package as the resolver needs it.
 */
struct WidgetExtensionEntry final {
  QString id;
  int scope;
  QString title;
  QString replaces;
};

/**
 * @brief The installed widget-extension catalog, republished on every rescan.
 */
struct WidgetExtensionCatalog final {
  QVector<WidgetExtensionEntry> entries;
};

/**
 * @brief The dashboard's persisted view state (the JSON the Historian bundles with a recording).
 */
struct DashboardViewState final {
  QString json;
};

/**
 * @brief A replay asked the dashboard to restore a bundled view state.
 */
struct DashboardViewStateRestoreRequested final {
  QString json;
};

/**
 * @brief A replay asked the dashboard to drop its view state before loading a recording.
 */
struct DashboardViewStateClearRequested final {
  int reserved;
};

/**
 * @brief A replay player asked the connection manager to close the live device.
 */
struct DisconnectRequested final {
  int reserved;
};

/**
 * @brief A device open attempt settled; bus is the Diagnostics::Bus ordinal.
 */
struct DeviceOpenAttempted final {
  int bus;
  bool ok;
  QString reason;
};

/**
 * @brief A Modbus register map was imported into the project; the Modbus driver applies it,
 *        replacing its groups, or appending to them when the map joined an open project.
 */
struct ModbusRegisterGroupsLoaded final {
  QJsonDocument groups;
  bool append;
};

/**
 * @brief A driver generated a project and asks the model to load it (the model switches to
 *        ProjectFile mode and restores the previous mode on a rejected document); saveWithDialog
 *        also opens the save-as dialog. requestId pairs the reply with the request.
 */
struct LoadGeneratedProjectRequested final {
  QJsonDocument json;
  bool saveWithDialog;
  quint64 requestId;
};

/**
 * @brief The reply to LoadGeneratedProjectRequested: whether the document loaded and, when a save
 *        dialog was asked for, whether the user accepted it (equal to loaded otherwise).
 */
struct GeneratedProjectLoadFinished final {
  quint64 requestId;
  bool loaded;
  bool accepted;
};

/**
 * @brief The Setup pane changed the single-source project's driver or settings; the project
 *        model mirrors the bus type, the settings when applySettings is set, and autosaves a
 *        project on disk when autosave is set.
 */
struct Source0ConnectionSettingsChanged final {
  int busType;
  QJsonObject settings;
  bool applySettings;
  bool autosave;
};

/**
 * @brief The project structure the device layer reads instead of the model (spec 0077 T49),
 *        retained and republished on every edit: sources, groups, Lua mode, frame detection, file
 *        path. change (a Change value) names what produced the snapshot and sourceId the source a
 *        Source or StreamLane change concerns, so a reader dispatches on it; Content is data only.
 */
struct ProjectStructureSnapshot final {
  enum Change : int {
    Content = 0,
    Structure,
    Source,
    StreamLane,
    LuaFastMode
  };

  std::vector<DataModel::Source> sources;
  std::vector<DataModel::Group> groups;
  QString filePath;
  bool luaFastMode;
  int frameDetection;
  int change;
  int sourceId;
  quint64 generation;
  QString transformLibrary;    ///< Shared Lua transform library (spec 0083)
  QString transformLibraryJs;  ///< Shared JS transform library (spec 0083 addendum), appended last
};

/**
 * @brief The connection manager is about to open the session's devices (ProjectFile mode); the
 *        control script runs its onConnect() hook inside this publish, so a script can start the
 *        server the session then dials.
 */
struct ConnectionAboutToOpen final {
  int sourceId;
};

/**
 * @brief The active UI-config driver's bus type and properties, retained: what a legacy project
 *        without a sources array seeds its default source from.
 */
struct ActiveUiDriverSettings final {
  int busType;
  QJsonObject settings;
};

/**
 * @brief The project asks the device layer to serialise the UI driver for @c busType into
 *        @c sourceId's connection settings; answered directly with
 * SourceConnectionSettingsCaptured.
 */
struct SourceSettingsCaptureRequested final {
  int sourceId;
  int busType;
};

/**
 * @brief The reply to SourceSettingsCaptureRequested: the driver's properties minus its secrets.
 */
struct SourceConnectionSettingsCaptured final {
  int sourceId;
  QJsonObject settings;
};

/**
 * @brief The project asks the device layer to apply @c sourceId's saved settings to its UI driver.
 */
struct SourceSettingsRestoreRequested final {
  int sourceId;
};

/**
 * @brief Hands out the request ids that pair a request topic with its reply, process-wide.
 */
[[nodiscard]] inline quint64 allocateRequestId() noexcept
{
  static std::atomic<quint64> next{1};
  return next.fetch_add(1, std::memory_order_relaxed);
}
}  // namespace Core::Bus
