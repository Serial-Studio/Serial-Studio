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
    property string companyName: ""
    property string authorName: ""
    property string logoPath: ""
    property int pageSizeIndex: 0
    property int orientationIndex: 0
    property bool includeCover: true
    property bool includeMetadata: true
    property bool includeStats: true
    property bool includeCharts: true
    property real lineWidth: 1.4
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
  // Page size enum values (match QPageSize::PageSizeId)
  //
  readonly property var pageSizes: [
    { label: qsTr("A4"),     value: 0 },
    { label: qsTr("A3"),     value: 8 },
    { label: qsTr("Letter"), value: 2 },
    { label: qsTr("Legal"),  value: 3 }
  ]

  //
  // Orientation enum values (QPageLayout::Orientation)
  //
  readonly property var orientations: [
    { label: qsTr("Portrait"),  value: 0 },
    { label: qsTr("Landscape"), value: 1 }
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

    _pageSizeCombo.currentIndex    = Math.max(0, _prefs.pageSizeIndex)
    _orientationCombo.currentIndex = Math.max(0, _prefs.orientationIndex)

    _coverCheck.checked    = _prefs.includeCover
    _metadataCheck.checked = _prefs.includeMetadata
    _statsCheck.checked    = _prefs.includeStats
    _chartsCheck.checked   = _prefs.includeCharts

    _lineWidthSpin.value      = Math.round(_prefs.lineWidth * 10)
    _lineStyleCombo.currentIndex = Math.max(0, _prefs.lineStyleIndex)

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
    _prefs.orientationIndex = _orientationCombo.currentIndex
    _prefs.includeCover     = _coverCheck.checked
    _prefs.includeMetadata  = _metadataCheck.checked
    _prefs.includeStats     = _statsCheck.checked
    _prefs.includeCharts    = _chartsCheck.checked
    _prefs.lineWidth        = _lineWidthSpin.value / 10
    _prefs.lineStyleIndex   = _lineStyleCombo.currentIndex
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
      "orientation":     root.orientations[_orientationCombo.currentIndex].value,
      "includeCover":    _coverCheck.checked,
      "includeMetadata": _metadataCheck.checked,
      "includeStats":    _statsCheck.checked,
      "includeCharts":   _chartsCheck.checked,
      "lineWidth":       _lineWidthSpin.value / 10,
      "lineStyle":       root.lineStyles[_lineStyleCombo.currentIndex].value,
      "outputFormat":    outputFormat
    }

    Cpp_Sessions_Manager.exportSessionToPdf(root.sessionId, options)
  }

  //
  // Dialog body (Settings-style: TabBar + StackLayout + bordered panels)
  //
  dialogContent: ColumnLayout {
    id: layout

    spacing: 12
    anchors.centerIn: parent

    //
    // Tab bar
    //
    TabBar {
      id: _tab

      implicitHeight: 24
      Layout.fillWidth: true

      TabButton {
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
        text: qsTr("Branding")
      }

      TabButton {
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
        text: qsTr("Page")
      }

      TabButton {
        height: _tab.height + 3
        width: implicitWidth + 2 * 8
        text: qsTr("Sections")
      }
    }

    //
    // Tab contents
    //
    StackLayout {
      id: _stack

      clip: true
      Layout.fillWidth: true
      Layout.minimumWidth: 560
      currentIndex: _tab.currentIndex
      Layout.topMargin: -parent.spacing - 1
      implicitHeight: Math.max(_brandingTab.implicitHeight,
                               _pageTab.implicitHeight,
                               _sectionsTab.implicitHeight)

      //
      // Branding tab
      //
      Item {
        id: _brandingTab

        Layout.fillWidth: true
        Layout.fillHeight: true
        implicitHeight: _brandingLayout.implicitHeight + 16

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
          anchors.fill: parent
          anchors.margins: 12

          Item { implicitHeight: 2; Layout.columnSpan: 2 }
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
          Item { implicitHeight: 2; Layout.columnSpan: 2 }

          Label {
            text: qsTr("Company")
            color: Cpp_ThemeManager.colors["text"]
          }
          TextField {
            id: _companyField

            Layout.fillWidth: true
            placeholderText: qsTr("e.g. Acme Test Systems")
            font: Cpp_Misc_CommonFonts.uiFont
          }

          Label {
            text: qsTr("Document title")
            color: Cpp_ThemeManager.colors["text"]
          }
          TextField {
            id: _titleField

            Layout.fillWidth: true
            placeholderText: qsTr("Session Report")
            font: Cpp_Misc_CommonFonts.uiFont
          }

          Label {
            text: qsTr("Author")
            color: Cpp_ThemeManager.colors["text"]
          }
          TextField {
            id: _authorField

            Layout.fillWidth: true
            placeholderText: qsTr("Prepared by (optional)")
            font: Cpp_Misc_CommonFonts.uiFont
          }

          Item { implicitHeight: 6; Layout.columnSpan: 2 }
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
          Item { implicitHeight: 2; Layout.columnSpan: 2 }

          Label {
            text: qsTr("File")
            color: Cpp_ThemeManager.colors["text"]
          }
          RowLayout {
            Layout.fillWidth: true
            spacing: 6

            TextField {
              id: _logoField

              Layout.fillWidth: true
              placeholderText: qsTr("PNG, JPG or SVG (optional)")
              font: Cpp_Misc_CommonFonts.uiFont
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
        implicitHeight: _pageLayout.implicitHeight + 16

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
          anchors.fill: parent
          anchors.margins: 12

          Item { implicitHeight: 2; Layout.columnSpan: 2 }
          Label {
            Layout.columnSpan: 2
            text: qsTr("Paper")
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
          Item { implicitHeight: 2; Layout.columnSpan: 2 }

          Label {
            text: qsTr("Page size")
            color: Cpp_ThemeManager.colors["text"]
          }
          ComboBox {
            id: _pageSizeCombo

            Layout.fillWidth: true
            model: root.pageSizes.map(p => p.label)
            font: Cpp_Misc_CommonFonts.uiFont
          }

          Label {
            text: qsTr("Orientation")
            color: Cpp_ThemeManager.colors["text"]
          }
          ComboBox {
            id: _orientationCombo

            Layout.fillWidth: true
            model: root.orientations.map(o => o.label)
            font: Cpp_Misc_CommonFonts.uiFont
          }

          //
          // Plot appearance sub-section
          //
          Item { implicitHeight: 6; Layout.columnSpan: 2 }
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
          Item { implicitHeight: 2; Layout.columnSpan: 2 }

          Label {
            text: qsTr("Line width")
            color: Cpp_ThemeManager.colors["text"]
          }
          RowLayout {
            Layout.fillWidth: true
            spacing: 8

            SpinBox {
              id: _lineWidthSpin

              // Stored value ×10 so SpinBox (int-only) can span 0.5..3.0 pt
              to: 30
              from: 5
              value: 14
              stepSize: 1
              editable: true
              font: Cpp_Misc_CommonFonts.uiFont
              textFromValue: (v) => (v / 10).toFixed(1) + " pt"
              valueFromText: (text) => Math.round(parseFloat(text) * 10)
            }
            Item { Layout.fillWidth: true }
          }

          Label {
            text: qsTr("Line style")
            color: Cpp_ThemeManager.colors["text"]
          }
          ComboBox {
            id: _lineStyleCombo

            Layout.fillWidth: true
            model: root.lineStyles.map(s => s.label)
            font: Cpp_Misc_CommonFonts.uiFont
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
        implicitHeight: _sectionsLayout.implicitHeight + 16

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
          anchors.fill: parent
          anchors.margins: 12

          Item {
            implicitHeight: 2
          }

          Label {
            text: qsTr("Include")
            color: Cpp_ThemeManager.colors["pane_section_label"]
            font: Cpp_Misc_CommonFonts.customUiFont(0.75, true)
            Component.onCompleted: font.capitalization = Font.AllUppercase
          }

          Rectangle {
            implicitHeight: 1
            Layout.fillWidth: true
            color: Cpp_ThemeManager.colors["groupbox_border"]
          }

          Item {
            implicitHeight: 6
          }

          CheckBox {
            id: _coverCheck

            text: qsTr("Cover page — logo, document title, test subtitle")
            font: Cpp_Misc_CommonFonts.uiFont
          }

          CheckBox {
            id: _metadataCheck

            text: qsTr("Test information — project, timestamps, classification, notes")
            font: Cpp_Misc_CommonFonts.uiFont
          }

          CheckBox {
            id: _statsCheck

            text: qsTr("Measurement summary — min, max, mean, std. deviation per parameter")
            font: Cpp_Misc_CommonFonts.uiFont
          }

          CheckBox {
            id: _chartsCheck

            text: qsTr("Parameter trends — time-series chart per numeric parameter")
            font: Cpp_Misc_CommonFonts.uiFont
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
        icon.source: "qrc:/rcc/icons/buttons/cancel.svg"
        icon.color: Cpp_ThemeManager.colors["button_text"]
        onClicked: root.close()
      }

      Button {
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
      }

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
