/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import SerialStudio

import "../"
import "../../Dialogs" as Dialogs

Item {
  id: root

  clip: true

  //
  // Widget data inputs
  //
  required property color color
  required property var windowRoot
  required property string widgetId
  required property AccelerometerModel model

  //
  // Local properties bound to model values (EMA-filtered in C++)
  //
  property real currentG: root.model.g
  property real currentRoll: root.model.roll
  property real displayMaxG: root.model.maxG
  property bool inputInG: root.model.inputInG
  property real currentPitch: root.model.pitch
  property real currentTheta: root.model.theta
  property real currentMagnitude: root.model.magnitude

  //
  // Peak-hold: highest G seen since load or double-click on the MAX G readout
  //
  property real peakG: 0
  onCurrentGChanged: {
    if (root.currentG > root.peakG)
      root.peakG = root.currentG
  }

  //
  // Size-aware font scale: shrinks widget text as the window gets small
  //
  readonly property real uiScale: Cpp_Misc_CommonFonts.autoScale(Math.min(width, height), 260)

  //
  // Suppresses the change auto-save while restore assigns persisted values
  //
  property bool restoringSettings: false
  readonly property bool angleLabelsVisible: {
    const minGaugeSize = 200
    const labelMargin = 40
    const noLabelMargin = 12
    const containerMargins = 16
    const readoutStripHeight = 42
    const toolbarHeight = 48

    const polarAreaWidth = root.width - containerMargins
    const polarAreaHeight = root.height - containerMargins - readoutStripHeight -
                            (root.hasToolbar ? toolbarHeight : 0)

    const widthWithLabels = polarAreaWidth - labelMargin * 2
    const heightWithLabels = polarAreaHeight - labelMargin * 2
    const gaugeSizeWithLabels = Math.min(widthWithLabels, heightWithLabels)

    const widthWithoutLabels = polarAreaWidth - noLabelMargin * 2
    const heightWithoutLabels = polarAreaHeight - noLabelMargin * 2
    const gaugeSizeWithoutLabels = Math.min(widthWithoutLabels, heightWithoutLabels)

    if (gaugeSizeWithLabels < minGaugeSize)
      return false

    if (polarAreaWidth < labelMargin * 2 + minGaugeSize ||
        polarAreaHeight < labelMargin * 2 + minGaugeSize)
      return false

    const aspectRatioOk = (widthWithLabels / heightWithLabels) >= 0.75 &&
                          (heightWithLabels / widthWithLabels) >= 0.75

    const shrinkageAcceptable = gaugeSizeWithLabels >= (gaugeSizeWithoutLabels * 0.8)

    return aspectRatioOk && shrinkageAcceptable && root.height >= 360 && root.width >= 320
  }

  //
  // Window flags
  //
  readonly property bool hasToolbar: toolbar.shown

  //
  // Sync configurable properties to model and persist changes
  //
  onDisplayMaxGChanged: {
    if (root.model)
      root.model.maxG = root.displayMaxG

    if (!root.restoringSettings)
      Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "displayMaxG", root.displayMaxG)
  }
  onInputInGChanged: {
    if (root.model)
      root.model.inputInG = root.inputInG

    if (!root.restoringSettings)
      Cpp_JSON_ProjectModel.saveWidgetSetting(widgetId, "inputInG", root.inputInG)
  }

  //
  // Utility function to normalize angles from 0 to 360
  //
  function normalize360(angle) {
    var normalized = angle % 360;
    if (normalized < 0)
      normalized += 360;

    return normalized;
  }


  //
  // Restore persisted settings
  //
  Component.onCompleted: {
    root.restoringSettings = true

    const s = Cpp_JSON_ProjectModel.widgetSettings(widgetId)

    if (s["displayMaxG"] !== undefined)
      root.displayMaxG = s["displayMaxG"]

    if (s["inputInG"] !== undefined)
      root.inputInG = s["inputInG"]

    root.restoringSettings = false
  }

  //
  // Config dialog
  //
  Dialogs.AccelerometerConfigDialog {
    id: configDialog
  }

  //
  // Toolbar
  //
  WidgetToolbar {
    id: toolbar

    minWidgetHeight: 296
    windowRoot: root.windowRoot

    anchors {
      leftMargin: 8
      top: parent.top
      left: parent.left
      right: parent.right
    }

    DashboardToolButton {
      text: qsTr("Settings")
      ToolTip.text: qsTr("Settings")
      onClicked: configDialog.openDialog(root.model)
      icon.source: Cpp_Misc_IconRegistry.icon("commands", "settings", 32)
    }

    Item {
      Layout.fillWidth: true
    }
  }

  //
  // Main container: polar plot on top, readout strip at the bottom
  //
  Item {
    id: container

    anchors {
      topMargin: 8
      leftMargin: 8
      rightMargin: 8
      bottomMargin: 8
      left: parent.left
      top: toolbar.bottom
      right: parent.right
      bottom: parent.bottom
    }

    //
    // Polar plot area
    //
    Item {
      id: polarArea

      property real margin: root.angleLabelsVisible ? 40 : 12

      anchors {
        bottomMargin: 4
        top: parent.top
        left: parent.left
        right: parent.right
        bottom: readouts.top
      }

      //
      // The gauge is always square, centered in the polar area
      //
      readonly property real gaugeSize: Math.min(width - margin * 2, height - margin * 2)

      //
      // Dark instrument face: fixed palette (dark dial, light grid, red
      // indicator), deliberately theme-blind like the Gyroscope horizon
      //

      // code-verify off
      Rectangle {
        id: polarCircle

        border.width: 1
        color: "#21262f"
        radius: width / 2
        border.color: "#3d4451"
        anchors.centerIn: parent
        width: polarArea.gaugeSize
        height: polarArea.gaugeSize
      }

      //
      // Polar plot contour rings
      //
      Item {
        id: rings

        anchors.centerIn: parent
        width: polarArea.gaugeSize
        height: polarArea.gaugeSize

        //
        // Radial dotted lines for each 30 deg angle
        //
        Canvas {
          id: radialLines

          opacity: 0.15
          anchors.fill: parent

          onPaint: {
            var ctx = getContext("2d");
            ctx.clearRect(0, 0, width, height);
            ctx.strokeStyle = "#e8e8e8";
            ctx.lineWidth = 1;
            ctx.setLineDash([4, 4]);

            var centerX = width / 2;
            var centerY = height / 2;
            var radius = Math.min(width, height) / 2;

            for (var i = 0; i < 12; i++) {
              var angle = (i * 30) * Math.PI / 180;
              var x = centerX + radius * Math.cos(angle);
              var y = centerY - radius * Math.sin(angle);

              ctx.beginPath();
              ctx.moveTo(centerX, centerY);
              ctx.lineTo(x, y);
              ctx.stroke();
            }
          }

          Component.onCompleted: requestPaint()
        }

        //
        // Concentric grid circles (drawn over radial lines)
        //
        Repeater {
          model: 4
          delegate: Rectangle {
            required property int index

            readonly property real fraction: 1.0 - (index * 0.25)

            z: 1
            opacity: 0.4
            border.width: 1
            radius: width / 2
            color: "transparent"
            border.color: "#e8e8e8"
            anchors.centerIn: parent
            width: rings.width * fraction
            height: rings.height * fraction
          }
        }

        //
        // Ring scale labels: magnitude at each grid circle, along the upper axis
        //
        Repeater {
          model: 4

          delegate: Text {
            required property int index

            readonly property real fraction: 1.0 - (index * 0.25)
            readonly property real ringValue: root.displayMaxG * fraction

            z: 2
            opacity: 0.55
            color: "#e8e8e8"
            x: rings.width / 2 + 4
            y: rings.height / 2 - rings.height / 2 * fraction + 2
            font: (Cpp_Misc_CommonFonts.widgetFontRevision,
                   Cpp_Misc_CommonFonts.widgetFont(0.55 * root.uiScale))
            text: (ringValue % 1 === 0 ? ringValue.toFixed(0) : ringValue.toFixed(1)) + "G"
          }
        }

        //
        // Center dot
        //
        Rectangle {
          z: 2
          width: 4
          height: 4
          opacity: 0.5
          color: "#e8e8e8"
          radius: width / 2
          anchors.centerIn: parent
        }

        //
        // Crosshair lines (main axes)
        //
        Rectangle {
          z: 1
          width: 1
          opacity: 0.3
          color: "#e8e8e8"

          anchors {
            top: parent.top
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
          }
        }

        Rectangle {
          z: 1
          height: 1
          opacity: 0.3
          color: "#e8e8e8"

          anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
          }
        }


        //
        // Angle labels around the perimeter (every 30 degrees)
        //
        Repeater {
          model: 12

          delegate: Text {
            required property int index

            readonly property real angle: index * 30
            readonly property real radius: rings.width / 2 + 18
            readonly property real angleRad: angle * Math.PI / 180

            visible: root.angleLabelsVisible
            x: rings.width / 2 + radius * Math.cos(angleRad) - width / 2
            y: rings.height / 2 - radius * Math.sin(angleRad) - height / 2

            opacity: 0.6
            text: angle + "°"
            color: Cpp_ThemeManager.colors["widget_text"]
            font: (Cpp_Misc_CommonFonts.widgetFontRevision,
                   Cpp_Misc_CommonFonts.widgetFont(0.6 * root.uiScale))
          }
        }
      }

      //
      // Polar position indicator (clamped to circle boundary)
      //
      Rectangle {
        id: indicator

        color: "#ff5252"
        radius: width / 2

        readonly property real dotSize: Math.max(
                                          6, Math.min(14, polarArea.gaugeSize / 20))

        width: dotSize
        height: dotSize

        property real halfGauge: polarArea.gaugeSize / 2
        property real normalizedMag: Math.min(
                                       root.currentMagnitude / root.displayMaxG, 1.0)

        x: polarArea.width / 2
           + normalizedMag * halfGauge
           * Math.cos(root.currentTheta * Math.PI / 180)
           - width / 2
        y: polarArea.height / 2
           - normalizedMag * halfGauge
           * Math.sin(root.currentTheta * Math.PI / 180)
           - height / 2

      }

      //
      // Mouse area for cursor tracking
      //
      MouseArea {
        id: cursorTracker

        hoverEnabled: true
        anchors.fill: rings
        acceptedButtons: Qt.NoButton
        propagateComposedEvents: true

        property real cursorAngle: 0
        property real cursorMagnitude: 0
        property bool isInsideCircle: false

        onPositionChanged: (mouse) => {
                             var centerX = rings.width / 2
                             var centerY = rings.height / 2
                             var dx = mouse.x - centerX
                             var dy = centerY - mouse.y

                             var distanceFromCenter = Math.sqrt(dx * dx + dy * dy)
                             var maxRadius = rings.width / 2

                             isInsideCircle = distanceFromCenter <= maxRadius

                             if (isInsideCircle) {
                               cursorMagnitude = (distanceFromCenter / maxRadius) * root.displayMaxG
                               cursorAngle = root.normalize360(Math.atan2(dy, dx) * 180 / Math.PI)
                             }
                           }

        //
        // Cursor crosshair (vertical arms)
        //
        Rectangle {
          width: 1
          height: 12
          color: "#ff5252"
          x: cursorTracker.mouseX - width / 2
          y: cursorTracker.mouseY - height - 4
          opacity: cursorTracker.containsMouse && cursorTracker.isInsideCircle ? 0.6 : 0
        }

        Rectangle {
          width: 1
          height: 12
          color: "#ff5252"
          y: cursorTracker.mouseY + 4
          x: cursorTracker.mouseX - width / 2
          opacity: cursorTracker.containsMouse && cursorTracker.isInsideCircle ? 0.6 : 0
        }

        //
        // Cursor crosshair (horizontal arms)
        //
        Rectangle {
          height: 1
          width: 12
          color: "#ff5252"
          x: cursorTracker.mouseX - width - 4
          y: cursorTracker.mouseY - height / 2
          opacity: cursorTracker.containsMouse && cursorTracker.isInsideCircle ? 0.6 : 0
        }

        Rectangle {
          height: 1
          width: 12
          color: "#ff5252"
          x: cursorTracker.mouseX + 4
          y: cursorTracker.mouseY - height / 2
          opacity: cursorTracker.containsMouse && cursorTracker.isInsideCircle ? 0.6 : 0
        }

        //
        // Cursor value label (positioned below and right of cursor to stay visible)
        //
        Rectangle {
          radius: 3
          border.width: 1
          width: valueLabel.width + 8
          height: valueLabel.height + 4
          color: Cpp_ThemeManager.colors["tooltip_base"]
          border.color: Cpp_ThemeManager.colors["tooltip_text"]
          x: Math.min(cursorTracker.mouseX + 16, rings.width - width - 4)
          visible: cursorTracker.containsMouse && cursorTracker.isInsideCircle
          y: Math.max(4, Math.min(cursorTracker.mouseY + 16, rings.height - height - 4))

          Label {
            id: valueLabel

            elide: Text.ElideRight
            anchors.centerIn: parent
            color: Cpp_ThemeManager.colors["tooltip_text"]
            text: cursorTracker.cursorMagnitude.toFixed(2) + "G @ " +
                  cursorTracker.cursorAngle.toFixed(0) + "°"
            font: (Cpp_Misc_CommonFonts.widgetFontRevision,
                   Cpp_Misc_CommonFonts.widgetFont(0.7 * root.uiScale))
          }
        }
      }
    }
    // code-verify on

    //
    // Readout strip: G-force, pitch and roll
    //
    Item {
      id: readouts

      visible: container.height >= 120
      height: visible ? readoutRow.implicitHeight : 0

      anchors {
        left: parent.left
        right: parent.right
        bottom: parent.bottom
      }

      RowLayout {
        id: readoutRow

        spacing: 4
        anchors.fill: parent

        Rectangle {
          border.width: 1
          Layout.fillWidth: true
          implicitHeight: gForceColumn.implicitHeight + 10
          color: Cpp_ThemeManager.colors["widget_base"]
          border.color: root.currentG > (root.displayMaxG * 0.75)
                        ? Cpp_ThemeManager.colors["alarm"]
                        : Cpp_ThemeManager.colors["widget_border"]

          Column {
            id: gForceColumn

            spacing: 1
            width: parent.width - 8
            anchors.centerIn: parent

            Label {
              opacity: 0.6
              width: parent.width
              text: qsTr("G-FORCE")
              elide: Text.ElideRight
              horizontalAlignment: Text.AlignHCenter
              color: Cpp_ThemeManager.colors["widget_text"]
              font: (Cpp_Misc_CommonFonts.widgetFontRevision,
                     Cpp_Misc_CommonFonts.widgetFont(0.66 * root.uiScale))
            }

            Label {
              width: parent.width
              elide: Text.ElideRight
              horizontalAlignment: Text.AlignHCenter
              color: Cpp_ThemeManager.colors["widget_text"]
              font: (Cpp_Misc_CommonFonts.widgetFontRevision,
                     Cpp_Misc_CommonFonts.widgetFont(root.uiScale))

              // code-verify off
              text: (root.currentG.toFixed(2) + "").padStart(5, ' ') + " @ " +
                    (root.normalize360(root.currentTheta).toFixed(0) + "").padStart(3, ' ') + "°"
              // code-verify on
            }
          }
        }

        Rectangle {
          border.width: 1
          Layout.fillWidth: true
          ToolTip.delay: 500
          ToolTip.visible: peakResetArea.containsMouse
          ToolTip.text: qsTr("Double-click to reset")
          implicitHeight: maxGColumn.implicitHeight + 10
          color: Cpp_ThemeManager.colors["widget_base"]
          border.color: root.peakG > (root.displayMaxG * 0.75)
                        ? Cpp_ThemeManager.colors["alarm"]
                        : Cpp_ThemeManager.colors["widget_border"]

          MouseArea {
            id: peakResetArea

            hoverEnabled: true
            anchors.fill: parent
            onDoubleClicked: root.peakG = root.currentG
          }

          Column {
            id: maxGColumn

            spacing: 1
            width: parent.width - 8
            anchors.centerIn: parent

            Label {
              opacity: 0.6
              width: parent.width
              text: qsTr("MAX G")
              elide: Text.ElideRight
              horizontalAlignment: Text.AlignHCenter
              color: Cpp_ThemeManager.colors["widget_text"]
              font: (Cpp_Misc_CommonFonts.widgetFontRevision,
                     Cpp_Misc_CommonFonts.widgetFont(0.66 * root.uiScale))
            }

            Label {
              width: parent.width
              elide: Text.ElideRight
              horizontalAlignment: Text.AlignHCenter
              color: Cpp_ThemeManager.colors["widget_text"]
              font: (Cpp_Misc_CommonFonts.widgetFontRevision,
                     Cpp_Misc_CommonFonts.widgetFont(root.uiScale))
              text: (root.peakG.toFixed(2) + "").padStart(5, ' ') + "G"
            }
          }
        }

        Rectangle {
          border.width: 1
          Layout.fillWidth: true
          implicitHeight: rollColumn.implicitHeight + 10
          color: Cpp_ThemeManager.colors["widget_base"]
          border.color: Cpp_ThemeManager.colors["widget_border"]

          Column {
            id: rollColumn

            spacing: 1
            width: parent.width - 8
            anchors.centerIn: parent

            Label {
              opacity: 0.6
              width: parent.width
              text: qsTr("ROLL ↔")
              elide: Text.ElideRight
              horizontalAlignment: Text.AlignHCenter
              color: Cpp_ThemeManager.colors["widget_text"]
              font: (Cpp_Misc_CommonFonts.widgetFontRevision,
                     Cpp_Misc_CommonFonts.widgetFont(0.66 * root.uiScale))
            }

            Label {
              width: parent.width
              elide: Text.ElideRight
              horizontalAlignment: Text.AlignHCenter
              color: Cpp_ThemeManager.colors["widget_text"]
              font: (Cpp_Misc_CommonFonts.widgetFontRevision,
                     Cpp_Misc_CommonFonts.widgetFont(root.uiScale))

              // code-verify off
              text: (root.currentRoll.toFixed(2) + "").padStart(7, ' ') + "°"
              // code-verify on
            }
          }
        }

Rectangle {
          border.width: 1
          Layout.fillWidth: true
          implicitHeight: pitchColumn.implicitHeight + 10
          color: Cpp_ThemeManager.colors["widget_base"]
          border.color: Cpp_ThemeManager.colors["widget_border"]

          Column {
            id: pitchColumn

            spacing: 1
            width: parent.width - 8
            anchors.centerIn: parent

            Label {
              opacity: 0.6
              width: parent.width
              text: qsTr("PITCH ↕")
              elide: Text.ElideRight
              horizontalAlignment: Text.AlignHCenter
              color: Cpp_ThemeManager.colors["widget_text"]
              font: (Cpp_Misc_CommonFonts.widgetFontRevision,
                     Cpp_Misc_CommonFonts.widgetFont(0.66 * root.uiScale))
            }

            Label {
              width: parent.width
              elide: Text.ElideRight
              horizontalAlignment: Text.AlignHCenter
              color: Cpp_ThemeManager.colors["widget_text"]
              font: (Cpp_Misc_CommonFonts.widgetFontRevision,
                     Cpp_Misc_CommonFonts.widgetFont(root.uiScale))

              // code-verify off
              text: (root.currentPitch.toFixed(2) + "").padStart(7, ' ') + "°"
              // code-verify on
            }
          }
        }
      }
    }
  }
}
