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

  //
  // Dialogs that host a WebEngineView (ExtensionManager, ExamplesBrowser,
  // HelpCenter) must be loaded synchronously on Linux. When loaded
  // asynchronously the QML scene is assembled on a worker thread while the
  // WebEngineView's GPU process handshake races with the Window creation,
  // which crashes on some Linux configurations. Other dialogs can keep
  // asynchronous loading by leaving this at its default value.
  //
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

  //
  // SmartWindow exposes displayWindow() which restores the previous geometry
  // and raises the window; SmartDialog only has the inherited show(). Prefer
  // displayWindow() when present so authoring windows reopen on the same
  // screen with the correct size after each unload.
  //
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
