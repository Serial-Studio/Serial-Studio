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

Page {
    id: root
    clip: true

    //
    // Custom properties
    //
    property int borderWidth: 3
    property alias icon: _bt.icon
    property bool gradient: false
    property int headerHeight: 32
    property bool headerVisible: true
    property alias showIcon: _bt.visible
    property int radius: root.borderWidth + 2
    property color titleColor: palette.brightText
    property color borderColor: palette.highlight
    property bool altButtonEnabled: false
    property alias altButtonIcon: altBt.icon
    property alias headerDoubleClickEnabled: headerMouseArea.enabled
    property color backgroundColor: Cpp_ThemeManager.widgetWindowBackground
    property color gradientColor1: root.gradient ? Cpp_ThemeManager.windowGradient1 :
                                                   root.borderColor
    property color gradientColor2: root.gradient ? Cpp_ThemeManager.windowGradient2 :
                                                   root.borderColor

    //
    // Signals
    //
    signal altButtonClicked()
    signal headerDoubleClicked()

    //
    // Animations
    //
    Behavior on Layout.preferredWidth {NumberAnimation{}}
    Behavior on Layout.preferredHeight {NumberAnimation{}}

    //
    // Layout properties
    //
    visible: opacity > 0
    Layout.preferredWidth: enabled ? implicitWidth : 0
    Layout.preferredHeight: enabled ? implicitHeight : 0

    //
    // Background widget
    //
    background: Rectangle {
        radius: root.radius
        color: root.backgroundColor
        border.width: root.borderWidth
        border.color: root.gradientColor1

        Rectangle {
            border.width: 1
            color: "transparent"
            anchors.fill: parent
            anchors.topMargin: header.height
            anchors.margins: root.borderWidth
            border.color: Cpp_ThemeManager.border
        }
    }

    //
    // Window title & controls
    //
    header: Rectangle {
        radius: root.radius
        color: root.borderColor
        height: root.headerHeight
        visible: root.headerVisible

        gradient: Gradient {
            GradientStop {
                position: 0
                color: root.gradientColor1
            }

            GradientStop {
                position: 1
                color: root.gradientColor2
            }
        }

        Rectangle {
            z: 5
            color: root.gradientColor1
            height: root.gradient ? 1 : parent.radius

            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
        }

        MouseArea {
            z: 2
            id: headerMouseArea
            anchors.fill: parent
            onDoubleClicked: root.headerDoubleClicked()

            onClicked: {
                if (mouse.x >= headerBt.x && mouse.x <= headerBt.x + headerBt.width)
                    root.headerDoubleClicked()
            }
        }

        RowLayout {
            spacing: 0
            anchors.fill: parent

            Icon {
                id: _bt
                color: root.titleColor
                source: "qrc:/icons/widget.svg"
                Layout.alignment: Qt.AlignVCenter
                Layout.maximumHeight: parent.height
                Layout.minimumHeight: parent.height
                Layout.minimumWidth: root.headerHeight
                Layout.maximumWidth: root.headerHeight
                icon.width: root.headerHeight * 20 / 32
                icon.height: root.headerHeight * 20 / 32
            }

            Label {
                font.bold: true
                text: root.title
                Layout.fillWidth: true
                color: root.titleColor
                Layout.alignment: Qt.AlignVCenter
                font.pixelSize: root.headerHeight * 14 / 32
                horizontalAlignment: root.showIcon ? Label.AlignLeft : Label.AlignHCenter
            }

            Icon {
                id: headerBt
                color: root.titleColor
                Layout.alignment: Qt.AlignVCenter
                Layout.maximumHeight: parent.height
                Layout.minimumHeight: parent.height
                icon.source: "qrc:/icons/expand.svg"
                Layout.minimumWidth: root.headerHeight
                Layout.maximumWidth: root.headerHeight
                visible: root.headerDoubleClickEnabled
                icon.width: root.headerHeight * 20 / 32
                icon.height: root.headerHeight * 20 / 32
            }

            Button {
                id: altBt
                flat: true
                enabled: visible
                icon.color: root.titleColor
                visible: root.altButtonEnabled
                Layout.alignment: Qt.AlignVCenter
                onClicked: root.altButtonClicked()
                Layout.maximumHeight: parent.height
                Layout.minimumHeight: parent.height
                Layout.minimumWidth: root.headerHeight
                Layout.maximumWidth: root.headerHeight
                icon.width: root.headerHeight * 20 / 32
                icon.height: root.headerHeight * 20 / 32
                background: Item {}
            }
        }
    }
}
