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
  title: qsTr("Data Export")
  icon: Cpp_Misc_IconRegistry.icon("editor", "tx-data", 16)

  //
  // The two live data sinks a project can carry, in tree order
  //
  entries: [
    {
      icon: Cpp_Misc_IconRegistry.icon("editor", "mqtt-publisher", 16),
      title: qsTr("MQTT Publisher"),
      summary: qsTr("Publish dataset values to an MQTT broker as they arrive, as plain topics "
                    + "or Sparkplug B."),
      open: function() { Cpp_JSON_ProjectEditor.selectMqttPublisher() }
    },
    {
      icon: Cpp_Misc_IconRegistry.icon("editor", "influx", 16),
      title: qsTr("InfluxDB Sink"),
      summary: qsTr("Write every published block to an InfluxDB 2.x bucket as line protocol."),
      open: function() { Cpp_JSON_ProjectEditor.selectInfluxSink() }
    }
  ]
}
