/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

import QtQuick 2.12
import QtQuick.Controls 2.12

MenuBar {
    id: root
    visible: mw.menubarEnabled

    //
    // Set background color + border
    //
    background: Rectangle {
        gradient: Gradient {
            GradientStop {
                position: 0
                color: Cpp_ThemeManager.menubarGradient1
            }

            GradientStop {
                position: 1
                color: Cpp_ThemeManager.menubarGradient2
            }
        }
    }

    //
    // Set this component as the application's default menubar upon creation
    //
    Component.onCompleted: mw.menuBar = this

    //
    // File menu
    //
    Menu {
        title: qsTr("File")

        DecentMenuItem {
            sequence: "ctrl+j"
            text: qsTr("Select JSON file") + "..."
            onTriggered: Cpp_JSON_Generator.loadJsonMap()
        }

        MenuSeparator {}

        Menu {
            title: qsTr("CSV export")

            DecentMenuItem {
                checkable: true
                text: qsTr("Enable CSV export")
                checked: Cpp_CSV_Export.exportEnabled
                onTriggered: Cpp_CSV_Export.exportEnabled = checked
            }

            DecentMenuItem {
                sequence: "ctrl+shift+o"
                enabled: Cpp_CSV_Export.isOpen
                text: qsTr("Show CSV in explorer")
                onTriggered: Cpp_CSV_Export.openCurrentCsv()
            }
        }

        DecentMenuItem {
            sequence: "ctrl+o"
            text: qsTr("Replay CSV") + "..."
            onTriggered: Cpp_CSV_Player.openFile()
            enabled: Cpp_JSON_Generator.operationMode == 0
        }

        MenuSeparator {}

        DecentMenuItem {
            sequence: StandardKey.Print
            text: qsTr("Print") + "..."
            enabled: Cpp_IO_Console.saveAvailable
            onTriggered: Cpp_IO_Console.print(app.monoFont)
        }

        DecentMenuItem {
            sequence: StandardKey.Save
            onClicked: Cpp_IO_Console.save()
            enabled: Cpp_IO_Console.saveAvailable
            text: qsTr("Export console output") + "..."
        }

        MenuSeparator {}

        DecentMenuItem {
            text: qsTr("Quit")
            onTriggered: Qt.quit()
            sequence: StandardKey.Quit
        }
    }

    //
    // Edit menu
    //
    Menu {
        title: qsTr("Edit")

        DecentMenuItem {
            text: qsTr("Copy")
            sequence: StandardKey.Copy
            onTriggered: mw.consoleCopy()
        }

        DecentMenuItem {
            sequence: StandardKey.SelectAll
            text: qsTr("Select all") + "..."
            onTriggered: mw.consoleSelectAll()
        }

        DecentMenuItem {
            sequence: StandardKey.Delete
            onTriggered: mw.consoleClear()
            text: qsTr("Clear console output")
        }

        MenuSeparator{}

        Menu {
            title: qsTr("Communication mode")

            DecentMenuItem {
                checkable: true
                text: qsTr("Device sends JSON")
                checked: Cpp_JSON_Generator.operationMode == 1
                onTriggered: Cpp_JSON_Generator.operationMode = checked ? 1 : 0
            }

            DecentMenuItem {
                checkable: true
                text: qsTr("Load JSON from computer")
                checked: Cpp_JSON_Generator.operationMode == 0
                onTriggered: Cpp_JSON_Generator.operationMode = checked ? 0 : 1
            }
        }
    }

    //
    // View menu
    //
    Menu {
        title: qsTr("View")

        DecentMenuItem {
            checkable: true
            sequence: "ctrl+t"
            text: qsTr("Console")
            checked: mw.consoleVisible
            onTriggered: mw.showConsole()
            onCheckedChanged: {
                if (mw.consoleVisible !== checked)
                    checked = mw.consoleVisible
            }
        }

        DecentMenuItem {
            checkable: true
            sequence: "ctrl+d"
            text: qsTr("Dashboard")
            checked: mw.dashboardVisible
            enabled: Cpp_UI_Dashboard.available
            onTriggered: mw.showDashboard()
            onCheckedChanged: {
                if (mw.dashboardVisible !== checked)
                    checked = mw.dashboardVisible
            }
        }

        MenuSeparator {}

        DecentMenuItem {
            checkable: true
            sequence: "ctrl+,"
            checked: mw.setupVisible
            text: qsTr("Show setup pane")
            onTriggered: mw.showSetup()
        }

        MenuSeparator {}

        DecentMenuItem {
            sequence: "alt+m"
            onTriggered: mw.toggleMenubar()
            text: root.visible ? qsTr("Hide menubar") :
                                 qsTr("Show menubar")
        }

        MenuSeparator {}

        DecentMenuItem {
            sequence: StandardKey.FullScreen
            onTriggered: mw.toggleFullscreen()
            text: mw.fullScreen ? qsTr("Exit full screen") :
                                   qsTr("Enter full screen")
        }
    }

    //
    // Console format
    //
    Menu {
        title: qsTr("Console")

        DecentMenuItem {
            checkable: true
            text: qsTr("Autoscroll")
            checked: Cpp_IO_Console.autoscroll
            onTriggered: Cpp_IO_Console.autoscroll = checked
        }

        DecentMenuItem {
            checkable: true
            text: qsTr("Show timestamp")
            checked: Cpp_IO_Console.showTimestamp
            onTriggered: Cpp_IO_Console.showTimestamp = checked
        }

        DecentMenuItem {
            checkable: true
            checked: mw.vt100emulation
            text: qsTr("VT-100 emulation")
            onTriggered: mw.vt100emulation = checked
        }

        DecentMenuItem {
            checkable: true
            text: qsTr("Echo user commands")
            checked: Cpp_IO_Console.echo
            onTriggered: Cpp_IO_Console.echo = checked
        }

        MenuSeparator{}

        Menu {
            title: qsTr("Display mode")

            DecentMenuItem {
                checkable: true
                text: qsTr("Normal (plain text)")
                checked: Cpp_IO_Console.displayMode == 0
                onTriggered: Cpp_IO_Console.displayMode = checked ? 0 : 1
            }

            DecentMenuItem {
                checkable: true
                text: qsTr("Binary (hexadecimal)")
                checked: Cpp_IO_Console.displayMode == 1
                onTriggered: Cpp_IO_Console.displayMode = checked ? 1 : 0
            }
        }

        Menu {
            title: qsTr("Line ending character")

            DecentMenuItem {
                checkable: true
                text: Cpp_IO_Console.lineEndings()[0]
                checked: Cpp_IO_Console.lineEnding === 0
                onTriggered: Cpp_IO_Console.lineEnding = 0
            }

            DecentMenuItem {
                checkable: true
                text: Cpp_IO_Console.lineEndings()[1]
                checked: Cpp_IO_Console.lineEnding == 1
                onTriggered: Cpp_IO_Console.lineEnding = 1
            }

            DecentMenuItem {
                checkable: true
                text: Cpp_IO_Console.lineEndings()[2]
                checked: Cpp_IO_Console.lineEnding == 2
                onTriggered: Cpp_IO_Console.lineEnding = 2
            }

            DecentMenuItem {
                checkable: true
                text: Cpp_IO_Console.lineEndings()[3]
                checked: Cpp_IO_Console.lineEnding == 3
                onTriggered: Cpp_IO_Console.lineEnding = 3
            }
        }
    }

    //
    // Help menu
    //
    Menu {
        title: qsTr("Help")

        DecentMenuItem {
            onTriggered: app.showAbout()
            text: qsTr("About %1").arg(Cpp_AppName)
        }

        DecentMenuItem {
            text: qsTr("About %1").arg("Qt")
            onTriggered: Cpp_Misc_Utilities.aboutQt()
        }

        MenuSeparator {
            visible: Cpp_UpdaterEnabled
            enabled: Cpp_UpdaterEnabled
        }

        DecentMenuItem {
            checkable: true
            visible: Cpp_UpdaterEnabled
            enabled: Cpp_UpdaterEnabled
            checked: mw.automaticUpdates
            onTriggered: mw.automaticUpdates = checked
            text: qsTr("Auto-updater")
        }

        DecentMenuItem {
            visible: Cpp_UpdaterEnabled
            enabled: Cpp_UpdaterEnabled
            onTriggered: app.checkForUpdates()
            text: qsTr("Check for updates") + "..."
        }

        MenuSeparator{}

        DecentMenuItem {
            text: qsTr("Project website") + "..."
            onTriggered: Qt.openUrlExternally("https://www.alex-spataru.com/serial-studio")
        }

        DecentMenuItem {
            sequence: StandardKey.HelpContents
            text: qsTr("Documentation/wiki") + "..."
            onTriggered: Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/wiki")
        }

        MenuSeparator{}

        DecentMenuItem {
            text: qsTr("Report bug") + "..."
            onTriggered: Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/issues")
        }
    }
}
