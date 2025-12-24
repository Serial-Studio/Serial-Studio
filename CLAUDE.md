# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Serial Studio is a cross-platform telemetry dashboard and real-time data visualization application built with Qt 6.9.2 and C++20. It supports data input from serial ports, Bluetooth LE, MQTT, TCP/UDP sockets, and audio devices.

## Build Commands

```bash
# Standard GPL build (default)
mkdir build && cd build
cmake ../ -DPRODUCTION_OPTIMIZATION=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)

# Debug build with sanitizers
cmake ../ -DDEBUG_SANITIZER=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j$(nproc)
```

**Linux prerequisites:**
```bash
sudo apt install libgl1-mesa-dev build-essential zlib1g-dev libexpat1-dev
```

**macOS prerequisites:**
```bash
# ZLIB and EXPAT are usually pre-installed
# If needed, install via Homebrew:
brew install zlib expat
```

**Windows prerequisites:**
- ZLIB and EXPAT can be installed via vcpkg:
```bash
vcpkg install zlib expat
```

### CMake Options
- `BUILD_GPL3=ON` (default): GPLv3-compliant build without commercial modules
- `BUILD_COMMERCIAL=ON`: Enables Pro features (requires valid license key)
- `PRODUCTION_OPTIMIZATION=ON`: Enable release optimizations
- `DEBUG_SANITIZER=ON`: Enable AddressSanitizer and UBSan

## Architecture

### Core Directories
- `app/src/` - Main application source code
- `app/qml/` - QML UI files
- `lib/` - Third-party libraries (KissFFT, QCodeEditor, QSimpleUpdater, OpenSSL, mdflib)

### Key Modules (app/src/)

**IO/** - Hardware abstraction and data acquisition
- `Manager` - Central I/O manager handling all communication protocols
- `Drivers/` - Protocol implementations: UART, Network (TCP/UDP), BluetoothLE, Audio (Pro)
- `FrameReader` - Parses incoming data streams into frames
- `HAL_Driver` - Abstract base class for all I/O drivers

**JSON/** - Data parsing and project management
- `Frame` - Represents a parsed data frame with groups and datasets
- `FrameBuilder` - Constructs Frame objects from raw data
- `FrameParser` - JavaScript-based custom frame parsing
- `ProjectModel` - Manages project configuration (the JSON schema defining dashboards)

**UI/** - Dashboard and visualization
- `Dashboard` - Main dashboard controller managing all widget instances
- `Widgets/` - Individual visualization widgets (Plot, Gauge, FFT, GPS, Accelerometer, etc.)
- `WindowManager` - Multi-window management
- `Taskbar` - Dashboard taskbar controller

**Misc/** - Application utilities
- `ThemeManager` - UI theming
- `ModuleManager` - Module initialization
- `Translator` - i18n support

**CSV/** - CSV file playback
- `Player` - CSV file player with timeline controls
- `Export` - Export telemetry data to CSV format

**MDF4/** - MDF4/MF4 binary file playback
- `Player` - MDF4 file player supporting CAN, LIN, FlexRay, analog channels
- Streaming implementation for large files using mdflib

**SerialStudio.h** - Central enums and utility class defining:
- `BusType` - Available data sources (UART, Network, BluetoothLE, Audio, ModBus, CanBus)
- `OperationMode` - Dashboard modes (ProjectFile, DeviceSendsJSON, QuickPlot)
- `DecoderMethod` - Data decoding (PlainText, Hexadecimal, Base64, Binary)
- Widget type enums for groups and datasets

### Data Flow
1. `IO::Manager` receives raw bytes from the selected driver
2. `IO::FrameReader` extracts frames using configured delimiters
3. `JSON::FrameBuilder` parses frames into `JSON::Frame` objects
4. `UI::Dashboard` renders the frame data using configured widgets

## Code Style

Uses clang-format with LLVM base style:
- 80 column limit
- 2-space indentation
- Braces on new lines (Allman style)
- `PointerBindsToType: false` (pointer binds to name)

Naming conventions (from .clang-tidy):
- Classes: `CamelCase`
- Functions: `CamelCase`
- Variables/parameters: `lower_case`
- Private members: `lower_case_` (trailing underscore)
- Constants/constexpr: `kCamelCase`
- Macros: `UPPER_CASE`

### Comments Policy

**IMPORTANT:** Inline end-of-line comments are prohibited in this codebase.

❌ **DO NOT** add comments like this:
```cpp
doSomething();  // Do x y and z
calculateValue(x, y);  // Calculate the result
```

✅ **DO** use these alternatives instead:
- **Self-documenting code:** Use clear variable and function names
- **Block comments:** Place explanatory comments on their own lines above the code
- **Function documentation:** Use proper Doxygen-style comments for functions
- **Code structure:** Organize code into well-named functions

**Examples of acceptable commenting:**

```cpp
// Good: Block comment explaining why
// We need to process data in chunks to avoid memory spikes
for (const auto &chunk : data)
  processChunk(chunk);

// Good: Doxygen function documentation
/**
 * @brief Calculates the checksum for the given data.
 *
 * Uses CRC16 algorithm to validate data integrity.
 *
 * @param data The data to checksum
 * @return The calculated checksum value
 */
uint16_t calculateChecksum(const QByteArray &data);
```

The rationale: Inline comments clutter the code and often become outdated. Well-structured code with meaningful names and block comments where necessary is more maintainable.

## Dual Licensing

Source files use SPDX headers indicating license:
- `GPL-3.0-only` - Open source only
- `LicenseRef-SerialStudio-Commercial` - Pro features only
- `GPL-3.0-only OR LicenseRef-SerialStudio-Commercial` - Dual-licensed

Commercial features (Pro) include: MQTT, 3D visualization, audio input, and advanced plotting.
