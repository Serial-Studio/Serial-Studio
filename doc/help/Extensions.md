# Extensions

## What are Extensions?

Extensions are community-created extensions that enhance Serial Studio. The first supported extension type is **themes**, which let you customize the entire look and feel of the application. Future extension types will include custom frame parsers, project templates, and more.

Extensions are downloaded from online repositories and installed locally to your workspace. You can browse, install, update, and uninstall extensions directly from the **Extension Manager** dialog.

## Opening the Extension Manager

Click the **Extensions** button in the toolbar (in the information section, next to Examples and Help Center). The Extension Manager dialog will open with three views:

- **Grid View** — Browse available extensions with search and category filtering.
- **Detail View** — View extension description, author, version, and license. Install, update, or uninstall from here.
- **Repository Settings** — Manage the list of extension repository URLs.

## Installing an Extension

1. Open the Extension Manager.
2. If the extension list is empty, click the **Refresh** button to fetch the latest catalog.
3. Browse or search for an extension.
4. Click on an extension card to open the detail view.
5. Click **Install**. A progress bar shows the download status.
6. Once installed, the extension is immediately available. For themes, the new theme appears in the theme dropdown in **Preferences**.

## Uninstalling an Extension

1. Open the Extension Manager.
2. Find the installed extension (it will show an **Installed** badge).
3. Click on the extension card, then click **Uninstall**.
4. The extension files are removed and it disappears from the application. If you were using an uninstalled theme, Serial Studio falls back to the **Default** theme.

## Updating an Extension

When a newer version of an installed extension is available in the repository, the extension card shows an **Update** badge. Click the card and press **Update** to download the latest version.

## Repositories

A repository is a URL that points to a `manifest.json` file listing available extensions. Serial Studio ships with a default repository hosted at [github.com/serial-studio/extensions](https://github.com/serial-studio/extensions).

You can add custom repository URLs to access extensions from other sources. This is useful for:

- **Company-internal extensions** — Host a private repository on your intranet or private GitHub repo.
- **Community collections** — Third-party extension collections hosted anywhere.
- **Development/testing** — Point to a local or staging server while developing extensions.

### Managing Repositories

1. Open the Extension Manager.
2. Click the **Settings** icon (gear) in the toolbar.
3. The Repository Settings view shows your configured URLs.
4. To add a repository, paste the full URL to the `manifest.json` file and click **Add**.
5. To remove a repository, click the **X** button next to it.
6. Repository URLs are saved across sessions.

## Creating Your Own Repository

Anyone can create an extension repository. A repository is simply a web-accessible directory with a `manifest.json` file at the root.

### Repository Structure

```
your-repo/
  manifest.json
  themes/
    my-theme/
      my-theme.json
      code-editor/
        my-theme.xml
      screenshot.png
    another-theme/
      another-theme.json
      code-editor/
        another-theme.xml
      screenshot.png
```

### The manifest.json Format

```json
{
  "version": 1,
  "repository": "My Company Extensions",
  "extensions": [
    {
      "id": "my-theme",
      "type": "theme",
      "title": "My Custom Theme",
      "description": "A dark theme with company branding colors.",
      "author": "Your Name",
      "version": "1.0.0",
      "license": "MIT",
      "category": "Dark",
      "screenshot": "themes/my-theme/screenshot.png",
      "files": [
        "themes/my-theme/my-theme.json",
        "themes/my-theme/code-editor/my-theme.xml"
      ]
    }
  ]
}
```

#### Field Reference

| Field | Required | Description |
|-------|----------|-------------|
| `id` | Yes | Unique identifier for the extension (lowercase, hyphens). |
| `type` | Yes | Extension type. Currently only `"theme"` is supported. |
| `title` | Yes | Display name shown in the Extension Manager. |
| `description` | Yes | Short description shown on the extension card. |
| `author` | Yes | Author name or organization. |
| `version` | Yes | Semantic version string (e.g., `"1.0.0"`). |
| `license` | No | License identifier (e.g., `"MIT"`, `"GPL-3.0"`, `"Proprietary"`). |
| `category` | No | Category for filtering (e.g., `"Dark"`, `"Light"`). |
| `screenshot` | No | Relative path to a preview screenshot. |
| `files` | Yes | Array of relative file paths to download. All paths are relative to the `manifest.json` URL. |

### Creating a Theme Extension

A theme extension consists of two files:

**1. Theme JSON** (e.g., `my-theme.json`)

This is the same format as the built-in themes in Serial Studio. It defines all UI colors, widget colors, and device colors. Use any existing theme file as a starting point — they are located in the `app/rcc/themes/` directory of the [Serial Studio repository](https://github.com/Serial-Studio/Serial-Studio/tree/master/app/rcc/themes).

The JSON must contain:
- `"title"` — The display name of the theme.
- `"parameters"` — Must include `"code-editor-theme"` matching the XML filename (without extension) and `"start-icon"` (use `"qrc:/rcc/logo/start-dark.svg"` for dark themes or `"qrc:/rcc/logo/start.svg"` for light themes).
- `"translations"` — Optional localized names.
- `"colors"` — The complete color palette. See the [built-in themes](https://github.com/Serial-Studio/Serial-Studio/tree/master/app/rcc/themes) for the full list of required color keys.

**2. Code Editor XML** (e.g., `code-editor/my-theme.xml`)

This defines syntax highlighting colors for the JavaScript code editor. Use an existing XML file from `app/rcc/themes/code-editor/` as a template.

### Hosting Options

**GitHub (recommended)**

1. Create a public GitHub repository (e.g., `your-org/serial-studio-extensions`).
2. Add your `manifest.json` and theme files.
3. Users add the raw URL as a repository: `https://raw.githubusercontent.com/your-org/serial-studio-extensions/main/manifest.json`

**Any Web Server**

Host the files on any HTTP(S) server. The only requirement is that the `manifest.json` and all referenced files are accessible via direct download URLs. Relative paths in the `files` array are resolved against the `manifest.json` URL.

**Private/Internal Repositories**

For company-internal use, host the repository on an internal web server or private GitHub repository. Users add the internal URL to their Extension Manager settings. Note that private GitHub repositories require authentication, which is not currently supported — use a web server with IP-based access control or VPN instead.

### Testing Your Repository

1. Host your files (even locally using `python3 -m http.server 8080`).
2. In Serial Studio, open the Extension Manager and go to Repository Settings.
3. Add the URL (e.g., `http://localhost:8080/manifest.json`).
4. Click Refresh. Your extensions should appear in the grid.
5. Install a theme and verify it appears in the Preferences theme dropdown.

## API Access

The Extension Manager is also accessible via the [MCP/API](nav:api-reference) on TCP port 7777:

| Command | Description |
|---------|-------------|
| `extensions.list` | List all available extensions. |
| `extensions.getInfo` | Get details for a specific extension (`extensionId` parameter). |
| `extensions.install` | Install an extension by index (`extensionIndex` parameter). |
| `extensions.uninstall` | Uninstall an extension by index (`extensionIndex` parameter). |
| `extensions.listRepositories` | List configured repository URLs. |
| `extensions.addRepository` | Add a repository URL (`url` parameter). |
| `extensions.removeRepository` | Remove a repository by index (`index` parameter). |
| `extensions.refresh` | Refresh extension catalogs from all repositories. |

## Installed Extension Location

Extensions are installed to your Serial Studio workspace under the `Extensions` directory:

```
~/Documents/Serial Studio/Extensions/
  installed.json          (tracks installed extensions and versions)
  themes/
    my-theme/
      my-theme.json
      code-editor/
        my-theme.xml
```

The default workspace location is `~/Documents/Serial Studio` on macOS/Linux and `%USERPROFILE%\Documents\Serial Studio` on Windows. You can change it in Preferences.
