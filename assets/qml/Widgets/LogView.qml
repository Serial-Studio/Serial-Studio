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

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

Rectangle {
    id: root
    clip: true
    smooth: false
    color: backgroundColor

    //
    // Custom properties
    //
    property int lineOffset: 0
    property Menu contextMenu: null
    property alias font: label.font
    property bool autoscroll: false
    property string selectedText: ""
    property string placeholderText: ""
    property alias model: listView.model
    readonly property int digits: listView.count.toString().length

    //
    // Set colors
    //
    property color textColor: "#72d084"
    property color caretLineColor: "#222228"
    property color backgroundColor: "#060601"
    property color lineCountTextColor: "#545454"
    property color lineCountBackgroundColor: "#121212"

    //
    // Gets the line number for the given @a index
    //
    function getLineNumber(index) {
        var lIndex = listView.indexAt(0, 0)
        if (lIndex < 0)
            lIndex = 0

        return lIndex + index + 1 + root.lineOffset
    }

    //
    // Placeholder text & font source for rest of widget
    //
    Text {
        id: label
        opacity: 0.5
        anchors.top: parent.top
        anchors.left: parent.left
        text: root.placeholderText
        visible: listView.count == 0
        anchors.margins: app.spacing
        anchors.leftMargin: lineCountRect.width
    }

    //
    // Line count rectangle
    //
    Rectangle {
        id: lineCountRect

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: root.border.width
        color: root.lineCountBackgroundColor
        width: root.font.pixelSize * (root.digits + 1)
    }

    //
    // Text view
    //
    ListView {
        id: listView
        cacheBuffer: 0
        currentIndex: 0
        model: root.model
        anchors.fill: parent
        anchors.leftMargin: 0
        anchors.margins: app.spacing
        highlightFollowsCurrentItem: false

        //
        // Scrollbar
        //
        ScrollBar.vertical: ScrollBar {
            id: scrollbar
        }

        //
        // Hacks to implement auto-scrolling & position preservation when new data is
        // added to the data model
        //
        property int currentContentY
        property int previousCurrentIndex
        onMovementEnded: {
            currentContentY = contentY
        }

        onCountChanged: {
            if (root.autoscroll) {
                listView.positionViewAtEnd()
                listView.currentIndex = listView.count - 2
                currentContentY = contentY
                previousCurrentIndex = currentIndex
                root.selectedText = root.model[currentIndex]
            }

            else {
                contentY = currentContentY
                currentIndex = previousCurrentIndex
            }
        }

        //
        // Highlight item
        //
        highlight: Rectangle {
            z: 0
            width: listView.width
            height: lineNumber.height
            y: listView.currentItem.y
            color: root.caretLineColor
            implicitWidth: listView.width

            Text {
                id: lineNumber
                font: root.font
                width: lineCountRect.width
                color: root.lineCountTextColor
                horizontalAlignment: Qt.AlignHCenter
                anchors.verticalCenter: parent.verticalCenter
                text: listView.currentIndex + root.lineOffset + 1
            }
        }

        //
        // Line delegate
        //
        delegate: Text {
            font: root.font
            text: modelData
            color: root.textColor
            height: font.pixelSize
            width: listView.width - x
            x: app.spacing + lineCountRect.width
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        }
    }

    //
    // Simple implementation of a vertical text cursor
    //
    MouseArea {
        hoverEnabled: true
        anchors.fill: parent
        onClicked: contextMenu.popup()
        acceptedButtons: Qt.RightButton
        onMouseYChanged: {
            if (containsMouse && (!autoscroll || !Cpp_IO_Manager.connected)) {
                var contentX = lineCountRect.width + 2 * app.spacing
                var contentY = mouseY + listView.contentY - root.font.pixelSize
                var index = listView.indexAt(contentX, contentY)

                if (index >= 0) {
                    listView.currentIndex = index
                    listView.previousCurrentIndex = index
                    root.selectedText = root.model[index]
                }
            }
        }
    }
}
