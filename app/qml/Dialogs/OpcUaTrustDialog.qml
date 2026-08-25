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

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets" as Widgets

//
// Server-certificate trust prompt, opened QUEUED from serverCertificateUntrusted; accepting only
// records the decision, so the reconnect stays a new attempt with its own verdict (spec 0067).
//
Widgets.SmartDialog {
  id: root

  property string reason: ""
  property var certificate: ({})

  staysOnTop: true
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight
  title: qsTr("Untrusted OPC UA Server")

  //
  // Shows the prompt for one refused certificate
  //
  function showCertificate(info, why) {
    root.certificate = info
    root.reason = why
    root.show()
    root.raise()
  }

  //
  // Renders a certificate validity date, or a dash when the certificate could not be parsed
  //
  function formatDate(value) {
    if (!value || isNaN(new Date(value).getTime()))
      return "n/a"

    return new Date(value).toLocaleString(Qt.locale(), Locale.ShortFormat)
  }

  dialogContent: ColumnLayout {
    id: layout

    spacing: 4

    Label {
      Layout.fillWidth: true
      wrapMode: Text.WordWrap
      Layout.minimumWidth: 420
      color: Cpp_ThemeManager.colors["error"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.9, true)
      text: root.reason
    }

    Label {
      opacity: 0.7
      Layout.fillWidth: true
      wrapMode: Text.WordWrap
      color: Cpp_ThemeManager.colors["text"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.9)
      text: qsTr("Check the fingerprint against the one the server operator gave you before accepting. Accepting records the decision for this installation; it does not reconnect, so press Connect again afterwards.")
    }

    Item {
      implicitHeight: 8
    }

    GroupBox {
      Layout.fillWidth: true

      background: Rectangle {
        radius: 2
        border.width: 1
        border.color: Cpp_ThemeManager.colors["groupbox_border"]
        color: Cpp_ThemeManager.colors["groupbox_background"]
      }

      GridLayout {
        columns: 2
        rowSpacing: 4
        columnSpacing: 8
        anchors.fill: parent

        Label {
          text: qsTr("Subject:")
          color: Cpp_ThemeManager.colors["text"]
        } Label {
          Layout.fillWidth: true
          wrapMode: Text.WordWrap
          text: root.certificate.subject || "n/a"
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.monoFont
        }

        Label {
          text: qsTr("Issuer:")
          color: Cpp_ThemeManager.colors["text"]
        } Label {
          Layout.fillWidth: true
          wrapMode: Text.WordWrap
          text: root.certificate.issuer || "n/a"
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.monoFont
        }

        Label {
          text: qsTr("Application URI:")
          color: Cpp_ThemeManager.colors["text"]
        } Label {
          Layout.fillWidth: true
          wrapMode: Text.WordWrap
          text: root.certificate.applicationUri || "n/a"
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.monoFont
        }

        Label {
          text: qsTr("Fingerprint:")
          color: Cpp_ThemeManager.colors["text"]
        } Label {
          Layout.fillWidth: true
          wrapMode: Text.WrapAnywhere
          text: root.certificate.fingerprint || "n/a"
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.monoFont
        }

        Label {
          text: qsTr("Valid From:")
          color: Cpp_ThemeManager.colors["text"]
        } Label {
          Layout.fillWidth: true
          color: Cpp_ThemeManager.colors["text"]
          font: Cpp_Misc_CommonFonts.monoFont
          text: root.formatDate(root.certificate.notBefore)
        }

        Label {
          text: qsTr("Valid Until:")
          color: Cpp_ThemeManager.colors["text"]
        } Label {
          Layout.fillWidth: true
          font: Cpp_Misc_CommonFonts.monoFont
          text: root.formatDate(root.certificate.notAfter)
          color: root.certificate.expired === true
                 ? Cpp_ThemeManager.colors["error"]
                 : Cpp_ThemeManager.colors["text"]
        }
      }
    }

    Item {
      implicitHeight: 8
    }

    RowLayout {
      spacing: 8
      Layout.alignment: Qt.AlignRight

      Item {
        Layout.fillWidth: true
      }

      Button {
        text: qsTr("Reject")
        onClicked: root.close()
      }

      Button {
        text: qsTr("Trust This Server")
        onClicked: {
          Cpp_IO_OpcUa.trustServerCertificate(root.certificate.fingerprint || "")
          root.close()
        }
      }
    }
  }
}
