/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../../../../Widgets" as Widgets

Item {
  id: root

  implicitHeight: layout.implicitHeight
  implicitWidth: layout.implicitWidth + 16

  GridLayout {
    id: layout

    columns: 2
    rowSpacing: 4
    columnSpacing: 4
    anchors.margins: 0
    anchors.fill: parent

    //
    // Gateway address
    //
    Label {
      text: qsTr("Gateway") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
    } Widgets.LineField {
      id: _hostField

      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      placeholderText: qsTr("IP Address")
      Component.onCompleted: text = Cpp_IO_Eip.host

      onTextEdited: {
        if (Cpp_IO_Eip.host !== text && text.length > 0)
          Cpp_IO_Eip.host = text
      }
      onEditingFinished: {
        if (Cpp_IO_Eip.host !== text)
          Cpp_IO_Eip.host = text
      }
    }

    //
    // CIP routing path from the gateway to the CPU
    //
    Label {
      text: qsTr("CIP Path") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
    } Widgets.LineField {
      id: _pathField

      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      placeholderText: qsTr("1,0")
      Component.onCompleted: text = Cpp_IO_Eip.cipPath

      onTextEdited: {
        if (Cpp_IO_Eip.cipPath !== text)
          Cpp_IO_Eip.cipPath = text
      }
      onEditingFinished: {
        if (Cpp_IO_Eip.cipPath !== text)
          Cpp_IO_Eip.cipPath = text
      }
    }

    //
    // Controller family
    //
    Label {
      text: qsTr("Controller") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
    } Widgets.Combo {
      id: _familyCombo

      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_Eip.plcTypeLabels
      currentIndex: Cpp_IO_Eip.plcTypeIndex
      onActivated: (index) => {
        if (enabled && Cpp_IO_Eip.plcTypeIndex !== index)
          Cpp_IO_Eip.plcTypeIndex = index
      }
    }

    //
    // Poll interval
    //
    Label {
      text: qsTr("Poll Interval (ms)") + ":"
    } Widgets.LineField {
      id: _intervalField

      Layout.fillWidth: true
      placeholderText: qsTr("Polling interval")
      validator: IntValidator { bottom: 50; top: 60000 }
      Component.onCompleted: text = Cpp_IO_Eip.pollInterval

      onTextEdited: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_Eip.pollInterval !== value)
          Cpp_IO_Eip.pollInterval = value
      }
      onEditingFinished: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_Eip.pollInterval !== value)
          Cpp_IO_Eip.pollInterval = value
      }
    }

    //
    // Tag list configuration
    //
    Item {
      Layout.columnSpan: 2
      Layout.topMargin: 16
      Layout.fillWidth: true
      implicitHeight: _tagsButton.height + _projectButton.height + _tagStatus.height + 12

      Button {
        id: _tagsButton

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        onClicked: _tagsDialog.show()
        text: qsTr("Configure Tags…")
      }

      Button {
        id: _projectButton

        anchors.topMargin: 4
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: _tagsButton.bottom
        enabled: Cpp_IO_Eip.tagCount > 0
        text: qsTr("Create Project from Tags")
        onClicked: Cpp_IO_Eip.generateProject()
      }

      Label {
        id: _tagStatus

        opacity: 0.5
        anchors.topMargin: 4
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: _projectButton.bottom
        font: Cpp_Misc_CommonFonts.customUiFont(0.85)
        horizontalAlignment: Text.AlignHCenter
        text: Cpp_IO_Eip.tagCount > 0 ?
                qsTr("%1 tag(s) configured").arg(Cpp_IO_Eip.tagCount) :
                qsTr("No tags configured")
      }
    }

    //
    // Session status
    //
    Item {
      Layout.topMargin: 4
      Layout.columnSpan: 2
      Layout.fillWidth: true
      implicitHeight: _sessionStatus.implicitHeight

      Label {
        id: _sessionStatus

        opacity: 0.5
        elide: Text.ElideRight
        anchors.left: parent.left
        text: Cpp_IO_Eip.statusText
        anchors.right: parent.right
        horizontalAlignment: Text.AlignHCenter
        font: Cpp_Misc_CommonFonts.customUiFont(0.85)
      }
    }

    EipTagsDialog {
      id: _tagsDialog
    }

    //
    // Spacer
    //
    Item {
      Layout.fillHeight: true
    }
  }
}
