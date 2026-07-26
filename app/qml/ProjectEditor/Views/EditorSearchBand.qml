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
import QtQuick.Layouts
import QtQuick.Controls

import "../../Widgets" as Widgets

//
// Search band shared by the editor views, placed above each view's secondary toolbar; form
// views hand it their TableDelegate, list views read the query property instead
//
Rectangle {
  id: root

  //
  // The TableDelegate whose rows this band filters (needs searchable: true); null for views
  // that filter a plain JS row list through the query property
  //
  property var tableDelegate: null

  //
  // Current search text; the band owns the field sync imperatively because the clear button
  // inside SearchField writes text directly, which would sever a declarative binding
  //
  property string query: ""
  onQueryChanged: {
    if (field.text !== query)
      field.text = query

    if (tableDelegate && tableDelegate.searchQuery !== query)
      tableDelegate.searchQuery = query
  }

  //
  // Rows the host currently shows; -1 disables the inline no-match note (form views get
  // their note from TableDelegate instead)
  //
  property int resultCount: -1

  //
  // A new selection repoints the form model; a filter typed for the old form must not
  // silently hide the fresh one
  //
  Connections {
    target: root.tableDelegate
    enabled: root.tableDelegate !== null

    function onModelPointerChanged() {
      root.query = ""
    }
  }

  //
  // Band geometry & palette (mirrors the Dataset Values view)
  //
  z: 2
  implicitHeight: 48
  Layout.topMargin: -1
  Layout.fillWidth: true
  color: Cpp_ThemeManager.colors["groupbox_background"]

  Widgets.SearchField {
    id: field

    onTextChanged: root.query = text
    placeholderText: qsTr("Search…")
    color: Cpp_ThemeManager.colors["base"]

    anchors {
      leftMargin: 6
      rightMargin: 6
      left: parent.left
      verticalCenter: parent.verticalCenter
      right: noMatchNote.visible ? noMatchNote.left : parent.right
    }
  }

  Label {
    id: noMatchNote

    opacity: 0.7
    text: qsTr("No matches")
    font: Cpp_Misc_CommonFonts.uiFont
    color: Cpp_ThemeManager.colors["text"]
    visible: root.resultCount === 0 && root.query.trim().length > 0

    anchors {
      rightMargin: 12
      right: parent.right
      verticalCenter: parent.verticalCenter
    }
  }

  Rectangle {
    height: 1
    color: Cpp_ThemeManager.colors["groupbox_border"]

    anchors {
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }
  }
}
