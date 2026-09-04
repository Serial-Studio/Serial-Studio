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

#include <QString>

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
 */

namespace Core::Bus {
/**
 * @brief A data source finished connecting, dropped, or had its stream paused.
 */
struct ConnectionStateChanged final {
  int sourceId;
  bool connected;
  bool paused;
};

/**
 * @brief A project file became the active project.
 */
struct ProjectLoaded final {
  QString path;
  QString title;
};

/**
 * @brief The active project gained or lost unsaved edits.
 */
struct ProjectModified final {
  bool modified;
};

/**
 * @brief A module reported something the user must see, without reaching the notification center.
 */
struct NotificationRaised final {
  int severity;
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
 * @brief A recording session opened or closed, pause included (spec 0075's sink contract).
 */
struct RecordingSessionBoundary final {
  bool connected;
  bool paused;
};

/**
 * @brief One settings key changed; the key names the setting, never the new value.
 */
struct SettingsChanged final {
  QString key;
};

/**
 * @brief The licensing state settled on a real token-validity transition.
 */
struct LicenseStateChanged final {
  bool activated;
};
}  // namespace Core::Bus
