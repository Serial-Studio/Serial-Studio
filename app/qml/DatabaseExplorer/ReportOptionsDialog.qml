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
  property int visibleRowCount: 0
  property string datasetSearch: ""
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
    _datasetSearch.text = ""
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
  // Build the collapsible folder/group/dataset tree, emitted depth-first.
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
          folderPath: (folderByGroup[d.group] !== undefined) ? folderByGroup[d.group] : "",
          items: []
        }
        order.push(key)
      }
      byKey[key].items.push(d)
    }

    const rootNode = { fullPath: "", children: {}, childOrder: [], groups: [] }
    for (let k = 0; k < order.length; ++k) {
      const path = byKey[order[k]].folderPath
      const segs = (path.length > 0) ? path.split("/") : []
      let node = rootNode
      for (let s = 0; s < segs.length; ++s) {
        const seg = segs[s]
        if (node.children[seg] === undefined) {
          node.children[seg] = {
            fullPath: (node.fullPath.length > 0) ? (node.fullPath + "/" + seg) : seg,
            children: {},
            childOrder: [],
            groups: []
          }
          node.childOrder.push(seg)
        }
        node = node.children[seg]
      }
      node.groups.push(order[k])
    }

    root.emitFolderNode(rootNode, 0, byKey)
    root.recomputeVisibility()
    root.refreshSelectedCount()
  }

  //
  // Depth-first emit of one folder node: nested subfolders first (recursively),
  // then the groups it holds, then each group's datasets.
  //
  function emitFolderNode(node, depth, byKey) {
    const parentId = (node.fullPath.length > 0) ? ("F:" + node.fullPath) : ""
    for (let c = 0; c < node.childOrder.length; ++c) {
      const seg = node.childOrder[c]
      const child = node.children[seg]
      _datasetModel.append({
        "kind": "folder",
        "nodeId": "F:" + child.fullPath,
        "parentId": parentId,
        "depth": depth,
        "expanded": false,
        "hasChildren": child.childOrder.length > 0 || child.groups.length > 0,
        "rowVisible": depth === 0,
        "label": seg,
        "sourceLabel": "",
        "checkState": Qt.Checked,
        "uniqueId": -1,
        "checked": true
      })
      root.emitFolderNode(child, depth + 1, byKey)
    }

    for (let g = 0; g < node.groups.length; ++g) {
      const grp = byKey[node.groups[g]]
      _datasetModel.append({
        "kind": "group",
        "nodeId": "G:" + node.groups[g],
        "parentId": parentId,
        "depth": depth,
        "expanded": false,
        "hasChildren": grp.items.length > 0,
        "rowVisible": depth === 0,
        "label": grp.group,
        "sourceLabel": grp.showSource ? grp.sourceTitle : "",
        "checkState": Qt.Checked,
        "uniqueId": -1,
        "checked": true
      })
      for (let j = 0; j < grp.items.length; ++j) {
        const it = grp.items[j]
        const lbl = (it.units && it.units.length > 0)
                    ? it.title + " (" + it.units + ")"
                    : it.title
        _datasetModel.append({
          "kind": "dataset",
          "nodeId": "D:" + it.uniqueId,
          "parentId": "G:" + node.groups[g],
          "depth": depth + 1,
          "expanded": false,
          "hasChildren": false,
          "rowVisible": false,
          "label": lbl,
          "sourceLabel": "",
          "checkState": Qt.Unchecked,
          "uniqueId": it.uniqueId,
          "checked": true
        })
      }
    }
  }

  //
  // Recompute row visibility, either from the expansion state or the search
  // filter when a query is active.
  //
  function recomputeVisibility() {
    if (root.datasetSearch.length === 0)
      root.recomputeCollapsedVisibility()
    else
      root.recomputeSearchVisibility()
  }

  //
  // Collapsed view: a row shows only when every ancestor is expanded.
  //
  function recomputeCollapsedVisibility() {
    let shownCount = 0
    const shown = {}
    for (let i = 0; i < _datasetModel.count; ++i) {
      const row = _datasetModel.get(i)
      const visible = (row.parentId.length === 0) ? true : (shown[row.parentId] === true)
      if (row.rowVisible !== visible)
        _datasetModel.setProperty(i, "rowVisible", visible)

      shown[row.nodeId] = visible && (row.expanded === true)
      if (visible)
        ++shownCount
    }

    root.visibleRowCount = shownCount
  }

  //
  // Search view: show every row that matches the query, plus its ancestors.
  //
  function recomputeSearchVisibility() {
    const q = root.datasetSearch
    const visible = new Array(_datasetModel.count)
    for (let i = 0; i < _datasetModel.count; ++i)
      visible[i] = false

    for (let i = 0; i < _datasetModel.count; ++i) {
      const row = _datasetModel.get(i)
      if (row.label.toLowerCase().indexOf(q) < 0)
        continue

      visible[i] = true
      root.markAncestorsVisible(row.parentId, visible)
    }

    let shownCount = 0
    for (let i = 0; i < _datasetModel.count; ++i) {
      if (_datasetModel.get(i).rowVisible !== visible[i])
        _datasetModel.setProperty(i, "rowVisible", visible[i])

      if (visible[i])
        ++shownCount
    }

    root.visibleRowCount = shownCount
  }

  //
  // Flag every ancestor of a matched row as visible.
  //
  function markAncestorsVisible(parentId, visible) {
    let pid = parentId
    for (let guard = 0; guard < _datasetModel.count && pid.length > 0; ++guard) {
      const idx = root.indexOfNode(pid)
      if (idx < 0)
        break

      visible[idx] = true
      pid = _datasetModel.get(idx).parentId
    }
  }

  //
  // Toggle the expansion of a folder or group row.
  //
  function toggleExpanded(index) {
    const row = _datasetModel.get(index)
    if (!row.hasChildren)
      return

    _datasetModel.setProperty(index, "expanded", !row.expanded)
    root.recomputeVisibility()
  }

  //
  // Expand or collapse every folder/group node at once.
  //
  function setAllExpanded(expanded) {
    for (let i = 0; i < _datasetModel.count; ++i) {
      const row = _datasetModel.get(i)
      if (row.hasChildren && row.expanded !== expanded)
        _datasetModel.setProperty(i, "expanded", expanded)
    }

    root.recomputeVisibility()
  }

  //
  // Linear lookup of a row by its stable node id.
  //
  function indexOfNode(nodeId) {
    for (let i = 0; i < _datasetModel.count; ++i)
      if (_datasetModel.get(i).nodeId === nodeId)
        return i

    return -1
  }

  //
  // Check or uncheck a node and its whole subtree (contiguous rows of greater
  // depth), then refresh every ancestor's tri-state.
  //
  function setSubtreeChecked(index, checked) {
    const base = _datasetModel.get(index)
    root.applyRowChecked(index, checked)
    for (let i = index + 1; i < _datasetModel.count; ++i) {
      if (_datasetModel.get(i).depth <= base.depth)
        break

      root.applyRowChecked(i, checked)
    }

    root.recomputeAncestors(base.parentId)
    root.refreshSelectedCount()
  }

  //
  // Write the checked state onto one row, respecting its kind.
  //
  function applyRowChecked(index, checked) {
    if (_datasetModel.get(index).kind === "dataset")
      _datasetModel.setProperty(index, "checked", checked)
    else
      _datasetModel.setProperty(index, "checkState", checked ? Qt.Checked : Qt.Unchecked)
  }

  //
  // Walk up the parent chain, recomputing each container's tri-state.
  //
  function recomputeAncestors(parentId) {
    let pid = parentId
    for (let guard = 0; guard < _datasetModel.count && pid.length > 0; ++guard) {
      const idx = root.indexOfNode(pid)
      if (idx < 0)
        break

      root.recomputeNode(idx)
      pid = _datasetModel.get(idx).parentId
    }
  }

  //
  // Recompute a folder/group header tri-state from the leaf datasets it holds.
  //
  function recomputeNode(headerIndex) {
    const base = _datasetModel.get(headerIndex)
    if (base.kind === "dataset")
      return

    let total = 0
    let checked = 0
    for (let i = headerIndex + 1; i < _datasetModel.count; ++i) {
      const row = _datasetModel.get(i)
      if (row.depth <= base.depth)
        break

      if (row.kind !== "dataset")
        continue

      ++total
      if (row.checked)
        ++checked
    }

    if (total === 0)
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
      if (row.kind === "dataset" && row.checked)
        ++n
    }

    root.selectedDatasetCount = n
  }

  //
  // Tree icon for a row kind (folder, group or dataset).
  //
  function iconForKind(kind) {
    if (kind === "folder")
      return "qrc:/icons/project-editor/treeview/folder.svg"

    if (kind === "group")
      return "qrc:/icons/project-editor/treeview/group.svg"

    return "qrc:/icons/project-editor/treeview/dataset.svg"
  }

  //
  // Collect the unique ids of every checked dataset for the export options.
  //
  function collectSelectedUniqueIds() {
    const ids = []
    for (let i = 0; i < _datasetModel.count; ++i) {
      const row = _datasetModel.get(i)
      if (row.kind === "dataset" && row.checked)
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

          RowLayout {
            spacing: 8
            Layout.fillWidth: true

            Label {
              text: qsTr("Include datasets")
              Layout.alignment: Qt.AlignVCenter
              font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
              color: Cpp_ThemeManager.colors["pane_section_label"]
              Component.onCompleted: font.capitalization = Font.AllUppercase
            }

            Item {
              Layout.fillWidth: true
            }

            Button {
              flat: true
              padding: 4
              text: qsTr("Expand All")
              enabled: _datasetModel.count > 0
              font: Cpp_Misc_CommonFonts.uiFont
              onClicked: root.setAllExpanded(true)
            }

            Button {
              flat: true
              padding: 4
              text: qsTr("Collapse All")
              enabled: _datasetModel.count > 0
              font: Cpp_Misc_CommonFonts.uiFont
              onClicked: root.setAllExpanded(false)
            }
          }

          Widgets.SearchField {
            id: _datasetSearch

            implicitHeight: 28
            Layout.fillWidth: true
            placeholderText: qsTr("Search datasets")
            color: Cpp_ThemeManager.colors["window"]
            onTextChanged: {
              root.datasetSearch = text.trim().toLowerCase()
              root.recomputeVisibility()
            }
          }

          Rectangle {
            implicitHeight: 1
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          }

          ListView {
            id: _datasetList

            clip: true
            spacing: 0
            model: _datasetModel
            Layout.fillWidth: true
            Layout.fillHeight: true
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar {
              policy: ScrollBar.AsNeeded
            }

            delegate: Item {
              visible: model.rowVisible
              width: ListView.view.width
              implicitHeight: model.rowVisible ? _rowLayout.implicitHeight + 4 : 0

              RowLayout {
                id: _rowLayout

                spacing: 6
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 4 + model.depth * 16
                anchors.verticalCenter: parent.verticalCenter

                Item {
                  implicitWidth: 10
                  implicitHeight: 10
                  Layout.alignment: Qt.AlignVCenter

                  Image {
                    anchors.centerIn: parent
                    sourceSize: Qt.size(8, 8)
                    visible: model.hasChildren
                    rotation: model.expanded ? 0 : 270
                    source: "qrc:/icons/project-editor/treeview/indicator.svg"
                  }

                  MouseArea {
                    anchors.fill: parent
                    enabled: model.hasChildren
                    onClicked: root.toggleExpanded(index)
                  }
                }

                CheckBox {
                  Layout.alignment: Qt.AlignVCenter
                  tristate: model.kind !== "dataset"
                  checkState: (model.kind !== "dataset")
                              ? model.checkState
                              : (model.checked ? Qt.Checked : Qt.Unchecked)
                  nextCheckState: function() { return checkState }
                  onClicked: {
                    if (model.kind === "dataset")
                      root.setSubtreeChecked(index, !model.checked)
                    else
                      root.setSubtreeChecked(index, model.checkState !== Qt.Checked)
                  }
                }

                Item {
                  implicitWidth: 14
                  implicitHeight: 14
                  Layout.alignment: Qt.AlignVCenter

                  Image {
                    anchors.centerIn: parent
                    sourceSize: Qt.size(14, 14)
                    source: root.iconForKind(model.kind)
                  }
                }

                Label {
                  Layout.fillWidth: true
                  elide: Text.ElideRight
                  text: model.label
                  Layout.alignment: Qt.AlignVCenter
                  color: Cpp_ThemeManager.colors["text"]
                  font: (model.kind !== "dataset")
                        ? Cpp_Misc_CommonFonts.customUiFont(1.0, true)
                        : Cpp_Misc_CommonFonts.uiFont

                  MouseArea {
                    anchors.fill: parent
                    enabled: model.hasChildren
                    onClicked: root.toggleExpanded(index)
                  }
                }

                Label {
                  opacity: 0.6
                  text: model.sourceLabel
                  Layout.alignment: Qt.AlignVCenter
                  font: Cpp_Misc_CommonFonts.uiFont
                  color: Cpp_ThemeManager.colors["text"]
                  visible: model.kind === "group" && model.sourceLabel.length > 0
                }
              }
            }

            Label {
              opacity: 0.5
              anchors.centerIn: parent
              color: Cpp_ThemeManager.colors["text"]
              font: Cpp_Misc_CommonFonts.uiFont
              visible: _datasetModel.count === 0
                       || (root.datasetSearch.length > 0 && root.visibleRowCount === 0)
              text: _datasetModel.count === 0
                    ? qsTr("Loading datasets...")
                    : qsTr("No datasets match your search.")
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
