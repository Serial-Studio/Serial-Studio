/*
 * Serial Studio - https://serial-studio.github.io/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-only
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
