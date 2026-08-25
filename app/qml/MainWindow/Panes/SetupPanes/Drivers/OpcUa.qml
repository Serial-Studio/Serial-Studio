/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
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
import QtQuick.Dialogs

import "../../../../Widgets" as Widgets
import "../../../../Dialogs" as Dialogs

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
    // Endpoint URL + discover
    //
    Label {
      text: qsTr("Endpoint") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
    } RowLayout {
      spacing: 4
      Layout.fillWidth: true

      Widgets.LineField {
        id: _urlField

        Layout.fillWidth: true
        enabled: app.ioEnabled
        opacity: enabled ? 1 : 0.5
        placeholderText: "opc.tcp://host:4840"
        Component.onCompleted: text = Cpp_IO_OpcUa.endpointUrl
        onEditingFinished: {
          if (Cpp_IO_OpcUa.endpointUrl !== text)
            Cpp_IO_OpcUa.endpointUrl = text
        }
      }

      Button {
        text: qsTr("Discover")
        enabled: app.ioEnabled && !Cpp_IO_OpcUa.discovering
        onClicked: {
          if (Cpp_IO_OpcUa.endpointUrl !== _urlField.text)
            Cpp_IO_OpcUa.endpointUrl = _urlField.text

          Cpp_IO_OpcUa.discoverEndpoints()
        }
      }
    }

    //
    // Publishing interval
    //
    Label {
      text: qsTr("Poll Interval (ms)") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
    } Widgets.LineField {
      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      placeholderText: qsTr("10-60000")
      validator: IntValidator { bottom: 10; top: 60000 }
      Component.onCompleted: text = Cpp_IO_OpcUa.publishingInterval
      onEditingFinished: {
        const value = parseInt(text)
        if (!isNaN(value) && Cpp_IO_OpcUa.publishingInterval !== value)
          Cpp_IO_OpcUa.publishingInterval = value
      }
    }

    //
    // Discovered endpoints (secured rows disabled)
    //
    Label {
      text: qsTr("Security") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
      visible: Cpp_IO_OpcUa.endpointList.length > 0
    } ComboBox {
      id: _endpointCombo

      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_OpcUa.endpointList
      visible: Cpp_IO_OpcUa.endpointList.length > 0

      //
      // The driver refuses a row this build cannot dial, so the combo re-syncs from it instead
      // of trusting the activation; a plain binding would break on the first user pick.
      //
      Component.onCompleted: currentIndex = Cpp_IO_OpcUa.endpointIndex
      onActivated: (index) => {
        if (enabled)
          Cpp_IO_OpcUa.endpointIndex = index

        currentIndex = Cpp_IO_OpcUa.endpointIndex
      }

      Connections {
        target: Cpp_IO_OpcUa

        function onEndpointIndexChanged() {
          _endpointCombo.currentIndex = Cpp_IO_OpcUa.endpointIndex
        }
      }

      delegate: ItemDelegate {
        required property int index
        required property string modelData

        width: _endpointCombo.width
        text: modelData
        enabled: Cpp_IO_OpcUa.endpointSelectable[index] === true
        opacity: enabled ? 1 : 0.4
        highlighted: _endpointCombo.highlightedIndex === index
        ToolTip.visible: !enabled && hovered
        ToolTip.text: qsTr("This endpoint offers no user identity token the selected authentication mode can present")
      }
    }

    //
    // Security policy and message mode
    //
    Label {
      text: qsTr("Policy") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
    } Widgets.Combo {
      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_OpcUa.securityPolicyList
      currentIndex: Cpp_IO_OpcUa.securityPolicyIndex
      onActivated: (index) => {
        if (enabled && Cpp_IO_OpcUa.securityPolicyIndex !== index)
          Cpp_IO_OpcUa.securityPolicyIndex = index
      }
    }

    Label {
      text: qsTr("Mode") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
      visible: Cpp_IO_OpcUa.securityPolicyIndex > 0
    } Widgets.Combo {
      Layout.fillWidth: true
      opacity: enabled ? 1 : 0.5
      model: [qsTr("Sign"), qsTr("Sign and Encrypt")]
      visible: Cpp_IO_OpcUa.securityPolicyIndex > 0
      enabled: app.ioEnabled && Cpp_IO_OpcUa.securityPolicyIndex > 0
      currentIndex: Math.max(0, Cpp_IO_OpcUa.securityMode - 2)
      onActivated: (index) => {
        if (enabled)
          Cpp_IO_OpcUa.securityMode = index + 2
      }
    }

    //
    // Deprecated-policy notice
    //
    Label {
      Layout.columnSpan: 2
      Layout.fillWidth: true
      wrapMode: Text.WordWrap
      color: Cpp_ThemeManager.colors["error"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.85)
      visible: Cpp_IO_OpcUa.securityPolicyDeprecated[Cpp_IO_OpcUa.securityPolicyIndex] === true
      text: qsTr("This policy is deprecated by the OPC Foundation (SHA-1 / RSA-1.5). Use it only for controllers that support nothing better.")
    }

    //
    // Authentication
    //
    Label {
      text: qsTr("Authentication") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
    } Widgets.Combo {
      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      model: Cpp_IO_OpcUa.authModeList
      currentIndex: Cpp_IO_OpcUa.authMode
      onActivated: (index) => {
        if (enabled && Cpp_IO_OpcUa.authMode !== index)
          Cpp_IO_OpcUa.authMode = index
      }
    }

    Label {
      text: qsTr("Username") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
      visible: Cpp_IO_OpcUa.authMode === 1
    } Widgets.LineField {
      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      visible: Cpp_IO_OpcUa.authMode === 1
      Component.onCompleted: text = Cpp_IO_OpcUa.username
      onEditingFinished: {
        if (Cpp_IO_OpcUa.username !== text)
          Cpp_IO_OpcUa.username = text
      }
    }

    Label {
      text: qsTr("Password") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
      visible: Cpp_IO_OpcUa.authMode === 1
    } Widgets.LineField {
      Layout.fillWidth: true
      enabled: app.ioEnabled
      opacity: enabled ? 1 : 0.5
      echoMode: TextInput.Password
      visible: Cpp_IO_OpcUa.authMode === 1
      Component.onCompleted: text = Cpp_IO_OpcUa.password
      onEditingFinished: {
        if (Cpp_IO_OpcUa.password !== text)
          Cpp_IO_OpcUa.password = text
      }
    }

    Label {
      text: qsTr("Certificate") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
      visible: Cpp_IO_OpcUa.authMode === 2
    } RowLayout {
      spacing: 4
      Layout.fillWidth: true
      visible: Cpp_IO_OpcUa.authMode === 2

      Widgets.LineField {
        readOnly: true
        enabled: app.ioEnabled
        Layout.fillWidth: true
        text: Cpp_IO_OpcUa.userCertificatePath
        placeholderText: qsTr("No certificate selected")
      }

      Button {
        text: qsTr("Browse…")
        enabled: app.ioEnabled
        onClicked: _userCertDialog.open()
      }
    }

    Label {
      text: qsTr("Private Key") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
      visible: Cpp_IO_OpcUa.authMode === 2
    } RowLayout {
      spacing: 4
      Layout.fillWidth: true
      visible: Cpp_IO_OpcUa.authMode === 2

      Widgets.LineField {
        readOnly: true
        enabled: app.ioEnabled
        Layout.fillWidth: true
        text: Cpp_IO_OpcUa.userKeyPath
        placeholderText: qsTr("No private key selected")
      }

      Button {
        text: qsTr("Browse…")
        enabled: app.ioEnabled
        onClicked: _userKeyDialog.open()
      }
    }

    //
    // Plaintext-credential warning: only an unencrypted channel exposes them (R15)
    //
    Label {
      Layout.columnSpan: 2
      Layout.fillWidth: true
      wrapMode: Text.WordWrap
      visible: Cpp_IO_OpcUa.credentialsExposed
      color: Cpp_ThemeManager.colors["error"]
      font: Cpp_Misc_CommonFonts.customUiFont(0.85)
      text: qsTr("Credentials travel in the clear on this channel. Choose a policy other than None with Sign and Encrypt to protect them.")
    }

    //
    // This installation's client certificate
    //
    Label {
      text: qsTr("Identity") + ":"
      opacity: enabled ? 1 : 0.5
      enabled: app.ioEnabled
      visible: Cpp_IO_OpcUa.securityPolicyIndex > 0
    } ColumnLayout {
      spacing: 4
      Layout.fillWidth: true
      visible: Cpp_IO_OpcUa.securityPolicyIndex > 0

      Label {
        opacity: 0.7
        Layout.fillWidth: true
        wrapMode: Text.WrapAnywhere
        font: Cpp_Misc_CommonFonts.monoFont
        text: Cpp_IO_OpcUa.clientCertificate.valid === true
              ? Cpp_IO_OpcUa.clientCertificate.subject + "\n"
                + Cpp_IO_OpcUa.clientCertificate.fingerprint
              : qsTr("Generated on the first secure connection")
      }

      RowLayout {
        spacing: 4
        Layout.fillWidth: true

        Button {
          text: qsTr("Export…")
          enabled: app.ioEnabled && Cpp_IO_OpcUa.clientCertificate.valid === true
          onClicked: _exportDialog.open()
        }

        Button {
          text: qsTr("Replace")
          enabled: app.ioEnabled
          onClicked: Cpp_IO_OpcUa.regenerateCertificate()
        }

        Item {
          Layout.fillWidth: true
        }
      }
    }

    //
    // Tag browser + project generation
    //
    Item {
      Layout.columnSpan: 2
      Layout.topMargin: 16
      Layout.fillWidth: true
      implicitHeight: _browseButton.height + _generateButton.height + _status.height + 12

      Button {
        id: _browseButton

        enabled: app.ioEnabled
        anchors.top: parent.top
        anchors.left: parent.left
        text: qsTr("Browse Tags…")
        anchors.right: parent.right
        onClicked: _browser.show()
      }

      Button {
        id: _generateButton

        anchors.topMargin: 4
        anchors.left: parent.left
        anchors.right: parent.right
        text: qsTr("Generate Project")
        anchors.top: _browseButton.bottom
        enabled: Cpp_IO_OpcUa.tagCount > 0
        onClicked: Cpp_IO_OpcUa.generateProject()
      }

      Label {
        id: _status

        opacity: 0.5
        anchors.topMargin: 4
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: _generateButton.bottom
        font: Cpp_Misc_CommonFonts.customUiFont(0.85)
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        text: Cpp_IO_OpcUa.tagCount > 0
              ? qsTr("%1 tag(s) selected").arg(Cpp_IO_OpcUa.tagCount) + "\n" + Cpp_IO_OpcUa.statusText
              : qsTr("No tags selected")
      }
    }

    OpcUaTagBrowser {
      id: _browser
    }

    Dialogs.OpcUaTrustDialog {
      id: _trustDialog
    }

    Connections {
      target: Cpp_IO_OpcUa

      function onServerCertificateUntrusted(certificate, reason) {
        _trustDialog.showCertificate(certificate, reason)
      }
    }

    FileDialog {
      id: _exportDialog

      defaultSuffix: "der"
      fileMode: FileDialog.SaveFile
      title: qsTr("Export Client Certificate")
      nameFilters: [qsTr("Certificate (*.der)"), qsTr("All files (*)")]
      onAccepted: Cpp_IO_OpcUa.exportCertificate(selectedFile)
    }

    FileDialog {
      id: _userCertDialog

      title: qsTr("Select User Certificate")
      nameFilters: [qsTr("Certificate (*.der *.pem *.crt)"), qsTr("All files (*)")]
      onAccepted: Cpp_IO_OpcUa.userCertificatePath = selectedFile
    }

    FileDialog {
      id: _userKeyDialog

      title: qsTr("Select Private Key")
      nameFilters: [qsTr("Private key (*.der *.pem *.key)"), qsTr("All files (*)")]
      onAccepted: Cpp_IO_OpcUa.userKeyPath = selectedFile
    }

    //
    // Spacer
    //
    Item {
      Layout.fillHeight: true
    }
  }
}
