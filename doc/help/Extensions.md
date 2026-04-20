# Extensions

## What are extensions?

Extensions are add-ons that extend Serial Studio. Supported types:

- **Themes.** Customize the whole look and feel, including code editor syntax colors.
- **Frame parsers.** JavaScript functions that decode custom binary or text protocols into structured data.
- **Project templates.** Ready-to-use `.ssproj` project files for common sensor configurations.
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
6. Once installed, the extension is immediately available:
   - **Themes** show up in the theme dropdown in Preferences.
   - **Plugins** can be launched from the detail view with the Run button.

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
- Plugins that use gRPC need the `grpcio` Python package (auto-installed on first run).
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

A repository is a URL pointing to a `manifest.json` file that lists available extensions. Serial Studio ships with a default community repository.

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

#### Field reference

| Field         | Required | Description |
|---------------|----------|-------------|
| `id`          | Yes      | Unique identifier (lowercase, hyphens). |
| `type`        | Yes      | `theme`, `frame-parser`, `project-template`, or `plugin`. |
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

### Hosting

**GitHub.** Create a repository and share the raw manifest URL:

```
https://raw.githubusercontent.com/your-org/extensions/main/manifest.json
```

**Local folder.** Use Browse in the Repository Settings to pick a folder that contains `manifest.json`.

**Any web server.** Host files over HTTP(S). Relative paths in `files` resolve against the `info.json` URL.

## Plugin state persistence

Plugin state (open windows, settings, configurations) is saved in the project file alongside widget layout data. That means different projects can have different plugin setups.

State is saved automatically when the device disconnects, when the plugin is stopped, or when Serial Studio exits. It's restored when the plugin starts or when a new device connects. Plugins that were running when Serial Studio closed are relaunched automatically on the next startup.

For details on using the state persistence API from code, see the [Plugin Development](Plugin-Development.md) guide.

## API access

The Extension Manager is accessible via the [API](API-Reference.md) on TCP port 7777 or [gRPC](gRPC-Server.md) on port 8888:

| Command                         | Description |
|---------------------------------|-------------|
| `extensions.list`               | List all available extensions. |
| `extensions.getInfo`            | Get details for a specific extension (`extensionId`). |
| `extensions.install`            | Install an extension by index (`addonIndex`). |
| `extensions.uninstall`          | Uninstall an extension by index (`addonIndex`). |
| `extensions.refresh`            | Refresh catalogs from all repositories. |
| `extensions.saveState`          | Save plugin state to the project (`pluginId`, `state`). |
| `extensions.loadState`          | Load plugin state from the project (`pluginId`). |
| `extensions.listRepositories`   | List repository URLs (Pro only). |
| `extensions.addRepository`      | Add a repository URL (Pro only). |
| `extensions.removeRepository`   | Remove a repository by index (Pro only). |

## Installed extension location

Extensions are installed to your workspace under `Extensions/`:

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
