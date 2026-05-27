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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  title: qsTr("Recover Backup")
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  property int selectedIndex: -1
  readonly property var entries: backupList.model

  //
  // Diff state, recomputed whenever the selected snapshot changes
  //
  property var currentSummary: ({})
  property var snapshotSummary: ({})
  property var addedGroups: []
  property var removedGroups: []

  onVisibleChanged: if (visible) refresh()
  onSelectedIndexChanged: refreshDiff()

  function refresh() {
    const rows = Cpp_Misc_BackupManager.list(50)
    backupList.model = rows
    selectedIndex = rows.length > 0 ? 0 : -1
    refreshDiff()
  }

  function refreshDiff() {
    currentSummary = Cpp_Misc_BackupManager.currentSummary() || {}
    if (selectedIndex < 0 || selectedIndex >= entries.length) {
      snapshotSummary = {}
      addedGroups = []
      removedGroups = []
      return
    }

    snapshotSummary = Cpp_Misc_BackupManager.summarize(entries[selectedIndex].path) || {}
    const cur  = currentSummary.groupTitles || []
    const snap = snapshotSummary.groupTitles || []
    addedGroups   = snap.filter(t => cur.indexOf(t) < 0)
    removedGroups = cur.filter(t => snap.indexOf(t) < 0)
  }

  //
  // ISO 8601 UTC timestamp -> locale short string in the user's timezone.
  //
  function humanTimestamp(iso) {
    if (!iso || iso.length === 0)
      return ""

    const d = new Date(iso)
    if (isNaN(d.getTime()))
      return iso

    return d.toLocaleString(Qt.locale(), Locale.ShortFormat)
  }

  //
  // Title-case prose for known checkpoint tokens; unknown labels fall through to a word-by-word transform.
  //
  function humanLabel(label) {
    if (!label || label.length === 0)
      return qsTr("Untitled")

    const known = {
      "load":                                  qsTr("Project Loaded"),
      "auto":                                  qsTr("Auto-save"),
      "pre-restore":                           qsTr("Before Restore"),
      "pre-datasetDelete":                     qsTr("Before Delete Dataset"),
      "pre-groupDelete":                       qsTr("Before Delete Group"),
      "pre-project.new":                       qsTr("Before New Project"),
      "pre-project.open":                      qsTr("Before Open Project"),
      "pre-project.loadJson":                  qsTr("Before Load JSON"),
      "pre-project.template.apply":            qsTr("Before Apply Template"),
      "pre-project.dataset.delete":            qsTr("Before Delete Dataset"),
      "pre-project.group.delete":              qsTr("Before Delete Group"),
      "pre-project.action.delete":             qsTr("Before Delete Action"),
      "pre-project.outputWidget.delete":       qsTr("Before Delete Output Widget"),
      "pre-project.dataset.move":              qsTr("Before Move Dataset"),
      "pre-project.group.move":                qsTr("Before Move Group"),
      "pre-project.workspace.delete":          qsTr("Before Delete Workspace"),
      "pre-project.workspace.clearAll":        qsTr("Before Clear All Workspaces"),
      "pre-project.workspace.removeWidget":    qsTr("Before Remove Widget"),
      "pre-project.workspace.reorder":         qsTr("Before Reorder Workspaces"),
      "pre-project.batch":                     qsTr("Before Batch Operation"),
      "pre-assistant.project.bulkApply":       qsTr("Before Batch Operation"),
      "pre-assistant.workspace.addTile":       qsTr("Before Add Tile"),
      "pre-assistant.restore":                 qsTr("Before Restore"),
    }

    if (known[label] !== undefined)
      return known[label]

    return label.split(/[\s\-_.]+/)
                .filter(w => w.length > 0)
                .map(w => w.charAt(0).toUpperCase() + w.slice(1).toLowerCase())
                .join(" ")
  }

  //
  // Joins group titles into a comma list, capped at three with an "and N more" suffix.
  //
  function formatGroupList(items) {
    if (items.length === 0)
      return ""

    const quoted = items.map(x => "\"" + x + "\"")
    if (quoted.length <= 3)
      return quoted.join(", ")

    return qsTr("%1 (and %2 more)")
            .arg(quoted.slice(0, 3).join(", "))
            .arg(quoted.length - 3)
  }

  //
  // Single prose sentence summarising what restoring the selected snapshot
  // would change. Empty when there's no selection.
  //
  function previewText() {
    if (selectedIndex < 0)
      return ""

    const titleChanged = (snapshotSummary.title || "") !== (currentSummary.title || "")
    const removed      = removedGroups
    const added        = addedGroups

    if (removed.length === 0 && added.length === 0) {
      if (titleChanged)
        return qsTr("Title changes from “%1” to “%2”. Group structure unchanged.")
                .arg(currentSummary.title || qsTr("Untitled"))
                .arg(snapshotSummary.title || qsTr("Untitled"))

      return qsTr("Same groups and datasets as your current project. Restoring may still revert field-level edits.")
    }

    const rmStr  = formatGroupList(removed)
    const addStr = formatGroupList(added)

    if (removed.length > 0 && added.length > 0)
      return qsTr("Restoring removes %1 and brings back %2.").arg(rmStr).arg(addStr)

    if (removed.length > 0)
      return qsTr("Restoring removes %1.").arg(rmStr)

    return qsTr("Restoring brings back %1.").arg(addStr)
  }

  function restoreSelected() {
    if (selectedIndex < 0 || selectedIndex >= entries.length)
      return

    const path = entries[selectedIndex].path
    if (Cpp_Misc_BackupManager.restore(path))
      root.close()
  }

  dialogContent: ColumnLayout {
    id: layout

    spacing: 10

    //
    // Instruction line: sentence case, no trailing heading
    //
    Label {
      Layout.fillWidth: true
      Layout.maximumWidth: 520
      wrapMode: Text.WordWrap
      font: Cpp_Misc_CommonFonts.uiFont
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("Pick a backup to restore. The current project is saved automatically first, so the restore is reversible.")
    }

    //
    // Snapshot list with two columns: time + label
    //
    Rectangle {
      Layout.fillWidth: true
      Layout.minimumWidth: 520
      Layout.preferredHeight: 280
      radius: 6
      border.width: 1
      color: Cpp_ThemeManager.colors["groupbox_background"]
      border.color: Cpp_ThemeManager.colors["groupbox_border"]

      ListView {
        id: backupList

        anchors.fill: parent
        anchors.margins: 1
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

        Keys.onEnterPressed: root.restoreSelected()
        Keys.onReturnPressed: root.restoreSelected()

        delegate: Rectangle {
          id: row

          required property int index
          required property var modelData

          readonly property bool selected: root.selectedIndex === index
          readonly property color rowText: selected
                                           ? Cpp_ThemeManager.colors["highlighted_text"]
                                           : Cpp_ThemeManager.colors["text"]

          width: backupList.width
          height: 32
          color: selected ? Cpp_ThemeManager.colors["highlight"] : "transparent"

          MouseArea {
            anchors.fill: parent
            onClicked: { root.selectedIndex = row.index; backupList.forceActiveFocus() }
            onDoubleClicked: { root.selectedIndex = row.index; root.restoreSelected() }
          }

          RowLayout {
            spacing: 16
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12

            Label {
              color: row.rowText
              elide: Text.ElideRight
              Layout.preferredWidth: 180
              font: Cpp_Misc_CommonFonts.uiFont
              text: root.humanTimestamp(row.modelData.timestamp)
            }

            Label {
              color: row.rowText
              Layout.fillWidth: true
              elide: Text.ElideRight
              font: Cpp_Misc_CommonFonts.uiFont
              text: root.humanLabel(row.modelData.label)
            }
          }
        }
      }
    }

    //
    // Empty-state message
    //
    Label {
      Layout.fillWidth: true
      visible: backupList.count === 0
      wrapMode: Text.WordWrap
      font: Cpp_Misc_CommonFonts.uiFont
      color: Cpp_ThemeManager.colors["text"]
      text: qsTr("No backups for this project yet. Edit or save the project to start the rolling backup.")
    }

    //
    // Preview: one prose line. Reserved height keeps the dialog from jumping
    // around when the selection changes.
    //
    Label {
      Layout.fillWidth: true
      Layout.maximumWidth: 520
      Layout.minimumHeight: implicitHeight
      visible: root.selectedIndex >= 0
      wrapMode: Text.WordWrap
      font: Cpp_Misc_CommonFonts.uiFont
      color: Cpp_ThemeManager.colors["text"]
      text: root.previewText()
    }

    //
    // Buttons in Apple HIG order: secondary utility on the left, Cancel,
    // then primary (default) action at far right.
    //
    RowLayout {
      spacing: 8
      Layout.topMargin: 4
      Layout.fillWidth: true

      Widgets.IconButton {
        text: qsTr("Open Folder")
        icon.source: "qrc:/icons/buttons/open.svg"
        onClicked: Cpp_Misc_Utilities.revealFile(Cpp_Misc_BackupManager.backupDirectory())
      }

      Item { Layout.fillWidth: true }

      Widgets.IconButton {
        text: qsTr("Cancel")
        icon.source: "qrc:/icons/buttons/close.svg"
        onClicked: root.close()
      }

      Widgets.IconButton {
        text: qsTr("Restore")
        enabled: root.selectedIndex >= 0
        icon.source: "qrc:/icons/buttons/apply.svg"
        onClicked: root.restoreSelected()
      }
    }
  }
}
