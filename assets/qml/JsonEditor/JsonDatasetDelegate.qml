/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
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

import QtQuick 2.3
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

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
    title: qsTr("Dataset %1 - %2").arg(dataset + 1).arg(Cpp_JSON_Editor.datasetTitle(group, dataset))

    //
    // Delete dataset button
    //
    altButtonEnabled: !showGroupWidget
    altButtonIcon.source: "qrc:/icons/close.svg"
    onAltButtonClicked: Cpp_JSON_Editor.deleteDataset(group, dataset)

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
        rowSpacing: app.spacing
        columnSpacing: app.spacing
        anchors.margins: app.spacing * 2

        //
        // Dataset title
        //
        Label {
            text: qsTr("Title:")
        } TextField {
            Layout.fillWidth: true
            text: Cpp_JSON_Editor.datasetTitle(group, dataset)
            placeholderText: qsTr("Sensor reading, uptime, etc...")
            onTextChanged: {
                Cpp_JSON_Editor.setDatasetTitle(group, dataset, text)
                root.title = qsTr("Dataset %1 - %2").arg(dataset + 1).arg(Cpp_JSON_Editor.datasetTitle(group, dataset))
            }
        }

        //
        // Dataset units
        //
        Label {
            text: qsTr("Units:")
        } TextField {
            Layout.fillWidth: true
            text: Cpp_JSON_Editor.datasetUnits(group, dataset)
            placeholderText: qsTr("Volts, meters, seconds, etc...")
            onTextChanged: Cpp_JSON_Editor.setDatasetUnits(group, dataset, text)
        }

        //
        // Frame index
        //
        Label {
            text: qsTr("Frame index:")
        } TextField {
            Layout.fillWidth: true
            text: Cpp_JSON_Editor.datasetIndex(group, dataset)
            onTextChanged: Cpp_JSON_Editor.setDatasetIndex(group, dataset, text)
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
            checked: Cpp_JSON_Editor.datasetLED(group, dataset)
            onCheckedChanged: Cpp_JSON_Editor.setDatasetLED(group, dataset, checked)
        }

        //
        // Dataset graph
        //
        Label {
            text: qsTr("Generate plot:")
        } Switch {
            id: linearPlot
            Layout.leftMargin: -app.spacing
            checked: Cpp_JSON_Editor.datasetGraph(group, dataset)
            onCheckedChanged: {
                if (!checked)
                    logPlot.checked = false

                Cpp_JSON_Editor.setDatasetGraph(group, dataset, checked)
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
            checked: Cpp_JSON_Editor.datasetLogPlot(group, dataset)
            onCheckedChanged: Cpp_JSON_Editor.setDatasetLogPlot(group, dataset, checked)
        }

        //
        // FFT plot
        //
        Label {
            text: qsTr("FFT plot:")
        } Switch {
            id: fftCheck
            Layout.leftMargin: -app.spacing
            checked: Cpp_JSON_Editor.datasetFftPlot(group, dataset)
            onCheckedChanged: Cpp_JSON_Editor.setDatasetFftPlot(group, dataset, checked)
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
            model: Cpp_JSON_Editor.availableDatasetLevelWidgets()
            currentIndex: Cpp_JSON_Editor.datasetWidgetIndex(group, dataset)
            onCurrentIndexChanged: {
                if (visible && currentIndex !== Cpp_JSON_Editor.datasetWidgetIndex(group, dataset))
                    Cpp_JSON_Editor.setDatasetWidget(group, dataset, currentIndex)
            }
        } TextField {
            readOnly: true
            Layout.fillWidth: true
            visible: showGroupWidget
            enabled: showGroupWidget
            text: Cpp_JSON_Editor.datasetWidget(group, dataset)
        }


        //
        // FFT max frequency
        //
        Label {
            text: qsTr("FFT Samples:")
            visible: root.fftSamplesVisible
        } TextField {
            id: fftSamples
            Layout.fillWidth: true
            visible: root.fftSamplesVisible
            text: Cpp_JSON_Editor.datasetFFTSamples(group, dataset)
            onTextChanged: Cpp_JSON_Editor.setDatasetFFTSamples(group, dataset, parseInt(text))
            validator: IntValidator {
                bottom: 8
                top: 40 * 1000
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
            text: Cpp_JSON_Editor.datasetWidgetMin(group, dataset)
            onTextChanged: Cpp_JSON_Editor.setDatasetWidgetMin(group, dataset, text)
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
            text: Cpp_JSON_Editor.datasetWidgetMax(group, dataset)
            onTextChanged: Cpp_JSON_Editor.setDatasetWidgetMax(group, dataset, text)

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
            text: Cpp_JSON_Editor.datasetWidgetAlarm(group, dataset)
            onTextChanged: Cpp_JSON_Editor.setDatasetWidgetAlarm(group, dataset, text)

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
