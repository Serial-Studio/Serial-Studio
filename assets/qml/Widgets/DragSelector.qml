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

MouseArea {
    id: root
    property alias selectionRect : selectionRect

    property int xStart: 0
    property int mouseX: 0
    property int mouseY: 0
    property int initialXPos: 0
    property int initialYPos: 0
    property bool wasHeld: false
    property ListView listView: null
    property int verticalTolerance: 0

    property alias selected: selectionRect.visible
    property alias selectionStart: selectionRect.y
    property alias selectionEnd: selectionRect.height

    onSelectionEndChanged: resetSelection()
    onSelectionStartChanged: resetSelection()

    function resetSelection() {
        for (var i = 0; i < listView.count; ++i) {
            var item = listView.itemAtIndex(i)
            if (item === null)
                continue

            if (!selectionRect.visible)
                item.selected = false

            else {
                var itemY = item.y - listView.contentY + font.pixelSize

                var rectHeight = selectionRect.height
                if (Math.abs(selectionRect.rotation) == 90)
                    rectHeight = selectionRect.width

                var y1 = selectionRect.y
                var y2 = y1 + rectHeight

                if (selectionRect.rotation < 0)
                    y2 = y1 - rectHeight

                var yStart = Math.min(y1, y2) - verticalTolerance
                var yEnd = Math.max(y1, y2)

                var inSelectionStart = (itemY >= yStart)
                var inSelectionEnd = (itemY <= yEnd)

                item.selected = inSelectionStart && inSelectionEnd
            }
        }
    }

    Rectangle {
        id: selectionRect

        x: 0
        y: 0
        z: 99
        width: 0
        height: 0
        rotation: 0
        visible: false
        border.width: 1
        color: "#5F227CEB"
        border.color: "#404AFE"
        transformOrigin: Item.TopLeft
    }

    onClicked: {
        if (mouse.button == Qt.RightButton) {
            mouse.accepted = false
            return
        }

        if (wasHeld || mouse.modifiers & Qt.ControlModifier )
            return

        mouseX = mouse.x
        mouseY = mouse.y
        initialXPos = mouse.x
        initialYPos = mouse.y
    }

    onDoubleClicked: {
        var item = listView.itemAt(root.xStart, mouseY + listView.contentY - verticalTolerance)
        if (item !== null)
            item.selected = true
    }

    onPressed: {
        if(mouse.button == Qt.RightButton) {
            mouse.accepted = false
            return
        }

        if (mouse.modifiers & Qt.ControlModifier) {
            mouse.accepted = false
            return
        }

        if (mouse.button == Qt.LeftButton) {
            initialXPos = mouse.x
            initialYPos = mouse.y + listView.contentY

            listView.interactive = false
            selectionRect.x = mouse.x
            selectionRect.y = mouse.y
            selectionRect.width = 0
            selectionRect.height = 0
        }
    }

    onPositionChanged: {
        if (mouse.button == Qt.RightButton) {
            mouse.accepted = false
            return;
        }

        if (mouse.modifiers & Qt.ControlModifier)
            return

        selectionRect.visible = true

        if (selectionRect.visible === false) {
            selectionRect.x = 0
            selectionRect.y = 0
            selectionRect.width = 0
            selectionRect.height = 0

            initialXPos = 0
            initialYPos = 0
        }

        if (selectionRect.visible === true) {
            mouseX = mouse.x
            mouseY = mouse.y

            if (Math.abs(mouseX, initialXPos) > 30)
                wasHeld = true

            if (Math.abs(mouseY, initialYPos) > 30)
                wasHeld = true

            if (mouse.x >= initialXPos) {
                if (mouse.y + listView.contentY >= initialYPos)
                    selectionRect.rotation = 0
                else
                    selectionRect.rotation = -90
            }

            else if (mouse.x <= initialXPos) {
                if (mouse.y + listView.contentY >= initialYPos)
                    selectionRect.rotation = 90
                else
                    selectionRect.rotation = -180
            }

            if (selectionRect.rotation == 0 || selectionRect.rotation == -180) {
                selectionRect.y = initialYPos - listView.contentY
                selectionRect.height = Math.abs(mouse.y - initialYPos + listView.contentY)
                selectionRect.width = Math.abs(mouse.x - selectionRect.x)
            }

            else {
                selectionRect.y = initialYPos - listView.contentY
                selectionRect.height = Math.abs(mouse.x - selectionRect.x)
                selectionRect.width = Math.abs(mouse.y - selectionRect.y)
            }
        }
    }

    onReleased: {
        listView.interactive = true
        selectionRect.visible = false
    }
}
