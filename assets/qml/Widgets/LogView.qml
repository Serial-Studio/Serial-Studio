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
    property color textColor: "#8ecd9d"
    property color backgroundColor: "#121218"

    //
    // Scroll up x positions
    //
    function scrollUp(position, currentIndex) {
        // Disable autoscroll
        Cpp_IO_Console.autoscroll = false

        // Get current index
        var index = listView.indexAt(dragSelector.xStart, listView.contentY)
        if (currentIndex)
            index = listView.currentIndex

        // Get new index
        var newIndex = index - position

        // Assign new index
        if (newIndex > 0) {
            listView.currentIndex = newIndex
            listView.previousCurrentIndex = listView.currentIndex
        }

        // We already reached the top of the document
        else {
            listView.currentIndex = 0
            listView.currentContentY = 0
            listView.previousCurrentIndex = 0
            listView.positionViewAtBeginning()
        }
    }

    //
    // Scroll down x positions
    //
    function scrollDown(position, currentIndex) {
        // Disable autoscroll
        Cpp_IO_Console.autoscroll = false

        // Get current index
        var index = listView.indexAt(dragSelector.xStart, listView.contentY + listView.height)
        if (currentIndex)
            index = listView.currentIndex

        // Get new index
        var newIndex = index + position

        // Assign new index
        if (newIndex < (listView.count - 1)) {
            listView.currentIndex = newIndex
            listView.previousCurrentIndex = listView.currentIndex
        }

        // Reached bottom of document, enable autoscroll
        else {
            Cpp_IO_Console.autoscroll = true
            listView.positionViewAtEnd()
        }
    }

    //
    // Goes to the bottom of the text & enables autoscroll
    //
    function scrollToBottom() {
        Cpp_IO_Console.autoscroll = true
        listView.positionViewAtEnd()
    }

    //
    // Scroll to the top of the document
    //
    function scrollToTop() {
        Cpp_IO_Console.autoscroll = false
        listView.currentIndex = 0
        listView.currentContentY = 0
        listView.previousCurrentIndex = 0
        listView.positionViewAtBeginning()
    }

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
        // Line delegate
        //
        delegate: Text {
            font: root.font
            text: modelData
            width: listView.width
            color: root.textColor
            height: root.lineHeight
            wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        }
    }

    //
    // Implementation of a rectangular selection that selects any line that is
    // is "touched" by the selector rectangle
    //
    DragSelector {
        id: dragSelector
        listView: listView
        anchors.fill: parent
        xStart: 2 * app.spacing
        itemHeight: root.lineHeight
        anchors.margins: app.spacing
        anchors.leftMargin: app.spacing
        //onMouseYChanged: root.updateCaretLineLocation(this)
        anchors.rightMargin: scrollbar.width + app.spacing - 2
        onRightClicked: contextMenu.popup()

        onDoubleClicked: {
            var index = listView.currentIndex

            if (index >= 0) {
                dragSelector.selectedLines = []
                dragSelector.selectedLines.push(index)
                dragSelector.selectionChanged()
            }
        }

        onWheel: {
            var yValue = wheel.angleDelta.y / 15
            if (Math.abs(yValue) < 2)
                return

            if (yValue > 0)
                root.scrollUp(yValue, true)
            else
                root.scrollDown(Math.abs(yValue), true)
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

        property real position: listView.contentY / Math.max(listView.height, listView.contentHeight)

        width: 18
        spacing: 0
        opacity: enabled ? 1 : 0.5
        enabled: listView.contentHeight > listView.height

        anchors {
            top: parent.top
            right: parent.right
            bottom: parent.bottom
        }

        function setPosition(ratio) {
            Cpp_IO_Console.autoscroll = false
            var fixedRatio = Math.max(0, Math.min(1, ratio))

            if (fixedRatio === 1)
                scrollToBottom()
            else if (fixedRatio === 0)
                scrollToTop()

            else {
                var yPos = fixedRatio * listView.contentHeight
                listView.currentIndex = listView.indexAt(100, yPos)
                listView.previousCurrentIndex = listView.currentIndex
                listView.contentY = yPos
                listView.currentContentY = yPos
            }

        }

        //
        // Scroll to top
        //
        Button {
            Layout.fillWidth: true
            icon.color: palette.text
            onClicked: root.scrollToTop()
            Layout.minimumHeight: scrollbar.width
            Layout.maximumHeight: scrollbar.width
            icon.source: "qrc:/icons/scroll-top.svg"
        }

        //
        // Scroll up
        //
        Button {
            id: scrollUp
            Layout.fillWidth: true
            icon.color: palette.text
            onClicked: root.scrollUp(10, false)
            Layout.minimumHeight: scrollbar.width
            Layout.maximumHeight: scrollbar.width
            icon.source: "qrc:/icons/scroll-up.svg"
        }

        //
        // Handle
        //
        Item {
            id: handleArea
            Layout.fillWidth: true
            Layout.fillHeight: true

            Button {
                opacity: 0.5
                checked: true
                anchors.fill: parent
            }

            Button {
                id: handle
                height: 32
                anchors.left: parent.left
                anchors.right: parent.right
                y: Math.max(0, scrollbar.position * (handleArea.height - height))

                MouseArea {
                    anchors.fill: parent
                    drag.target: parent
                    drag.axis: Drag.YAxis
                    drag.minimumY: 0
                    drag.maximumY: scrollbar.height - handle.height

                    onPositionChanged: {
                        scrollbar.setPosition(handle.y / handleArea.height)
                    }
                }
            }
        }

        //
        // Scroll down
        //
        Button {
            id: scrollDown
            Layout.fillWidth: true
            icon.color: palette.text
            onClicked: root.scrollDown(10, false)
            Layout.minimumHeight: scrollbar.width
            Layout.maximumHeight: scrollbar.width
            icon.source: "qrc:/icons/scroll-down.svg"
        }

        //
        // Scroll to bottom
        //
        Button {
            Layout.fillWidth: true
            icon.color: palette.text
            onClicked: root.scrollToBottom()
            Layout.minimumHeight: scrollbar.width
            Layout.maximumHeight: scrollbar.width
            icon.source: "qrc:/icons/scroll-bottom.svg"
        }
    }

    //
    // Key navigation
    //
    focus: true
    Shortcut { sequence: "up";        onActivated: root.scrollUp(1, true) }
    Shortcut { sequence: "down";      onActivated: root.scrollDown(1, true) }
    Shortcut { sequence: "escape";    onActivated: deselect() }
    Shortcut { sequence: "space";     onActivated: deselect() }
    Shortcut { sequence: StandardKey.MoveToPreviousPage; onActivated: root.scrollUp(50, true) }
    Shortcut { sequence: StandardKey.MoveToNextPage;     onActivated: root.scrollDown(50, true) }
}
