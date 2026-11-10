# Preferences

The plot scrolls past too fast, the text is too small, or you want a dark theme. Preferences is where all of that lives. It is available in every edition; the **Notifications** tab and **Save Videos by Default** are Pro only.

Click **Preferences** on the main toolbar, click **Settings** on the dashboard taskbar (the same dialog), or search for Preferences in the [command palette](Command-Palette.md). On macOS the shortcut is **Cmd+,**.

To make a plot show the last 30 seconds, open the **Plotting** tab and choose 30 s from the **Time Range** list. For bigger text, open **Layout** and change **Font Size**; for a dark theme, pick one under **Theme** on the **General** tab.

Changes apply as you make them, so **Apply** and **Close** both just close the dialog. These settings need a restart:

- **Rendering Backend**, **Display Scaling**, **Custom Scale (%)**, and **HDR Output**, all on the **Startup** tab. Serial Studio prompts for the restart when you change one.
- **Custom Window Decorations** (**General** tab), **Apply Performance Hints**, and **Keep Display Awake** (**Startup** tab). They take effect the next time Serial Studio starts, with no prompt.

## Reset

**Reset**, at the bottom left of the dialog, restores defaults on every tab at once, except the settings listed below. It turns **Enable API Server** and **Allow External API Connections** off, sets **Theme** to Default and **Custom Scale (%)** to 100, and returns **Rendering Backend** to Automatic and **Display Scaling** to Fractional where the platform lets you change them. It leaves these as they are:

- **Language**, **Icon Pack**, **Widget Toolbar Icons**, and **Workspace Folder**
- **HDR Output**, **SIMD Instruction Set**, and **API Server Port**
- **Row Interval (ms)** and the two extension update switches

## General

| Setting | What it does | Default |
|---------|--------------|---------|
| **Language** | Interface language. Disabled while a device is connected. | The system language |
| **Theme** | Colors and surfaces of the whole interface. **Get Themes…** opens the [extension catalog](Extensions.md) filtered to themes. | Default |
| **Icon Pack** | Artwork of the color icons on the toolbar, the Project Editor, the dashboard, and the dialogs. **Follow theme** uses the pack the theme names, or the standard set when it names none. **Office** pins the standard set; a build without it shows **Basic** there instead. | Follow theme |
| **Widget Toolbar Icons** | Artwork of the small buttons on dashboard widgets, chosen separately from **Icon Pack**. **Follow theme** uses the set the theme names, or the standard set when it names none. Every other entry draws that set's own artwork. | Follow theme |
| **Custom Window Decorations** | Draws Serial Studio's own window title bars. Not offered on macOS or Windows 11. Takes effect after restart. | On |
| **Workspace Folder** | Where Serial Studio keeps its files. Disabled while a device is connected. | `Documents/Serial Studio` |
| **Enable API Server** | Lets a script you write, for example in Python, read data from and control Serial Studio over TCP. Leave it off unless you need it. See the [API Reference](API-Reference.md). | Off |
| **API Server Port** | TCP port of the API server, 1024 to 65535. Needs **Enable API Server** on. | 7777 |
| **Allow External API Connections** | Accepts connections from other computers. Needs **Enable API Server** on. | Off |

With **Enable API Server** and **Allow External API Connections** both on, **API Access Token** becomes available. It is the token external clients authenticate with, and the button beside it generates a new one.

**Export Protobuf File** writes the `.proto` file for [gRPC](gRPC-Server.md) clients. It is an action, needs **Enable API Server** on, and is shown in builds that include gRPC.

## Startup

| Setting | What it does | Default |
|---------|--------------|---------|
| **Rendering Backend** | Graphics API that draws the interface: **Automatic (Platform Default)**, or OpenGL, Vulkan, Direct3D 11, or **Software (Fallback)** where the platform has them. Safe to leave on Automatic. Not offered on macOS. Takes effect after restart. | Automatic |
| **Display Scaling** | How the interface follows the display's scale factor: **Automatic (System Default)**, **Fractional**, **Round to Nearest**, **Round Up**, **Round Down**, **Disable Scaling**, or **Custom…**. Shown only where the platform allows changing it. Takes effect after restart. | Fractional |
| **Custom Scale (%)** | Scale used by **Custom…**, 50 to 400 in steps of 25. Takes effect after restart. | 100 |
| **HDR Output** | See [HDR Output](#hdr-output). Takes effect after restart. | Off |
| **SIMD Instruction Set** | CPU instructions used by the signal kernels. Auto picks the best set your CPU supports. See [The Acquisition Pipeline](Data-Hotpath.md#simd-instruction-set). | Auto |
| **Reduce Motion** | Stops menus, dialogs, and buttons from scaling, sliding, and bouncing. Short fades remain. | Off (On with **Software (Fallback)**) |
| **Apply Performance Hints** | Runs Serial Studio at a higher priority and keeps the system from slowing it down to save power. Takes effect after restart. | On |
| **Keep Display Awake** | Stops the system and the display from going to sleep while Serial Studio runs. Takes effect after restart. | On |
| **Automatically Check for Updates** | Checks for a new version at startup. | On |
| **Check for Extension Updates** | Checks installed extensions and themes at startup. | On |
| **Install Extension Updates Automatically** | Installs those updates without asking. Disabled while the check above is off. | Off |

If a launch with a non-automatic **Rendering Backend** crashes, the next launch returns to **Automatic (Platform Default)**. If a launch with **HDR Output** on crashes, the next launch turns it off.

### HDR Output

Skip this unless you have an HDR display. On one, **HDR Output** lets the parts of the dashboard that carry a live signal glow brighter than the white of the interface, so a needle or a lit lamp stands out while everything else looks the same.

HDR Output is free in every edition. The row exists on macOS with Metal and on Windows with Direct3D 11, which is what **Automatic (Platform Default)** selects there. Linux has no HDR path, and the row disappears on Windows when **Rendering Backend** is OpenGL, Vulkan, or **Software (Fallback)**.

| Element | Brightness, relative to interface white |
|---------|-----------------------------------------|
| Gauge and Meter needles, Bar Panel fills and band stripes, LED Panel lamps | 1.4× on dark themes, 1.2× on light themes |
| Blinking alarm boxes, and the [annunciator](Aural-Alerts.md) bell in its fast flash | 1.7× |
| Thin strokes: plot curves and [frequency marker](Widget-Reference.md#frequency-markers) lines | 2.0× on dark themes, no boost on light themes |

Gauge and Meter band arcs stay at normal brightness, so they do not overpower the needle.

Serial Studio caps each level at the headroom the display reports, so a display with no HDR headroom looks the same as with the setting off. There is no brightness knob; the display decides.

## Plotting

On the **Plotting** tab, **Time Range** sets how far back a plot reaches, **Point Count** sets how many samples it keeps, and **UI Refresh Rate (Hz)** sets how many times per second the dashboard redraws (lower it on a slow computer). A longer history costs more to redraw: at 5 minutes a fast signal squeezes into a dense band, so shorten **Time Range** when you need to see individual cycles.

| Setting | Range | Default |
|---------|-------|---------|
| **Time Range** | Presets from 1 ms to 300 s; a typed value snaps to the nearest preset. | 10 s |
| **Point Count** | 10 to 100000, in steps of 10 | 1000 |
| **UI Refresh Rate (Hz)** | 1 to 240 | 60 |

**Ask Devices for Their Dashboard**, off by default, is for devices running the Serial Studio [firmware library](Device-Declarations.md). On connect in Quick Plot mode (the no-project mode, see [Operation Modes](Operation-Modes.md)), it sends one request line and the device answers with its dashboard declaration.

**Time Range** and **Point Count** apply to the live dashboard. The same two fields in the Project Editor (the screen where you define a project file) are saved into the project file; [Plots](Plots.md) explains how they differ.

## Layout

On the **Layout** tab, **Font Family** and **Font Size** set the lettering of dashboard widgets. The console has its own pair on the **Console** tab.

| Setting | What it does | Default |
|---------|--------------|---------|
| **Font Family** | Font of dashboard widgets. | The interface's monospace font |
| **Font Size** | **Small**, **Normal**, **Large**, **Extra Large**, or **Custom**, which takes a percentage from 50 to 300. | Normal |
| **Layout Margin** | Space around the dashboard, in pixels, 0 or more. | 0 |
| **Layout Spacing** | Gap between widgets, in pixels, -1 or more. The default of -1 lets neighboring widgets share one border, so a dense dashboard looks like a single sheet. | -1 |
| **Show Actions Panel** | Shows the actions panel on the dashboard. | On |
| **Auto-Hide Toolbar** | Hides the dashboard toolbar until it is needed. | Off |
| **Show Alignment Guides** | Shows alignment guides while you drag widget windows. | Off |

## Taskbar

The **Taskbar** tab controls the bar of window buttons at the bottom of the dashboard.

| Setting | What it does | Default |
|---------|--------------|---------|
| **Always Show Taskbar Buttons** | Shows a button for every widget window. When off, a widget gets a button only while it is closed or minimized. | Off |
| **Show Search Field** | Shows the search field on the taskbar. | On |
| **Auto-hide Taskbar** | Hides the taskbar when the pointer leaves it. Moving the pointer to the bottom edge of the dashboard brings it back. | Off |
| **Hide Delay (ms)** | How long the taskbar waits before hiding, 200 to 10000. Enabled with **Auto-hide Taskbar** on. | 1500 |

**Pinned Buttons** chooses which of Settings (this dialog), Console, Notifications, Clock, Stopwatch, Pause / Resume, File Transmission, and AI Assistant sit on the taskbar. Settings, Console, Notifications, and Pause / Resume are pinned by default. Drag a pinned button on the taskbar to reorder it.

## Console

If your sketch prints with `Serial.println`, the defaults already show it as text. Change **Line Ending** only if your sketch waits for a newline on what you send from the console.

| Setting | What it does | Default |
|---------|--------------|---------|
| **Display Mode** | How received bytes are shown: **Text** or **Hex**. | Text |
| **Font Family**, **Font Size** | Console lettering, 6 to 72 points. The dashboard widgets have their own pair on the **Layout** tab. | The interface's monospace font |
| **Scrollback Lines** | Lines kept in the console, 100 to 100000. | 1000 |
| **Show Timestamps** | Prefixes each line with its arrival time. | Off |
| **Line Ending** | Bytes appended to text you send: **No Line Ending**, **New Line**, **Carriage Return**, or **CR + NL**. | No Line Ending |
| **Input Mode** | How typed text is read: **ASCII** or **HEX**. | ASCII |
| **Text Encoding** | Encoding of typed text, chosen from a dropdown that includes UTF-8, UTF-16 LE, UTF-16 BE, and Latin-1. | UTF-8 |
| **Checksum** | Checksum appended to what you send, chosen from a dropdown. | No Checksum |
| **Echo Sent Data** | Shows what you send in the console. | On |
| **VT100 Emulation** | Interprets terminal escape codes. Locked while an Image View widget is active. | On |
| **ANSI Colors** | Applies color codes. Needs **VT100 Emulation**. | On |

## Export

On the **Export** tab, **Row Interval (ms)** controls [CSV export](CSV-Export-Playback.md). Choose 0, 10, 100, or 1000, or type any whole number of milliseconds from 0. The default of 0 writes a row for every frame received. Any other value writes one row with every channel at that interval, which keeps files small for fast or multi-device projects.

**Save Videos by Default**, Pro only and off by default, turns on video export for Image View widgets.

## Sounds

On the **Sounds** tab, **Enable Sounds** is off by default, and nothing plays until it is on. See [Aural Alerts](Aural-Alerts.md) for the volume, device, and per-event settings.

## Notifications

The **Notifications** tab (Pro only) has two switches, both off by default.

| Setting | What it does | Default |
|---------|--------------|---------|
| **System Notifications** | Shows Warning and Critical events as desktop notifications while Serial Studio is not the foreground window. | Off |
| **Route Warnings to Notifications** | Sends application warnings to the [notification center](Notifications.md). Off by default because Serial Studio logs many minor warnings that would bury real alarms. Critical messages are always sent. | Off |

## See also

- [Toolbar & Button Reference](Toolbar-Reference.md): the button that opens this dialog.
- [Extensions](Extensions.md): install more themes, and write your own.
- [Plots](Plots.md): how **Time Range** and **Point Count** shape a plot.
- [Aural Alerts](Aural-Alerts.md): the **Sounds** tab in full.
- [FAQ](FAQ.md): short answers on themes, icon packs, and animations.
