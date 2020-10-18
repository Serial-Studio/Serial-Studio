/*
 * Copyright (c) 2020 Alex Spataru <https://github.com/alex-spataru>
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
import QtGraphicalEffects 1.0

import Dataset 1.0

Item {
    height: 14
    property Dataset dataset: null

    Label {
        text: dataset.title
        elide: Label.ElideRight
        font.family: app.monoFont
        horizontalAlignment: Text.AlignLeft
        color: Qt.rgba(230/255, 224/255, 178/255, 0.6)

        anchors {
            leftMargin: 0
            left: parent.left
            right: center.left
            margins: app.spacing / 2
            verticalCenter: parent.verticalCenter
        }
    }

    Image {
        id: center
        width: sourceSize.width
        height: sourceSize.height
        sourceSize: Qt.size(18, 18)
        source: "qrc:/icons/ethernet.svg"

        anchors {
            centerIn: parent
        }

        ColorOverlay {
            source: parent
            anchors.fill: parent
            color: Qt.rgba(81/255, 116/255, 151/255, 1)
        }
    }

    Label {
        text: dataset.value
        elide: Label.ElideRight
        font.family: app.monoFont
        horizontalAlignment: Text.AlignLeft
        color: Qt.rgba(230/255, 224/255, 178/255, 1)

        anchors {
            left: center.right
            right: units.left
            margins: app.spacing / 2
            verticalCenter: parent.verticalCenter
        }
    }

    Label {
        id: units
        font.family: app.monoFont
        text: "[" + dataset.units + "]"
        visible: dataset.units.length > 0
        horizontalAlignment: Text.AlignRight
        color: Qt.rgba(81/255, 116/255, 151/255, 1)

        anchors {
            rightMargin: 0
            right: parent.right
            margins: app.spacing / 2
            verticalCenter: parent.verticalCenter
        }
    }
}
