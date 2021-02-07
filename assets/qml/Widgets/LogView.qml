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
    readonly property int digits: Math.max(3, listView.count.toString().length)

    //
    // Set colors
    //
    property color textColor: "#72d084"
    property color caretLineColor: "#222228"
    property color backgroundColor: "#060601"
    property color lineCountTextColor: "#545454"
    property color lineCountBackgroundColor: "#121212"

    //
    // Returns @c true if text is selected
    //
    function copyAvailable() {
        if (selectedText.length > 0)
            return true

        var item = null
        var selected = false;
        for (var i = 0; i < listView.count; ++i) {
            item = listView.itemAtIndex(i)
            if (item !== null) {
                if (item.selected) {
                    selected = true
                    break
                }
            }
        }

        return selected
    }

    //
    // Copy text
    //
    function getTextToCopy() {
        var text = ""
        var item = null
        for (var i = 0; i < listView.count; ++i) {
            item = listView.itemAtIndex(i)
            if (item !== null) {
                if (item.selected)
                    text += item.text + "\n"
            }
        }

        if (text.length <= 0)
            return root.selectedText

        return text
    }

    //
    //  Selects all the text
    //
    function selectAll() {
        var item = null
        for (var i = 0; i < listView.count; ++i) {
            item = listView.itemAtIndex(i)
            if (item !== null)
                item.selected = true
        }
    }

    //
    // Updates the caret line location so that its shown in the vertical location of
    // the given @a mouse area
    //
    function updateCaretLineLocation(mouseArea) {
        if (mouseArea.containsMouse && (!autoscroll || !Cpp_IO_Manager.connected)) {
            var contentX = lineCountRect.width + 2 * app.spacing
            var contentY = mouseArea.mouseY + listView.contentY - root.font.pixelSize
            var index = listView.indexAt(contentX, contentY)

            if (index >= 0) {
                listView.currentIndex = index
                listView.previousCurrentIndex = index
                root.selectedText = listView.currentItem.text
            }
        }
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
        interactive: false
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

                if (currentIndex >= 0) {
                    previousCurrentIndex = currentIndex
                    root.selectedText = root.model[currentIndex]
                }
            }

            else {
                contentY = currentContentY
                currentIndex = previousCurrentIndex
            }
        }

        //
        // Caret line + line number
        //
        highlight: Rectangle {
            z: 0
            width: listView.width
            color: root.caretLineColor
            implicitWidth: listView.width
            y: listView.currentItem !== null ? listView.currentItem.y : 0
            height: listView.currentItem !== null ? listView.currentItem.height :
                                                    lineNumber.implicitHeight

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
            z: 1
            font: root.font
            text: modelData
            color: root.textColor
            height: implicitHeight
            width: listView.width - x
            x: app.spacing + lineCountRect.width
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere

            property bool selected: false

            Rectangle {
                z: 0
                x: 0
                y: 0
                opacity: 0.5
                width: parent.width
                height: parent.height
                color: parent.selected ? "#5F227CEB" : "transparent"
            }
        }
    }

    //
    // Simple implementation of a vertical text cursor
    //
    MouseArea {
        hoverEnabled: true
        anchors.fill: parent
        cursorShape: Qt.IBeamCursor
        acceptedButtons: Qt.RightButton
        onClicked: contextMenu.popup()
        onMouseYChanged: root.updateCaretLineLocation(this)
    }

    //
    // Implementation of a rectangular selection that selects any line that is
    // is "touched" by the selector rectangle
    //
    DragSelector {
        listView: listView
        anchors.fill: parent
        verticalTolerance: font.pixelSize
        xStart: lineCountRect.width + 2 * app.spacing
        onMouseYChanged: root.updateCaretLineLocation(this)
    }
}
