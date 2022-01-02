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

import QtQuick 2.3
import Qt.labs.platform 1.0

MenuBar {
    //
    // File menu
    //
    Menu {
        title: qsTr("File")

        MenuItem {
            shortcut: "ctrl+j"
            text: qsTr("Select JSON file") + "..."
            onTriggered: Cpp_JSON_Generator.loadJsonMap()
        }

        MenuSeparator {}

        Menu {
            title: qsTr("CSV export")

            MenuItem {
                checkable: true
                text: qsTr("Enable CSV export")
                checked: Cpp_CSV_Export.exportEnabled
                onTriggered: Cpp_CSV_Export.exportEnabled = checked
            }

            MenuItem {
                shortcut: "ctrl+shift+o"
                enabled: Cpp_CSV_Export.isOpen
                text: qsTr("Show CSV in explorer")
                onTriggered: Cpp_CSV_Export.openCurrentCsv()
            }
        }

        MenuItem {
            shortcut: "ctrl+o"
            text: qsTr("Replay CSV") + "..."
            onTriggered: Cpp_CSV_Player.openFile()
            enabled: Cpp_JSON_Generator.operationMode === 0
        }

        MenuSeparator {}

        MenuItem {
            shortcut: StandardKey.Print
            text: qsTr("Print") + "..."
            enabled: Cpp_IO_Console.saveAvailable
            onTriggered: Cpp_IO_Console.print(app.monoFont)
        }

        MenuItem {
            shortcut: StandardKey.Save
            onTriggered: Cpp_IO_Console.save()
            enabled: Cpp_IO_Console.saveAvailable
            text: qsTr("Export console output") + "..."
        }

        MenuSeparator {}

        MenuItem {
            text: qsTr("Quit")
            onTriggered: Qt.quit()
            shortcut: StandardKey.Quit
        }
    }

    //
    // Edit menu
    //
    Menu {
        title: qsTr("Edit")

        MenuItem {
            text: qsTr("Copy")
            shortcut: StandardKey.Copy
            onTriggered: mainWindow.consoleCopy()
        }

        MenuItem {
            shortcut: StandardKey.SelectAll
            text: qsTr("Select all") + "..."
            onTriggered: mainWindow.consoleSelectAll()
        }

        MenuItem {
            shortcut: StandardKey.Delete
            onTriggered: mainWindow.consoleClear()
            text: qsTr("Clear console output")
        }

        MenuSeparator{}

        Menu {
            title: qsTr("Communication mode")

            MenuItem {
                checkable: true
                text: qsTr("Device sends JSON")
                checked: Cpp_JSON_Generator.operationMode === 1
                onTriggered: Cpp_JSON_Generator.operationMode = checked ? 1 : 0
            }

            MenuItem {
                checkable: true
                text: qsTr("Load JSON from computer")
                checked: Cpp_JSON_Generator.operationMode === 0
                onTriggered: Cpp_JSON_Generator.operationMode = checked ? 0 : 1
            }
        }
    }

    //
    // View menu
    //
    Menu {
        title: qsTr("View")

        MenuItem {
            checkable: true
            shortcut: "ctrl+t"
            text: qsTr("Console")
            checked: mainWindow.consoleVisible
            onTriggered: mainWindow.showConsole()
            onCheckedChanged: {
                if (mainWindow.consoleVisible !== checked)
                    checked = mainWindow.consoleVisible
            }
        }

        MenuItem {
            checkable: true
            shortcut: "ctrl+d"
            text: qsTr("Dashboard")
            checked: mainWindow.dashboardVisible
            enabled: Cpp_UI_Dashboard.available
            onTriggered: mainWindow.showDashboard()
            onCheckedChanged: {
                if (mainWindow.dashboardVisible !== checked)
                    checked = mainWindow.dashboardVisible
            }
        }

        MenuSeparator {}

        MenuItem {
            checkable: true
            shortcut: "ctrl+,"
            checked: mainWindow.setupVisible
            text: qsTr("Show setup pane")
            onTriggered: mainWindow.showSetup()
        }

        MenuSeparator {}

        DecentMenuItem {
            sequence: StandardKey.FullScreen
            onTriggered: mainWindow.toggleFullscreen()
            text: mainWindow.fullScreen ? qsTr("Exit full screen") : qsTr("Enter full screen")
        }
    }

    //
    // Console format
    //
    Menu {
        title: qsTr("Console")

        MenuItem {
            checkable: true
            text: qsTr("Autoscroll")
            checked: Cpp_IO_Console.autoscroll
            onTriggered: Cpp_IO_Console.autoscroll = checked
        }

        MenuItem {
            checkable: true
            text: qsTr("Show timestamp")
            checked: Cpp_IO_Console.showTimestamp
            onTriggered: Cpp_IO_Console.showTimestamp = checked
        }

        MenuItem {
            checkable: true
            checked: mainWindow.vt100emulation
            text: qsTr("VT-100 emulation")
            onTriggered: mainWindow.vt100emulation = checked
        }

        MenuItem {
            checkable: true
            text: qsTr("Echo user commands")
            checked: Cpp_IO_Console.echo
            onTriggered: Cpp_IO_Console.echo = checked
        }

        MenuSeparator{}

        Menu {
            title: qsTr("Display mode")

            MenuItem {
                checkable: true
                text: qsTr("Normal (plain text)")
                checked: Cpp_IO_Console.displayMode === 0
                onTriggered: Cpp_IO_Console.displayMode = checked ? 0 : 1
            }

            MenuItem {
                checkable: true
                text: qsTr("Binary (hexadecimal)")
                checked: Cpp_IO_Console.displayMode === 1
                onTriggered: Cpp_IO_Console.displayMode = checked ? 1 : 0
            }
        }

        Menu {
            title: qsTr("Line ending character")

            MenuItem {
                checkable: true
                text: Cpp_IO_Console.lineEndings()[0]
                checked: Cpp_IO_Console.lineEnding === 0
                onTriggered: Cpp_IO_Console.lineEnding = 0
            }

            MenuItem {
                checkable: true
                text: Cpp_IO_Console.lineEndings()[1]
                checked: Cpp_IO_Console.lineEnding === 1
                onTriggered: Cpp_IO_Console.lineEnding = 1
            }

            MenuItem {
                checkable: true
                text: Cpp_IO_Console.lineEndings()[2]
                checked: Cpp_IO_Console.lineEnding === 2
                onTriggered: Cpp_IO_Console.lineEnding = 2
            }

            MenuItem {
                checkable: true
                text: Cpp_IO_Console.lineEndings()[3]
                checked: Cpp_IO_Console.lineEnding === 3
                onTriggered: Cpp_IO_Console.lineEnding = 3
            }
        }
    }

    //
    // Help menu
    //
    Menu {
        title: qsTr("Help")

        MenuItem {
            onTriggered: app.aboutDialog.show()
            text: qsTr("About %1").arg(Cpp_AppName)
        }

        MenuItem {
            text: qsTr("About %1").arg("Qt")
            onTriggered: Cpp_Misc_Utilities.aboutQt()
        }

        MenuSeparator {
            visible: Cpp_UpdaterEnabled
            enabled: Cpp_UpdaterEnabled
        }

        MenuItem {
            checkable: true
            visible: Cpp_UpdaterEnabled
            enabled: Cpp_UpdaterEnabled
            checked: mainWindow.automaticUpdates
            onTriggered: mainWindow.automaticUpdates = checked
            text: qsTr("Auto-updater")
        }

        MenuItem {
            visible: Cpp_UpdaterEnabled
            enabled: Cpp_UpdaterEnabled
            onTriggered: app.checkForUpdates()
            text: qsTr("Check for updates") + "..."
        }

        MenuSeparator{}

        MenuItem {
            text: qsTr("Project website") + "..."
            onTriggered: Qt.openUrlExternally("https://www.alex-spataru.com/serial-studio")
        }

        MenuItem {
            shortcut: StandardKey.HelpContents
            text: qsTr("Documentation/wiki") + "..."
            onTriggered: Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/wiki")
        }

        MenuSeparator{}

        MenuItem {
            text: qsTr("Report bug") + "..."
            onTriggered: Qt.openUrlExternally("https://github.com/Serial-Studio/Serial-Studio/issues")
        }
    }
}
