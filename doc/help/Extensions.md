# Extensions

## What are Extensions?

Extensions are add-ons that enhance Serial Studio. Supported types include:

- **Themes** — Customize the entire look and feel of the application, including code editor syntax colors.
- **Frame Parsers** — JavaScript functions that decode custom binary or text protocols into structured data.
- **Project Templates** — Ready-to-use `.ssproj` project files for common sensor configurations.
- **Plugins** — External programs (Python, native binaries, etc.) that connect to Serial Studio's API server to provide custom real-time data processing, visualization, and analysis.

Extensions are downloaded from online repositories and installed locally. You can browse, install, update, and uninstall them from the **Extension Manager** dialog.

## Opening the Extension Manager

Click the **Extensions** button in the toolbar. The Extension Manager has three views:

- **Grid View** — Browse available extensions with search and type filtering.
- **Detail View** — View README, metadata, screenshot, and install/uninstall controls.
- **Repository Settings** — Manage repository URLs (Pro only).

## Installing an Extension

1. Open the Extension Manager.
2. Click **Refresh** to fetch the latest catalog.
3. Browse or search for an extension. Use the type dropdown to filter by Themes, Plugins, etc.
4. Click an extension card to open the detail view.
5. Click **Install**. A progress bar shows download status.
6. Once installed, the extension is immediately available:
   - **Themes** appear in the theme dropdown in Preferences.
   - **Plugins** can be launched from the detail view with the Run button.

## Uninstalling an Extension

1. Open the Extension Manager and find the installed extension (it shows an **Installed** badge).
2. Click the card, then click **Uninstall**.
3. For plugins: stop the plugin first if it's running.
4. If you were using an uninstalled theme, Serial Studio falls back to **Default**.

## Updating an Extension

When a newer version is available, the card shows an **Update** badge. Click the card and press **Install** to update.

## Plugins

Plugins are external processes that connect to Serial Studio's [MCP/API](nav:api-reference) server on TCP port 7777. They can read live frame data, compute statistics, display custom visualizations, and more.

### Running a Plugin

1. Install the plugin from the Extension Manager.
2. Click the plugin card to open the detail view.
3. Click **Run**. The plugin starts and its output appears in the log panel.
4. Click **Stop** to terminate it.

### Plugin Requirements

- The API server must be enabled (Serial Studio will prompt you if it's not).
- Python plugins require Python 3.6+ installed on your system.
- All running plugins are automatically stopped when Serial Studio exits.

### Platform Support

Plugins can provide platform-specific entry points for different operating systems and CPU architectures. When a plugin uses the `platforms` field in its metadata, Serial Studio automatically selects the correct binary or script for your system.

Platform keys use the format `os/arch` or `os/*` (for universal builds):
- **macOS**: `darwin/*` (always universal)
- **Linux**: `linux/x86_64`, `linux/arm64`
- **Windows**: `windows/*`, `windows/x86_64`

If a plugin is not available for your platform, the Install button is disabled and an **Unavailable** badge is shown.

## Repositories

A repository is a URL pointing to a `manifest.json` file that lists available extensions. Serial Studio ships with a default community repository.

### Managing Repositories (Pro)

Custom repository management is available in Serial Studio Pro. This enables:

- **Company-internal extensions** — Host private repositories on your intranet or private GitHub.
- **Community collections** — Third-party extension collections hosted anywhere.
- **Local development** — Point to a local folder while developing extensions.

To manage repositories:

1. Open the Extension Manager.
2. Click the **Repos** button in the toolbar.
3. Add URLs, browse for local folders, or remove existing entries.
4. Click **Reset** to restore the default community repository.

## Creating Extensions

### Repository Structure

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

### manifest.json

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

### info.json Format

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

#### Plugin info.json

Plugins add `entry`, `runtime`, and optionally `platforms`:

```json
{
  "id": "my-plugin",
  "type": "plugin",
  "title": "My Plugin",
  "entry": "plugin.py",
  "runtime": "python3",
  "terminal": false,
  "files": ["info.json", "plugin.py"],
  "platforms": {
    "darwin/*":  { "entry": "run.sh",  "runtime": "", "files": ["run.sh"] },
    "linux/*":   { "entry": "run.sh",  "runtime": "", "files": ["run.sh"] },
    "windows/*": { "entry": "run.cmd", "runtime": "", "files": ["run.cmd"] }
  }
}
```

- `runtime: ""` means the entry is a native executable (no interpreter).
- `terminal: true` opens the plugin in a system terminal window.
- Platform-specific `files` are merged with base `files` during installation.

#### Field Reference

| Field | Required | Description |
|-------|----------|-------------|
| `id` | Yes | Unique identifier (lowercase, hyphens). |
| `type` | Yes | `theme`, `frame-parser`, `project-template`, or `plugin`. |
| `title` | Yes | Display name in the Extension Manager. |
| `description` | Yes | Short description shown on the card. |
| `author` | Yes | Author name or organization. |
| `version` | Yes | Semantic version string (e.g., `"1.0.0"`). |
| `license` | No | License identifier (e.g., `"MIT"`). |
| `category` | No | Category for filtering. |
| `screenshot` | No | Relative path to a preview image. |
| `files` | Yes | Array of relative file paths to download/install. |
| `entry` | Plugins | Script or binary entry point. |
| `runtime` | Plugins | Interpreter command (e.g., `"python3"`). Empty for native binaries. |
| `terminal` | Plugins | `true` to launch in a system terminal window. |
| `platforms` | Plugins | Per-platform overrides for `entry`, `runtime`, and `files`. |

### Hosting

**GitHub**: Create a repository and share the raw manifest URL:
```
https://raw.githubusercontent.com/your-org/extensions/main/manifest.json
```

**Local folder**: Use Browse in the Repository Settings to select a folder containing `manifest.json`.

**Any web server**: Host files on HTTP(S). Relative paths in `files` resolve against the `info.json` URL.

## Plugin State Persistence

Plugin state (open windows, settings, configurations) is saved in the project file alongside widget layout data. This means different projects can have different plugin setups — a project monitoring temperature might have gauge windows configured differently than one monitoring audio.

State is saved automatically:
- When the device disconnects
- When the plugin is closed
- When Serial Studio exits

State is restored automatically:
- When the plugin starts (from saved project data)
- When a new device connects (project may have changed)

Plugins that were running when Serial Studio closed are automatically relaunched on the next startup.

## API Access

The Extension Manager is accessible via the [MCP/API](nav:api-reference) on TCP port 7777:

| Command | Description |
|---------|-------------|
| `extensions.list` | List all available extensions. |
| `extensions.getInfo` | Get details for a specific extension (`extensionId`). |
| `extensions.install` | Install an extension by index (`addonIndex`). |
| `extensions.uninstall` | Uninstall an extension by index (`addonIndex`). |
| `extensions.refresh` | Refresh catalogs from all repositories. |
| `extensions.saveState` | Save plugin state to the project (`pluginId`, `state`). |
| `extensions.loadState` | Load plugin state from the project (`pluginId`). |
| `extensions.listRepositories` | List repository URLs (Pro only). |
| `extensions.addRepository` | Add a repository URL (Pro only). |
| `extensions.removeRepository` | Remove a repository by index (Pro only). |

### Lifecycle Events

The API server broadcasts events to all connected clients:

| Event | When |
|-------|------|
| `{"event": "connected"}` | Device connected |
| `{"event": "disconnected"}` | Device disconnected |

Plugins should listen for these to save/restore their state at the appropriate times.

## Installed Extension Location

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
