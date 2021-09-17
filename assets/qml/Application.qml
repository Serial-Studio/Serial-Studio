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
import QtQuick.Dialogs 1.1
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

import Qt.labs.settings 1.0

import "Windows"

ApplicationWindow {
    id: app

    //
    // Global properties
    //
    readonly property int spacing: 8
    property bool firstValidPacket: false
    readonly property string monoFont: qmlMain.monoFont

    //
    // We use this variable to ask the user if he/she wants to enable/disable
    // automatic update checking on the second run
    //
    property int appLaunchCount: 0
    property bool automaticUpdates: false

    //
    // Hacks to fix window maximized behavior
    //
    property bool firstChange: true
    property bool windowMaximized: false
    onVisibilityChanged: {
        if (visibility == Window.Maximized) {
            if (!windowMaximized)
                firstChange = false

            windowMaximized = true
            fullScreen = false
        }

        else if (visibility === Window.FullScreen) {
            if (!fullScreen)
                firstChange = false

            windowMaximized = false
            fullScreen = true
        }

        else if (visibility !== Window.Hidden) {
            if (windowMaximized || fullScreen && firstChange) {
                app.x = 100
                app.y = 100
                app.width = app.minimumWidth
                app.height = app.minimumHeight
            }

            fullScreen = false
            windowMaximized = false
        }

    }

    //
    // Application UI status variables (used for the menubar)
    //
    property alias vt100emulation: terminal.vt100emulation
    readonly property bool setupVisible: setup.visible
    readonly property bool dashboardVisible: data.visible
    readonly property bool widgetsVisible: widgets.visible
    readonly property bool consoleVisible: terminal.visible
    readonly property bool dashboardAvailable: Cpp_UI_Provider.groupCount > 0
    readonly property bool widgetsAvailable: Cpp_UI_WidgetProvider.totalWidgetCount > 0

    //
    // Menubar status
    //
    property bool menubarEnabled: true

    //
    // Fullscreen status
    //
    property bool fullScreen: false

    //
    // Check for updates (non-silent mode)
    //
    function checkForUpdates() {
        Cpp_Updater.setNotifyOnFinish(Cpp_AppUpdaterUrl, true)
        Cpp_Updater.checkForUpdates(Cpp_AppUpdaterUrl)
    }

    //
    // Display about dialog
    //
    function showAbout() {
        about.show()
    }

    //
    // Display the console
    //
    function showConsole() {
        toolbar.consoleClicked()
    }

    //
    // Display the dashboard
    //
    function showDashboard() {
        toolbar.dataClicked()
    }

    //
    // Display the widgets
    //
    function showWidgets() {
        toolbar.widgetsClicked()
    }

    //
    // Toggle preferences pane
    //
    function togglePreferences() {
        toolbar.setupClicked()
    }

    //
    // Clears console output
    //
    function clearConsole() {
        terminal.clearConsole()
    }

    //
    // Copy console selection
    //
    function copyConsole() {
        terminal.copy()
    }

    //
    // Hide/show menubar
    //
    function toggleMenubar() {
        app.menubarEnabled = !app.menubarEnabled
    }

    //
    // Select all console text
    //
    function selectAllConsole() {
        terminal.selectAll()
    }

    //
    // Toggle fullscreen state
    //
    function toggleFullscreen() {
        app.fullScreen = !app.fullScreen
        if (app.fullScreen)
            app.showFullScreen()
        else
            app.showNormal()
    }

    //
    // Show donations dialog
    //
    function showDonationsDialog() {
        donations.show()
    }

    //
    // Window geometry (different minimum size for non-macOS
    // operating systems because of the global menubar in macOS)
    //
    visible: false
    minimumWidth: 1100
    title: Cpp_AppName
    minimumHeight: Qt.platform.os == "osx" ? 720 : 740

    //
    // Set user interface font
    //
    font.family: qmlMain.uiFont
    
    //
    // Define default window size to avoid issues with
    // special window managers (such as Haiku)
    //
    width: minimumWidth
    height: minimumHeight

    //
    // Theme options
    //
    palette.text: Cpp_ThemeManager.text
    palette.buttonText: Cpp_ThemeManager.text
    palette.windowText: Cpp_ThemeManager.text
    palette.window: Cpp_ThemeManager.windowBackground
    background: Rectangle {
        color: Cpp_ThemeManager.windowBackground
    }

    //
    // Startup code
    //
    Component.onCompleted: {
        // Hide dialogs, show devices pane
        about.hide()
        setup.show()
        csvPlayer.hide()

        // Hide everything except the terminal
        data.opacity = 0
        widgets.opacity = 0
        terminal.opacity = 1

        // Load welcome guide
        terminal.showWelcomeGuide()

        // Load JSON map file (if any)
        Cpp_JSON_Generator.readSettings()

        // Display the window & check for updates in 500 ms (we do this so that
        // we wait for the window to read settings before showing it)
        timer.start()
    }

    //
    // Application menubar loader (we need to use a different version in macOS)
    //
    Loader {
        asynchronous: false
        source: {
            if (Qt.platform.os === "osx")
                return "qrc:/qml/PlatformDependent/MenubarMacOS.qml"

            return "qrc:/qml/PlatformDependent/Menubar.qml"
        }
    }

    //
    // Startup timer
    //
    Timer {
        id: timer
        interval: 500
        onTriggered: {
            // Startup verifications to ensure that bad settings
            // do not make our app reside outside screen
            if (x < 0 || x >= Screen.desktopAvailableWidth)
                x = 100
            if (y < 0 || y >= Screen.desktopAvailableHeight)
                y = 100

            // Startup verifications to ensure that app fits in current screen
            if (width > Screen.desktopAvailableWidth) {
                x = 100
                width = Screen.desktopAvailableWidth - x
            }

            // Startup verifications to ensure that app fits in current screen
            if (height > Screen.desktopAvailableHeight) {
                y = 100
                height = Screen.desktopAvailableHeight - y
            }

            // Show app window
            if (app.fullScreen)
                app.showFullScreen()
            else if (app.windowMaximized)
                app.showMaximized()
            else
                app.showNormal()

            // Increment app launch count
            ++appLaunchCount

            // Show donations dialog every 15 launches
            if (appLaunchCount % 15 == 0 && !donations.doNotShowAgain)
                donations.showAutomatically()

            // Ask user if he/she wants to enable automatic updates
            if (appLaunchCount == 2)
                automaticUpdatesMessageDialog.visible = Cpp_UpdaterEnabled

            // Check for updates (if we are allowed)
            if (automaticUpdates && Cpp_UpdaterEnabled)
                Cpp_Updater.checkForUpdates(Cpp_AppUpdaterUrl)
        }
    }

    //
    // Hide console & device manager when we receive first valid packet
    //
    Connections {
        target: Cpp_UI_Provider
        enabled: !app.firstValidPacket
        function onUpdated()  {
            if ((Cpp_IO_Manager.connected || Cpp_CSV_Player.isOpen) && Cpp_UI_Provider.frameValid()) {
                app.firstValidPacket = true
                setup.hide()
                app.showDashboard()
            } else {
                toolbar.consoleClicked()
                setup.show()
                app.firstValidPacket = false
            }
        }
    }

    //
    // Show console tab on serial disconnect
    //
    Connections {
        target: Cpp_UI_Provider
        function onDataReset() {
            toolbar.consoleClicked()
            setup.show()
            app.firstValidPacket = false
        }
    }

    //
    // Save window size & position
    //
    Settings {
        property alias appX: app.x
        property alias appY: app.y
        property alias appW: app.width
        property alias appH: app.height
        property alias appStatus: app.appLaunchCount
        property alias windowFullScreen: app.fullScreen
        property alias autoUpdater: app.automaticUpdates
        property alias appMaximized: app.windowMaximized
        property alias menubarVisible: app.menubarEnabled
    }

    //
    // Main layout
    //
    ColumnLayout {
        spacing: 0
        anchors.fill: parent

        Toolbar {
            z: 1
            id: toolbar
            Layout.fillWidth: true
            Layout.minimumHeight: 48
            Layout.maximumHeight: 48
            setupChecked: app.setupVisible
            dataChecked: app.dashboardVisible
            widgetsChecked: app.widgetsVisible
            consoleChecked: app.consoleVisible
            onSetupClicked: setup.visible ? setup.hide() : setup.show()

            onDataClicked: {
                if (app.dashboardAvailable) {
                    data.opacity     = 1
                    terminal.opacity = 0
                    widgets.opacity  = 0
                    dataChecked      = true
                    consoleChecked   = false
                    widgetsChecked   = false
                }

                else
                    app.showConsole()
            }

            onConsoleClicked: {
                data.opacity     = 0
                terminal.opacity = 1
                widgets.opacity  = 0
                consoleChecked   = true
                dataChecked      = false
                widgetsChecked   = false
            }

            onWidgetsClicked: {
                if (app.widgetsAvailable) {
                    data.opacity     = 0
                    terminal.opacity = 0
                    widgets.opacity  = 1
                    dataChecked      = false
                    widgetsChecked   = true
                    consoleChecked   = false
                }

                else
                    app.showConsole()
            }
        }

        RowLayout {
            spacing: 0
            clip: true
            Layout.fillWidth: true
            Layout.fillHeight: true

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true

                Console {
                    id: terminal
                    anchors.fill: parent

                    // Animate on show
                    enabled: opacity > 0
                    visible: opacity > 0
                    Behavior on opacity {NumberAnimation{}}

                    // Show translated welcome text on lang. change
                    Connections {
                        target: Cpp_Misc_Translator
                        function onLanguageChanged() {
                            terminal.showWelcomeGuide()
                        }
                    }
                }

                DataGrid {
                    id: data
                    anchors.fill: parent

                    // Animate on show
                    visible: opacity > 0
                    Behavior on opacity {NumberAnimation{}}
                }

                Widgets {
                    id: widgets
                    anchors.fill: parent

                    // Animate on show
                    enabled: opacity > 0
                    visible: opacity > 0
                    Behavior on opacity {NumberAnimation{}}
                }
            }

            Setup {
                id: setup
                property int displayedWidth: 340 + app.spacing * 1.5

                function show() {
                    opacity = 1
                    displayedWidth = 340 + app.spacing * 1.5
                }

                function hide() {
                    opacity = 0
                    displayedWidth = 0
                }

                visible: opacity > 0
                Layout.fillHeight: true
                Layout.minimumWidth: displayedWidth
                Layout.maximumWidth: displayedWidth

                Behavior on opacity {NumberAnimation{}}
                Behavior on displayedWidth {NumberAnimation{}}
            }
        }
    }

    //
    // About window
    //
    About {
        id: about
    }

    //
    // CSV player window
    //
    CsvPlayer {
        id: csvPlayer
    }

    //
    // Enable/disable automatic updates dialog
    //
    MessageDialog {
        id: automaticUpdatesMessageDialog

        title: Cpp_AppName
        icon: StandardIcon.Question
        modality: Qt.ApplicationModal
        standardButtons: StandardButton.Yes | StandardButton.No
        text: "<h3>" + qsTr("Check for updates automatically?") + "</h3>"
        informativeText: qsTr("Should %1 automatically check for updates? " +
                              "You can always check for updates manually from " +
                              "the \"Help\" menu").arg(Cpp_AppName);

        // Behavior when the user clicks on "Yes"
        onAccepted: {
            app.automaticUpdates = true
            Cpp_Updater.checkForUpdates(Cpp_AppUpdaterUrl)
        }

        // Behavior when the user clicks on "No"
        onRejected: {
            app.automaticUpdates = false
        }
    }

    //
    // Donations dialog
    //
    Donate {
        id: donations
    }

    //
    // Intuitive UI stuff for drag and drop
    //
    Rectangle {
        id: dropRectangle

        function hide() {
            rectTimer.start()
        }

        opacity: 0
        border.width: 1
        color: palette.highlight
        anchors.centerIn: parent
        border.color: Cpp_ThemeManager.text
        width: dropLayout.implicitWidth + 6 * app.spacing
        height: dropLayout.implicitHeight + 6 * app.spacing


        ColumnLayout {
            id: dropLayout
            spacing: app.spacing * 2
            anchors.centerIn: parent

            ToolButton {
                flat: true
                enabled: false
                icon.width: 128
                icon.height: 128
                icon.color: Cpp_ThemeManager.text
                Layout.alignment: Qt.AlignHCenter
                icon.source: "qrc:/icons/drag-drop.svg"
            }

            Label {
                font.bold: true
                font.pixelSize: 24
                color: Cpp_ThemeManager.text
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Drop JSON and CSV files here")
            }
        }

        Timer {
            id: rectTimer
            interval: 200
            onTriggered: dropRectangle.opacity = 0
        }

        Behavior on opacity {NumberAnimation{}}
    }

    //
    // File drop area
    //
    DropArea {
        id: dropArea
        anchors.fill: parent
        enabled: terminal.visible

        //
        // Show rectangle and set color based on file extension on drag enter
        //
        onEntered: {
            // Get file name & set color of rectangle accordingly
            var path = drag.urls[0].toString()
            if (path.endsWith(".json") || path.endsWith(".csv")) {
                drag.accept(Qt.LinkAction)
                dropRectangle.color = Qt.darker(palette.highlight, 1.4)
            }

            // Invalid file name, show red rectangle
            else
                dropRectangle.color = Cpp_ThemeManager.alternativeHighlight

            // Show drag&drop rectangle
            dropRectangle.opacity = 0.8
        }

        //
        // Open *.json & *.csv files on drag drop
        //
        onDropped: {
            // Hide rectangle
            dropRectangle.hide()
            
            // Get dropped file URL and remove prefixed "file://"
            var path = drop.urls[0].toString()
            if (Qt.platform.os != "windows")
                path = path.replace(/^(file:\/{2})/,"");
            else
                path = path.replace(/^(file:\/{3})/,"");

            // Unescape html codes like '%23' for '#'
            var cleanPath = decodeURIComponent(path);

            // Process JSON files
            if (cleanPath.endsWith(".json")) {
                Cpp_JSON_Generator.setOperationMode(0)
                Cpp_JSON_Generator.loadJsonMap(cleanPath, false)
            }

            // Process CSV files
            else if (cleanPath.endsWith(".csv"))
                Cpp_CSV_Player.openFile(cleanPath)
        }

        //
        // Hide drag & drop rectangle on drag exit
        //
        onExited: {
            dropRectangle.hide()
        }
    }
}
