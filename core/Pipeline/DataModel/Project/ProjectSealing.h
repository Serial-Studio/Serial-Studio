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

#include "Core/DataModel/Frame.h"

/**
 * @brief The one home for what a sealed copy of a project omits (spec 0096): every script source
 *        (parser, transform, transform library, Canvas, output transmit, control script), parser
 *        and transform params, every source's connection settings, the MQTT publisher and InfluxDB
 *        blocks, and the project-lock hash. Structure (groups, datasets, widgets, workspaces) is
 *        kept, so a mirror viewer and a historian replay can still render the data widgets.
 */
namespace DataModel {
inline constexpr QLatin1StringView kSealedRecordingKey{"sealedRecording"};

void stripSealedContent(QJsonObject& project);
void stripSealedContent(Frame& frame);
[[nodiscard]] QJsonObject sealedCopy(QJsonObject project);
[[nodiscard]] bool isSealedRecording(const QString& projectJson);
}  // namespace DataModel
