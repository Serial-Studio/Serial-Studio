# Appearance and Preferences

How the application looks and where its settings live. No command sets
the theme, the icon pack or the language: the user changes them in the
Preferences dialog. Name the tab and the row, and when asked which option
is best, give one recommendation with a reason. Theme and icon pack are
per-computer preferences, so they never change a project file.

## Themes

**Preferences > General > Theme.** The bundled themes are Default, Fluent
Light, Fluent Dark, Millennium Light, Millennium Dark and Starliner.
**Get Themes…** beside the selector opens the extension catalog filtered
to themes.

Millennium Light, Millennium Dark and Starliner are material themes: metal
panel surfaces, with LED Panel indicators drawn as panel lamps. Choose one
for a control-panel look. Default and the two Fluent themes are flatter
and quieter.

## Icon packs

**Preferences > General > Icon Pack.** The change applies immediately,
without a restart. It restyles the colour icons of the toolbar, the
Project Editor, the dashboard and the dialogs. Button glyphs and window
controls stay the same under every pack.

| Choice            | Look                               | Theme that names it               |
|-------------------|------------------------------------|-----------------------------------|
| Office (default)  | Flat colour icons, the standard set | Default                          |
| Flat Color        | Flat multicolour icons             | none                              |
| Fluent            | Microsoft Fluent colour icons      | Fluent Light, Fluent Dark         |
| Deco              | Decorative colour icons            | Millennium Light, Millennium Dark |
| Glass             | Coloured glass                     | none                              |
| Glass Translucent | Lighter, more transparent glass    | none                              |
| Fine Line         | Thin monochrome line art           | Starliner                         |
| Outline           | Regular-weight monochrome outlines | none                              |

**Follow theme**, the default choice, uses the pack the current theme
names, or Office when it names none. Fine Line and Outline are mono packs,
drawn in the theme's text colour, so their contrast follows the theme.

How to recommend one:

- Stay on **Follow theme** unless the user dislikes the pairing. The pack
  was chosen for the theme and changes with it.
- For dense telemetry dashboards, where colour should mean alarm state,
  recommend a mono pack: Outline, or Fine Line for a lighter look.
- For finding commands quickly by colour, recommend Office or Fluent.
- Deco, Glass and Glass Translucent are the most decorative. Recommend them
  when the user wants the look and the dashboard is not crowded.

## Where settings live

| Tab           | Rows |
|---------------|------|
| General       | Language, Theme, Icon Pack, Custom Window Decorations, Workspace Folder, the API Server (enable, port, external connections, access token), Export Protobuf File |
| Startup       | Rendering Backend, Display Scaling, HDR Output, SIMD Instruction Set, Reduce Motion, Apply Performance Hints, Keep Display Awake, update checks |
| Plotting      | Time Range, Point Count, UI Refresh Rate, Ask Devices for Their Dashboard (Quick Plot) |
| Layout        | Dashboard font, Layout Margin, Layout Spacing, Show Actions Panel, Auto-Hide Toolbar, Show Alignment Guides |
| Taskbar       | Always Show Taskbar Buttons, Show Search Field, Auto-hide Taskbar, Hide Delay, Pinned Buttons |
| Console       | Display Mode, font, Scrollback Lines, Show Timestamps, Line Ending, Input Mode, Text Encoding, Checksum, Echo Sent Data, VT100 Emulation, ANSI Colors |
| Export        | CSV Row Interval, Video Export |
| Sounds        | Enable Sounds, Volume, Output Device, the alarm sequence and sounds (load the `alarms` skill) |
| Notifications | System Notifications, Application Logs, Route Warnings to Notifications |

**HDR Output** appears only on macOS with Metal and on Windows with
Direct3D 11, and needs a restart. It lets lit elements such as panel lamps
glow brighter than the rest of the interface, and a display with no HDR
headroom looks unchanged. **Reduce Motion** turns off the scaling and
sliding animations; fades stay.

Every row and its default is in `meta.fetchHelp{path:"Preferences"}`. For
anything else about the interface, call `meta.searchDocs{query}`, which
covers the help manual.
