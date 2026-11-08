# Extensions

## Overview

Extensions are add-ons that extend Serial Studio. Supported types:

- **Themes.** Customize the whole look and feel, including code editor syntax colors.
- **Frame parsers.** JavaScript functions that decode custom binary or text protocols into structured data.
- **Project templates.** Ready-to-use `.ssproj` project files for common sensor configurations.
- **Widgets.** Dashboard visualizations written in QML that render live data on the canvas like a built-in widget. Unlike plugins, they run inside Serial Studio, with the same privileges as the application itself — see [Widget Extension Development](Widget-Extension-Development.md).
- **Plugins.** External programs (Python, native binaries, and so on) that connect to Serial Studio's API server to provide custom real-time data processing, visualization, and analysis.

Extensions are downloaded from online repositories and installed locally. You can browse, install, update, and uninstall them from the **Extension Manager** dialog.

## Opening the Extension Manager

Click the **Extensions** button in the toolbar. The Extension Manager has three views:

- **Grid view.** Browse available extensions with search and type filtering.
- **Detail view.** README, metadata, screenshot, and install/uninstall controls.
- **Repository settings.** Manage repository URLs (Pro only).

## Installing an extension

1. Open the Extension Manager.
2. Click **Refresh** to fetch the latest catalog.
3. Browse or search for an extension. Use the type dropdown to filter by Themes, Plugins, and so on.
4. Click an extension card to open the detail view.
5. Click **Install**. A progress bar shows download status.
6. Once installed, the extension is available:
   - **Themes** show up in the theme dropdown in Preferences.
   - **Plugins** can be launched from the detail view with the Run button.
   - **Frame parsers** are copied into your workspace's Extensions folder as `.js` files;
     use the **Open** button in the frame parser editor toolbar (it imports an external
     script file) to load one into your project.
   - **Project templates** are copied into your workspace's Extensions folder as `.ssproj`
     files; use **Open Project** in the toolbar to load one.
   - **Widgets** appear in the project editor's Widget list after a restart, for the entity kind
     they declare. The first time one renders, Serial Studio asks whether to allow it to run.

## Uninstalling an extension

1. Open the Extension Manager and find the installed extension (it shows an **Installed** badge).
2. Click the card, then click **Uninstall**.
3. For plugins: stop the plugin first if it's running.
4. If you were using the uninstalled theme, Serial Studio falls back to **Default**.

## Updating an extension

When a newer version is available, the card shows an **Update** badge. Click the card and press **Install** to update.

## Plugins

Plugins are external processes that connect to Serial Studio to receive live data, compute statistics, display custom visualizations, and more. Plugins can connect via:

- **gRPC (port 8888).** High-performance binary streaming. Recommended for real-time frame data. See the [gRPC Server](gRPC-Server.md) docs.
- **TCP/JSON (port 7777).** JSON-based protocol. See the [API Reference](API-Reference.md) for details.

### Running a plugin

1. Install the plugin from the Extension Manager.
2. Click the plugin card to open the detail view.
3. Click **Run**. The plugin starts, and its output appears in the log panel.
4. Click **Stop** to terminate it.

### Plugin requirements

- The API server has to be enabled (Serial Studio will prompt you if it isn't).
- Python plugins need Python 3.6+ installed.
- Plugins that use gRPC need the `grpcio` Python package. Repository plugins include a launcher script that installs it into a local virtual environment on first run.
- All running plugins are stopped automatically when Serial Studio exits.

### Platform support

Plugins can provide platform-specific entry points for different operating systems and CPU architectures. When a plugin uses the `platforms` field in its metadata, Serial Studio automatically picks the right binary or script for your system.

Platform keys use the format `os/arch` or `os/*` for universal builds:

- **macOS:** `darwin/*` (always universal).
- **Linux:** `linux/x86_64`, `linux/arm64`.
- **Windows:** `windows/*`, `windows/x86_64`.

If a plugin isn't available for your platform, the Install button is disabled and an **Unavailable** badge is shown.

> Building your own plugin? See the [Plugin Development](Plugin-Development.md) guide for the full `info.json` reference, code examples, state persistence, and distribution instructions.

## Repositories

A repository is a URL pointing to a `manifest.json` file that lists available extensions. Serial Studio is bundled with a default community repository.

### Managing repositories (Pro)

Custom repository management is available in Serial Studio Pro. That enables:

- **Company-internal extensions.** Host private repositories on your intranet or on a private GitHub.
- **Community collections.** Third-party extension collections hosted anywhere.
- **Local development.** Point to a local folder while developing extensions.

To manage repositories:

1. Open the Extension Manager.
2. Click the **Repos** button in the toolbar.
3. Add URLs, browse for local folders, or remove existing entries.
4. Click **Reset** to restore the default community repository.

## Creating extensions

### Repository structure

```
my-repo/
  manifest.json
  theme/my-theme/
    info.json
    my-theme.json
    code-editor/my-theme.xml
  plugin/my-plugin/
    info.json
    plugin.py
    run.sh
    run.cmd
  frame-parser/my-parser/
    info.json
    my-parser.js
  widget/com.example.level-bar/
    info.json
    LevelBar.qml
```

### `manifest.json`

The manifest lists paths to each extension's `info.json`:

```json
{
  "version": 1,
  "repository": "My Extensions",
  "extensions": [
    "theme/my-theme/info.json",
    "plugin/my-plugin/info.json"
  ]
}
```

### `info.json` format

Each extension has an `info.json` with metadata:

```json
{
  "id": "my-theme",
  "type": "theme",
  "title": "My Custom Theme",
  "description": "A dark theme with company branding.",
  "author": "Your Name",
  "version": "1.0.0",
  "license": "MIT",
  "category": "Dark",
  "screenshot": "screenshot.png",
  "files": [
    "info.json",
    "my-theme.json",
    "code-editor/my-theme.xml"
  ]
}
```

#### Plugin `info.json`

Plugins add `entry`, `runtime`, and optionally `platforms` and `grpc` fields. See the [Plugin Development](Plugin-Development.md) guide for the full reference and examples.

#### Widget `info.json`

Widgets add a `widget` block declaring the entity scope, the QML entry file, what data the widget accepts, and its settings. See the [Widget Extension Development](Widget-Extension-Development.md) guide for the full reference, the data model, and the trust model.

#### Field reference

| Field         | Required | Description |
|---------------|----------|-------------|
| `id`          | Yes      | Unique identifier (lowercase, hyphens). |
| `type`        | Yes      | `theme`, `frame-parser`, `project-template`, `widget`, or `plugin`. |
| `title`       | Yes      | Display name in the Extension Manager. |
| `description` | Yes      | Short description shown on the card. |
| `author`      | Yes      | Author name or organization. |
| `version`     | Yes      | Semantic version string (for example `"1.0.0"`). |
| `license`     | No       | License identifier (for example `"MIT"`). |
| `category`    | No       | Category for filtering. |
| `screenshot`  | No       | Relative path to a preview image. |
| `files`       | Yes      | Array of relative file paths to download/install. |
| `entry`       | Plugins  | Script or binary entry point. |
| `runtime`     | Plugins  | Interpreter command (for example `"python3"`). Empty for native binaries. |
| `terminal`    | Plugins  | `true` to launch in a system terminal window. |
| `grpc`        | Plugins  | `true` if the plugin uses gRPC streaming (port 8888) instead of TCP/JSON. |
| `platforms`   | Plugins  | Per-platform overrides for `entry`, `runtime`, and `files`. |

### Theme materials and lettering

A theme file is a color table: `title`, `translations`, `parameters` and `colors`. It may
also carry five optional sections. A theme without them looks exactly as it always did,
and an older Serial Studio reads the same file as a color-only theme, so keep the color
table complete.

```json
{
  "fonts": {
    "engraving": { "family": "DIN Alternate", "scale": 0.9, "capitals": true, "spacing": 0.8 }
  },
  "materials": {
    "panel": { "recipe": "brushed-metal", "color": "#B7BDB7", "grain": 0.6 },
    "frame": { "recipe": "gradient", "top": "#7A8189", "bottom": "#5C636B" },
    "well":  { "recipe": "glass", "top": "#0C0E10", "bottom": "#1C2025" }
  },
  "surfaces": { "dashboard": "frame", "widget": "panel" },
  "widgets": { "led": "lamp", "output": "hardware" }
}
```

| Section | What it does |
|---------|--------------|
| `fonts` | Named lettering roles. `engraving` is used for dashboard widget titles and control legends. The family must already be installed on the computer; when it is not, or for text it has no glyphs for, the interface font is used. Leave the section out to keep the interface fonts. |
| `materials` | Named surfaces. `recipe` is `flat` (`color`), `gradient` (`top`, `bottom`), `brushed-metal` (`color`, `grain` from 0 to 1, `shade` from 0 to 1 for the top-to-bottom lighting, `bevel` true or false for the edge lines) or `glass` (`top`, `bottom`). |
| `surfaces` | Which material a part of the window is made of: `dashboard` (behind the widgets) and `widget` (the body of a dashboard widget). |
| `styles` | Per-control rules. `instrument.face` is the dial and bar-well color, apart from the plot area, and `instrument.band-opacity` (0 to 1) is the strength of alarm bands on bars, and `instrument.marker` is the color of a bar's value mark (`"level"` takes a darker shade of the bar's current color). `caption.active-lift` (for example 1.14) makes the focused dashboard window's caption lighter at the top while both states share the bottom color. |
| `widgets` | `"led": "lamp"` draws LED Panel indicators as panel lamps. Standard output buttons are drawn as panel push buttons; `"output": "plain"` keeps the flat button. |
| `parameters` | Beside `code-editor-theme` and `start-icon`, an optional `"icon-pack"` names the bundled icon pack the theme looks best with (`fluent`, `glass`, `outline`). It applies while the user's pack preference is *Follow theme*; a name this build does not bundle falls back to the standard set. A theme carries no icon artwork of its own. |

A theme is data. It selects recipes Serial Studio already has and cannot load a file or
run anything. A name Serial Studio does not know is skipped with a warning in the log.
The bundled **Millennium Light**, **Millennium Dark** and **Starliner** themes use
`materials`, `surfaces` and `widgets`, and keep the interface fonts. **Get Themes…** beside the theme selector in
Preferences opens this catalog filtered to themes.

### Hosting

**GitHub.** Create a repository and share the raw manifest URL:

```
https://raw.githubusercontent.com/your-org/extensions/main/manifest.json
```

**Local folder.** Use Browse in the Repository Settings to pick a folder that contains `manifest.json`.

**Any web server.** Host files over HTTP(S). Relative paths in `files` resolve against the `info.json` URL.

## Plugin state persistence

Plugin state (open windows, settings, configurations) is saved in the project file alongside widget layout data. That means different projects can have different plugin setups.

Serial Studio never calls `extensions.saveState` on its own: a plugin must call it explicitly (typically on device disconnect, on its own stop, or before Serial Studio exits) for its state to persist. State is restored when the plugin starts or when a new device connects. Plugins that were running when Serial Studio closed are relaunched automatically the next time the dashboard becomes available (a device connects or a file player opens), unless you stopped them by hand.

For details on using the state persistence API from code, see the [Plugin Development](Plugin-Development.md) guide.

## API access

The Extension Manager is accessible via the [API](API-Reference.md) on TCP port 7777 or [gRPC](gRPC-Server.md) on port 8888:

| Command                         | Description |
|---------------------------------|-------------|
| `extensions.list`               | List all available extensions. |
| `extensions.getInfo`            | Get details for a specific extension (`extensionId`). |
| `extensions.install`            | Install an extension by index (`addonIndex`). |
| `extensions.uninstall`          | Uninstall an extension by index (`addonIndex`). Destructive: removes the extension files irreversibly. Pass `dryRun:true` to preview the removal without committing it. |
| `extensions.refresh`            | Refresh catalogs from all repositories. |
| `extensions.saveState`          | Save plugin state to the project (`pluginId`, `state`). |
| `extensions.loadState`          | Load plugin state from the project (`pluginId`). |
| `extensions.listRepositories`   | List repository URLs (Pro only). |
| `extensions.addRepository`      | Add a repository URL (Pro only). |
| `extensions.removeRepository`   | Remove a repository by index (Pro only). |

## Installed extension location

Extensions are installed to your workspace folder (Documents/Serial Studio by default; configurable in Settings) under `Extensions/`:

```
~/Documents/Serial Studio/Extensions/
  installed.json
  theme/my-theme/
    info.json
    my-theme.json
    code-editor/my-theme.xml
  plugin/my-plugin/
    info.json
    plugin.py
    run.sh
```
