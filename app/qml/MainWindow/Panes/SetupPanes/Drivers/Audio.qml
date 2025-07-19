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

Item {
  id: root
  implicitHeight: layout.implicitHeight

  //
  // Save settings
  //
  Settings {
    category: "AudioDriver"
    property alias sampleRate: _rate.currentIndex
    property alias inputDevice: _inDev.currentIndex
    property alias outputDevice: _outDev.currentIndex
    property alias inputSampleFormat: _inFmt.currentIndex
    property alias outputSampleFormat: _outFmt.currentIndex
    property alias inputChannelConfiguration: _inChan.currentIndex
    property alias outputChannelConfiguration: _outChan.currentIndex
  }

  //
  // No Mic Found Indicator
  //
  ColumnLayout {
    spacing: 4
    anchors.centerIn: parent
    visible: _inDev.count <= 0

    Image {
      sourceSize: Qt.size(96, 96)
      Layout.alignment: Qt.AlignHCenter
      source: "qrc:/rcc/images/no-mic.svg"
    }

    Item {
      implicitHeight: 4
    }

    Label {
      wrapMode: Label.WordWrap
      Layout.alignment: Qt.AlignHCenter
      Layout.maximumWidth: root.width - 64
      text: qsTr("No Microphone Detected")
      font: Cpp_Misc_CommonFonts.customUiFont(1.4, true)
    }

    Label {
      opacity: 0.8
      wrapMode: Label.WordWrap
      Layout.alignment: Qt.AlignHCenter
      Layout.maximumWidth: root.width - 64
      text: qsTr("Connect a mic or check your settings")
      font: Cpp_Misc_CommonFonts.customUiFont(1.2, false)
    }
  }

  //
  // Layout
  //
  GridLayout {
    id: layout
    columns: 2
    rowSpacing: 4
    columnSpacing: 4
    anchors.margins: 0
    anchors.fill: parent
    opacity: enabled ? 1 : 0.5
    visible: _inDev.count > 0
    enabled: !Cpp_IO_Manager.isConnected

    //
    // Input device selection
    //
    Label {
      visible: _inDev.visible
      text: qsTr("Input Device") + ":"
    } ComboBox {
      id: _inDev
      visible: count > 0
      Layout.fillWidth: true
      model: Cpp_IO_Audio.inputDeviceList
      currentIndex: Cpp_IO_Audio.selectedInputDevice
      onCurrentIndexChanged: {
        if (currentIndex < 0 || count <= currentIndex)
          currentIndex = 0

        if (Cpp_IO_Audio.selectedInputDevice !== currentIndex)
          Cpp_IO_Audio.selectedInputDevice = currentIndex
      }
    }

    //
    // Input sample rate selection
    //
    Label {
      visible: _rate.visible
      text: qsTr("Sample Rate") + ":"
    } ComboBox {
      id: _rate
      Layout.fillWidth: true
      visible: count > 0 && _inDev.visible
      model: Cpp_IO_Audio.sampleRates
      currentIndex: Cpp_IO_Audio.selectedSampleRate
      onCurrentIndexChanged: {
        if (currentIndex < 0 || count <= currentIndex)
          currentIndex = 0

        if (Cpp_IO_Audio.selectedSampleRate !== currentIndex)
          Cpp_IO_Audio.selectedSampleRate = currentIndex
      }
    }

    //
    // Input sample format selection
    //
    Label {
      visible: _inFmt.visible
      text: qsTr("Sample Format") + ":"
    } ComboBox {
      id: _inFmt
      Layout.fillWidth: true
      visible: count > 0 && _inDev.visible
      model: Cpp_IO_Audio.inputSampleFormats
      currentIndex: Cpp_IO_Audio.selectedInputSampleFormat
      onCurrentIndexChanged: {
        if (currentIndex < 0 || count <= currentIndex)
          currentIndex = 0

        if (Cpp_IO_Audio.selectedInputSampleFormat !== currentIndex)
          Cpp_IO_Audio.selectedInputSampleFormat = currentIndex
      }
    }

    //
    // Input channels
    //
    Label {
      visible: _inChan.visible
      text: qsTr("Channels") + ":"
    } ComboBox {
      id: _inChan
      Layout.fillWidth: true
      visible: count > 0 && _inDev.visible
      model: Cpp_IO_Audio.inputChannelConfigurations
      currentIndex: Cpp_IO_Audio.selectedInputChannelConfiguration
      onCurrentIndexChanged: {
        if (currentIndex < 0 || count <= currentIndex)
          currentIndex = 0

        if (Cpp_IO_Audio.selectedInputChannelConfiguration !== currentIndex)
          Cpp_IO_Audio.selectedInputChannelConfiguration = currentIndex
      }
    }


    //
    // Spacer
    //
    Item {
      Layout.minimumHeight: 8 / 2
      Layout.maximumHeight: 8 / 2
    } Item {
      Layout.minimumHeight: 8 / 2
      Layout.maximumHeight: 8 / 2
    }

    //
    // Output device selection
    //
    Label {
      visible: _outDev.visible
      text: qsTr("Output Device") + ":"
    } ComboBox {
      id: _outDev
      visible: count > 0
      Layout.fillWidth: true
      model: Cpp_IO_Audio.outputDeviceList
      currentIndex: Cpp_IO_Audio.selectedOutputDevice
      onCurrentIndexChanged: {
        if (currentIndex < 0 || count <= currentIndex)
          currentIndex = 0

        if (Cpp_IO_Audio.selectedOutputDevice !== currentIndex)
          Cpp_IO_Audio.selectedOutputDevice = currentIndex
      }
    }

    //
    // Output sample format selection
    //
    Label {
      visible: _outFmt.visible
      text: qsTr("Sample Format") + ":"
    } ComboBox {
      id: _outFmt
      Layout.fillWidth: true
      visible: count > 0 && _outDev.visible
      model: Cpp_IO_Audio.outputSampleFormats
      currentIndex: Cpp_IO_Audio.selectedOutputSampleFormat
      onCurrentIndexChanged: {
        if (currentIndex < 0 || count <= currentIndex)
          currentIndex = 0

        if (Cpp_IO_Audio.selectedOutputSampleFormat !== currentIndex)
          Cpp_IO_Audio.selectedOutputSampleFormat = currentIndex
      }
    }

    //
    // Output channels
    //
    Label {
      visible: _outChan.visible
      text: qsTr("Channels") + ":"
    } ComboBox {
      id: _outChan
      Layout.fillWidth: true
      visible: count > 0 && _outDev.visible
      model: Cpp_IO_Audio.outputChannelConfigurations
      currentIndex: Cpp_IO_Audio.selectedOutputChannelConfiguration
      onCurrentIndexChanged: {
        if (currentIndex < 0 || count <= currentIndex)
          currentIndex = 0

        if (Cpp_IO_Audio.selectedOutputChannelConfiguration !== currentIndex)
          Cpp_IO_Audio.selectedOutputChannelConfiguration = currentIndex
      }
    }

    //
    // Spacer
    //
    Item {
      Layout.minimumHeight: 8 / 2
      Layout.maximumHeight: 8 / 2
    } Item {
      Layout.minimumHeight: 8 / 2
      Layout.maximumHeight: 8 / 2
    }

    //
    // Flexible spacer to push widgets up
    //
    Item {
      Layout.fillHeight: true
    } Item {
      Layout.fillHeight: true
    }
  }
}
