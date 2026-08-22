/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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

//
// Resolves the icon placeholders used by doc/help so the manual never hardcodes an
// artwork path (spec 0028: icons resolve only through Misc::IconRegistry):
//
//   <img src="cmd:app.projectEditor">          command icon from the command manifests
//   <img src="cmd:io.pause:checked">           the command's iconChecked artwork
//   <img src="icon:commands/crosshair">        a registry id for non-command buttons
//
// Both forms rewrite to the qrc: URL the registry serves at the requested pixel tier.
// Unknown ids fall through to the registry's missing-icon placeholder, which also
// warns once on the console. Deliberately not a `.pragma library` so the Cpp_*
// context properties stay reachable through the importing component's context.
//

var kPlaceholderPattern = /src="(cmd|icon):([A-Za-z0-9_./:-]+)"/g

//
// Returns the registry URL for one placeholder body ("cmd" or "icon" scheme).
//
function resolveOne(scheme, body, px) {
  if (scheme === "icon")
    return Cpp_Misc_IconRegistry.iconById(body, px)

  var checked = body.endsWith(":checked")
  var id = checked ? body.slice(0, -":checked".length) : body
  var command = Cpp_UI_CommandRegistry.command(id)
  var iconId = command ? (checked ? command.iconChecked : command.icon) : ""
  if (!iconId)
    return Cpp_Misc_IconRegistry.icon("system", "missing", px)

  return Cpp_Misc_IconRegistry.iconById(iconId, px)
}

//
// Rewrites every cmd:/icon: image placeholder in a markdown document.
//
function resolve(markdown, px) {
  if (!markdown || markdown.indexOf("src=\"") < 0)
    return markdown

  return markdown.replace(kPlaceholderPattern, function(match, scheme, body) {
    return 'src="' + resolveOne(scheme, body, px || 16) + '"'
  })
}
