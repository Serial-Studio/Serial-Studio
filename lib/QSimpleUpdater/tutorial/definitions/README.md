# What is this?

The [updates.json](updates.json) file is the update definitions file. It lists the latest versions, changelogs and download URLs for each platform.

The example project downloads the [updates.json](updates.json) file and analyzes it in order to:

- Compare the local (user set) version and the remote version (specified in [updates.json](updates.json))
- Download the change log for the current platform
- Obtain the download URLs
- Obtain the URL to open (if specified)
- Check if the update is mandatory (via the `mandatory-update` field)

## JSON Structure

```json
{
  "updates": {
    "<platform-key>": {
      "open-url": "",
      "latest-version": "1.0",
      "download-url": "https://example.com/update.bin",
      "changelog": "HTML changelog text here...",
      "mandatory-update": false
    }
  }
}
```

Valid platform keys are: `windows`, `osx`, `linux`, `ios`, `android` (or any custom key set via `setPlatformKey()`).
