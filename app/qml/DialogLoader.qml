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
  asynchronous: true

  property var dialog: null

  function activate() {
    if (!active)
      active = true

    else if (dialog) {
      dialog.raise()
      dialog.requestActivate()
    }
  }

  onLoaded: {
    root.dialog = item
    dialog.show()
    dialog.onClosing.connect(function() {
      root.active = false;
    })
  }
}
