# Serial Studio Application

> **Modern C++20 telemetry dashboard and real-time data visualization application**

[![License: GPL-3.0](https://img.shields.io/badge/License-GPL%203.0-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Qt Version](https://img.shields.io/badge/Qt-6.9.2-green.svg)](https://www.qt.io/)
[![C++ Standard](https://img.shields.io/badge/C++-20-orange.svg)](https://en.cppreference.com/w/cpp/20)

## Overview

Serial Studio is a cross-platform telemetry visualization application that enables real-time data monitoring and analysis from multiple sources including serial ports, Bluetooth LE, TCP/UDP networks, MQTT, Modbus, CAN Bus, and audio devices.

**Key Features:**
- **Multi-Protocol Support**: UART, TCP/UDP, Bluetooth LE, Audio, Modbus TCP/RTU, CAN Bus
- **Real-Time Visualization**: 15+ widget types (plots, gauges, maps, 3D graphics, FFT)
- **File Playback**: CSV and MDF4/MF4 (automotive) file support with timeline controls
- **High Performance**: Lock-free data paths supporting 1000+ Hz frame rates
- **Custom Parsing**: JavaScript-based frame parsing with JSON schema definition
- **Cross-Platform**: Windows, macOS, Linux with native integrations

**Technical Highlights:**
- C++20 with concepts, constexpr optimizations, and move semantics
- Lock-free circular buffers with atomic operations
- Hardware abstraction layer (HAL) for protocol-agnostic data acquisition
- Qt 6.9.2 with QML for responsive UI
- Zero-copy data distribution via immutable shared pointers
- Profile-Guided Optimization (PGO) support for 10-20% performance gains

## Architecture

### Directory Structure

```
app/
├── src/                    # C++ application source code (~100K LOC)
│   ├── IO/                 # Hardware abstraction & I/O drivers
│   ├── DataModel/          # Frame parsing & project management
│   ├── UI/                 # Dashboard & visualization widgets
│   ├── Misc/               # Utilities & application infrastructure
│   ├── CSV/                # CSV file playback & export
│   ├── MDF4/               # Automotive MDF4 file support
│   ├── Console/            # Debug console & terminal
│   ├── Licensing/          # License validation (Pro features)
│   ├── MQTT/               # MQTT client (Pro)
│   ├── Platform/           # Platform-specific integrations
│   ├── Plugins/            # Plugin server
│   └── ThirdParty/         # Embedded libraries
├── qml/                    # QML declarative UI files
│   ├── Dialogs/            # Application dialogs
│   ├── ProjectEditor/      # JSON project editor UI
│   ├── Widgets/            # Dashboard widget views
│   └── Windows/            # Main window & panels
├── assets/                 # Application resources
├── translations/           # Internationalization files
└── CMakeLists.txt          # Build configuration

External Dependencies (lib/):
├── KissFFT                 # FFT library for spectrum analysis
├── QCodeEditor             # Syntax-highlighted code editor
├── QSimpleUpdater          # Auto-update framework
├── OpenSSL                 # Cryptography for licensing
└── mdflib                  # MDF4 file format library
```

### Core Modules

#### IO Module - Hardware Abstraction Layer
```
app/src/IO/
├── Manager.{h,cpp}               # Central I/O coordinator
├── FrameReader.{h,cpp}           # Stream-to-frame parser
├── HAL_Driver.h                  # Abstract driver interface
├── CircularBuffer.h              # Lock-free SPSC ring buffer
├── Checksum.{h,cpp}              # CRC8/16/32 validation
├── FrameConsumer.h               # Threaded data consumer pattern
└── Drivers/
    ├── UART.{h,cpp}              # Serial port driver
    ├── Network.{h,cpp}           # TCP/UDP socket driver
    ├── BluetoothLE.{h,cpp}       # BLE GATT client
    ├── Audio.{h,cpp}             # Audio device input (Pro)
    ├── Modbus.{h,cpp}            # Modbus TCP/RTU (Pro)
    └── CANBus.{h,cpp}            # CAN with DBC support (Pro)
```

**Design Pattern**: Hardware Abstraction Layer (HAL)
- Abstract base class `HAL_Driver` defines protocol-agnostic interface
- Each driver implements: `open()`, `close()`, `write()`, `isOpen()`, `isReadable()`, `isWritable()`
- Enables seamless switching between data sources without upper-layer changes
- Lock-free circular buffer (`CircularBuffer.h`) provides SPSC thread-safe data path
- `FrameReader` parses byte streams into frames using configurable delimiters

**Key Features**:
- **Lock-Free Data Path**: Atomic operations with `memory_order_acquire/release`
- **Zero-Copy Architecture**: Immutable `shared_ptr<const QByteArray>` for data distribution
- **Pattern Matching**: KMP algorithm for O(n+m) delimiter detection in circular buffer
- **Overflow Protection**: Atomic overflow counters with 10 MB default capacity
- **Immutability-Based Thread Safety**: `FrameReader` achieves thread safety through immutability rather than locks. Configuration is set once in the constructor; when settings change, `IO::Manager::resetFrameReader()` recreates the instance with updated settings.

#### DataModel Module - Frame Processing
```
app/src/DataModel/
├── Frame.{h,cpp}                 # Parsed data frame container
├── FrameBuilder.{h,cpp}          # Frame construction & parsing
├── FrameParser.{h,cpp}           # JavaScript custom parser
├── FrameConsumer.h               # Generic threaded consumer
├── Group.{h,cpp}                 # Dataset grouping
├── Dataset.{h,cpp}               # Individual data point
├── ProjectModel.{h,cpp}          # JSON project configuration
└── DBCImporter.{h,cpp}           # CAN DBC file parser (Pro)
```

**Design Pattern**: Immutable Data + Lock-Free Queues
- `Frame` objects are immutable and reference-counted via `shared_ptr<const Frame>`
- `FrameConsumer<T>` provides generic threaded consumer with dual-trigger flushing:
  - **Time-based**: Periodic flush every N milliseconds
  - **Threshold-based**: Flush when queue reaches N items
- Lock-free queue (`moodycamel::ReaderWriterQueue`) for zero-mutex data distribution
- Frames distributed simultaneously to Dashboard, CSV export, MDF4 export without copying

**Hotpath Optimization** (`FrameBuilder::hotpathTxFrame`):
- Pre-allocated `shared_ptr<Frame>` reused across calls to avoid allocation overhead
- Timestamped frames created only when consumers are enabled (CSV/MDF4 export, Plugin server)
- Cached `m_timestampedFramesEnabled` flag updated via signals to avoid per-frame checks

**Parsing Pipeline**:
1. `IO::FrameReader` extracts raw frame bytes from stream
2. `FrameBuilder` parses frames into structured `Frame` objects
3. Custom JavaScript parsing supported via `FrameParser` (QuickJS engine)
4. `ProjectModel` defines JSON schema mapping frames to dashboard widgets

#### UI Module - Visualization
```
app/src/UI/
├── Dashboard.{h,cpp}             # Main dashboard controller
├── WindowManager.{h,cpp}         # Multi-window management
├── Taskbar.{h,cpp}               # Dashboard taskbar
└── Widgets/                      # Visualization components
    ├── Plot.{h,cpp}              # Time-series line plot
    ├── MultiPlot.{h,cpp}         # Multi-channel plotting
    ├── FFTPlot.{h,cpp}           # Frequency spectrum analyzer
    ├── Gauge.{h,cpp}             # Analog gauge
    ├── Bar.{h,cpp}               # Bar graph & level meter
    ├── Compass.{h,cpp}           # Directional compass
    ├── Gyroscope.{h,cpp}         # 3-axis gyroscope
    ├── Accelerometer.{h,cpp}     # 3-axis accelerometer
    ├── GPS.{h,cpp}               # GPS map visualization
    ├── Plot3D.{h,cpp}            # 3D scatter plot (Pro)
    ├── DataGroup.{h,cpp}         # Grouped dataset view
    └── LEDPanel.{h,cpp}          # LED indicator panel
```

**Design Pattern**: Model-View separation with Qt's Model/View framework
- C++ model layer (`Dashboard`, widget controllers) + QML view layer
- Widgets update at user defined frame rate regardless of data frame rate
- Qt parent-child ownership ensures automatic memory management
- Each widget subscribes to specific datasets from `Frame` objects

**Performance**:
- **Frame Rate**: Supports 1000+ Hz data acquisition
- **Update Rate**: 60 Hz UI refresh (configurable)
- **Widget Capacity**: 50+ simultaneous widgets without performance degradation

#### Additional Modules

**CSV Module** (`app/src/CSV/`)
- **Player**: Timeline-based CSV file playback with speed control
- **Export**: Real-time CSV export with configurable delimiters

**MDF4 Module** (`app/src/MDF4/`) - Pro Feature
- **Player**: Automotive MDF4/MF4 file playback (CAN, LIN, FlexRay, analog)
- **Export**: Real-time MDF4 export with channel mapping
- Streaming implementation for multi-GB files using `mdflib`

**Console Module** (`app/src/Console/`)
- **Terminal**: ANSI/VT100 terminal emulation
- **Handler**: Debug message router
- **Export**: Console log export

**Misc Module** (`app/src/Misc/`)
- **ModuleManager**: Application lifecycle management
- **ThemeManager**: Light/dark theme support
- **Translator**: i18n/l10n framework
- **CommonFonts**: Platform-specific font management
- **JsonValidator**: Security-hardened JSON validator (DoS prevention)
- **TimerEvents**: High-precision event scheduling

### Data Flow

#### Live Data Reception
```
Hardware Device
    ↓
[Worker Thread] IO::Driver (UART/Network/BLE/etc.)
    ↓ processData() → Circular buffer with KMP pattern matching
[Signal] HAL_Driver::dataReceived(ByteArrayPtr)
    ↓
[Main Thread] IO::Manager
    ↓ frameReady(QByteArray)
IO::FrameReader::readFrames()
    ↓ Extract frames using start/end delimiters or fixed length
DataModel::FrameBuilder::hotpathRxFrame()
    ↓ Parse JSON/CSV/custom format + checksum validation
DataModel::Frame (immutable, shared_ptr)
    ↓ Zero-copy distribution to consumers
┌─────────────────┬──────────────────┬──────────────────┐
│  UI::Dashboard  │  CSV::Export     │  MDF4::Export    │
│  (main thread)  │  (worker thread) │  (worker thread) │
└─────────────────┴──────────────────┴──────────────────┘
```

## Build Instructions

### Prerequisites

**All Platforms:**
- CMake 3.20+
- Qt 6.9.2
- C++20 compiler (GCC 11+, Clang 14+, MSVC 2022+)

**Linux:**
```bash
sudo apt install libgl1-mesa-dev build-essential zlib1g-dev libexpat1-dev
```

**macOS:**
```bash
# Xcode Command Line Tools required
xcode-select --install

# Optional: install via Homebrew if not present
brew install zlib expat
```

**Windows:**
```powershell
# Install via vcpkg
vcpkg install zlib expat
```

### Build Commands

**Standard GPL Build** (recommended for development):
```bash
mkdir build && cd build
cmake ../ -DPRODUCTION_OPTIMIZATION=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

**Debug Build with Sanitizers**:
```bash
cmake ../ -DDEBUG_SANITIZER=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j$(nproc)
```

**Hardened Production Build** (security flags):
```bash
cmake ../ \
  -DPRODUCTION_OPTIMIZATION=ON \
  -DENABLE_HARDENING=ON \
  -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

**Profile-Guided Optimization** (10-20% performance gain):
```bash
# Stage 1: Build with instrumentation
cmake ../ \
  -DENABLE_PGO=ON \
  -DPGO_STAGE=GENERATE \
  -DPRODUCTION_OPTIMIZATION=ON \
  -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)

# Stage 2: Run with typical workloads
./app/Serial-Studio-GPL3
# (Load projects, visualize data, stress test)

# Stage 3: Rebuild with profile data
cmake ../ \
  -DENABLE_PGO=ON \
  -DPGO_STAGE=USE \
  -DPRODUCTION_OPTIMIZATION=ON \
  -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

### CMake Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_GPL3` | `ON` | Build GPL-licensed version (no Pro features) |
| `BUILD_COMMERCIAL` | `OFF` | Enable Pro features (requires license key) |
| `PRODUCTION_OPTIMIZATION` | `OFF` | Enable -O3, LTO, vectorization |
| `ENABLE_HARDENING` | `OFF` | Security flags (stack canaries, RELRO, NX) |
| `ENABLE_PGO` | `OFF` | Profile-Guided Optimization |
| `DEBUG_SANITIZER` | `OFF` | AddressSanitizer + UBSan (debug only) |
| `USE_SYSTEM_ZLIB` | `OFF` | Use system zlib (required for Flathub) |
| `USE_SYSTEM_EXPAT` | `OFF` | Use system expat (required for Flathub) |

## Code Style

This project uses **clang-format** with LLVM base style:
- **Column Limit**: 80 characters
- **Indentation**: 2 spaces
- **Braces**: Allman style (braces on new lines)
- **Pointers**: `int *ptr` (pointer binds to name)

**Naming Conventions** (enforced by clang-tidy):
- **Classes**: `CamelCase`
- **Functions**: `camelCase`
- **Variables/Parameters**: `lower_case`
- **Private Members**: `lower_case_` (trailing underscore)
- **Constants/constexpr**: `kCamelCase`
- **Macros**: `UPPER_CASE`

**Comment Policy**:
- ✅ **Block comments** above code
- ✅ **Doxygen-style** function documentation
- ❌ **Inline end-of-line comments** (prohibited)

**Example**:
```cpp
/**
 * @brief Calculates CRC16 checksum for the given data.
 *
 * Uses polynomial 0x1021 (CRC-16-CCITT).
 *
 * @param data Input byte array
 * @return 16-bit checksum value
 */
uint16_t calculateCRC16(const QByteArray &data);
```

## Performance Characteristics

**Optimization Techniques**:
- Lock-free circular buffers (SPSC) with atomic operations
- Zero-copy data distribution via `shared_ptr<const T>`
- Branch prediction hints (`[[likely]]`, `[[unlikely]]`)
- Compile-time checksums with `constexpr`
- KMP pattern matching algorithm (O(n+m))
- SIMD-optimized math (platform-dependent)
- Profile-Guided Optimization (PGO) support

## Testing

**Current Status**: ⚠️ No automated test infrastructure

**Planned Testing Framework**:
- Unit tests: Google Test or Catch2
- Integration tests: Qt Test
- Performance benchmarks: Google Benchmark

**Critical Testing Targets**:
- Frame parsing (`FrameBuilder`, `FrameReader`)
- Checksum validation (`IO::Checksum`)
- Circular buffer operations (`CircularBuffer`)
- JSON validation (`JsonValidator`)
- Driver connection/disconnection lifecycle

## TODO List

### High Priority (Critical for Maintainability)

- [ ] **Refactor ProjectModel.cpp** (4,062 lines → 5 focused classes)
  - Split into: `ProjectLoader`, `ProjectValidator`, `ProjectSerializer`, `ItemModelBuilder`, `ProjectController`
  - Add unit tests for each component
  - Reduce complexity and improve maintainability
  - **Impact**: Easier to modify project schema, safer refactoring

- [ ] **Implement Test Infrastructure**
  - Add Google Test or Catch2 framework
  - Write unit tests for critical paths (Frame parsing, checksums, circular buffer)
  - Target 70%+ code coverage
  - Set up CI/CD pipeline for automated testing
  - **Impact**: Safe refactoring, regression prevention, confidence in changes

- [ ] **Reduce Singleton Usage**
  - Implement dependency injection container
  - Create interfaces: `IIOManager`, `IFrameBuilder`, `IDashboard`
  - Pass dependencies via constructors instead of `instance()`
  - **Impact**: Improved testability, clearer dependencies, safer lifecycle

### Medium Priority (Code Quality & Performance)

- [ ] **Refactor Large Widget Classes**
  - `Terminal.cpp` (1,708 lines) → Model-View-Controller split
  - `GPS.cpp` (1,417 lines) → Extract `MapRenderer` and `GPSDataModel`
  - `Plot3D.cpp` (1,521 lines) → Separate rendering engine
  - **Impact**: Easier widget maintenance, reusable components

- [ ] **Reduce Conditional Compilation**
  - Consolidate platform-specific code in `Platform/` module
  - Use strategy pattern instead of `#ifdef` blocks
  - Implement runtime feature flags where appropriate
  - **Impact**: Better code readability, simpler build matrix

- [ ] **Extract Magic Numbers to Constants**
  - Create `Constants.h` with named constants
  - Document rationale for buffer sizes, thresholds, timer intervals
  - Make values configurable via settings where appropriate
  - **Impact**: Clearer intent, easier tuning, better documentation

- [ ] **Implement Object Pooling for QStandardItem**
  - `ProjectModel.cpp` has 83 allocations without pooling
  - Measure memory/performance impact on large projects
  - Consider alternative data models (not QStandardItemModel)
  - **Impact**: Reduced memory fragmentation, faster project loading

### Low Priority (Nice to Have)

- [ ] **Add Performance Benchmarks**
  - Benchmark frame parsing throughput
  - Measure widget update performance
  - Profile memory usage patterns
  - Establish performance regression baselines

- [ ] **Improve Error Handling**
  - Add structured exception handling (currently only 15 try/catch blocks)
  - Implement error recovery for driver failures
  - Add user-friendly error messages with recovery suggestions

- [ ] **API Documentation**
  - Generate Doxygen documentation site
  - Add architecture diagrams
  - Create developer onboarding guide
  - Document extension points (custom widgets, drivers)

- [ ] **Plugin System Enhancements**
  - Expand plugin API for custom drivers
  - Add widget plugin support
  - Implement plugin marketplace/discovery

### Future Enhancements

- [ ] **WebAssembly Build**
  - Port to Qt for WebAssembly
  - Browser-based Serial Studio (limited driver support)

- [ ] **Cloud Integration**
  - Real-time cloud data streaming
  - Remote device monitoring
  - Collaborative project sharing

- [ ] **Machine Learning Integration**
  - Anomaly detection in telemetry streams
  - Predictive analytics
  - Auto-classification of data patterns

## Licensing

Serial Studio uses **dual licensing**:

**GPL-3.0** (Open Source):
- Free for open-source projects
- Must share modifications under GPL-3.0
- Files marked with `SPDX-License-Identifier: GPL-3.0-only`

**Commercial** (Pro Features):
- Proprietary license for commercial use
- Includes: MQTT, Modbus, CAN Bus, MDF4, 3D widgets, audio input
- Files marked with `SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial`

**Dual-Licensed Components**:
- Core modules available under both licenses
- Files marked with `GPL-3.0-only OR LicenseRef-SerialStudio-Commercial`

## Contributing

See the main repository `CONTRIBUTING.md` for contribution guidelines.

**Development Workflow**:
1. Fork the repository
2. Create a feature branch
3. Follow code style guidelines (clang-format)
4. Add tests for new functionality
5. Submit pull request with clear description

**Code Review Checklist**:
- [ ] Follows naming conventions
- [ ] No inline comments (block comments only)
- [ ] SPDX license header present
- [ ] Doxygen documentation for public APIs
- [ ] No memory leaks (use smart pointers)
- [ ] Thread safety documented
- [ ] Performance impact considered

## Resources

- **Project Website**: [serial-studio.com](https://serial-studio.com)
- **Documentation**: [docs.serial-studio.com](https://docs.serial-studio.com)
- **Issue Tracker**: [GitHub Issues](https://github.com/Serial-Studio/Serial-Studio/issues)
- **Discussion Forum**: [GitHub Discussions](https://github.com/Serial-Studio/Serial-Studio/discussions)

**Third-Party Libraries**:
- **KissFFT**: Fast Fourier Transform library
- **QCodeEditor**: Syntax-highlighted code editor
- **QSimpleUpdater**: Auto-update framework
- **mdflib**: MDF4 file format library
- **QuickJS**: JavaScript engine for custom parsing
- **moodycamel::ReaderWriterQueue**: Lock-free queue implementation

**License**: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
**Version**: See CMakeLists.txt for current version
**Last Updated**: 2026-01-05
