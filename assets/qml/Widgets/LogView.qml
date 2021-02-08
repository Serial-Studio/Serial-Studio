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
    property string selectedText: ""
    property string placeholderText: ""
    property alias model: listView.model
    readonly property int digits: Math.max(3, listView.count.toString().length)

    //
    // Line properties
    //
    readonly property int lineHeight: metrics.height
    FontMetrics {
        id: metrics
        font: root.font
    }

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

        return dragSelector.selectedLines.length > 0
    }

    //
    // Copy text
    //
    function getTextToCopy() {
        var text = ""
        var item = null
        for (var i = 0; i < dragSelector.selectedLines.length; ++i) {
            item = listView.itemAtIndex(dragSelector.selectedLines[i])

            if (item !== null)
                text += item.text + "\n"
        }

        if (text.length <= 0)
            return root.selectedText

        return text
    }

    //
    // Selects all the text
    //
    function selectAll() {
        dragSelector.selectedLines = []
        for (var i = 0; i < listView.count; ++i)
            dragSelector.selectedLines.push(i)

        dragSelector.selectionChanged()
    }

    //
    // De-selects all the text
    //
    function deselect() {
        dragSelector.selectedLines = []
        dragSelector.selectionChanged()
    }

    //
    // Updates the caret line location so that its shown in the vertical location of
    // the given @a mouse area
    //
    function updateCaretLineLocation(mouseArea) {
        if (mouseArea.containsMouse && (!Cpp_IO_Console.autoscroll || !Cpp_IO_Manager.connected)) {
            var contentX = lineCountRect.width + 2 * app.spacing
            var contentY = mouseArea.mouseY + listView.contentY
            var index = listView.indexAt(contentX, contentY)
            if (index < 0)
                index = listView.indexAt(contentX, contentY + root.lineHeight)

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
        width: root.font.pixelSize * (root.digits)
    }

    //
    // Text view
    //
    ListView {
        id: listView
        cacheBuffer: 0
        currentIndex: 0
        model: root.model
        interactive: false
        anchors.fill: parent
        anchors.leftMargin: 0
        snapMode: ListView.SnapToItem
        anchors.margins: app.spacing
        highlightFollowsCurrentItem: false
        anchors.rightMargin: scrollbar.width + app.spacing - 2

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
            if (Cpp_IO_Console.autoscroll) {
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

            if (count == 0)
                root.deselect()

            dragSelector.resetSelection()
        }

        //
        // Caret line + line number
        //
        highlight: Rectangle {
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
            font: root.font
            text: modelData
            color: root.textColor
            height: root.lineHeight
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
        id: dragSelector
        listView: listView
        anchors.fill: parent
        anchors.margins: app.spacing
        itemHeight: root.lineHeight
        xStart: lineCountRect.width + 2 * app.spacing
        onMouseYChanged: root.updateCaretLineLocation(this)
        anchors.leftMargin: lineCountRect.width + app.spacing
        anchors.rightMargin: scrollbar.width + app.spacing - 2

        onDoubleClicked: {
            var index = listView.currentIndex

            if (index >= 0) {
                dragSelector.selectedLines = []
                dragSelector.selectedLines.push(index)
                dragSelector.selectionChanged()
            }
        }
    }

    //
    // Click-to-deselect mouse area
    //
    MouseArea {
        id: deselector
        anchors.fill: parent

        Connections {
            target: dragSelector
            function onSelectionChanged() {
                deselector.enabled = dragSelector.selectedLines.length > 0
            }
        }

        onClicked: {
            root.deselect()
            dragSelector.wasHeld = false
        }
    }

    //
    // Line selector rectangles
    //
    ColumnLayout {
        spacing: 0
        anchors.fill: parent
        anchors.margins: app.spacing
        anchors.leftMargin: app.spacing + lineCountRect.width
        anchors.rightMargin: scrollbar.width + app.spacing - 2

        Repeater {
            model: Math.floor(parent.height / root.lineHeight)

            delegate: Rectangle {
                id: rect
                color: "transparent"
                Layout.fillWidth: true
                Layout.minimumHeight: root.lineHeight
                Layout.maximumHeight: root.lineHeight

                function update() {
                    if (dragSelector.selectedLines.length == 0) {
                        rect.color = "transparent"
                        return
                    }

                    var i = index + Math.round(listView.contentY / root.lineHeight)
                    var s = dragSelector.selectedLines.indexOf(i)

                    if (s >= 0)
                        rect.color = "#2F227CEB"
                    else
                        rect.color = "transparent"
                }

                Connections {
                    target: dragSelector

                    function onSelectionChanged() {
                        rect.update()
                    }
                }

                Connections {
                    target: listView


                    function onContentYChanged() {
                        rect.update()
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }

    //
    // Navigation buttons
    //
    ColumnLayout {
        id: scrollbar

        width: 24
        spacing: 0
        opacity: enabled ? 1 : 0.5
        enabled: listView.contentHeight > listView.height

        anchors {
            top: parent.top
            right: parent.right
            bottom: parent.bottom
        }

        Button {
            Layout.fillWidth: true
            icon.color: palette.text
            icon.source: "qrc:/icons/scroll-top.svg"
            onClicked: {
                Cpp_IO_Console.autoscroll = false
                listView.currentIndex = 0
                listView.currentContentY = 0
                listView.previousCurrentIndex = 0
                listView.positionViewAtBeginning()
            }
        }

        Button {
            Layout.fillWidth: true
            icon.color: palette.text
            icon.source: "qrc:/icons/scroll-up.svg"
            onClicked: {
                Cpp_IO_Console.autoscroll = false

                var topMostIndex = listView.indexAt(dragSelector.xStart, listView.contentY)
                var newIndex = topMostIndex - 10

                if (newIndex > 0) {
                    listView.currentIndex = newIndex
                    listView.previousCurrentIndex = listView.currentIndex
                }

                else {
                    listView.currentIndex = 0
                    listView.currentContentY = 0
                    listView.previousCurrentIndex = 0
                    listView.positionViewAtBeginning()
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Button {
                opacity: 0.5
                checked: true
                anchors.fill: parent
            }

            Button {
                anchors.left: parent.left
                anchors.right: parent.right
                height: Math.max(32, ratio * parent.height)
                y: Math.max(0, parent.height * Math.min(1, position) - height)
                property real ratio: Math.min(1, listView.height / listView.contentHeight)
                property real position: (listView.contentY + listView.height) / listView.contentHeight
            }
        }

        Button {
            Layout.fillWidth: true
            icon.color: palette.text
            icon.source: "qrc:/icons/scroll-down.svg"

            onClicked: {
                Cpp_IO_Console.autoscroll = false

                var bottomIndex = listView.indexAt(dragSelector.xStart, listView.contentY + listView.height)
                var newIndex = bottomIndex + 10

                if (newIndex < listView.count) {
                    listView.currentIndex = newIndex
                    listView.previousCurrentIndex = listView.currentIndex
                }

                else {
                    Cpp_IO_Console.autoscroll = true
                    listView.positionViewAtEnd()
                }
            }
        }

        Button {
            Layout.fillWidth: true
            icon.color: palette.text
            icon.source: "qrc:/icons/scroll-bottom.svg"
            onClicked: {
                Cpp_IO_Console.autoscroll = true
                listView.positionViewAtEnd()
            }
        }
    }
}
