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

import SerialStudio

QtObject {
  id: root

  //
  // Palette/search context served by this model ("app", "dashboard" or "editor") and the
  // binding providers (each exposing a `map` of command id -> behavior entry).
  //
  required property string context
  property var bindingSets: []

  //
  // Bumped when the registry retranslates; consumers re-pull by referencing it.
  //
  property int revision: 0

  readonly property Connections retranslateHook: Connections {
    target: Cpp_UI_CommandRegistry
    function onCommandsChanged() {
      root.revision = root.revision + 1
    }
  }

  //
  // Resolves the behavior entry bound to a command id (null when no set binds it).
  //
  function binding(id) {
    for (var i = 0; i < root.bindingSets.length; ++i) {
      var set = root.bindingSets[i]
      if (set && set.map && set.map[id] !== undefined)
        return set.map[id]
    }

    return null
  }

  //
  // Joins registry metadata with behavior for one command id; null when unbound/unknown.
  //
  function entryFor(id) {
    var cmd = Cpp_UI_CommandRegistry.command(id)
    if (!cmd || cmd.id === undefined || cmd.id === "")
      return null

    return join(cmd, root.binding(id))
  }

  //
  // Returns visible commands matching `filter` in registry order, provider-compatible
  // (name/icon/run) plus iconId/shortcut/checked for size tiers and hints.
  //
  function items(filter) {
    var f = (filter || "").trim().toLowerCase()
    var commands = Cpp_UI_CommandRegistry.commands(root.context)
    var out = []
    for (var i = 0; i < commands.length; ++i) {
      var entry = join(commands[i], root.binding(commands[i].id))
      if (entry === null || !entry.visible)
        continue

      if (f.length > 0 && !SerialStudio.searchMatches(f, entry.name))
        continue

      out.push(entry)
    }

    return out
  }

  //
  // Merges one registry command with its behavior entry; checked toggles swap title/icon, and a
  // behavior may override the label outright for state-dependent text (bulk counts).
  //
  function join(cmd, b) {
    if (b === null || b === undefined)
      return null

    var checked = (cmd.kind === "toggle" && b.checked === true)
    var iconRef = (checked && cmd.iconChecked.length > 0) ? cmd.iconChecked : cmd.icon
    var label = (checked && cmd.titleChecked.length > 0) ? cmd.titleChecked : cmd.title
    if (b.title !== undefined && b.title.length > 0)
      label = b.title

    return {
      id: cmd.id,
      kind: cmd.kind,
      checked: checked,
      category: cmd.category,
      name: label,
      tooltip: (b.tooltip !== undefined) ? b.tooltip : cmd.tooltip,
      iconId: iconRef,
      icon: (iconRef.length > 0) ? Cpp_Misc_IconRegistry.iconById(iconRef, 32) : "",
      shortcut: cmd.shortcutText,
      visible: b.visible !== false,
      enabled: b.enabled !== false,
      binding: b,
      run: b.run
    }
  }
}
