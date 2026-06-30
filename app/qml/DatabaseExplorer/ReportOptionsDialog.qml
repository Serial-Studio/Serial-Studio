/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  //
  // Window options
  //
  staysOnTop: true
  title: Cpp_HasWebEngine ? qsTr("Generate PDF Report") : qsTr("Generate Report")

  //
  // Direct CSD size hints (bypasses Page implicit-size propagation)
  //
  preferredWidth: layout.implicitWidth
  preferredHeight: layout.implicitHeight

  //
  // Dialog state
  //
  property int sessionId: -1
  property int selectedDatasetCount: 0

  //
  // Persisted branding + layout preferences
  //
  Settings {
    id: _prefs

    category: "SessionPdfReport"
    property string logoPath: ""
    property real lineWidth: 1.4
    property int pageSizeIndex: 0
    property int lineStyleIndex: 0
    property string authorName: ""
    property string companyName: ""
    property bool includeCover: true
    property bool includeStats: true
    property bool includeCharts: true
    property bool includeMetadata: true
    property bool annotateChartStats: false
  }

  //
  // Per-session dataset selection model (flattened group headers + dataset rows)
  //
  ListModel {
    id: _datasetModel
  }

  //
  // Line style options: index maps to C++ string keys
  //
  readonly property var lineStyles: [
    { label: qsTr("Solid"),  value: "solid"  },
    { label: qsTr("Dashed"), value: "dashed" },
    { label: qsTr("Dotted"), value: "dotted" }
  ]

  //
  // Receive the chosen logo path from the C++ file picker
  //
  Connections {
    target: Cpp_Sessions_Manager
    function onReportLogoPicked(path) {
      _logoField.text = path
    }

    function onSessionDatasetsReady(sessionId, datasets) {
      if (sessionId === root.sessionId)
        root.buildDatasetModel(datasets)
    }
  }

  //
  // QPageSize::PageSizeId values, reports always print landscape.
  //
  readonly property var pageSizes: [
    { label: qsTr("A4 (210 × 297 mm)"),          value: 0  },
    { label: qsTr("A3 (297 × 420 mm)"),          value: 8  },
    { label: qsTr("A2 (420 × 594 mm)"),          value: 7  },
    { label: qsTr("A1 (594 × 841 mm)"),          value: 6  },
    { label: qsTr("A0 (841 × 1189 mm)"),         value: 5  },
    { label: qsTr("A5 (148 × 210 mm)"),          value: 9  },
    { label: qsTr("A6 (105 × 148 mm)"),          value: 10 },
    { label: qsTr("B4 (250 × 353 mm)"),          value: 16 },
    { label: qsTr("B5 (176 × 250 mm)"),          value: 1  },
    { label: qsTr("Letter (8.5 × 11 in)"),       value: 2  },
    { label: qsTr("Legal (8.5 × 14 in)"),        value: 3  },
    { label: qsTr("Executive (7.25 × 10.5 in)"), value: 4  },
    { label: qsTr("Tabloid (11 × 17 in)"),       value: 29 },
    { label: qsTr("Ledger (17 × 11 in)"),        value: 28 }
  ]

  //
  // Open the dialog for a given session
  //
  function openFor(id) {
    root.sessionId = id
    _datasetModel.clear()
    root.selectedDatasetCount = 0
    Cpp_Sessions_Manager.requestSessionDatasets(id)

    const meta = Cpp_Sessions_Manager.sessionMetadata(id)
    if (meta && meta.project_title)
      _titleField.text = qsTr("%1 — Session Report").arg(meta.project_title)
    else
      _titleField.text = qsTr("Session Report")

    _companyField.text = _prefs.companyName
    _authorField.text  = _prefs.authorName
    _logoField.text    = _prefs.logoPath

    _pageSizeCombo.currentIndex = Math.max(0, _prefs.pageSizeIndex)

    _coverCheck.checked    = _prefs.includeCover
    _metadataCheck.checked = _prefs.includeMetadata
    _statsCheck.checked    = _prefs.includeStats
    _chartsCheck.checked   = _prefs.includeCharts
    _annotateStatsCheck.checked  = _prefs.annotateChartStats

    _lineWidthSpin.value      = Math.round(_prefs.lineWidth * 10)
    _lineStyleCombo.currentIndex = Math.max(0, _prefs.lineStyleIndex)

    root.show()
    root.raise()
  }

  //
  // Save the form back into QSettings so next open pre-fills the branding
  //
  function persistPreferences() {
    _prefs.companyName         = _companyField.text
    _prefs.authorName          = _authorField.text
    _prefs.logoPath            = _logoField.text
    _prefs.pageSizeIndex       = _pageSizeCombo.currentIndex
    _prefs.includeCover        = _coverCheck.checked
    _prefs.includeMetadata     = _metadataCheck.checked
    _prefs.includeStats        = _statsCheck.checked
    _prefs.includeCharts       = _chartsCheck.checked
    _prefs.lineWidth           = _lineWidthSpin.value / 10
    _prefs.lineStyleIndex      = _lineStyleCombo.currentIndex
    _prefs.annotateChartStats  = _annotateStatsCheck.checked
  }

  //
  // Build the options map and invoke the C++ export slot. The C++ side
  // opens the native QFileDialog save picker when outputPath is empty.
  //
  function dispatchExport(outputFormat) {
    const options = {
      "outputPath":          "",
      "companyName":         _companyField.text,
      "documentTitle":       _titleField.text,
      "authorName":          _authorField.text,
      "logoPath":            _logoField.text,
      "pageSize":            root.pageSizes[_pageSizeCombo.currentIndex].value,
      "includeCover":        _coverCheck.checked,
      "includeMetadata":     _metadataCheck.checked,
      "includeStats":        _statsCheck.checked,
      "includeCharts":       _chartsCheck.checked,
      "lineWidth":           _lineWidthSpin.value / 10,
      "lineStyle":           root.lineStyles[_lineStyleCombo.currentIndex].value,
      "includeStatsOverlay": _annotateStatsCheck.checked,
      "selectedUniqueIds":   root.collectSelectedUniqueIds(),
      "outputFormat":        outputFormat
    }

    const id = root.sessionId
    Qt.callLater(() => Cpp_Sessions_Manager.exportSessionToPdf(id, options))
  }

  //
  // Build the flattened group/dataset selection model from the worker payload.
  // Datasets arrive in column order; groups keep first-seen order.
  //
  function buildDatasetModel(datasets) {
    _datasetModel.clear()

    const sourcesPerGroup = {}
    for (let i = 0; i < datasets.length; ++i) {
      const g = datasets[i].group
      if (!sourcesPerGroup[g])
        sourcesPerGroup[g] = {}

      sourcesPerGroup[g][datasets[i].sourceTitle] = true
    }

    //
    // Folder path per group title from the current project (empty = top level)
    //
    const folderByGroup = Cpp_JSON_ProjectEditor.groupFolderPaths()

    const order = []
    const byKey = {}
    for (let i = 0; i < datasets.length; ++i) {
      const d = datasets[i]
      const key = d.sourceTitle + " " + d.group
      if (!byKey[key]) {
        const distinct = Object.keys(sourcesPerGroup[d.group] || {}).length
        byKey[key] = {
          group: d.group,
          sourceTitle: d.sourceTitle,
          showSource: distinct > 1 && d.sourceTitle.length > 0,
          folderKey: (folderByGroup[d.group] !== undefined) ? folderByGroup[d.group] : "",
          items: []
        }
        order.push(key)
      }
      byKey[key].items.push(d)
    }

    //
    // Bucket groups under their folder, preserving first-seen folder order
    //
    const folderOrder = []
    const groupsByFolder = {}
    for (let k = 0; k < order.length; ++k) {
      const fk = byKey[order[k]].folderKey
      if (groupsByFolder[fk] === undefined) {
        groupsByFolder[fk] = []
        folderOrder.push(fk)
      }
      groupsByFolder[fk].push(order[k])
    }

    for (let f = 0; f < folderOrder.length; ++f) {
      const fk = folderOrder[f]
      if (fk.length > 0) {
        _datasetModel.append({
          "isFolder": true,
          "isGroup": false,
          "folderKey": fk,
          "groupKey": "",
          "folderLabel": fk,
          "groupLabel": "",
          "sourceLabel": "",
          "checkState": Qt.Checked,
          "uniqueId": -1,
          "datasetLabel": "",
          "checked": true
        })
      }

      const gkeys = groupsByFolder[fk]
      for (let k = 0; k < gkeys.length; ++k) {
        const grp = byKey[gkeys[k]]
        _datasetModel.append({
          "isFolder": false,
          "isGroup": true,
          "folderKey": fk,
          "groupKey": gkeys[k],
          "folderLabel": "",
          "groupLabel": grp.group,
          "sourceLabel": grp.showSource ? grp.sourceTitle : "",
          "checkState": Qt.Checked,
          "uniqueId": -1,
          "datasetLabel": "",
          "checked": true
        })
        for (let j = 0; j < grp.items.length; ++j) {
          const it = grp.items[j]
          const lbl = (it.units && it.units.length > 0)
                      ? it.title + " (" + it.units + ")"
                      : it.title
          _datasetModel.append({
            "isFolder": false,
            "isGroup": false,
            "folderKey": fk,
            "groupKey": gkeys[k],
            "folderLabel": "",
            "groupLabel": "",
            "sourceLabel": "",
            "checkState": Qt.Unchecked,
            "uniqueId": it.uniqueId,
            "datasetLabel": lbl,
            "checked": true
          })
        }
      }
    }

    root.refreshSelectedCount()
  }

  //
  // Check or uncheck an entire folder and everything it contains.
  //
  function setFolderChecked(folderKey, checked) {
    for (let i = 0; i < _datasetModel.count; ++i) {
      const row = _datasetModel.get(i)
      if (row.folderKey !== folderKey)
        continue

      if (row.isFolder || row.isGroup)
        _datasetModel.setProperty(i, "checkState", checked ? Qt.Checked : Qt.Unchecked)
      else
        _datasetModel.setProperty(i, "checked", checked)
    }

    root.refreshSelectedCount()
  }

  //
  // Check or uncheck an entire group and all of its datasets, then refresh its folder.
  //
  function setGroupChecked(groupKey, checked) {
    let folderKey = ""
    for (let i = 0; i < _datasetModel.count; ++i) {
      const row = _datasetModel.get(i)
      if (row.groupKey !== groupKey)
        continue

      folderKey = row.folderKey
      if (row.isGroup)
        _datasetModel.setProperty(i, "checkState", checked ? Qt.Checked : Qt.Unchecked)
      else
        _datasetModel.setProperty(i, "checked", checked)
    }

    root.recomputeFolder(folderKey)
    root.refreshSelectedCount()
  }

  //
  // Toggle one dataset, then refresh its group header tri-state.
  //
  function toggleDataset(index, checked) {
    _datasetModel.setProperty(index, "checked", checked)
    root.recomputeGroup(_datasetModel.get(index).groupKey)
    root.recomputeFolder(_datasetModel.get(index).folderKey)
    root.refreshSelectedCount()
  }

  //
  // Recompute a group header check state from its datasets.
  //
  function recomputeGroup(groupKey) {
    let total = 0
    let checked = 0
    let headerIndex = -1
    for (let i = 0; i < _datasetModel.count; ++i) {
      const row = _datasetModel.get(i)
      if (row.groupKey !== groupKey)
        continue

      if (row.isGroup) {
        headerIndex = i
        continue
      }

      ++total
      if (row.checked)
        ++checked
    }

    if (headerIndex < 0)
      return

    const state = (checked === 0) ? Qt.Unchecked
                : ((checked === total) ? Qt.Checked : Qt.PartiallyChecked)
    _datasetModel.setProperty(headerIndex, "checkState", state)
  }

  //
  // Recompute a folder header check state from the datasets it contains.
  //
  function recomputeFolder(folderKey) {
    if (folderKey.length === 0)
      return

    let total = 0
    let checked = 0
    let headerIndex = -1
    for (let i = 0; i < _datasetModel.count; ++i) {
      const row = _datasetModel.get(i)
      if (row.folderKey !== folderKey)
        continue

      if (row.isFolder) {
        headerIndex = i
        continue
      }

      if (row.isGroup)
        continue

      ++total
      if (row.checked)
        ++checked
    }

    if (headerIndex < 0)
      return

    const state = (checked === 0) ? Qt.Unchecked
                : ((checked === total) ? Qt.Checked : Qt.PartiallyChecked)
    _datasetModel.setProperty(headerIndex, "checkState", state)
  }

  //
  // Refresh the count of selected datasets (drives the export guard).
  //
  function refreshSelectedCount() {
    let n = 0
    for (let i = 0; i < _datasetModel.count; ++i) {
      const row = _datasetModel.get(i)
      if (!row.isGroup && !row.isFolder && row.checked)
        ++n
    }

    root.selectedDatasetCount = n
  }

  //
  // Collect the unique ids of every checked dataset for the export options.
  //
  function collectSelectedUniqueIds() {
    const ids = []
    for (let i = 0; i < _datasetModel.count; ++i) {
      const row = _datasetModel.get(i)
      if (!row.isGroup && !row.isFolder && row.checked)
        ids.push(row.uniqueId)
    }

    return ids
  }

  //
  // Dialog body (Settings-style: TabBar + StackLayout + bordered panels)
  //
  dialogContent: ColumnLayout {
    id: layout

    spacing: 12
    anchors.fill: parent

    //
    // Tab bar
    //
    TabBar {
      id: _tab

      implicitHeight: 24
      Layout.fillWidth: true

      TabButton {
        text: qsTr("Branding")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Page")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Sections")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }

      TabButton {
        text: qsTr("Datasets")
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
      }
    }

    //
    // Fixed preferred size keeps the dialog from ballooning on tall system fonts
    //
    StackLayout {
      id: _stack

      clip: true
      Layout.fillWidth: true
      Layout.minimumWidth: 520
      Layout.preferredWidth: 540
      Layout.preferredHeight: 280
      currentIndex: _tab.currentIndex
      Layout.topMargin: -parent.spacing - 1

      //
      // Branding tab
      //
      Item {
        id: _brandingTab

        Layout.fillWidth: true
        Layout.fillHeight: true

        Rectangle {
          radius: 2
          border.width: 1
          anchors.fill: parent
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        GridLayout {
          id: _brandingLayout

          columns: 2
          rowSpacing: 6
          columnSpacing: 8
          anchors.margins: 12
          anchors.fill: parent

          Label {
            Layout.columnSpan: 2
            text: qsTr("Identity")
            color: Cpp_ThemeManager.colors["pane_section_label"]
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            Component.onCompleted: font.capitalization = Font.AllUppercase
          }

          Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          }

          Label {
            text: qsTr("Company")
            color: Cpp_ThemeManager.colors["text"]
          } Widgets.LineField {
            id: _companyField

            Layout.fillWidth: true
            font: Cpp_Misc_CommonFonts.uiFont
            placeholderText: qsTr("e.g. Acme Test Systems")
          }

          Label {
            text: qsTr("Document title")
            color: Cpp_ThemeManager.colors["text"]
          } Widgets.LineField {
            id: _titleField

            Layout.fillWidth: true
            font: Cpp_Misc_CommonFonts.uiFont
            placeholderText: qsTr("Session Report")
          }

          Label {
            text: qsTr("Author")
            color: Cpp_ThemeManager.colors["text"]
          } Widgets.LineField {
            id: _authorField

            Layout.fillWidth: true
            font: Cpp_Misc_CommonFonts.uiFont
            placeholderText: qsTr("Prepared by (optional)")
          }

          Item {
            implicitHeight: 4
            Layout.columnSpan: 2
          }

          Label {
            text: qsTr("Logo")
            Layout.columnSpan: 2
            color: Cpp_ThemeManager.colors["pane_section_label"]
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          }

          Label {
            text: qsTr("File")
            color: Cpp_ThemeManager.colors["text"]
          } RowLayout {
            spacing: 6
            Layout.fillWidth: true

            Widgets.LineField {
              id: _logoField

              Layout.fillWidth: true
              font: Cpp_Misc_CommonFonts.uiFont
              placeholderText: qsTr("PNG, JPG or SVG (optional)")
            } Button {
              text: qsTr("Browse…")
              onClicked: Cpp_Sessions_Manager.pickReportLogo()
            } Button {
              text: qsTr("Clear")
              enabled: _logoField.text.length > 0
              onClicked: _logoField.text = ""
            }
          }

          Item {
            Layout.columnSpan: 2
            Layout.fillHeight: true
          }
        }
      }

      //
      // Page tab
      //
      Item {
        id: _pageTab

        Layout.fillWidth: true
        Layout.fillHeight: true

        Rectangle {
          radius: 2
          border.width: 1
          anchors.fill: parent
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        GridLayout {
          id: _pageLayout

          columns: 2
          rowSpacing: 6
          columnSpacing: 8
          anchors.margins: 12
          anchors.fill: parent

          Label {
            Layout.columnSpan: 2
            text: qsTr("Paper")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          } Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          }

          Label {
            text: qsTr("Page size")
            color: Cpp_ThemeManager.colors["text"]
          } Widgets.Combo {
            id: _pageSizeCombo
            Layout.fillWidth: true
            model: root.pageSizes.map(p => p.label)
          }

          Item {
            implicitHeight: 4
            Layout.columnSpan: 2
          }

          Label {
            Layout.columnSpan: 2
            text: qsTr("Plot appearance")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          }

          Rectangle {
            implicitHeight: 1
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          }

          Label {
            text: qsTr("Line width")
            color: Cpp_ThemeManager.colors["text"]
          } RowLayout {
            spacing: 8
            Layout.fillWidth: true

            SpinBox {
              id: _lineWidthSpin

              to: 30
              from: 5
              value: 14
              stepSize: 1
              editable: true
              font: Cpp_Misc_CommonFonts.uiFont
              textFromValue: (v) => (v / 10).toFixed(1) + " pt"
              valueFromText: (text) => {
                const parsed = parseFloat(String(text).replace(",", "."))
                if (isNaN(parsed))
                  return _lineWidthSpin.value

                return Math.round(parsed * 10)
              }
              validator: RegularExpressionValidator {
                regularExpression: /^\s*\d+([.,]\d*)?\s*(pt)?\s*$/
              }
            } Item {
              Layout.fillWidth: true
            }
          }

          Label {
            text: qsTr("Line style")
            color: Cpp_ThemeManager.colors["text"]
          } Widgets.Combo {
            id: _lineStyleCombo
            Layout.fillWidth: true
            model: root.lineStyles.map(s => s.label)
          }

          Item {
            Layout.columnSpan: 2
            Layout.fillHeight: true
          }
        }
      }

      //
      // Sections tab
      //
      Item {
        id: _sectionsTab

        Layout.fillWidth: true
        Layout.fillHeight: true

        Rectangle {
          radius: 2
          border.width: 1
          anchors.fill: parent
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        ColumnLayout {
          id: _sectionsLayout

          spacing: 6
          anchors.margins: 12
          anchors.fill: parent

          Label {
            text: qsTr("Include")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          }

          Rectangle {
            implicitHeight: 1
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          }

          CheckBox {
            id: _coverCheck

            text: qsTr("Cover page (logo, document title, test subtitle)")
          } CheckBox {
            id: _metadataCheck
            text: qsTr("Test information (project, timestamps, classification and notes)")
          } CheckBox {
            id: _statsCheck
            text: qsTr("Measurement summary (min, max, mean, std. deviation per parameter)")
          } CheckBox {
            id: _chartsCheck
            text: qsTr("Parameter trends (time-series chart per numeric parameter)")
          } CheckBox {
            id: _annotateStatsCheck
            text: qsTr("Annotate min, max, and mean values on plots")
          }

          Item {
            Layout.fillHeight: true
          }
        }
      }

      //
      // Datasets tab
      //
      Item {
        id: _datasetsTab

        Layout.fillWidth: true
        Layout.fillHeight: true

        Rectangle {
          radius: 2
          border.width: 1
          anchors.fill: parent
          color: Cpp_ThemeManager.colors["groupbox_background"]
          border.color: Cpp_ThemeManager.colors["groupbox_border"]
        }

        ColumnLayout {
          spacing: 6
          anchors.margins: 12
          anchors.fill: parent

          Label {
            text: qsTr("Include datasets")
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            color: Cpp_ThemeManager.colors["pane_section_label"]
            Component.onCompleted: font.capitalization = Font.AllUppercase
          }

          Rectangle {
            implicitHeight: 1
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          }

          ListView {
            id: _datasetList

            clip: true
            spacing: 2
            model: _datasetModel
            Layout.fillWidth: true
            Layout.fillHeight: true
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar {
              policy: ScrollBar.AsNeeded
            }

            delegate: Item {
              width: ListView.view.width
              implicitHeight: _rowLayout.implicitHeight + 4

              RowLayout {
                id: _rowLayout

                spacing: 6
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: model.isFolder ? 0 : (model.isGroup ? 18 : 36)

                CheckBox {
                  Layout.alignment: Qt.AlignVCenter
                  tristate: model.isFolder || model.isGroup
                  checkState: (model.isFolder || model.isGroup)
                              ? model.checkState
                              : (model.checked ? Qt.Checked : Qt.Unchecked)
                  nextCheckState: function() { return checkState }
                  onClicked: {
                    if (model.isFolder)
                      root.setFolderChecked(model.folderKey, model.checkState !== Qt.Checked)
                    else if (model.isGroup)
                      root.setGroupChecked(model.groupKey, model.checkState !== Qt.Checked)
                    else
                      root.toggleDataset(index, !model.checked)
                  }
                }

                Label {
                  Layout.fillWidth: true
                  elide: Text.ElideRight
                  Layout.alignment: Qt.AlignVCenter
                  color: Cpp_ThemeManager.colors["text"]
                  text: model.isFolder
                        ? model.folderLabel
                        : (model.isGroup ? model.groupLabel : model.datasetLabel)
                  font: (model.isFolder || model.isGroup)
                        ? Cpp_Misc_CommonFonts.customUiFont(1.0, true)
                        : Cpp_Misc_CommonFonts.uiFont
                }

                Label {
                  opacity: 0.6
                  text: model.sourceLabel
                  Layout.alignment: Qt.AlignVCenter
                  font: Cpp_Misc_CommonFonts.uiFont
                  color: Cpp_ThemeManager.colors["text"]
                  visible: model.isGroup && model.sourceLabel.length > 0
                }
              }
            }

            Label {
              opacity: 0.5
              anchors.centerIn: parent
              visible: _datasetModel.count === 0
              color: Cpp_ThemeManager.colors["text"]
              font: Cpp_Misc_CommonFonts.uiFont
              text: qsTr("Loading datasets...")
            }
          }
        }
      }
    }

    //
    // Action buttons
    //
    RowLayout {
      spacing: 8
      Layout.fillWidth: true

      Label {
        opacity: 0.7
        Layout.alignment: Qt.AlignVCenter
        font: Cpp_Misc_CommonFonts.uiFont
        color: Cpp_ThemeManager.colors["text"]
        visible: _datasetModel.count > 0 && root.selectedDatasetCount === 0
        text: qsTr("Select at least one dataset to include.")
      }

      Item {
        Layout.fillWidth: true
      }

      Widgets.IconButton {
        iconSize: 16
        text: qsTr("Cancel")
        onClicked: root.close()
        icon.source: "qrc:/icons/buttons/close.svg"
      }

      Widgets.IconButton {
        iconSize: 16
        highlighted: true
        text: Cpp_HasWebEngine ? qsTr("Export PDF") : qsTr("Export HTML")
        icon.source: Cpp_HasWebEngine ? "qrc:/icons/buttons/pdf.svg"
                                      : "qrc:/icons/buttons/html.svg"
        enabled: (_coverCheck.checked
                  || _metadataCheck.checked
                  || _statsCheck.checked
                  || _chartsCheck.checked)
                 && root.selectedDatasetCount > 0
        onClicked: {
          root.persistPreferences()
          root.close()
          root.dispatchExport(Cpp_HasWebEngine ? "pdf" : "html")
        }
      }
    }
  }
}
