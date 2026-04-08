# CLAUDE.md

## Behavioral Rules

- **Read before writing.** Never edit a file you haven't read this session.
- **Read existing signal/slot connections** in a file before adding or changing any.
- **Plan before multi-file changes.** For >3 files, state the plan and get confirmation.
- **Edit, don't rewrite.** Use targeted `Edit` calls. Rewrite only when asked or >70% changes.
- **No preamble, no trailing summary.** Lead with the action or result.
- **Do not create markdown/doc files** unless explicitly asked. Share info conversationally.
- **Update CLAUDE.md** for any significant architectural change.

## Build

### CMake (standalone)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# With tests
cmake -B build -DQSIMPLE_UPDATER_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
ctest --test-dir build
```

### CMake (as subdirectory, e.g. Serial Studio)

```cmake
set(QSIMPLE_UPDATER_BUILD_TUTORIAL OFF CACHE BOOL "" FORCE)
set(QSIMPLE_UPDATER_BUILD_TESTS OFF CACHE BOOL "" FORCE)
# Optional: set QSU_QT_VERSION_MAJOR to skip find_package(QT NAMES ...)
set(QSU_QT_VERSION_MAJOR 6)
add_subdirectory(lib/QSimpleUpdater)
target_link_libraries(YourApp PRIVATE QSimpleUpdater)
```

### qmake

```bash
# Include in your .pro file:
include(path/to/QSimpleUpdater.pri)
```

Key CMake options: `QSIMPLE_UPDATER_BUILD_TUTORIAL`, `QSIMPLE_UPDATER_BUILD_TESTS`, `QSU_QT_VERSION_MAJOR`.

## Project Overview

QSimpleUpdater: cross-platform auto-update library for Qt applications.
MIT license. Supports Qt 5 and Qt 6. C++17.

Features: JSON-based update definitions, semantic version comparison (with pre-release suffixes), integrated download UI, mandatory update enforcement, HTTP basic auth, custom appcast parsing.

## Directory Structure

```
include/
└── QSimpleUpdater.h       Public API (singleton, version comparison)
src/
├── QSimpleUpdater.cpp     Singleton implementation, compareVersions()
├── Updater.h/.cpp         Per-URL updater: fetches JSON, parses, prompts user
├── Downloader.h/.cpp/.ui  Download widget with progress bar
├── AuthenticateDialog.h/.cpp/.ui  HTTP basic auth dialog
tutorial/
├── src/                   Example application
└── definitions/           Example updates.json
tests/
├── main.cpp               Test runner
├── Test_Versioning.h      Semantic version comparison tests
├── Test_Updater.h         (stub)
├── Test_Downloader.h      (stub)
└── Test_QSimpleUpdater.h  (stub)
```

## Architecture

### Singleton + Per-URL Updaters

`QSimpleUpdater` is a singleton (`getInstance()`). It lazily creates `Updater` instances keyed by URL. Each `Updater` owns one `Downloader` and one `QNetworkAccessManager`.

```
QSimpleUpdater (singleton)
├── Updater for URL A → Downloader A
├── Updater for URL B → Downloader B
└── ...
```

### Signal Flow

```
User calls checkForUpdates(url)
  → Updater downloads JSON from url
  → Updater::onReply() parses JSON, extracts platform info
  → Updater::setUpdateAvailable() shows QMessageBox
  → User clicks Yes → Downloader::startDownload()
  → Downloader emits downloadFinished(url, filepath)
  → Signal propagates: Updater → QSimpleUpdater → application
```

### Update Definitions JSON Format

```json
{
  "updates": {
    "<platform-key>": {
      "open-url": "",
      "latest-version": "2.0.0",
      "download-url": "https://example.com/update.bin",
      "changelog": "<p>HTML changelog</p>",
      "mandatory-update": false
    }
  }
}
```

Platform keys: `windows`, `osx`, `linux`, `ios`, `android` (or custom via `setPlatformKey()`).

### Ownership & Lifetime

| Object | Owner | Cleanup |
|--------|-------|---------|
| `QSimpleUpdater` | Static singleton | Destructor cleans up all Updaters |
| `Updater` | Static list `UPDATERS` | `deleteLater()` in `~QSimpleUpdater` |
| `Downloader` | `Updater` (member) | `delete` in `~Updater` |
| `QNetworkAccessManager` | `Updater` + `Downloader` | `delete` in destructors |
| `QNetworkReply` | `Downloader::m_reply` | `deleteLater()` on cleanup, nullptr after use |

### Key Invariants

- `m_reply` is always `nullptr` when no download is active. Null-checked before every access.
- `reply->deleteLater()` is called in `Updater::onReply()` to prevent memory leaks.
- `Downloader::startDownload()` cleans up any previous `m_reply` before starting a new one.
- All `connect()` calls use type-safe functor syntax (no `SIGNAL()`/`SLOT()` macros).

## Code Style

### Formatting

100-column limit, 2-space indent. Pointer/reference bind to type (`int* ptr`, `const Foo& ref`).
Run `clang-format` (config in `.clang-format`).

### Comments & Doxygen

- `@brief` on every class (in `.h`) and every function (in `.cpp`).
- Tags: `@brief`, `@param` (non-trivial), `@return` (non-void), `@warning`, `@note`.
- Function body comments: one-line `//` section headers. No `/* */` blocks inside functions.

### Signals & Connections

- Always functor-based `connect()`. Never `SIGNAL()`/`SLOT()` macros.
- `signals:`, `public slots:`, `private slots:` (lowercase, no `Q_` prefix).

### License

MIT. Copyright header in every source file: `Copyright (c) 2014-2025 Alex Spataru`.
