# Spec 0029 — task checklist

## Wave 1 — licensing
- [x] T1 Trial ctor: drop `isActivated()` gate, always readSettings() (Trial.cpp:86)
- [x] T2 Trial::onServerReply: clear token only when variant == "Trial" (Trial.cpp:333)
- [x] T3 Remove Hobbyist tier + LicensingHandler case; drop tier compares at ~20 gate sites
      (SerialStudio, Dashboard x4, GPS, Output/Base, MDF4 Player/Export, ConnectionManager x7,
      IO MQTT x2, Sessions/Export, MQTT/Publisher, NotificationCenter, Console/Export x2)
- [x] T4 Demo GPL gate: DemoLauncher::startDemo #ifndef guard; QML entries (main.qml,
      Welcome.qml, About.qml) on Cpp_CommercialBuild; system.startDemo API refusal
- [x] T5 buildWidgetGroups: dropped datasetless grid must not shift relativeIndex of later
      data-grid groups (Dashboard.cpp:2065)
- [x] T6 Fix stale doc comment on clearLicenseCache re token re-claim ordering

## Wave 2 — operator/lock gating
- [x] T7 Export submenu !runtimeMode (DashboardCommandBindings.qml:211, StartMenu.qml:616)
- [x] T8 Shortcut delegates AND visible (MainWindow.qml:478, AppCommandBindings.qml:104)
- [x] T9 Editor palette/shortcuts blocked by lock/mode overlay (ProjectEditor.qml:148)
- [x] T10 app.fileTransmission hidden when runtime refuses (DashboardCommandBindings.qml:143)
- [x] T11 editor.save/saveAs: visible always, enabled=canSave (ProjectEditorCommandBindings.qml:97)

## Wave 3 — palette
- [x] T12 Keyboard nav scrolls highlight into view (CommandPalette.qml:208)
- [x] T13 Cmd/Ctrl+W closes palette while open (CommandPalette.qml:236)
- [x] T14 Block window shortcuts while palette open (CommandPalette.qml:243 + app flag)
- [x] T15 Catcher not hit-testable during close fade (CommandPalette.qml:39)
- [x] T16 Close palette when model swaps (MainWindow.qml:765)
- [x] T17 Editor palette section titles (PaletteModel.qml:145 extraTitle)
- [x] T18 StartMenuToggle checked from source of truth (StartMenu.qml:225)

## Wave 4 — icons + translation
- [x] T19 Fix 24 stale legacy remap targets (IconRegistryLegacy.cpp / icon-map.csv + regen)
- [x] T20 Tier mismatches: GroupsView:34 headers, FlowDiagram:405, ProjectModelWorkspaces:683
- [x] T21 workspaces.list/get remap legacy icon paths (WorkspacesHandler.cpp:733)
- [x] T22 generate-command-strings.py harvests DatabaseExplorer manifests
- [x] T23 collapsedTitle translated (CommandRegistry.cpp:396)
- [x] T24 ThemeManager theme-name localization ar/he/vi (ThemeManager.cpp:460)

## Verify
- [x] V1 scripts/code-verify.py --check on touched C++/QML
- [x] V2 scripts/registry-verify.py
- [x] V3 self-review diff vs spec
