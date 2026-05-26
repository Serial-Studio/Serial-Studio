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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtCore
import QtQuick
import QtQuick.Controls

SplitView {
  id: root

  orientation: Qt.Horizontal

  //
  // Public API
  //
  property int minLeftWidth: 200
  property int minRightWidth: 200
  property string settingsKey: ""
  property Component leftPanel: null
  property Component rightPanel: null
  property bool captionSeparatorVisible: true

  //
  // 1px handle matching the app theme
  //
  handle: Rectangle {
    implicitWidth: 1
    color: Cpp_ThemeManager.colors["setup_border"]

    Rectangle {
      height: 32
      visible: root.captionSeparatorVisible
      color: Cpp_ThemeManager.colors["pane_caption_border"]
      anchors {
        top: parent.top
        left: parent.left
        right: parent.right
      }
    }
  }

  //
  // Persist user-chosen width of the logical "left" panel (Project Structure
  // etc.), independent of which physical loader hosts it under RTL.
  //
  Settings {
    id: settings

    property int panelWidth: root.minLeftWidth
    category: (root.settingsKey !== "" ? root.settingsKey : "PaneSplitter")
  }

  //
  // Physical first slot: holds logical "right" panel in RTL, "left" in LTR
  //
  Loader {
    id: firstLoader

    SplitView.fillHeight: true
    SplitView.fillWidth: Cpp_Misc_Translator.rtl
    SplitView.preferredWidth: Cpp_Misc_Translator.rtl ? -1 : settings.panelWidth
    SplitView.minimumWidth: Cpp_Misc_Translator.rtl ? root.minRightWidth
                                                    : root.minLeftWidth

    sourceComponent: Cpp_Misc_Translator.rtl ? root.rightPanel : root.leftPanel

    onWidthChanged: {
      if (root.width > 0 && !Cpp_Misc_Translator.rtl)
        settings.panelWidth = width
    }
  }

  //
  // Physical second slot: holds logical "left" panel in RTL, "right" in LTR
  //
  Loader {
    id: secondLoader

    SplitView.fillHeight: true
    SplitView.fillWidth: !Cpp_Misc_Translator.rtl
    SplitView.preferredWidth: Cpp_Misc_Translator.rtl ? settings.panelWidth : -1
    SplitView.minimumWidth: Cpp_Misc_Translator.rtl ? root.minLeftWidth
                                                    : root.minRightWidth

    sourceComponent: Cpp_Misc_Translator.rtl ? root.leftPanel : root.rightPanel

    onWidthChanged: {
      if (root.width > 0 && Cpp_Misc_Translator.rtl)
        settings.panelWidth = width
    }
  }
}
