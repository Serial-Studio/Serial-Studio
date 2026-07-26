/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

import "." as Widgets

Item {
  id: root

  //
  // The CommandModel providing behavior joins and the layout manifest surface to render.
  //
  required property var model
  required property string surface

  //
  // Layout tree re-pulled when the registry retranslates (revision dependency).
  //
  readonly property var tree: {
    void root.model.revision
    return Cpp_UI_CommandRegistry.layout(root.surface)
  }

  implicitWidth: _ribbon.implicitWidth
  implicitHeight: _ribbon.implicitHeight

  //
  // Renders one command entry as a toolbar button with live behavior bindings.
  //
  component CommandButton: Widgets.ToolbarButton {
    id: _button

    required property var node
    readonly property var behavior: root.model.binding(node.id)
    readonly property var hints: node.hints !== undefined ? node.hints : ({})

    readonly property bool isChecked: node.kind === "toggle" && behavior !== null
                                      && behavior.checked === true
    readonly property string iconRef: (isChecked && node.iconChecked !== undefined
                                       && node.iconChecked.length > 0) ? node.iconChecked
                                                                       : node.icon

    checked: isChecked
    horizontalLayout: hints.horizontalLayout === true
    visible: behavior !== null && behavior.visible !== false
    enabled: behavior !== null && behavior.enabled !== false
    iconSize: hints.iconSize !== undefined ? hints.iconSize : 32
    text: (isChecked && node.titleChecked !== undefined && node.titleChecked.length > 0)
            ? node.titleChecked : node.title
    icon.source: iconRef.length > 0 ? Cpp_Misc_IconRegistry.iconById(iconRef, iconSize) : ""
    onClicked: {
      if (behavior !== null)
        behavior.run()
    }

    ToolTip.text: (behavior !== null && behavior.tooltip !== undefined && behavior.tooltip)
                    ? behavior.tooltip : (node.tooltip !== undefined ? node.tooltip : "")
  }

  Widgets.RibbonToolbar {
    id: _ribbon

    anchors.fill: parent

    Repeater {
      model: root.tree.sections !== undefined ? root.tree.sections : []

      delegate: Widgets.RibbonSection {
        id: _section

        required property var modelData

        collapsible: modelData.collapsiblePro === true ? Cpp_CommercialBuild
                                                       : modelData.collapsible === true
        collapsePriority: modelData.collapsePriority !== undefined ? modelData.collapsePriority
                                                                   : 0
        showSeparator: modelData.showSeparator !== false
        collapsedText: modelData.collapsedTitle !== undefined ? modelData.collapsedTitle : ""
        collapsedIcon: modelData.collapsedIcon !== undefined && modelData.collapsedIcon.length > 0
                         ? Cpp_Misc_IconRegistry.iconById(modelData.collapsedIcon, 32)
                         : ""

        Repeater {
          model: _section.modelData.items

          delegate: Loader {
            id: _item

            required property var modelData

            Layout.alignment: Qt.AlignVCenter
            Layout.fillHeight: modelData.type === "grid"
            sourceComponent: modelData.type === "command"
                               ? _commandComponent
                               : (modelData.type === "grid" ? _gridComponent : null)

            Component {
              id: _commandComponent

              CommandButton {
                node: _item.modelData
              }
            }

            Component {
              id: _gridComponent

              GridLayout {
                readonly property bool driverGrid: _item.modelData.role === "drivers"
                readonly property int gridRows: _item.modelData.rows !== undefined
                                                ? _item.modelData.rows : 3

                rows: gridRows
                rowSpacing: 0
                columnSpacing: 4
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignVCenter
                columns: driverGrid ? (Cpp_CommercialBuild ? 3 : 1)
                                    : Math.ceil(_item.modelData.items.length / gridRows)

                Repeater {
                  model: _item.modelData.items

                  delegate: CommandButton {
                    id: _gridButton

                    required property var modelData

                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignLeft
                    node: root.mergedHints(modelData, _item.modelData.itemHints)
                  }
                }
              }
            }
          }
        }
      }
    }

    Widgets.RibbonSpacer {}
  }

  //
  // Applies a grid's itemHints to nodes that carry no hints of their own.
  //
  function mergedHints(node, itemHints) {
    if (node.hints !== undefined || itemHints === undefined)
      return node

    var copy = {}
    for (var key in node)
      copy[key] = node[key]

    copy.hints = itemHints
    return copy
  }
}
