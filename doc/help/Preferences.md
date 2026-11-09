# Preferences

The Preferences dialog holds Serial Studio's application settings: language and theme, graphics, plotting, the dashboard layout, the taskbar, the console, export, sounds, and notifications. It is available in every edition. The **Notifications** tab and **Save Videos by Default** appear only in Pro builds.

Click **Preferences** on the main toolbar, click **Settings** on the dashboard taskbar, or search for Preferences in the [command palette](Command-Palette.md). On macOS the shortcut is **Cmd+,**.

A change takes effect as you make it, and **Apply** and **Close** both dismiss the dialog. The exceptions need a restart:

| Setting | When it applies |
|---------|-----------------|
| Rendering Backend, Display Scaling, Custom Scale (%), HDR Output | After a restart, which Serial Studio prompts for when you change one. |
| Custom Window Decorations, Apply Performance Hints, Keep Display Awake | The next time Serial Studio starts. |

**Reset**, at the bottom left, restores the defaults listed on this page. It leaves **Language**, **Icon Pack**, **HDR Output**, **SIMD Instruction Set**, **Workspace Folder**, **API Server Port**, **Row Interval (ms)**, and the two extension update switches as they are.

## General

| Setting | What it does | Default |
|---------|--------------|---------|
| **Language** | Interface language. Disabled while a device is connected. | |
| **Theme** | Colors and surfaces of the whole interface. **Get Themes…** opens the [extension catalog](Extensions.md) filtered to themes. | Default |
| **Icon Pack** | Artwork of the color icons on the toolbar, the Project Editor, the dashboard, and the dialogs. **Follow theme** uses the pack the theme names, or the standard set when it names none. **Office (default)** pins the standard set. | Follow theme |
| **Custom Window Decorations** | Draws Serial Studio's own window title bars. Not offered on macOS or Windows 11. | On |
| **Workspace Folder** | Where Serial Studio keeps its files. Disabled while a device is connected. | `Documents/Serial Studio` |
| **Enable API Server** | Accepts commands from local programs over TCP. See the [API Reference](API-Reference.md). | Off |
| **API Server Port** | TCP port of the API server, 1024 to 65535. | 7777 |
| **Allow External API Connections** | Accepts connections from other computers. | Off |
| **API Access Token** | The token external clients authenticate with. The button beside it generates a new one. Enabled only with external connections on. | |
| **Export Protobuf File** | Writes the `.proto` file for [gRPC](gRPC-Server.md) clients. Shown in builds that include gRPC. | |

## Startup

| Setting | What it does | Default |
|---------|--------------|---------|
| **Rendering Backend** | Graphics API that draws the interface: **Automatic (Platform Default)**, or OpenGL, Vulkan, Direct3D 11, or **Software (Fallback)** where the platform has them. Not offered on macOS. | Automatic |
| **Display Scaling** | How the interface follows the display's scale factor: **Automatic (System Default)**, **Fractional**, **Round to Nearest**, **Round Up**, **Round Down**, **Disable Scaling**, or **Custom…**. Shown only where the platform allows changing it. | Fractional |
| **Custom Scale (%)** | Scale used by **Custom…**, 50 to 400 in steps of 25. | 100 |
| **HDR Output** | See [HDR Output](#hdr-output). | Off |
| **SIMD Instruction Set** | CPU instructions used by the signal kernels. See [The Acquisition Pipeline](Data-Hotpath.md#simd-instruction-set). | Auto |
| **Reduce Motion** | Stops menus, dialogs, and buttons from scaling, sliding, and bouncing. Short fades remain. | Off (On with **Software (Fallback)**) |
| **Apply Performance Hints** | Runs Serial Studio at a higher priority and keeps the system from slowing it down to save power. | On |
| **Keep Display Awake** | Stops the system and the display from going to sleep while Serial Studio runs. | On |
| **Automatically Check for Updates** | Checks for a new version at startup. | On |
| **Check for Extension Updates** | Checks installed extensions and themes at startup. | On |
| **Install Extension Updates Automatically** | Installs those updates without asking. Disabled while the check above is off. | Off |

If a launch with a non-automatic **Rendering Backend** crashes, the next launch returns to **Automatic (Platform Default)**.

### HDR Output

On an HDR display, **HDR Output** lets the parts of the dashboard that carry a live signal glow brighter than the white of the interface, so a needle or a lit lamp stands out while everything else looks the same. It is free in every edition. The row exists on macOS with Metal and on Windows with Direct3D 11, which is what **Automatic (Platform Default)** selects there. Linux has no HDR path, and the row disappears on Windows when **Rendering Backend** is OpenGL, Vulkan, or **Software (Fallback)**.

| Element | Brightness, relative to interface white |
|---------|-----------------------------------------|
| Gauge and Meter needles, Bar Panel fills and band stripes, LED Panel lamps | 1.4× on dark themes, 1.2× on light themes |
| Blinking alarm boxes, and the [annunciator](Aural-Alerts.md) bell in its fast flash | 1.7× |
| Thin strokes: plot curves and [frequency marker](Widget-Reference.md#frequency-markers) lines | 2.0× on dark themes, no boost on light themes |

The top of a Waterfall's color scale brightens too. Gauge and Meter band arcs stay at normal brightness, because they are reference marks that would otherwise overpower the needle. On a light theme a brighter thin stroke would wash out against the background, which is why strokes get no boost there.

Serial Studio caps each level at the headroom the display reports, so a display with no HDR headroom looks the same as with the setting off. There is no brightness control. If a launch with HDR Output on crashes, the next launch turns it off.

## Plotting

| Setting | Range | Default |
|---------|-------|---------|
| **Time Range** | 18 presets from 1 ms to 300 s | 10 s |
| **Point Count** | 10 to 100000, in steps of 10 | 1000 |
| **UI Refresh Rate (Hz)** | 1 to 240 | 60 |
| **Ask Devices for Their Dashboard** | On connect in Quick Plot, sends one request line so a device using the [firmware library](Device-Declarations.md) answers with its dashboard declaration. | Off |

**Time Range** and **Point Count** apply to the live dashboard. The same two fields in the Project Editor are saved into the project file; [Plots](Plots.md) explains how they differ.

## Layout

**Font Family** and **Font Size** set the lettering of dashboard widgets. **Font Size** offers **Small**, **Normal**, **Large**, **Extra Large**, and **Custom**, which takes a percentage from 50 to 300.

**Layout Margin** is the space around the dashboard, 0 by default. **Layout Spacing** is the gap between widgets; the default of -1 makes neighboring widgets share one border line.

**Show Actions Panel** is on by default. **Auto-Hide Toolbar** and **Show Alignment Guides** are off.

## Taskbar

| Setting | What it does | Default |
|---------|--------------|---------|
| **Always Show Taskbar Buttons** | Shows a button for every widget window. When off, a widget gets a button only while it is closed or minimized. | Off |
| **Show Search Field** | Shows the search field on the taskbar. | On |
| **Auto-hide Taskbar** | Hides the taskbar when the pointer leaves it. Moving the pointer to the bottom edge of the dashboard brings it back. | Off |
| **Hide Delay (ms)** | How long the taskbar waits before hiding, 200 to 10000. Enabled with **Auto-hide Taskbar** on. | 1500 |

**Pinned Buttons** chooses which of Settings, Console, Notifications, Clock, Stopwatch, Pause / Resume, File Transmission, and AI Assistant sit on the taskbar. Drag a pinned button on the taskbar to reorder it.

## Console

| Setting | What it does | Default |
|---------|--------------|---------|
| **Display Mode** | How received bytes are shown. | First in the list |
| **Font Family**, **Font Size** | Console lettering, 6 to 72 points. | The interface's monospace font |
| **Scrollback Lines** | Lines kept in the console, 100 to 100000. | 1000 |
| **Show Timestamps** | Prefixes each line with its arrival time. | Off |
| **Line Ending**, **Input Mode**, **Text Encoding**, **Checksum** | How text typed into the console is sent. | First in each list |
| **Echo Sent Data** | Shows what you send in the console. | On |
| **VT100 Emulation** | Interprets terminal escape codes. Locked while an Image View widget is active. | On |
| **ANSI Colors** | Applies color codes. Needs **VT100 Emulation**. | On |

## Export

**Row Interval (ms)** controls [CSV export](CSV-Export-Playback.md). Choose 0, 10, 100, or 1000, or type another value. The default of 0 writes a row for every frame received. Any other value writes one row with every channel at that interval, which keeps files small for fast or multi-device projects.

**Save Videos by Default**, off by default and shown only in Pro builds, turns on video export for Image View widgets.

## Sounds

**Enable Sounds** is off by default, and nothing plays until it is on. The tab also holds the volume, the output device, the alarm sequence, one row per alarm signal, and one row per application event. [Aural Alerts](Aural-Alerts.md) describes each of them.

## Notifications

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
