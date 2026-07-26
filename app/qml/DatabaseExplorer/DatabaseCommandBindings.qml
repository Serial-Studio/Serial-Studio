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
  // Report-options dialog opened by the Export PDF command.
  //
  property var reportDialog: null

  //
  // Session-database command behavior, keyed by manifest id (Sessions is a Pro-only
  // surface, so this file is loaded solely inside the commercial Database Explorer).
  //
  readonly property var map: ({
    "db.open": root.cmdDbOpen,
    "db.close": root.cmdDbClose,
    "db.replay": root.cmdDbReplay,
    "db.delete": root.cmdDbDelete,
    "db.lock": root.cmdDbLock,
    "db.exportCsv": root.cmdDbExportCsv,
    "db.exportPdf": root.cmdDbExportPdf,
    "db.restoreProject": root.cmdDbRestoreProject
  })

  readonly property QtObject cmdDbOpen: QtObject {
    function run() { Cpp_Sessions_Manager.openDatabase() }
  }

  readonly property QtObject cmdDbClose: QtObject {
    readonly property bool enabled: Cpp_Sessions_Manager.isOpen
    function run() { Cpp_Sessions_Manager.closeDatabase() }
  }

  readonly property QtObject cmdDbReplay: QtObject {
    readonly property bool enabled: Cpp_Sessions_Manager.isOpen
                                    && Cpp_Sessions_Manager.selectedSessionId >= 0
    function run() { Cpp_Sessions_Manager.replaySelectedSession() }
  }

  readonly property QtObject cmdDbDelete: QtObject {
    readonly property bool enabled: Cpp_Sessions_Manager.isOpen
                                    && Cpp_Sessions_Manager.selectedSessionId >= 0
                                    && !Cpp_Sessions_Manager.locked
    readonly property string tooltip: Cpp_Sessions_Manager.locked
                                      ? qsTr("Unlock the session file to delete sessions")
                                      : qsTr("Delete the selected session")
    function run() {
      Cpp_Sessions_Manager.confirmDeleteSession(Cpp_Sessions_Manager.selectedSessionId)
    }
  }

  readonly property QtObject cmdDbLock: QtObject {
    readonly property bool checked: Cpp_Sessions_Manager.locked
    readonly property bool enabled: Cpp_Sessions_Manager.isOpen
    readonly property string tooltip: Cpp_Sessions_Manager.locked
                                      ? qsTr("Unlock the session file to allow deletions")
                                      : qsTr("Set a password to prevent session deletions")
    function run() {
      if (Cpp_Sessions_Manager.locked)
        Cpp_Sessions_Manager.unlockDatabase()
      else
        Cpp_Sessions_Manager.lockDatabase()
    }
  }

  readonly property QtObject cmdDbExportCsv: QtObject {
    readonly property bool enabled: Cpp_Sessions_Manager.isOpen
                                    && Cpp_Sessions_Manager.selectedSessionId >= 0
                                    && !Cpp_Sessions_Manager.csvExportBusy
    function run() {
      Cpp_Sessions_Manager.exportSessionToCsv(Cpp_Sessions_Manager.selectedSessionId)
    }
  }

  readonly property QtObject cmdDbExportPdf: QtObject {
    readonly property bool enabled: Cpp_Sessions_Manager.isOpen
                                    && Cpp_Sessions_Manager.selectedSessionId >= 0
                                    && !Cpp_Sessions_Manager.pdfExportBusy
    function run() {
      if (root.reportDialog)
        root.reportDialog.openFor(Cpp_Sessions_Manager.selectedSessionId)
    }
  }

  readonly property QtObject cmdDbRestoreProject: QtObject {
    readonly property bool enabled: Cpp_Sessions_Manager.isOpen
                                    && Cpp_Sessions_Manager.selectedSessionId >= 0
    function run() { Cpp_Sessions_Manager.restoreProjectFromDb() }
  }
}
