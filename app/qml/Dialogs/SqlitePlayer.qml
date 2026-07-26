/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtQuick

PlayerDialog {
  loadingUi: true
  player: Cpp_Sessions_Player
  title: qsTr("Session Player")
  busy: Cpp_Sessions_Player.loading
  loadingText: qsTr("Loading session…")
}
