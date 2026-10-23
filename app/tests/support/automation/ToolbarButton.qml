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

//
// The shape of the repository's Item-based controls: an Item with declared properties, so Qt
// gives it a dynamic meta-object named after this file, which the catalog parses for its role.
//
Item {
  id: root

  property alias text: _label.text
  property bool checked: false

  signal clicked()

  width: 80
  height: 32

  Text {
    id: _label

    anchors.centerIn: parent
  }

  MouseArea {
    anchors.fill: parent
    onClicked: root.clicked()
  }
}
