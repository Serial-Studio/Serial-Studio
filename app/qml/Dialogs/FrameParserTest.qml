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
import SerialStudio

import "../Widgets" as Widgets

Widgets.SmartDialog {
  id: root

  //
  // Window setup: resizable, always on top (it accompanies the project editor)
  //
  fixedSize: false
  staysOnTop: true
  preferredWidth: 700
  preferredHeight: 600
  title: qsTr("Test Frame Parser")

  //
  // Public interface: works with every frame parser language (JS, Lua, Native)
  //
  property int sourceId: 0

  function openForSource(id) {
    root.sourceId = id
    clearResults()
    root.show()
    root.raise()
    root.requestActivate()
  }

  //
  // Preview state
  //
  property var treeNodes: []
  property var previewFrames: []
  property var expandedRows: ({})
  property string previewStats: ""
  property string previewError: ""

  readonly property bool needsEndDelimiter: tester.detectionIndex === 0
                                            || tester.detectionIndex === 1
  readonly property bool needsStartDelimiter: tester.detectionIndex === 1
                                              || tester.detectionIndex === 2

  //
  // Reminder when the typed sample misses the configured delimiters (no frame would extract).
  // The pipeline properties are referenced so the binding re-evaluates on delimiter edits.
  //
  readonly property bool delimitersCovered: {
    const start     = tester.frameStart
    const end       = tester.frameEnd
    const detection = tester.detectionIndex
    const hexMode   = tester.hexDelimiters
    return tester.inputContainsDelimiters(sampleInput.text, hexToggle.checked)
  }

  //
  // Uppercase section label, shared by every section (same idiom as the trigger dialog)
  //
  component SectionLabel: Label {
    font: Cpp_Misc_CommonFonts.customUiFont(0.8, true)
    color: Cpp_ThemeManager.colors["pane_section_label"]
    Component.onCompleted: font.capitalization = Font.AllUppercase
  }

  //
  // Tree of results: Frame -> Decoder + Rows -> Row -> cells
  //
  function rebuildTree() {
    const nodes = []
    for (let i = 0; i < root.previewFrames.length; ++i) {
      const f = root.previewFrames[i]
      nodes.push({depth: 0, stage: qsTr("Frame %1").arg(i + 1),
                  value: f.rawHex, expandable: false, key: ""})
      nodes.push({depth: 1, stage: qsTr("Decoder"),
                  value: f.decoderOutput, expandable: false, key: ""})
      nodes.push({depth: 1, stage: qsTr("Rows"),
                  value: qsTr("%1 row(s)").arg(f.rows.length), expandable: false, key: ""})

      for (let r = 0; r < f.rows.length; ++r) {
        const key = i + ":" + r
        nodes.push({depth: 2, stage: qsTr("Row %1").arg(r + 1),
                    value: f.rows[r].join(", "), expandable: true, key: key})

        if (root.expandedRows[key] === true)
          for (let c = 0; c < f.rows[r].length; ++c)
            nodes.push({depth: 3, stage: "[" + (c + 1) + "]",
                        value: f.rows[r][c], expandable: false, key: ""})
      }
    }

    root.treeNodes = nodes
  }

  function toggleRow(key) {
    const expanded = root.expandedRows
    expanded[key] = expanded[key] !== true
    root.expandedRows = expanded
    root.rebuildTree()
  }

  //
  // Parse on demand only (Evaluate button / Return key)
  //
  function evaluate() {
    tester.dryRun(sampleInput.text, hexToggle.checked)
  }

  function clearResults() {
    sampleInput.clear()
    root.treeNodes = []
    root.previewFrames = []
    root.expandedRows = ({})
    root.previewStats = ""
    root.previewError = ""
  }

  //
  // C++ bridge: pipeline settings + language-aware dry-run for the bound source
  //
  NativeParserEditor {
    id: tester

    sourceId: root.sourceId

    onPreviewReady: function(frames, error, stats) {
      root.previewFrames = frames
      root.previewError = error
      root.previewStats = stats
      root.rebuildTree()
    }
  }

  dialogContent: ColumnLayout {
    spacing: 4

    //
    // Pipeline configuration section
    //
    SectionLabel {
      text: qsTr("Pipeline Configuration")
    }

    GroupBox {
      Layout.fillWidth: true

      background: Rectangle {
        radius: 2
        border.width: 1
        border.color: Cpp_ThemeManager.colors["groupbox_border"]
        color: Cpp_ThemeManager.colors["groupbox_background"]
      }

      GridLayout {
        columns: 4
        rowSpacing: 4
        columnSpacing: 8
        anchors.fill: parent

        Label {
          Layout.row: 0
          Layout.column: 0
          text: qsTr("Detection")
          font: Cpp_Misc_CommonFonts.uiFont
          color: Cpp_ThemeManager.colors["text"]
        }

        Widgets.Combo {
          Layout.row: 0
          Layout.column: 1
          Layout.fillWidth: true
          model: tester.detectionOptions
          currentIndex: tester.detectionIndex

          onActivated: {
            tester.setDetectionIndex(currentIndex)
            currentIndex = Qt.binding(function() {
              return tester.detectionIndex
            })
          }
        }

        Label {
          Layout.row: 0
          Layout.column: 2
          text: qsTr("Decoder")
          font: Cpp_Misc_CommonFonts.uiFont
          color: Cpp_ThemeManager.colors["text"]
        }

        Widgets.Combo {
          Layout.row: 0
          Layout.column: 3
          Layout.fillWidth: true
          model: tester.decoderOptions
          currentIndex: tester.decoderIndex

          onActivated: {
            tester.setDecoderIndex(currentIndex)
            currentIndex = Qt.binding(function() {
              return tester.decoderIndex
            })
          }
        }

        Label {
          Layout.row: 1
          Layout.column: 0
          text: qsTr("Start")
          opacity: enabled ? 1 : 0.5
          enabled: root.needsStartDelimiter
          font: Cpp_Misc_CommonFonts.uiFont
          color: Cpp_ThemeManager.colors["text"]
        }

        TextField {
          Layout.row: 1
          Layout.column: 1
          Layout.fillWidth: true
          text: tester.frameStart
          opacity: enabled ? 1 : 0.5
          enabled: root.needsStartDelimiter
          font: Cpp_Misc_CommonFonts.monoFont
          onEditingFinished: tester.setFrameStart(text)
        }

        Label {
          Layout.row: 1
          Layout.column: 2
          text: qsTr("End")
          opacity: enabled ? 1 : 0.5
          enabled: root.needsEndDelimiter
          font: Cpp_Misc_CommonFonts.uiFont
          color: Cpp_ThemeManager.colors["text"]
        }

        TextField {
          Layout.row: 1
          Layout.column: 3
          text: tester.frameEnd
          Layout.fillWidth: true
          opacity: enabled ? 1 : 0.5
          enabled: root.needsEndDelimiter
          font: Cpp_Misc_CommonFonts.monoFont
          onEditingFinished: tester.setFrameEnd(text)
        }

        Label {
          Layout.row: 2
          Layout.column: 0
          text: qsTr("Checksum")
          font: Cpp_Misc_CommonFonts.uiFont
          color: Cpp_ThemeManager.colors["text"]
        }

        Widgets.Combo {
          Layout.row: 2
          Layout.column: 1
          Layout.fillWidth: true
          model: tester.checksumOptions
          currentIndex: tester.checksumIndex

          onActivated: {
            tester.setChecksumIndex(currentIndex)
            currentIndex = Qt.binding(function() {
              return tester.checksumIndex
            })
          }
        }

        CheckBox {
          Layout.row: 2
          Layout.column: 3
          Layout.leftMargin: -6
          opacity: enabled ? 1 : 0.5
          text: qsTr("Hex Delimiters")
          checked: tester.hexDelimiters
          onClicked: tester.setHexDelimiters(checked)
          enabled: root.needsStartDelimiter || root.needsEndDelimiter
        }
      }
    }

    Item {
      implicitHeight: 4
    }

    //
    // Frame data input section
    //
    SectionLabel {
      text: qsTr("Frame Data Input")
    }

    GroupBox {
      Layout.fillWidth: true

      background: Rectangle {
        radius: 2
        border.width: 1
        border.color: Cpp_ThemeManager.colors["groupbox_border"]
        color: Cpp_ThemeManager.colors["groupbox_background"]
      }

      ColumnLayout {
        spacing: 6
        anchors.fill: parent

        RowLayout {
          spacing: 4
          Layout.fillWidth: true

          TextField {
            id: sampleInput

            Layout.fillWidth: true
            onAccepted: root.evaluate()
            font: Cpp_Misc_CommonFonts.monoFont
            placeholderText: hexToggle.checked ? qsTr("Enter hex bytes (e.g. 01 A2 FF)")
                                               : qsTr("Enter raw stream bytes here...")

            onTextChanged: {
              if (hexToggle.checked) {
                const formattedText = Cpp_Console_Handler.formatUserHex(text)
                const isValid = Cpp_Console_Handler.validateUserHex(formattedText)
                if (text !== formattedText)
                  text = formattedText

                color = isValid ? Cpp_ThemeManager.colors["text"]
                                : Cpp_ThemeManager.colors["alarm"]
              } else
                color = Cpp_ThemeManager.colors["text"]
            }
          }

          CheckBox {
            id: hexToggle

            text: qsTr("HEX")
          }
        }

        //
        // Reminder: extraction needs the delimiters typed into the sample too
        //
        RowLayout {
          spacing: 6
          Layout.fillWidth: true
          visible: sampleInput.text.length > 0 && !root.delimitersCovered

          Image {
            opacity: 0.7
            sourceSize: Qt.size(16, 16)
            source: "qrc:/images/tip.svg"
            Layout.alignment: Qt.AlignVCenter
          }

          Label {
            opacity: 0.7
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Cpp_ThemeManager.colors["text"]
            font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
            text: qsTr("The sample does not contain the configured frame delimiters, so no "
                       + "frame will be extracted. Type them into the sample (e.g. \\n for a "
                       + "newline) or adjust the detection mode.")
          }
        }
      }
    }

    Item {
      implicitHeight: 4
    }

    //
    // Pipeline results section
    //
    RowLayout {
      spacing: 8
      Layout.fillWidth: true

      SectionLabel {
        text: qsTr("Pipeline Results")
      }

      Item {
        Layout.fillWidth: true
      }

      Label {
        opacity: 0.7
        elide: Text.ElideLeft
        text: root.previewStats
        color: Cpp_ThemeManager.colors["text"]
        font: Cpp_Misc_CommonFonts.customUiFont(0.9, false)
      }
    }

    GroupBox {
      Layout.fillWidth: true
      Layout.fillHeight: true
      Layout.minimumHeight: 180

      background: Rectangle {
        radius: 2
        border.width: 1
        border.color: Cpp_ThemeManager.colors["groupbox_border"]
        color: Cpp_ThemeManager.colors["groupbox_background"]
      }

      ColumnLayout {
        spacing: 6
        anchors.fill: parent

        //
        // Error banner (invalid params, invalid hex, pipeline failures)
        //
        Rectangle {
          id: errorBanner

          readonly property string message: root.previewError.length > 0 ? root.previewError
                                                                         : tester.paramError

          radius: 2
          visible: message.length > 0
          Layout.fillWidth: true
          Layout.minimumHeight: visible ? errorLabel.implicitHeight + 12 : 0
          Layout.maximumHeight: Layout.minimumHeight
          color: Qt.alpha(Cpp_ThemeManager.colors["alarm"], 0.15)

          Label {
            id: errorLabel

            wrapMode: Text.WordWrap
            text: errorBanner.message
            font: Cpp_Misc_CommonFonts.uiFont
            color: Cpp_ThemeManager.colors["alarm"]

            anchors {
              margins: 8
              left: parent.left
              right: parent.right
              verticalCenter: parent.verticalCenter
            }
          }
        }

        //
        // Tree header (Stage | Value)
        //
        RowLayout {
          spacing: 8
          Layout.fillWidth: true

          Label {
            text: qsTr("Stage")
            Layout.minimumWidth: 180
            font: Cpp_Misc_CommonFonts.boldUiFont
            color: Cpp_ThemeManager.colors["text"]
          }

          Label {
            text: qsTr("Value")
            Layout.fillWidth: true
            font: Cpp_Misc_CommonFonts.boldUiFont
            color: Cpp_ThemeManager.colors["text"]
          }
        }

        //
        // Results tree
        //
        ListView {
          id: resultsTree

          clip: true
          Layout.fillWidth: true
          Layout.fillHeight: true
          model: root.treeNodes
          ScrollBar.vertical: ScrollBar {}

          //
          // Empty state
          //
          ColumnLayout {
            spacing: 8
            anchors.centerIn: parent
            visible: resultsTree.count === 0 && errorBanner.message.length === 0

            Image {
              opacity: 0.4
              sourceSize: Qt.size(48, 48)
              Layout.alignment: Qt.AlignHCenter
              source: "qrc:/icons/buttons/test.svg"
            }

            Label {
              opacity: 0.5
              wrapMode: Text.WordWrap
              Layout.alignment: Qt.AlignHCenter
              Layout.maximumWidth: resultsTree.width * 0.8
              horizontalAlignment: Text.AlignHCenter
              color: Cpp_ThemeManager.colors["text"]
              font: Cpp_Misc_CommonFonts.uiFont
              text: root.previewStats.length > 0
                    ? qsTr("Extraction did not produce a complete frame. Check the start / end "
                           + "delimiters and the detection mode.")
                    : qsTr("Enter sample data above and press Evaluate to preview the parsed "
                           + "output")
            }
          }

          delegate: Rectangle {
            id: nodeRow

            required property var modelData
            required property int index

            width: resultsTree.width
            height: 22
            color: index % 2 === 0 ? "transparent"
                                   : Cpp_ThemeManager.colors["alternate_base"]

            RowLayout {
              spacing: 8

              anchors {
                left: parent.left
                right: parent.right
                verticalCenter: parent.verticalCenter
              }

              RowLayout {
                spacing: 4
                Layout.minimumWidth: 180
                Layout.maximumWidth: 180

                Item {
                  implicitHeight: 1
                  implicitWidth: nodeRow.modelData.depth * 14
                }

                Image {
                  visible: nodeRow.modelData.expandable
                  sourceSize: Qt.size(10, 10)
                  Layout.alignment: Qt.AlignVCenter
                  source: "qrc:/icons/buttons/dropdown.svg"
                  rotation: root.expandedRows[nodeRow.modelData.key] === true ? 0 : -90
                }

                Label {
                  elide: Text.ElideRight
                  Layout.fillWidth: true
                  text: nodeRow.modelData.stage
                  color: Cpp_ThemeManager.colors["text"]
                  font: nodeRow.modelData.depth === 0 ? Cpp_Misc_CommonFonts.boldUiFont
                                                      : Cpp_Misc_CommonFonts.uiFont
                }
              }

              Label {
                elide: Text.ElideRight
                Layout.fillWidth: true
                text: nodeRow.modelData.value
                color: Cpp_ThemeManager.colors["text"]
                font: Cpp_Misc_CommonFonts.monoFont
              }
            }

            MouseArea {
              anchors.fill: parent
              enabled: nodeRow.modelData.expandable
              cursorShape: nodeRow.modelData.expandable ? Qt.PointingHandCursor : Qt.ArrowCursor
              onClicked: root.toggleRow(nodeRow.modelData.key)
            }
          }
        }
      }
    }

    //
    // Buttons: Apple HIG order (secondary left, primary at far right)
    //
    RowLayout {
      spacing: 8
      Layout.topMargin: 6
      Layout.fillWidth: true

      Widgets.IconButton {
        text: qsTr("Clear")
        onClicked: root.clearResults()
        icon.source: "qrc:/icons/buttons/clear.svg"
        enabled: sampleInput.text.length > 0 || root.treeNodes.length > 0
      }

      Item {
        Layout.fillWidth: true
      }

      Widgets.IconButton {
        text: qsTr("Close")
        onClicked: root.close()
        icon.source: "qrc:/icons/buttons/close.svg"
      }

      Widgets.IconButton {
        rightPadding: 8
        text: qsTr("Evaluate")
        onClicked: root.evaluate()
        icon.source: "qrc:/icons/buttons/media-play.svg"
      }
    }
  }
}
