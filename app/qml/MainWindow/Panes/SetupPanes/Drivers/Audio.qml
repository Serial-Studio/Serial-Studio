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
  // Settings
  //
  Settings {
    category: "Audio"
    property alias bufferSize: _bufferSize.currentIndex
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
    enabled: !Cpp_IO_Manager.isConnected

    //
    // Buffer size
    //
    Label {
      text: qsTr("Buffer Size") + ":"
    } ComboBox {
      id: _bufferSize
      Layout.fillWidth: true
      model: Cpp_IO_Audio.bufferSizes
      currentIndex: Cpp_IO_Audio.bufferSize
      onCurrentIndexChanged: {
        if (Cpp_IO_Audio.bufferSize !== currentIndex)
          Cpp_IO_Audio.bufferSize = currentIndex
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
        if (Cpp_IO_Audio.selectedInputDevice !== currentIndex)
          Cpp_IO_Audio.selectedInputDevice = currentIndex
      }
    }

    //
    // Input sample rate selection
    //
    Label {
      visible: _inRate.visible
      text: qsTr("Input Sample Rate") + ":"
    } ComboBox {
      id: _inRate
      Layout.fillWidth: true
      visible: count > 0 && _inDev.visible
      model: Cpp_IO_Audio.inputSampleRates
      currentIndex: Cpp_IO_Audio.selectedInputSampleRate
      onCurrentIndexChanged: {
        if (Cpp_IO_Audio.selectedInputSampleRate !== currentIndex)
          Cpp_IO_Audio.selectedInputSampleRate = currentIndex
      }
    }

    //
    // Input sample format selection
    //
    Label {
      visible: _inFmt.visible
      text: qsTr("Input Sample Format") + ":"
    } ComboBox {
      id: _inFmt
      Layout.fillWidth: true
      visible: count > 0 && _inDev.visible
      model: Cpp_IO_Audio.inputSampleFormats
      currentIndex: Cpp_IO_Audio.selectedInputSampleFormat
      onCurrentIndexChanged: {
        if (Cpp_IO_Audio.selectedInputSampleFormat !== currentIndex)
          Cpp_IO_Audio.selectedInputSampleFormat = currentIndex
      }
    }

    //
    // Input channels
    //
    Label {
      visible: _inChan.visible
      text: qsTr("Input Channels") + ":"
    } ComboBox {
      id: _inChan
      Layout.fillWidth: true
      visible: count > 0 && _inDev.visible
      model: Cpp_IO_Audio.inputChannelConfigurations
      currentIndex: Cpp_IO_Audio.selectedInputChannelConfiguration
      onCurrentIndexChanged: {
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
        if (Cpp_IO_Audio.selectedOutputDevice !== currentIndex)
          Cpp_IO_Audio.selectedOutputDevice = currentIndex
      }
    }

    //
    // Output sample rate selection
    //
    Label {
      visible: _outRate.visible
      text: qsTr("Output Sample Rate") + ":"
    } ComboBox {
      id: _outRate
      Layout.fillWidth: true
      visible: count > 0 && _outDev.visible
      model: Cpp_IO_Audio.outputSampleRates
      currentIndex: Cpp_IO_Audio.selectedOutputSampleRate
      onCurrentIndexChanged: {
        if (Cpp_IO_Audio.selectedOutputSampleRate !== currentIndex)
          Cpp_IO_Audio.selectedOutputSampleRate = currentIndex
      }
    }

    //
    // Output sample format selection
    //
    Label {
      visible: _outFmt.visible
      text: qsTr("Output Sample Format") + ":"
    } ComboBox {
      id: _outFmt
      Layout.fillWidth: true
      visible: count > 0 && _outDev.visible
      model: Cpp_IO_Audio.outputSampleFormats
      currentIndex: Cpp_IO_Audio.selectedOutputSampleFormat
      onCurrentIndexChanged: {
        if (Cpp_IO_Audio.selectedOutputSampleFormat !== currentIndex)
          Cpp_IO_Audio.selectedOutputSampleFormat = currentIndex
      }
    }

    //
    // Output channels
    //
    Label {
      visible: _outChan.visible
      text: qsTr("Output Channels") + ":"
    } ComboBox {
      id: _outChan
      Layout.fillWidth: true
      visible: count > 0 && _outDev.visible
      model: Cpp_IO_Audio.outputChannelConfigurations
      currentIndex: Cpp_IO_Audio.selectedOutputChannelConfiguration
      onCurrentIndexChanged: {
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
