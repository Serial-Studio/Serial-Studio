/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Controls

import SerialStudio

Item {
  id: root

  required property color color
  required property var windowRoot
  required property var model
  required property string widgetId

  onWidthChanged: Qt.callLater(relayout)
  onHeightChanged: Qt.callLater(relayout)
  Component.onCompleted: Qt.callLater(relayout)

  function relayout() {
    if (root.model && root.width > 0 && root.height > 0)
      root.model.updateLayout(root.width, root.height)
  }

  Flickable {
    id: flickable

    clip: true
    contentWidth: width
    anchors.fill: parent
    contentHeight: {
      if (!root.model || root.model.count === 0)
        return 0

      var geo = root.model.geometry
      var maxY = 0
      for (var i = 0; i < geo.length; ++i) {
        var bottom = geo[i].y + geo[i].h
        if (bottom > maxY)
          maxY = bottom
      }

      return maxY + 4
    }

    ScrollBar.vertical: ScrollBar {
      policy: flickable.contentHeight > flickable.height
              ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
    }

    Repeater {
      model: root.model ? root.model.count : 0

      delegate: Rectangle {
        id: cell

        property var geo: root.model ? root.model.geometry[index] : null

        x: geo ? geo.x : 0
        y: geo ? geo.y : 0
        width: geo ? geo.w : 0
        height: geo ? geo.h : 0

        radius: 4
        color: Cpp_ThemeManager.colors["groupbox_background"]
        border.width: (cell.txError || cell.statePending) ? 2 : 1
        border.color: cell.txError ? Cpp_ThemeManager.colors["alarm"]
                      : cell.statePending ? cell.accentColor
                      : Cpp_ThemeManager.colors["groupbox_border"]

        property var owData: root.model ? root.model.widgets[index] : null
        property var owModel: root.model ? root.model.models[index] : null

        property int owType: owData ? owData.type : 0
        property string owColor: owData ? (owData.color || "") : ""
        property bool stateUnknown: cell.stateBound && !cell.stateKnown
        property bool stateBound: cell.owModel ? cell.owModel.stateBound : false
        property bool stateKnown: cell.owModel ? cell.owModel.stateKnown : false
        property bool statePending: cell.owModel ? cell.owModel.statePending : false
        property color accentColor: cell.owColor.length > 0
                                    ? cell.owColor
                                    : SerialStudioHelpers.getDatasetAccentColor()

        //
        // Transmit error indicator: flash a red border + tooltip when the
        // control's transmit script times out, fails, or overflows the payload
        //
        property bool txError: false
        property string txErrorText: ""

        Connections {
          target: cell.owModel
          ignoreUnknownSignals: true
          function onTransmitError(error) {
            cell.txError = true
            cell.txErrorText = error
            txErrorTimer.restart()
          }
        }

        Timer {
          id: txErrorTimer

          interval: 4000
          onTriggered: cell.txError = false
        }

        ToolTip.text: cell.txErrorText
        ToolTip.visible: cell.txError && cellHover.hovered

        HoverHandler {
          id: cellHover
        }

        //
        // State readout: says plainly when the source has not been heard from, so an unknown
        // state can never be mistaken for a settled one.
        //
        Label {
          z: 5
          visible: cell.stateBound
          anchors.top: parent.top
          anchors.right: parent.right
          anchors.margins: 4
          font: Cpp_Misc_CommonFonts.customUiFont(0.7, false)
          color: cell.stateUnknown ? Cpp_ThemeManager.colors["alarm"]
                                   : Cpp_ThemeManager.colors["placeholder_text"]
          text: cell.stateUnknown ? qsTr("no data")
                : cell.statePending ? qsTr("waiting…")
                : qsTr("live")
        }

        //
        // One control per cell, and the SAME control the transmit-function preview builds: two
        // renderings of one widget is what let the panel and the preview drift apart.
        //
        Loader {
          anchors.fill: parent

          sourceComponent: {
            if (cell.owType === SerialStudio.OutputButton)    return buttonControl
            if (cell.owType === SerialStudio.OutputSlider)    return sliderControl
            if (cell.owType === SerialStudio.OutputToggle)    return toggleControl
            if (cell.owType === SerialStudio.OutputTextField) return textFieldControl
            if (cell.owType === SerialStudio.OutputKnob)      return knobControl
            return null
          }
        }

        Component {
          id: buttonControl

          DashboardButton {
            model: cell.owModel
            widget: cell.owData
            color: cell.accentColor
          }
        }

        Component {
          id: sliderControl

          DashboardSlider {
            model: cell.owModel
            widget: cell.owData
            color: cell.accentColor
          }
        }

        Component {
          id: toggleControl

          DashboardToggle {
            model: cell.owModel
            widget: cell.owData
            color: cell.accentColor
          }
        }

        Component {
          id: textFieldControl

          DashboardTextField {
            model: cell.owModel
            widget: cell.owData
            color: cell.accentColor
          }
        }

        Component {
          id: knobControl

          DashboardKnob {
            model: cell.owModel
            widget: cell.owData
            color: cell.accentColor
          }
        }

      }
    }
  }
}
