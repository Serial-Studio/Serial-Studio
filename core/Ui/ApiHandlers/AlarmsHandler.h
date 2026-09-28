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

#include <memory>

#include "Core/Api/CommandProtocol.h"

namespace UI::Alarms {
class AlarmAnnunciator;
}  // namespace UI::Alarms

namespace API {
namespace Handlers {

/**
 * @brief The alarms.* commands (spec 0087 R20): the annunciator state, the five operator actions
 *        and the project sound overrides. Available in every build; reads the root-owned facade
 *        through the root's owning slot (null once released), never through an instance() reach.
 */
class AlarmsHandler {
public:
  static void registerCommands(const std::unique_ptr<UI::Alarms::AlarmAnnunciator>* annunciator);

private:
  static CommandResponse state(const QString& id, const QJsonObject& params);
  static CommandResponse acknowledge(const QString& id, const QJsonObject& params);
  static CommandResponse silence(const QString& id, const QJsonObject& params);
  static CommandResponse clear(const QString& id, const QJsonObject& params);
  static CommandResponse reset(const QString& id, const QJsonObject& params);
  static CommandResponse test(const QString& id, const QJsonObject& params);
  static CommandResponse setMuted(const QString& id, const QJsonObject& params);
  static CommandResponse getProjectSounds(const QString& id, const QJsonObject& params);
  static CommandResponse setProjectSounds(const QString& id, const QJsonObject& params);
};

}  // namespace Handlers
}  // namespace API
