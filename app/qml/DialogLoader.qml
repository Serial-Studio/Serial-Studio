/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick

Loader {
  id: root

  active: false

  // WebEngineView-hosting dialogs must override this to false on Linux to avoid GPU races.
  asynchronous: true

  //
  // Pointer to the generated dialog
  //
  property var dialog: null

  //
  // Utility function to show the dialog
  //
  function activate() {
    if (!active)
      active = true

    else if (dialog) {
      dialog.raise()
      dialog.requestActivate()
    }
  }

  //
  // Utility function to hide the dialog
  //
  function hide() {
    if (active && dialog)
      dialog.hide()
  }

  //
  // Utility function to close the dialog
  //
  function close() {
    if (active && dialog) {
      dialog.close()
      dialog = null
      active = false
    }
  }

  // Prefer SmartWindow.displayWindow() so authoring windows restore geometry on reopen.
  onLoaded: {
    root.dialog = item
    dialog.onClosing.connect(function() {
      root.active = false;
    })

    Qt.callLater(function() {
      if (typeof dialog.displayWindow === "function")
        dialog.displayWindow()
      else
        dialog.show()
    })
  }
}
