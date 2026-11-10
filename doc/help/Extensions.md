# Extensions

Your board already prints values. A color theme your whole team likes, a ready-made `.ssproj` for a common sensor, or a Python script that logs frames to a database was probably written by someone else already. Extensions are how you get it: open the **Extension Manager**, pick one, click **Install**.

There are five types:

- **Themes** change the look of the whole application, including code editor syntax colors.
- **Frame parsers** are JavaScript functions that decode a custom binary or text protocol. You only need one if your device sends something other than plain text lines.
- **Project templates** are ready-to-use `.ssproj` project files for common sensor setups.
- **Widgets** are QML dashboard visualizations that render live data like a built-in widget. A widget runs inside Serial Studio with the application's own privileges, so it asks for consent before it renders (see [Widget Extension Development](Widget-Extension-Development.md)).
- **Plugins** are external programs (Python, native binaries) that connect to the API Server to process or display live data.

Quickest test: **Extensions** > filter **Themes** > pick one > **Install** > choose it in **Preferences > General**.

Extensions are downloaded from online repositories and installed locally. Browsing and installing needs an internet connection, because the catalog is fetched from the repository each time you click **Refresh**.

## Opening the Extension Manager

Click the **Extensions** button in the toolbar. The Extension Manager has three views:

- **Grid view** lists available extensions, with a search field and a type filter. When the grid is empty, the catalog has not loaded: check your connection and click **Refresh**.
- **Detail view** shows the README, metadata, screenshot and the install and uninstall controls.
- **Repositories view** manages repository URLs. It opens from the **Repos** button, which exists only in the commercial (Pro) build; a trial or license does not add it to the free build (see [Managing repositories](#managing-repositories-pro)).

## Installing an extension

1. Open the Extension Manager.
2. Click **Refresh** to fetch the latest catalog.
3. Browse or search for an extension. Use the type dropdown to filter by Themes, Plugins, and so on.
4. Click an extension card to open the detail view.
5. Click **Install**. A progress bar shows download status.
6. Serial Studio makes the extension available according to its type:

| Type | After install |
|------|---------------|
| Theme | Appears in the theme dropdown in **Preferences > General**. **Get Themes…** beside the dropdown opens this catalog filtered to themes. |
| Plugin | Click **Run** in the detail view. |
| Frame parser | Copied to the [Extensions folder](#installed-extension-location) as a `.js` file. In the frame parser editor of the Project Editor, click **Open** in its toolbar (it imports a script file) and pick that file. |
| Project template | Copied to the [Extensions folder](#installed-extension-location) as an `.ssproj` file. Click **Open Project** in the toolbar to load it. |
| Widget | Restart Serial Studio. The widget then appears in the Project Editor's Widget list, where you pick a widget for a group or dataset. The first time it renders, a dialog asks you to allow it, because a widget runs with Serial Studio's own permissions. The answer is stored per package version, so an updated widget asks again. |

## Uninstalling an extension

1. Open the Extension Manager and find the installed extension (it shows an **Installed** badge).
2. For plugins: click **Stop** first if it is running. A process that is still writing to its folder does not enjoy being deleted.
3. Click the card, then click **Uninstall**.
4. If you were using the uninstalled theme, Serial Studio falls back to **Default**.

## Updating an extension

When a newer version is available, the card shows an **Update** badge. Click the card and press **Install** to update.

Serial Studio can also check for you. In **Preferences > Startup**, **Check for Extension Updates** looks for newer versions of installed extensions, and **Install Extension Updates Automatically** skips the question. Without the second option, a prompt lists the extensions and offers **Yes**, **No** and **Always update**. Choosing **Always update** turns on **Install Extension Updates Automatically**. With **Check for Extension Updates** off, nothing is checked. If you decline, Serial Studio does not ask about those extensions again until you restart it.

## Plugins

A plugin is a separate program that talks to Serial Studio while it runs: it receives live data, computes statistics, draws its own windows. Both ways of connecting belong to the API Server, a local network port Serial Studio opens so other programs can read your data. It is off by default. Turn it on with **Preferences > General > API & Plugins > Enable API Server**, or let **Run** do it when it asks.

- **gRPC (port 8888)** is high-performance binary streaming, recommended for real-time frame data. The gRPC server runs only while the API Server is enabled, and only in builds with gRPC support. See the [gRPC Server](gRPC-Server.md) docs.
- **TCP/JSON (port 7777)** is a JSON-based protocol. See the [API Reference](API-Reference.md) for details.

### Running a plugin

1. Install the plugin from the Extension Manager.
2. Click the plugin card to open the detail view.
3. Click **Run**. The plugin starts, and its output appears in the log panel.
4. Click **Stop** to terminate it.

### Plugin requirements

- The API Server must be enabled. If it is off when you click **Run**, Serial Studio shows an **API Server Required** prompt that offers to turn it on; answering **No** cancels the launch.
- Python plugins need Python installed.
- Plugins that use gRPC need the `grpcio` Python package. Repository plugins include a launcher script that installs it into a local virtual environment on first run.
- Running plugins are stopped when the dashboard goes away (the device disconnects or the file player closes) and when Serial Studio exits. They relaunch when the dashboard comes back, unless you stopped them by hand.

### Platform support

If a plugin is not available for your platform, the **Install** button is disabled and an **Unavailable** badge is shown.

> Building your own plugin? See the [Plugin Development](Plugin-Development.md) guide for the full `info.json` reference, code examples, state persistence, and distribution instructions.

## Repositories

A repository is a URL pointing to a `manifest.json` file that lists available extensions. Serial Studio is bundled with a default community repository, and **Refresh** downloads its catalog over the internet. Every edition can install from it; adding your own repositories is a Pro feature.

### Managing repositories (Pro)

The **Repos** button in the Extension Manager toolbar exists only in the commercial (Pro) build; the free build does not show it. With it you can add:

- Company-internal repositories on your intranet (served over HTTPS) or a private GitHub.
- Third-party collections hosted anywhere.
- A local folder, while you develop extensions.

To manage repositories:

1. Open the Extension Manager.
2. Click the **Repos** button in the toolbar.
3. Add URLs, click **Browse…** to pick a local folder, or remove existing entries. Accepted values are a local folder or an `https://` URL; plain `http://` is refused with "Repositories must be a local folder or an https:// URL." Adding or removing an entry refreshes the catalog immediately.
4. Click **Reset** to restore the default community repository. This also uninstalls every installed extension, stops running plugins and removes your custom repositories; a **Reset Extensions** confirmation asks first.

## Plugin state persistence

Plugin state, such as open windows and settings, is saved in the project file alongside widget layout data. Different projects can have different plugin setups.

A plugin has to ask for its state to be saved: Serial Studio never calls `extensions.saveState` on its own. State is restored when the plugin starts or when a new device connects. Plugins that were running when you closed Serial Studio start again the next time you connect a device or open a file, unless you stopped them yourself.

For details on using the state persistence API from code, see the [Plugin Development](Plugin-Development.md) guide.

## API access

The Extension Manager is available over the [API Server](API-Reference.md), which is off by default; enable it in **Preferences > General > API & Plugins** first. Connect on TCP port 7777, or on [gRPC](gRPC-Server.md) port 8888 in builds with gRPC support:

| Command                         | Description |
|---------------------------------|-------------|
| `extensions.list`               | List all available extensions. |
| `extensions.getInfo`            | Get details for a specific extension (`extensionId`). |
| `extensions.install`            | Install an extension by index (`addonIndex`). |
| `extensions.uninstall`          | Uninstall an extension by index (`addonIndex`). Destructive: removes the extension files irreversibly. Pass `dryRun:true` to preview the removal without committing it. |
| `extensions.refresh`            | Refresh catalogs from all repositories. |
| `extensions.saveState`          | Save plugin state to the project (`pluginId`, `state`). |
| `extensions.loadState`          | Load plugin state from the project (`pluginId`). |
| `extensions.listRepositories`   | List repository URLs. Pro only. |
| `extensions.addRepository`      | Add a repository URL (`url`). Pro only. |
| `extensions.removeRepository`   | Remove a repository by index (`index`). Pro only. |

The three repository commands are not registered at all in the free build.

## Installed extension location

Extensions are installed under `Extensions/` in your workspace folder, `Documents/Serial Studio` by default. Change the folder with **Workspace Folder** in **Preferences > General**. Each extension gets its own folder, `<type>/<id>/`:

```text
Documents/Serial Studio/Extensions/
  installed.json
  theme/my-theme/
    info.json
    my-theme.json
    code-editor/my-theme.xml
  plugin/my-plugin/
    info.json
    plugin.py
    run.sh
    run.cmd
```

## Creating extensions

This part is for extension authors. Installing only? You are done here.

### Repository structure

Directory names are convention; the `type` field in `info.json` decides what an extension is.

```text
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
  project-template/my-template/
    info.json
    my-template.ssproj
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

Plugins add `entry`, `runtime`, and optionally `platforms` and `grpc` fields.

A plugin lists the platforms it supports in its `platforms` field; Serial Studio picks the matching entry. Matching tries the exact `os/arch` key first, then `os/*` for every architecture of one system, then a bare `*`. It reads `aarch64` as `arm64` and `amd64` as `x86_64`. Valid keys: `darwin/*` (always universal), `linux/x86_64`, `linux/arm64`, `windows/*`, `windows/x86_64`.

See the [Plugin Development](Plugin-Development.md) guide for the full reference and examples.

#### Widget `info.json`

Widgets add a `widget` block declaring the entity scope, the QML entry file, what data the widget accepts, and its settings. See the [Widget Extension Development](Widget-Extension-Development.md) guide for the full reference, the data model, and the trust model.

#### Field reference

| Field         | Required | Description |
|---------------|----------|-------------|
| `id`          | Yes      | Unique identifier (lowercase, hyphens; the widget example uses dots, as in `com.example.level-bar`). |
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

To use a theme rather than write one, see **Preferences > General** or the quick test at the top of this page.

A Serial Studio theme extension (type `theme`) is a JSON color table: `title`, `translations`, `parameters` and `colors`. Five optional sections extend it (`fonts`, `materials`, `surfaces`, `styles`, `widgets`), and `parameters` accepts one new key, `icon-pack`. A theme without them looks exactly as it always did, and an older Serial Studio reads the same file as a color-only theme, so keep the color table complete.

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
| `styles` | Per-control rules. `instrument.face` is the dial and bar-well color, apart from the plot area. `instrument.band-opacity` (0 to 1) is the strength of alarm bands on bars. `instrument.marker` is the color of a bar's value mark (`"level"` takes a darker shade of the bar's current color). `caption.active-lift` (for example 1.14) makes the focused dashboard window's caption lighter at the top; both states share the bottom color. |
| `widgets` | `"led": "lamp"` draws LED Panel indicators as panel lamps. Standard output buttons are drawn as panel push buttons; `"output": "plain"` keeps the flat button. |
| `parameters` | Beside `code-editor-theme` and `start-icon`, an optional `"icon-pack"` names the bundled icon pack the theme looks best with (`fluent`, `glass`, `outline`). It applies while the user's pack preference is *Follow theme*; a name this build does not bundle falls back to the standard set. A theme carries no icon artwork of its own. |

A theme is data. It selects recipes Serial Studio already has and cannot load a file or run anything. A name Serial Studio does not know is skipped with a warning in the log.

### Hosting

On GitHub, create a repository and share the raw manifest URL:

```text
https://raw.githubusercontent.com/your-org/extensions/main/manifest.json
```

For a local folder, click **Repos** in the Extension Manager toolbar (Pro), then **Browse…** to pick a folder that contains `manifest.json`. Any web server also works if it serves HTTPS, and relative paths in `files` resolve against the `info.json` URL. Plain `http://` URLs are refused.
