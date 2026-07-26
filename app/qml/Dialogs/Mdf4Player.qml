/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick

PlayerDialog {
  player: Cpp_MDF4_Player
  title: qsTr("MDF4 Player")
  busy: Cpp_MDF4_Player.indexing
  indexing: Cpp_MDF4_Player.indexing
  indexProgress: Cpp_MDF4_Player.indexProgress
}
