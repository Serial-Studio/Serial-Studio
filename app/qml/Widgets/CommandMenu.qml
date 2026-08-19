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
import QtQuick.Controls

import SerialStudio

//
// Layout-driven context menu: entries from a registry menu surface, behavior from a
// CommandModel. Built on open, destroyed on close, so nothing outlives a model rebuild.
//
Menu {
  id: root

  //
  // The CommandModel providing behavior joins, and the handlers that fill "dynamic" nodes
  // (role -> function(menu), each appending its own items to the menu it is given).
  //
  required property var model
  property var dynamicHandlers: ({})

  //
  // Objects created for the current surface, in menu order: {type, obj, owner}, where owner is
  // the menu they were added to.
  //
  property var builtNodes: []

  //
  // Builds a surface and opens the menu at the cursor.
  //
  function openSurface(surface) {
    root.rebuild(surface)
    root.popup()
  }

  //
  // Destroys every object built for the previous surface, then materializes the new one.
  //
  function rebuild(surface) {
    root.clearBuilt()
    const tree = Cpp_UI_CommandRegistry.layout(surface)
    const nodes = tree.items !== undefined ? tree.items : []
    root.builtNodes = root.buildInto(root, nodes)
  }

  //
  // Removes and destroys the objects of the previous build; safe to call when empty.
  //
  function clearBuilt() {
    const built = root.builtNodes
    for (let i = built.length - 1; i >= 0; --i) {
      const node = built[i]
      if (!node.obj)
        continue

      const owner = node.owner !== undefined ? node.owner : root
      if (node.type === "menu")
        owner.removeMenu(node.obj)
      else
        owner.removeItem(node.obj)

      node.obj.destroy()
    }

    root.builtNodes = []
  }

  //
  // Materializes `nodes` into `menu` in order; separators survive only between two visible
  // neighbours, so no menu opens with a leading, trailing or doubled divider.
  //
  function buildInto(menu, nodes) {
    let built = []
    let pendingSeparator = false
    for (let i = 0; i < nodes.length; ++i) {
      const node = nodes[i]
      if (node.type === "separator") {
        pendingSeparator = built.length > 0
        continue
      }

      const created = root.createNode(menu, node)
      if (created === null)
        continue

      if (pendingSeparator) {
        const separator = _separatorComponent.createObject(menu)
        menu.insertItem(menu.count, separator)
        built.push({ "type": "separator", "obj": separator, "owner": menu })
        pendingSeparator = false
      }

      built = built.concat(created)
    }

    return built
  }

  //
  // Materializes one non-separator node; returns null when it resolves to nothing.
  //
  function createNode(menu, node) {
    if (node.type === "command")
      return root.createCommand(menu, node)

    if (node.type === "submenu")
      return root.createSubMenu(menu, node)

    if (node.type === "dynamic")
      return root.createDynamic(menu, node)

    return null
  }

  //
  // Creates one command entry, unless no binding set provides it or the binding hides it.
  //
  function createCommand(menu, node) {
    const behavior = root.model.binding(node.id)
    if (behavior === null || behavior.visible === false)
      return null

    const checked = node.kind === "toggle" && behavior.checked === true
    const iconRef = (checked && node.iconChecked !== undefined && node.iconChecked.length > 0)
                  ? node.iconChecked : node.icon
    const titleChecked = node.titleChecked !== undefined ? node.titleChecked : ""
    let label = (checked && titleChecked.length > 0) ? titleChecked : node.title
    if (behavior.title !== undefined && behavior.title.length > 0)
      label = behavior.title

    const item = _itemComponent.createObject(menu, {
                                               "text": label,
                                               "behavior": behavior,
                                               "checkable": node.kind === "toggle",
                                               "checked": checked,
                                               "enabled": behavior.enabled !== false,
                                               "iconRef": iconRef !== undefined ? iconRef : ""
                                             })
    menu.addItem(item)
    return [{ "type": "item", "obj": item, "owner": menu }]
  }

  //
  // Builds a cascade, dropping it when every child resolved invisible.
  //
  function createSubMenu(menu, node) {
    const items = node.items !== undefined ? node.items : []
    const sub = _menuComponent.createObject(menu, {
                                              "title": node.title,
                                              "enabled": node.proGated === true
                                                         ? Cpp_CommercialBuild : true,
                                              "icon.source": root.iconFor(node.icon)
                                            })
    const children = root.buildInto(sub, items)
    if (children.length === 0) {
      sub.destroy()
      return null
    }

    sub.ownedNodes = children
    menu.addMenu(sub)
    return [{ "type": "menu", "obj": sub, "owner": menu }]
  }

  //
  // Hands the menu to the host handler registered for this role; the handler appends its own
  // items and returns the objects it created, so they are destroyed with the rest.
  //
  function createDynamic(menu, node) {
    const handler = root.dynamicHandlers[node.role]
    if (handler === undefined || handler === null)
      return null

    const created = handler(menu)
    if (created === undefined || created === null || created.length === 0)
      return null

    return created.map(entry => ({ "type": entry.type, "obj": entry.obj,
                                   "owner": entry.owner !== undefined ? entry.owner : menu }))
  }

  //
  // Resolves a registry icon id at menu size; an empty id stays empty.
  //
  function iconFor(reference) {
    if (reference === undefined || reference === null || reference.length === 0)
      return ""

    return Cpp_Misc_IconRegistry.iconById(reference, 16)
  }

  //
  // Nothing built survives a close: the ids captured at open go stale as soon as a command
  // rebuilds the model that produced them.
  //
  onClosed: root.clearBuilt()

  //
  // One registry command as a menu entry.
  //
  Component {
    id: _itemComponent

    MenuItem {
      id: _item

      property var behavior: null
      property string iconRef: ""

      icon.width: 16
      icon.height: 16
      icon.source: root.iconFor(iconRef)
      onTriggered: {
        if (_item.behavior !== null)
          _item.behavior.run()
      }
    }
  }

  //
  // One cascading submenu; `ownedNodes` keeps its children alive for the destroy pass.
  //
  Component {
    id: _menuComponent

    Menu {
      property var ownedNodes: []

      icon.width: 16
      icon.height: 16
    }
  }

  //
  // Concern-group divider.
  //
  Component {
    id: _separatorComponent

    MenuSeparator {}
  }
}
