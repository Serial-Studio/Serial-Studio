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
import QtQuick
import SerialStudio

EntryListView {
  title: qsTr("Project Scripts")
  icon: Cpp_Misc_IconRegistry.icon("commands", "macro", 16)

  //
  // The three project-level scripts, in tree order
  //
  entries: [
    {
      icon: Cpp_Misc_IconRegistry.icon("editor", "code", 16),
      title: qsTr("Control Loop"),
      summary: qsTr("JavaScript setup() and loop() that run while the project is connected: "
                    + "timers, watchdogs, automated commands."),
      open: function() { Cpp_JSON_ProjectEditor.selectControlScript() }
    },
    {
      icon: Cpp_Misc_IconRegistry.icon("editor", "code", 16),
      title: qsTr("Lua Library"),
      summary: qsTr("Functions every Lua value transform of this project can call, so a "
                    + "formula lives in one place."),
      open: function() { Cpp_JSON_ProjectEditor.selectTransformLibrary() }
    },
    {
      icon: Cpp_Misc_IconRegistry.icon("editor", "code", 16),
      title: qsTr("JavaScript Library"),
      summary: qsTr("Functions every JavaScript value transform of this project can call."),
      open: function() { Cpp_JSON_ProjectEditor.selectJsLibrary() }
    }
  ]
}
