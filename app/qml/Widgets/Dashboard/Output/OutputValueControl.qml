/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick

Item {
  id: root

  required property color color
  required property var model
  required property var widget

  property string idPrefix: ""

  readonly property real from: root.widget ? (root.widget.minValue || 0) : 0
  readonly property real to: root.widget ? (root.widget.maxValue || 100) : 100
  readonly property real step: root.widget ? (root.widget.stepSize || 1) : 1
  readonly property string title: root.widget ? (root.widget.title || "") : ""
  readonly property bool unknown: root.model
                                  ? (root.model.stateBound && !root.model.stateKnown)
                                  : false

  //
  // Restorable after a drag clears the binding, so feedback can move the control again
  //
  readonly property real desiredValue: root.model
                                       && root.model.currentValue !== undefined
                                       ? root.model.currentValue
                                       : root.from

  readonly property string valueText: root.model
                                      ? (root.model.formatted(root.desiredValue)
                                         + (root.model.units ? " " + root.model.units : ""))
                                      : ""

  //
  // Moves the held value by whole Steps through the model, which clamps to Min and Max. Writing
  // the model rather than control.value keeps the control's binding to desiredValue alive.
  //
  function nudge(steps) {
    if (root.model)
      root.model.currentValue = root.model.stepped(root.model.currentValue, steps)
  }

  function keySteps(event) {
    const factor = (event.modifiers & Qt.ShiftModifier) ? 10 : 1
    if (event.key === Qt.Key_Up || event.key === Qt.Key_Right)
      return factor

    if (event.key === Qt.Key_Down || event.key === Qt.Key_Left)
      return -factor

    return 0
  }

  function handleKey(event) {
    const steps = root.keySteps(event)
    if (steps === 0)
      return

    root.nudge(steps)
    event.accepted = true
  }

  function commitMove(value) {
    if (root.model)
      root.model.currentValue = value
  }

  //
  // A drag holds state feedback off; the release re-binds the control to desiredValue, which the
  // drag's own writes cleared.
  //
  function trackPress(control) {
    if (!root.model)
      return

    if (control.pressed) {
      root.model.beginInteraction()
      return
    }

    root.model.endInteraction()
    control.value = Qt.binding(function() { return root.desiredValue })
  }
}
