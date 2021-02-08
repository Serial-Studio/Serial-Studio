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

    //
    // Declare custom properties
    //
    property int xStart: 0
    property int mouseX: 0
    property int mouseY: 0
    property int itemHeight: 0
    property int initialXPos: 0
    property int initialYPos: 0
    property bool wasHeld: false
    property var selectedLines: []
    property ListView listView: null

    //
    // Signals
    //
    signal rightClicked()
    signal selectionChanged()

    //
    // Accepted buttons
    //
    acceptedButtons: Qt.RightButton | Qt.LeftButton

    //
    // Selects all the items that are "touched" by the selection rectangle
    //
    function resetSelection() {
        // Get rectangle *displayed* height
        var rectHeight = selectionRect.height
        if (Math.abs(selectionRect.rotation) == 90)
            rectHeight = selectionRect.width

        // Get start & end y coordinate of rectangle
        var y1 = selectionRect.y
        var y2 = y1 + rectHeight
        if (selectionRect.rotation < 0)
            y2 = y1 - rectHeight

        // Add tolerances & order Y coordinates
        var yStart = Math.min(y1, y2)
        var yEnd = Math.max(y1, y2)

        // Selection rectangle visible
        if (!selectionRect.visible)
            return;

        // Check if item is selected
        for (var i = 0; i < listView.count; ++i) {
            // Get item & validate it
            var item = listView.itemAtIndex(i)
            if (item === null)
                continue

            // Get item relative vertical position
            var itemY = item.y - listView.contentY

            // Check if item is positioned inside the rectangle
            var inSelectionStart = (itemY >= yStart || itemY + root.itemHeight >= yStart)
            var inSelectionEnd = (itemY <= yEnd || itemY + root.itemHeight <= yEnd)

            // Check if selected lines already contains this line, if not,
            // add it to the selected lines array
            var index = root.selectedLines.indexOf(i)
            if (inSelectionStart && inSelectionEnd) {
                if (index < 0)
                    root.selectedLines.push(i)
            }

            // Selection does not contain this line, remove it from the array
            else if (index >= 0)
                root.selectedLines.splice(root.selectedLines.indexOf(i), 1)

        }

        // Update UI
        root.selectionChanged()
    }

    //
    // Draw the rectangle
    //
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
        border.color: "#4284d0"
        transformOrigin: Item.TopLeft
        onYChanged: root.resetSelection()
        onHeightChanged: root.resetSelection()
    }

    //
    // Update values when the user clicks anywhere on the screen
    //
    onClicked: {
        if (mouse.button == Qt.RightButton) {
            root.rightClicked()
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

    //
    // Anchor the rectangle when the user clicks and begins dragging
    //
    onPressed: {
        if(mouse.button == Qt.RightButton) {
            root.rightClicked()
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

    //
    // Draw selection rectangle when position is changed
    //
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
            var mX = Math.min(root.width, Math.max(0, mouse.x))
            var mY = Math.min(root.height, Math.max(0, mouse.y))

            mouseX = mX
            mouseY = mY

            if (Math.abs(mouseX, initialXPos) > 30)
                wasHeld = true

            if (Math.abs(mouseY, initialYPos) > 30)
                wasHeld = true

            if (mX >= initialXPos) {
                if (mY + listView.contentY >= initialYPos)
                    selectionRect.rotation = 0
                else
                    selectionRect.rotation = -90
            }

            else if (mX <= initialXPos) {
                if (mY + listView.contentY >= initialYPos)
                    selectionRect.rotation = 90
                else
                    selectionRect.rotation = -180
            }

            if (selectionRect.rotation == 0 || selectionRect.rotation == -180) {
                selectionRect.y = initialYPos - listView.contentY
                selectionRect.height = Math.abs(mY - initialYPos + listView.contentY)
                selectionRect.width = Math.abs(mX - selectionRect.x)
            }

            else {
                selectionRect.y = initialYPos - listView.contentY
                selectionRect.height = Math.abs(mX - selectionRect.x)
                selectionRect.width = Math.abs(mY - selectionRect.y)
            }
        }
    }

    //
    // Hide selection rectangle when the user stops touching the mouse
    //
    onReleased: {
        selectionRect.visible = false
    }
}
