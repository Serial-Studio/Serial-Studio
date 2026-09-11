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

class QJSEngine;

namespace DataModel {

/**
 * @brief Which host surface a transmit script is compiled against.
 */
enum class TransmitScriptSurface {
  Live,
  Judging,
};

/**
 * @brief Installs the host surface an output-widget transmit script runs against. Every surface
 *        that compiles a transmit script goes through this one function -- the live control, the
 *        editor's validation, the preview and the assistant's dry run -- because a verdict is only
 *        comparable across them while the names behind it are identical (spec 0079).
 *
 *        @c Judging installs exactly the same names but makes the effectful ones inert: compiling
 *        a script EXECUTES its top level, so a script that calls deviceWrite(), actionFire(),
 *        apiCall(), tableSet() or mqttPublish() outside transmit() would otherwise actuate
 *        hardware, fire project actions, mutate the live table store or reach a broker every time
 *        it is validated or previewed. Reads stay real, because a script that reads a table to
 *        build its payload must still preview as itself. It also installs the table API by name
 *        only, without the pipeline marshal and the session-long capture flags the live path arms.
 *
 *        Returns false when the judging surface could not be established, which callers MUST treat
 *        as "this engine may not run a script": the effectful bridges are installed first and
 *        masked second, so an unmasked engine holds the live ones.
 */
[[nodiscard]] bool prepareTransmitScriptEngine(
  QJSEngine& engine, int sourceId, TransmitScriptSurface surface = TransmitScriptSurface::Live);

}  // namespace DataModel
