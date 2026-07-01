/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtCore
import QtQuick
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  //
  // Window options
  //
  title: qsTr("Activate Offline")
  preferredWidth: column.implicitWidth
  preferredHeight: column.implicitHeight
  minimumWidth: 580 + (contentPadding * 2)

  //
  // Wizard body: three steps to trade a device file for a license file
  //
  dialogContent: ColumnLayout {
    id: column

    spacing: 24
    anchors.centerIn: parent

    Label {
      Layout.fillWidth: true
      Layout.maximumWidth: 560
      wrapMode: Label.WrapAtWordBoundaryOrAnywhere
      text: qsTr("Activate Serial Studio Pro on a machine with no internet access. No account or connection is needed on this computer.")
    }

    //
    // Step 1: export the device file
    //
    RowLayout {
      spacing: 16
      Layout.fillWidth: true

      Rectangle {
        radius: width / 2
        implicitWidth: 30
        implicitHeight: 30
        color: palette.highlight
        Layout.alignment: Qt.AlignTop

        Label {
          text: "1"
          anchors.centerIn: parent
          color: palette.highlightedText
          font: Cpp_Misc_CommonFonts.boldUiFont
        }
      }

      ColumnLayout {
        spacing: 6
        Layout.fillWidth: true

        Label {
          Layout.fillWidth: true
          text: qsTr("Save your device file")
          font: Cpp_Misc_CommonFonts.boldUiFont
        }

        Label {
          opacity: 0.75
          Layout.fillWidth: true
          Layout.maximumWidth: 520
          wrapMode: Label.WrapAtWordBoundaryOrAnywhere
          text: qsTr("Save this computer's device file. It identifies this machine and contains no personal information.")
        }

        Widgets.IconButton {
          horizontalPadding: 8
          onClicked: _saveDialog.open()
          Layout.alignment: Qt.AlignLeft
          text: qsTr("Save Device File…")
          icon.source: "qrc:/icons/buttons/save.svg"
        }
      }
    }

    //
    // Step 2: obtain the license file from the portal (URL shown for another machine)
    //
    RowLayout {
      spacing: 16
      Layout.fillWidth: true

      Rectangle {
        radius: width / 2
        implicitWidth: 30
        implicitHeight: 30
        color: palette.highlight
        Layout.alignment: Qt.AlignTop

        Label {
          text: "2"
          anchors.centerIn: parent
          color: palette.highlightedText
          font: Cpp_Misc_CommonFonts.boldUiFont
        }
      }

      ColumnLayout {
        spacing: 6
        Layout.fillWidth: true

        Label {
          Layout.fillWidth: true
          text: qsTr("Get your license file")
          font: Cpp_Misc_CommonFonts.boldUiFont
        }

        Label {
          opacity: 0.75
          Layout.fillWidth: true
          Layout.maximumWidth: 520
          wrapMode: Label.WrapAtWordBoundaryOrAnywhere
          text: qsTr("On another computer with internet access, go to the address below, upload the device file, and enter your email and license key.")
        }

        Widgets.LineField {
          readOnly: true
          Layout.fillWidth: true
          onTextChanged: cursorPosition = 0
          text: Cpp_Licensing_OfflineLicense.activationUrl
        }

        Widgets.IconButton {
          horizontalPadding: 8
          text: qsTr("Open in Browser")
          Layout.alignment: Qt.AlignLeft
          icon.source: "qrc:/icons/buttons/download.svg"
          onClicked: Cpp_Licensing_OfflineLicense.openActivationPortal()
        }
      }
    }

    //
    // Step 3: import the signed license file
    //
    RowLayout {
      spacing: 16
      Layout.fillWidth: true

      Rectangle {
        radius: width / 2
        implicitWidth: 30
        implicitHeight: 30
        color: palette.highlight
        Layout.alignment: Qt.AlignTop

        Label {
          text: "3"
          anchors.centerIn: parent
          color: palette.highlightedText
          font: Cpp_Misc_CommonFonts.boldUiFont
        }
      }

      ColumnLayout {
        spacing: 6
        Layout.fillWidth: true

        Label {
          Layout.fillWidth: true
          text: qsTr("Import your license")
          font: Cpp_Misc_CommonFonts.boldUiFont
        }

        Label {
          opacity: 0.75
          Layout.fillWidth: true
          Layout.maximumWidth: 520
          wrapMode: Label.WrapAtWordBoundaryOrAnywhere
          text: qsTr("Copy the license file to this computer and import it. Pro features unlock immediately.")
        }

        Widgets.IconButton {
          horizontalPadding: 8
          Layout.alignment: Qt.AlignLeft
          onClicked: _importDialog.open()
          text: qsTr("Import License File…")
          icon.source: "qrc:/icons/buttons/activate.svg"
        }
      }
    }
  }

  //
  // Step 1 file picker (write the .ssmachine device file)
  //
  FileDialog {
    id: _saveDialog

    defaultSuffix: "ssmachine"
    fileMode: FileDialog.SaveFile
    title: qsTr("Save Device File")
    onAccepted: Cpp_Licensing_OfflineLicense.exportMachineInfo(selectedFile)
    nameFilters: [qsTr("Serial Studio device file (*.ssmachine)")]
  }

  //
  // Step 3 file picker (read the signed .sslic license file)
  //
  FileDialog {
    id: _importDialog

    title: qsTr("Import License File")
    nameFilters: [qsTr("Serial Studio license (*.sslic)"), qsTr("All files (*)")]
    onAccepted: {
      if (Cpp_Licensing_OfflineLicense.activateFromFile(selectedFile))
        root.close()
    }
  }
}
