/*
 * Copyright (c) 2020-2023 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets" as Widgets

Widgets.Window {
  id: root

  //
  // Window properties
  //
  headerDoubleClickEnabled: false
  icon.source: "qrc:/icons/dataset.svg"
  borderColor: Cpp_ThemeManager.widgetWindowBorder
  palette.window: Cpp_ThemeManager.widgetWindowBackground
  title: qsTr("Dataset %1 - %2").arg(dataset + 1).arg(Cpp_Project_Model.datasetTitle(group, dataset))

  //
  // Delete dataset button
  //
  altButtonEnabled: !showGroupWidget
  altButtonIcon.source: "qrc:/icons/close.svg"
  onAltButtonClicked: Cpp_Project_Model.deleteDataset(group, dataset)

  //
  // Custom properties
  //
  property int group
  property int dataset
  property bool multiplotGroup
  property bool showGroupWidget

  //
  // Convenience variables
  //
  readonly property bool fftSamplesVisible: fftCheck.checked
  readonly property bool alarmVisible: widget.currentIndex === 2
  readonly property bool minMaxVisible: widget.currentIndex === 1 ||
                                        widget.currentIndex === 2 ||
                                        logPlot.checked ||
                                        linearPlot.checked ||
                                        root.multiplotGroup

  //
  // User interface
  //
  GridLayout {
    x: 0
    columns: 2
    anchors.fill: parent
    columnSpacing: app.spacing
    anchors.margins: app.spacing
    rowSpacing: app.spacing / 2
    anchors.leftMargin: app.spacing * 2
    anchors.rightMargin: app.spacing * 2

    //
    // Dataset title
    //
    Label {
      text: qsTr("Title:")
    } TextField {
      Layout.fillWidth: true
      text: Cpp_Project_Model.datasetTitle(group, dataset)
      placeholderText: qsTr("Sensor reading, uptime, etc...")
      onTextChanged: {
        Cpp_Project_Model.setDatasetTitle(group, dataset, text)
        root.title = qsTr("Dataset %1 - %2").arg(dataset + 1).arg(Cpp_Project_Model.datasetTitle(group, dataset))
      }
    }

    //
    // Dataset units
    //
    Label {
      text: qsTr("Units:")
    } TextField {
      Layout.fillWidth: true
      text: Cpp_Project_Model.datasetUnits(group, dataset)
      placeholderText: qsTr("Volts, meters, seconds, etc...")
      onTextChanged: Cpp_Project_Model.setDatasetUnits(group, dataset, text)
    }

    //
    // Frame index
    //
    Label {
      text: qsTr("Frame index:")
    } TextField {
      Layout.fillWidth: true
      text: Cpp_Project_Model.datasetIndex(group, dataset)
      onTextChanged: Cpp_Project_Model.setDatasetIndex(group, dataset, text)
      validator: IntValidator {
        bottom: 1
        top: 100
      }
    }

    //
    // Dataset LED
    //
    Label {
      text: qsTr("Display LED:")
    } Switch {
      id: led
      Layout.leftMargin: -app.spacing
      checked: Cpp_Project_Model.datasetLED(group, dataset)
      onCheckedChanged: Cpp_Project_Model.setDatasetLED(group, dataset, checked)
    }

    //
    // Dataset graph
    //
    Label {
      text: qsTr("Generate plot:")
    } Switch {
      id: linearPlot
      Layout.leftMargin: -app.spacing
      checked: Cpp_Project_Model.datasetGraph(group, dataset)
      onCheckedChanged: {
        if (!checked)
          logPlot.checked = false

        Cpp_Project_Model.setDatasetGraph(group, dataset, checked)
      }
    }

    //
    // Log plot
    //
    Label {
      text: qsTr("Logarithmic plot:")
      visible: linearPlot.checked
    } CheckBox {
      id: logPlot
      visible: linearPlot.checked
      Layout.leftMargin: -app.spacing
      checked: Cpp_Project_Model.datasetLogPlot(group, dataset)
      onCheckedChanged: Cpp_Project_Model.setDatasetLogPlot(group, dataset, checked)
    }

    //
    // FFT plot
    //
    Label {
      text: qsTr("FFT plot:")
    } Switch {
      id: fftCheck
      Layout.leftMargin: -app.spacing
      checked: Cpp_Project_Model.datasetFftPlot(group, dataset)
      onCheckedChanged: Cpp_Project_Model.setDatasetFftPlot(group, dataset, checked)
    }

    //
    // Dataset widget (user selectable or group-level constant)
    //
    Label {
      text: qsTr("Widget:")
    } ComboBox {
      id: widget
      Layout.fillWidth: true
      visible: !showGroupWidget
      enabled: !showGroupWidget
      model: Cpp_Project_Model.availableDatasetLevelWidgets()
      currentIndex: Cpp_Project_Model.datasetWidgetIndex(group, dataset)
      onCurrentIndexChanged: {
        if (visible && currentIndex !== Cpp_Project_Model.datasetWidgetIndex(group, dataset))
          Cpp_Project_Model.setDatasetWidget(group, dataset, currentIndex)
      }
    } TextField {
      readOnly: true
      Layout.fillWidth: true
      visible: showGroupWidget
      enabled: showGroupWidget
      text: Cpp_Project_Model.datasetWidget(group, dataset)
    }


    //
    // FFT max frequency
    //
    Label {
      text: qsTr("FFT Window Size:")
      visible: root.fftSamplesVisible
    } ComboBox {
      id: fftSamples
      Layout.fillWidth: true
      visible: root.fftSamplesVisible
      model: [8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384]
      onCurrentValueChanged: Cpp_Project_Model.setDatasetFFTSamples(group, dataset, currentValue)

      property int actualValue: Cpp_Project_Model.datasetFFTSamples(group, dataset)
      Component.onCompleted: {
        var index = model.indexOf(actualValue)
        if (index !== -1)
          fftSamples.currentIndex = index
        else
          fftSamples.currentIndex = 7
      }
    }

    //
    // Widget minimum value
    //
    Label {
      text: qsTr("Min value:")
      visible: root.minMaxVisible
    } TextField {
      id: min
      Layout.fillWidth: true
      visible: root.minMaxVisible
      text: Cpp_Project_Model.datasetWidgetMin(group, dataset)
      onTextChanged: Cpp_Project_Model.setDatasetWidgetMin(group, dataset, text)
      validator: DoubleValidator {
        top: parseFloat(max.text)
      }
    }

    //
    // Widget maximum value
    //
    Label {
      text: qsTr("Max value:")
      visible: root.minMaxVisible
    } TextField {
      id: max
      Layout.fillWidth: true
      visible: root.minMaxVisible
      text: Cpp_Project_Model.datasetWidgetMax(group, dataset)
      onTextChanged: Cpp_Project_Model.setDatasetWidgetMax(group, dataset, text)

      validator: DoubleValidator {
        bottom: parseFloat(min.text)
      }
    }

    //
    // Bar alarm level
    //
    Label {
      text: qsTr("Alarm level:")
      visible: root.alarmVisible
    } TextField {
      id: alarm
      Layout.fillWidth: true
      visible: root.alarmVisible
      text: Cpp_Project_Model.datasetWidgetAlarm(group, dataset)
      onTextChanged: Cpp_Project_Model.setDatasetWidgetAlarm(group, dataset, text)

      validator: DoubleValidator {
        top: parseFloat(max.text)
        bottom: parseFloat(min.text)
      }
    }

    //
    // Vertical spacer
    //
    Item {
      Layout.fillHeight: true
    } Item {
      Layout.fillHeight: true
    }

    //
    // Compass note label
    //
    Widgets.Icon {
      width: 32
      height: 32
      color: palette.text
      source: "qrc:/icons/compass.svg"
      Layout.alignment: Qt.AlignHCenter
      visible: widget.currentIndex === 3
    } Label {
      font.pixelSize: 16
      Layout.fillWidth: true
      wrapMode: Label.WordWrap
      visible: widget.currentIndex === 3
      text: "<b>" + qsTr("Note:") + "</b> " + qsTr("The compass widget expects values from 0° to 360°.")
    }

    //
    // Vertical spacer
    //
    Item {
      Layout.fillHeight: true
    } Item {
      Layout.fillHeight: true
    }
  }
}
