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
import QtQuick.Controls

//
// Offscreen fixture for tst_ui_automation (spec 0101): one control per role the catalog
// classifies, a password field whose contents must never be reported, a disabled control, an
// unnamed rectangle that must not be listed, a named plain item that must, and a modal popup.
//
Item {
  id: root

  width: 400
  height: 300

  property int disabledClicks: 0

  function openPopup() {
    _popup.open()
  }

  function closePopup() {
    _popup.close()
  }

  Column {
    x: 10
    y: 10
    spacing: 8

    Button {
      objectName: "fixture.button"
      text: "Press"
      width: 120
      height: 32
    }

    CheckBox {
      objectName: "fixture.check"
      text: "Check"
    }

    TextField {
      objectName: "fixture.field"
      placeholderText: "Type here"
      width: 200
    }

    ComboBox {
      objectName: "fixture.combo"
      model: ["Alpha", "Beta"]
      width: 120
    }

    TextField {
      objectName: "fixture.secret"
      echoMode: TextInput.Password
      text: "hunter2"
      width: 200
    }

    ToolbarButton {
      objectName: "fixture.toolbar"
      text: "Tool"
    }

    Button {
      objectName: "fixture.disabled"
      text: "Off"
      enabled: false
      width: 120
      height: 32
      onClicked: root.disabledClicks++
    }

    Rectangle {
      width: 40
      height: 20
      color: "red"
    }

    Rectangle {
      objectName: "fixture.card"
      width: 60
      height: 20
      color: "gray"

      MouseArea {
        anchors.fill: parent
      }
    }
  }

  Popup {
    id: _popup

    objectName: "fixture.popup"
    modal: true
    x: 100
    y: 100
    width: 160
    height: 80

    Button {
      objectName: "fixture.popup.close"
      text: "Close"
      onClicked: _popup.close()
    }
  }
}
