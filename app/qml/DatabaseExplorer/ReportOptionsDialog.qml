/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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
  title: qsTr("Generate PDF Report")

  //
  // Dialog state
  //
  property int sessionId: -1

  //
  // Persisted branding + layout preferences (QSettings, category "SessionPdfReport")
  //
  Settings {
    id: _prefs

    category: "SessionPdfReport"
    property string logoPath: ""
    property real lineWidth: 1.4
    property int pageSizeIndex: 0
    property string authorName: ""
    property string companyName: ""
    property bool includeCover: true
    property bool includeStats: true
    property bool includeCharts: true
    property bool includeMetadata: true
    property bool annotateChartStats: false
    property int lineStyleIndex: 0   // 0=Solid, 1=Dashed, 2=Dotted
  }

  //
  // Line style options — index maps to C++ string keys
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
  }

  //
  // Page size enum values (match QPageSize::PageSizeId). Reports always
  // print landscape — a chart with a long X-axis looks squashed on a
  // portrait page — so the dialog doesn't expose an orientation control.
  //
  readonly property var pageSizes: [
    { label: qsTr("A4 (210 × 297 mm)"),         value: 0  },
    { label: qsTr("A3 (297 × 420 mm)"),         value: 8  },
    { label: qsTr("A2 (420 × 594 mm)"),         value: 7  },
    { label: qsTr("A1 (594 × 841 mm)"),         value: 6  },
    { label: qsTr("A0 (841 × 1189 mm)"),        value: 5  },
    { label: qsTr("A5 (148 × 210 mm)"),         value: 9  },
    { label: qsTr("A6 (105 × 148 mm)"),         value: 10 },
    { label: qsTr("B4 (250 × 353 mm)"),         value: 16 },
    { label: qsTr("B5 (176 × 250 mm)"),         value: 1  },
    { label: qsTr("Letter (8.5 × 11 in)"),      value: 2  },
    { label: qsTr("Legal (8.5 × 14 in)"),       value: 3  },
    { label: qsTr("Executive (7.25 × 10.5 in)"), value: 4  },
    { label: qsTr("Tabloid (11 × 17 in)"),      value: 29 },
    { label: qsTr("Ledger (17 × 11 in)"),       value: 28 }
  ]

  //
  // Public API — open the dialog for a given session
  //
  function openFor(id) {
    root.sessionId = id

    // Title is derived fresh each time — never persisted across sessions
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

    _lineWidthSpin.value      = Math.round(_prefs.lineWidth * 10)
    _lineStyleCombo.currentIndex = Math.max(0, _prefs.lineStyleIndex)
    _annotateStatsCheck.checked  = _prefs.annotateChartStats

    root.show()
    root.raise()
  }

  //
  // Save the form back into QSettings so next open pre-fills the branding
  //
  function persistPreferences() {
    _prefs.companyName      = _companyField.text
    _prefs.authorName       = _authorField.text
    _prefs.logoPath         = _logoField.text
    _prefs.pageSizeIndex    = _pageSizeCombo.currentIndex
    _prefs.includeCover     = _coverCheck.checked
    _prefs.includeMetadata  = _metadataCheck.checked
    _prefs.includeStats     = _statsCheck.checked
    _prefs.includeCharts    = _chartsCheck.checked
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
      "outputPath":      "",
      "companyName":     _companyField.text,
      "documentTitle":   _titleField.text,
      "authorName":      _authorField.text,
      "logoPath":        _logoField.text,
      "pageSize":        root.pageSizes[_pageSizeCombo.currentIndex].value,
      "includeCover":    _coverCheck.checked,
      "includeMetadata": _metadataCheck.checked,
      "includeStats":    _statsCheck.checked,
      "includeCharts":   _chartsCheck.checked,
      "lineWidth":           _lineWidthSpin.value / 10,
      "lineStyle":           root.lineStyles[_lineStyleCombo.currentIndex].value,
      "includeStatsOverlay": _annotateStatsCheck.checked,
      "outputFormat":        outputFormat
    }

    Cpp_Sessions_Manager.exportSessionToPdf(root.sessionId, options)
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
    }

    //
    // Tab contents. A fixed preferred size pins the dialog so the window
    // doesn't balloon on platforms whose system font renders taller than
    // macOS (Windows Segoe UI, some Linux DE fonts). Each tab uses
    // anchors.fill so its contents expand into the reserved box instead of
    // pushing the container outward.
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
          }
          TextField {
            id: _companyField

            Layout.fillWidth: true
            font: Cpp_Misc_CommonFonts.uiFont
            placeholderText: qsTr("e.g. Acme Test Systems")
          }

          Label {
            text: qsTr("Document title")
            color: Cpp_ThemeManager.colors["text"]
          }
          TextField {
            id: _titleField

            Layout.fillWidth: true
            font: Cpp_Misc_CommonFonts.uiFont
            placeholderText: qsTr("Session Report")
          }

          Label {
            text: qsTr("Author")
            color: Cpp_ThemeManager.colors["text"]
          }
          TextField {
            id: _authorField

            Layout.fillWidth: true
            font: Cpp_Misc_CommonFonts.uiFont
            placeholderText: qsTr("Prepared by (optional)")
          }

          Item { implicitHeight: 4; Layout.columnSpan: 2 }
          Label {
            Layout.columnSpan: 2
            text: qsTr("Logo")
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
            text: qsTr("File")
            color: Cpp_ThemeManager.colors["text"]
          }
          RowLayout {
            spacing: 6
            Layout.fillWidth: true

            TextField {
              id: _logoField

              Layout.fillWidth: true
              font: Cpp_Misc_CommonFonts.uiFont
              placeholderText: qsTr("PNG, JPG or SVG (optional)")
            }
            Button {
              text: qsTr("Browse…")
              onClicked: Cpp_Sessions_Manager.pickReportLogo()
            }
            Button {
              text: qsTr("Clear")
              enabled: _logoField.text.length > 0
              onClicked: _logoField.text = ""
            }
          }

          Item { Layout.fillHeight: true; Layout.columnSpan: 2 }
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
            text: qsTr("Page size")
            color: Cpp_ThemeManager.colors["text"]
          } ComboBox {
            id: _pageSizeCombo
            Layout.fillWidth: true
            model: root.pageSizes.map(p => p.label)
          }

          //
          // Plot appearance sub-section
          //
          Item { implicitHeight: 4; Layout.columnSpan: 2 }
          Label {
            Layout.columnSpan: 2
            text: qsTr("Plot appearance")
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
            text: qsTr("Line width")
            color: Cpp_ThemeManager.colors["text"]
          }
          RowLayout {
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
            }
            Item { Layout.fillWidth: true }
          }

          Label {
            text: qsTr("Line style")
            color: Cpp_ThemeManager.colors["text"]
          } ComboBox {
            id: _lineStyleCombo
            Layout.fillWidth: true
            model: root.lineStyles.map(s => s.label)
          }

          CheckBox {
            id: _annotateStatsCheck

            checked: true
            Layout.columnSpan: 2
            text: qsTr("Annotate min, max, and mean values on plots")
          }

          Item { Layout.fillHeight: true; Layout.columnSpan: 2 }
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
          }

          CheckBox {
            id: _metadataCheck

            text: qsTr("Test information (project, timestamps, classification and notes)")
          }

          CheckBox {
            id: _statsCheck

            text: qsTr("Measurement summary (min, max, mean, std. deviation per parameter)")
          }

          CheckBox {
            id: _chartsCheck

            text: qsTr("Parameter trends (time-series chart per numeric parameter)")
          }

          Item { Layout.fillHeight: true }
        }
      }
    }

    //
    // Action buttons
    //
    RowLayout {
      spacing: 8
      Layout.fillWidth: true

      Item { Layout.fillWidth: true }

      Button {
        text: qsTr("Cancel")
        icon.width: 16
        icon.height: 16
        icon.source: "qrc:/rcc/icons/buttons/close.svg"
        icon.color: Cpp_ThemeManager.colors["button_text"]
        onClicked: root.close()
      }

      /*Button {
        text: qsTr("Export HTML")
        icon.width: 16
        icon.height: 16
        icon.source: "qrc:/rcc/icons/buttons/html.svg"
        icon.color: Cpp_ThemeManager.colors["button_text"]
        enabled: _coverCheck.checked
                 || _metadataCheck.checked
                 || _statsCheck.checked
                 || _chartsCheck.checked
        onClicked: {
          root.persistPreferences()
          root.close()
          root.dispatchExport("html")
        }
      }*/

      Button {
        highlighted: true
        text: qsTr("Export PDF")
        icon.width: 16
        icon.height: 16
        icon.color: Cpp_ThemeManager.colors["button_text"]
        icon.source: "qrc:/rcc/icons/buttons/pdf.svg"
        enabled: _coverCheck.checked
                 || _metadataCheck.checked
                 || _statsCheck.checked
                 || _chartsCheck.checked
        onClicked: {
          root.persistPreferences()
          root.close()
          root.dispatchExport("pdf")
        }
      }
    }
  }
}
