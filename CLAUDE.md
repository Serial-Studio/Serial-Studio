# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Serial Studio is a cross-platform telemetry dashboard and real-time data visualization application built with Qt 6.9.2 and C++20. It supports data input from serial ports, Bluetooth LE, MQTT, Modbus TCP/RTU, CAN Bus, TCP/UDP sockets, and audio devices.

**Key Features:**
- **Multi-Protocol Support**: UART, TCP/UDP, Bluetooth LE, Audio, Modbus TCP/RTU, CAN Bus
- **Real-Time Visualization**: 15+ widget types (plots, gauges, maps, 3D graphics, FFT)
- **File Playback**: CSV and MDF4/MF4 (automotive) file support with timeline controls
- **High Performance**: Lock-free data paths optimized for high-frequency data 
- **Custom Parsing**: JavaScript-based frame parsing with JSON schema definition

**Technical Highlights:**
- C++20 with concepts, constexpr optimizations, and move semantics
- Lock-free circular buffers with atomic operations
- Hardware abstraction layer (HAL) for protocol-agnostic data acquisition
- Zero-copy dashboard updates via const references (no heap allocation per frame)
- Nanosecond-precision timestamps via `std::chrono::steady_clock`
- Single-allocation timestamped frames for async consumers (CSV/MDF4 export)
- Profile-Guided Optimization (PGO) support for possible performance gains

## Build Commands

```bash
# Standard GPL build (default)
mkdir build && cd build
cmake ../ -DPRODUCTION_OPTIMIZATION=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)

# Debug build with sanitizers
cmake ../ -DDEBUG_SANITIZER=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j$(nproc)

# Build using system libraries (for Flathub/sandboxed environments)
# Note: ENABLE_HARDENING is auto-enabled when USE_SYSTEM_ZLIB or USE_SYSTEM_EXPAT is ON
cmake ../ -DUSE_SYSTEM_ZLIB=ON -DUSE_SYSTEM_EXPAT=ON -DPRODUCTION_OPTIMIZATION=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)

# Hardened production build (with security flags)
cmake ../ -DPRODUCTION_OPTIMIZATION=ON -DENABLE_HARDENING=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)

# Profile-Guided Optimization build (3-stage process for maximum performance)
# Stage 1: Build with instrumentation
cmake ../ -DENABLE_PGO=ON -DPGO_STAGE=GENERATE -DPRODUCTION_OPTIMIZATION=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
# Stage 2: Run the application with typical workloads to collect profile data
./app/Serial-Studio-GPL3  # Use the app normally (load projects, visualize data, etc.)
# Stage 3: Rebuild with profile optimization
cmake ../ -DENABLE_PGO=ON -DPGO_STAGE=USE -DPRODUCTION_OPTIMIZATION=ON -DCMAKE_BUILD_TYPE=Release
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
- `PRODUCTION_OPTIMIZATION=ON`: Enable release optimizations (O3, LTO, vectorization)
- `ENABLE_HARDENING=ON`: Enable security hardening flags (stack protection, ASLR, RELRO, etc.)
  - Auto-enabled for Flathub builds (when USE_SYSTEM_ZLIB or USE_SYSTEM_EXPAT is ON)
  - Adds: stack canaries, FORTIFY_SOURCE, control-flow protection, full RELRO (-z relro -z now)
- `ENABLE_PGO=ON`: Enable Profile-Guided Optimization (requires 3-stage build, see above)
  - `PGO_STAGE=GENERATE`: Build with instrumentation to collect profile data
  - `PGO_STAGE=USE`: Build with profile optimization
- `DEBUG_SANITIZER=ON`: Enable AddressSanitizer and UBSan (debug builds only)
- `USE_SYSTEM_ZLIB=ON`: Use system-provided zlib instead of downloading from GitHub (required for Flathub builds)
- `USE_SYSTEM_EXPAT=ON`: Use system-provided expat instead of downloading from GitHub (required for Flathub builds)

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
│   ├── API/                # API Server & command handlers
│   ├── ThirdParty/         # Embedded libraries
│   ├── SerialStudio.h      # Central enums and constants
│   └── Concepts.h          # C++20 concepts for type constraints
├── qml/                    # QML declarative UI files
│   ├── Dialogs/            # Application dialogs
│   ├── ProjectEditor/      # JSON project editor UI
│   ├── Widgets/            # Dashboard widget views
│   └── Windows/            # Main window & panels
├── assets/                 # Application resources
└── translations/           # Internationalization files

lib/                        # Third-party dependencies
├── KissFFT                 # FFT library for spectrum analysis
├── QCodeEditor             # Syntax-highlighted code editor
├── QSimpleUpdater          # Auto-update framework
├── OpenSSL                 # Cryptography for licensing
└── mdflib                  # MDF4 file format library
```

### Core Modules

#### IO Module - Hardware Abstraction Layer

**Location:** `app/src/IO/`

**Files:**
- `Manager.{h,cpp}` - Central I/O coordinator singleton (app/src/IO/Manager.h:145)
- `FrameReader.{h,cpp}` - Lock-free stream-to-frame parser with immutability-based thread safety (app/src/IO/FrameReader.h:52)
- `HAL_Driver.h` - Abstract driver interface for protocol-agnostic data acquisition (app/src/IO/HAL_Driver.h:95)
- `CircularBuffer.h` - Lock-free SPSC ring buffer with KMP pattern matching (app/src/IO/CircularBuffer.h:34)
- `Checksum.{h,cpp}` - CRC8/16/32 validation with constexpr optimizations (app/src/IO/Checksum.h:30)
- `FrameConsumer.h` - Generic threaded consumer pattern template (app/src/IO/FrameConsumer.h:45)
- `Drivers/` - Protocol implementations (UART, Network, BluetoothLE, Audio, Modbus, CANBus)

**Design Pattern:** Hardware Abstraction Layer (HAL)
- Abstract base class `HAL_Driver` defines protocol-agnostic interface
- Each driver implements: `open()`, `close()`, `write()`, `isOpen()`, `isReadable()`, `isWritable()`
- Enables seamless switching between data sources without upper-layer changes
- Lock-free circular buffer provides SPSC thread-safe data path
- `FrameReader` parses byte streams into frames using configurable delimiters

**Critical Performance Features:**

1. **Lock-Free CircularBuffer (app/src/IO/CircularBuffer.h)**
   - SPSC (Single Producer Single Consumer) using `std::atomic` with memory_order_acquire/release
   - KMP algorithm for O(n+m) delimiter detection in circular buffer
   - Default capacity: 10 MB (`1024 * 1024 * 10`)
   - Atomic overflow counters with zero mutex overhead
   - Branch prediction hints: `[[likely]]` and `[[unlikely]]` for hotpath optimization

2. **Immutability-Based Thread Safety (app/src/IO/FrameReader.h:52)**
   - **NO locks, NO mutexes** - achieves thread safety through immutability
   - Configuration set **ONCE** in constructor
   - When settings change, `IO::Manager::resetFrameReader()` **destroys and recreates** the instance
   - `processData()` can safely read member variables without synchronization
   - Lock-free queue: `moodycamel::ReaderWriterQueue<QByteArray> m_queue{4096}`
   - **DO NOT add mutexes to FrameReader** - this design is intentional for 256 KHz+ data rates

3. **Immutable Data Distribution (app/src/IO/HAL_Driver.h:32)**
   - `ByteArrayPtr = shared_ptr<const QByteArray>` for zero-copy sharing
   - Factory functions: `makeByteArray(const QByteArray&)` and `makeByteArray(QByteArray&&)`
   - Thread-safe via atomic reference counting
   - Compile-time const-correctness

#### DataModel Module - Frame Processing

**Location:** `app/src/DataModel/`

**Files:**
- `Frame.{h,cpp}` - Parsed data frame container with groups and datasets (app/src/DataModel/Frame.h:1013)
- `FrameBuilder.{h,cpp}` - Frame construction & parsing with hotpath optimization (app/src/DataModel/FrameBuilder.cpp:817)
- `FrameParser.{h,cpp}` - JavaScript custom parser using QuickJS engine
- `FrameConsumer.h` - Generic threaded consumer with dual-trigger flushing (app/src/DataModel/FrameConsumer.h:406)
- `Group.{h,cpp}` - Dataset grouping
- `Dataset.{h,cpp}` - Individual data point
- `ProjectModel.{h,cpp}` - JSON project configuration (⚠️ 4,062 lines - refactoring target)
- `DBCImporter.{h,cpp}` - CAN DBC file parser (Pro)

**Design Pattern:** Immutable Data + Lock-Free Queues
- `Frame` objects are value types that can be efficiently copied when needed
- `FrameConsumer<T>` provides generic threaded consumer with dual-trigger flushing:
  - **Time-based**: Periodic flush every N milliseconds
  - **Threshold-based**: Flush when queue reaches N items
- Lock-free queue (`moodycamel::ReaderWriterQueue`) for zero-mutex data distribution
- Dashboard receives frames by const reference; exports receive isolated copies

**Critical Hotpath Optimization (app/src/DataModel/FrameBuilder.cpp:817):**

```cpp
void DataModel::FrameBuilder::hotpathTxFrame(const DataModel::Frame &frame)
{
  static auto &csvExport = CSV::Export::instance();      // Cached reference
  static auto &mdf4Export = MDF4::Export::instance();    // Cached reference
  static auto &dashboard = UI::Dashboard::instance();    // Cached reference
  static auto &pluginsServer = API::Server::instance(); // Cached reference

  // HOTPATH: Dashboard receives frame by const reference (zero-copy, no allocation)
  dashboard.hotpathRxFrame(frame);

  // UNLIKELY PATH: Only when exports/plugins enabled
  if (m_timestampedFramesEnabled) [[unlikely]]
  {
    // Single allocation: Frame embedded by value + nanosecond timestamp
    auto timestampedFrame = std::make_shared<DataModel::TimestampedFrame>(frame);
    csvExport.hotpathTxFrame(timestampedFrame);
    mdf4Export.hotpathTxFrame(timestampedFrame);
    pluginsServer.hotpathTxFrame(timestampedFrame);
  }
}
```

**Key Optimizations:**
1. **Zero-copy dashboard path**: Dashboard receives `const Frame&` reference - no heap allocation
2. **Single allocation for exports**: `make_shared<TimestampedFrame>` embeds Frame **by value** (not nested shared_ptr)
3. **Cached flag**: `m_timestampedFramesEnabled` updated via signal connections (avoids per-frame checks)
4. **Static references**: Cached singleton references avoid repeated `instance()` calls
5. **Branch prediction**: `[[unlikely]]` hint for export path (typically disabled)

**TimestampedFrame Structure (app/src/DataModel/Frame.h:860):**

```cpp
struct TimestampedFrame
{
  using SteadyClock = std::chrono::steady_clock;
  using SteadyTimePoint = SteadyClock::time_point;

  DataModel::Frame data;           // Embedded by VALUE (not pointer!)
  SteadyTimePoint timestamp;       // Nanosecond precision, 8 bytes

  // Move-only semantics (prevent accidental copies)
  TimestampedFrame(TimestampedFrame &&) noexcept = default;
  TimestampedFrame(const TimestampedFrame &) = delete;
};
```

**Memory Layout:**
- **Single allocation**: When used with `make_shared`, only ONE heap allocation (vs. two with nested shared_ptr)
- **Nanosecond precision**: `steady_clock` provides monotonic time with minimal syscall overhead
- **CSV format**: 9 decimal places in timestamp column (relative time from first frame)
- **MDF4 format**: Wall-clock derived from cached baseline (avoids clock drift)

**FrameConsumer Pattern (app/src/DataModel/FrameConsumer.h:406):**

Template design for async export workers:
- Lock-free queue: `moodycamel::ReaderWriterQueue<T>`
- Atomic state flags: `std::atomic<bool> m_consumerEnabled`
- Dual-trigger flushing: time-based (1000ms) + threshold-based (1024 items)
- Dedicated worker thread with graceful shutdown
- Batch processing: drains entire queue into local buffer

**Usage Examples:**
- `CSV::Export` - Config: {queueCapacity: 8192, flushThreshold: 1024, timerIntervalMs: 1000}
- `MDF4::Export` - Config: {queueCapacity: 8192, flushThreshold: 1024, timerIntervalMs: 1000}
- `API::Server` - Config: {queueCapacity: 2048, flushThreshold: 512, timerIntervalMs: 1000}

#### UI Module - Visualization

**Location:** `app/src/UI/`

**Files:**
- `Dashboard.{h,cpp}` - Main dashboard controller
- `WindowManager.{h,cpp}` - Multi-window management
- `Taskbar.{h,cpp}` - Dashboard taskbar
- `Widgets/` - 15+ visualization components

**Widget Types:**
- `Plot.{h,cpp}` - Time-series line plot
- `MultiPlot.{h,cpp}` - Multi-channel plotting
- `FFTPlot.{h,cpp}` - Frequency spectrum analyzer
- `Gauge.{h,cpp}` - Analog gauge
- `Bar.{h,cpp}` - Bar graph & level meter
- `Compass.{h,cpp}` - Directional compass
- `Gyroscope.{h,cpp}` - 3-axis gyroscope
- `Accelerometer.{h,cpp}` - 3-axis accelerometer
- `GPS.{h,cpp}` - GPS map visualization
- `Plot3D.{h,cpp}` - 3D scatter plot (Pro)
- `DataGroup.{h,cpp}` - Grouped dataset view
- `LEDPanel.{h,cpp}` - LED indicator panel

**Design Pattern:** Model-View separation with Qt's Model/View framework
- C++ model layer (`Dashboard`, widget controllers) + QML view layer
- Widgets update at configurable frame rate (default 60 Hz) regardless of data frame rate
- Qt parent-child ownership ensures automatic memory management
- Each widget subscribes to specific datasets from `Frame` objects

#### Additional Modules

**CSV Module** (`app/src/CSV/`)
- **Player**: Timeline-based CSV file playback with speed control
- **Export**: Real-time CSV export with configurable delimiters and 9 decimal timestamp precision

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
- **ThemeManager**: Light/dark theme support with platform-native colors
- **Translator**: i18n/l10n framework
- **CommonFonts**: Platform-specific font management
- **JsonValidator**: Security-hardened JSON validator (DoS prevention)
- **TimerEvents**: High-precision event scheduling

**Concepts.h** (`app/src/Concepts.h`) - C++20 Type Constraints
- `Numeric` - Constrains to integral or floating-point types
- `ByteLike` - Constrains to byte-sized integral types (uint8_t, char, etc.)
- `Comparable` - Ensures operator== and operator!= support
- `Hashable` - Ensures std::hash specialization exists
- `Serializable` - Ensures read()/serialize() methods exist
- `Frameable` - Ensures Frame-like structure (groups, actions, title)
- `Driver` - Ensures HAL_Driver interface compliance

**SerialStudio.h** (`app/src/SerialStudio.h`) - Central Enums
- `BusType` - Available data sources (UART, Network, BluetoothLE, Audio, ModBus, CanBus)
- `OperationMode` - Dashboard modes (ProjectFile, DeviceSendsJSON, QuickPlot)
- `DecoderMethod` - Data decoding (PlainText, Hexadecimal, Base64, Binary)
- `FrameDetection` - Frame parsing strategies (EndDelimiterOnly, StartAndEndDelimiter, NoDelimiters, StartDelimiterOnly)
- Widget type enums for groups, datasets, and dashboards

### Delimiter Types and Configuration

Serial Studio uses **three distinct types of delimiters** that operate at different levels. Understanding these is critical for correct project configuration and debugging parsing issues.

#### 1. Frame Delimiters (Stream-Level)

**Purpose:** Define frame boundaries in the incoming byte stream.

**Configuration:**
- `frameStart` - Start delimiter (e.g., `/*`, `{`, `<frame>`)
- `frameEnd` - End delimiter (e.g., `*/`, `}`, `</frame>`, `;`, `\n`)
- `frameDetection` - Detection mode (enum FrameDetection):
  - `0` - EndDelimiterOnly (most common)
  - `1` - StartAndEndDelimiter
  - `2` - NoDelimiters (fixed-length frames)
  - `3` - StartDelimiterOnly

**Implementation:**
- File: `app/src/IO/FrameReader.cpp`
- Algorithm: KMP pattern matching for delimiter detection
- Processed BEFORE frame content parsing
- The semicolon `;` or newline `\n` CAN be used as frame end delimiters

**Example:**
```json
{
  "frameStart": "/*",
  "frameEnd": ";",
  "frameDetection": 0
}
```

Incoming stream: `/*{"value":123};/*{"value":456};`
Extracted frames: `{"value":123}` and `{"value":456}`

#### 2. Line Terminators (Stream-Level)

**Purpose:** Optional suffixes after the frame end delimiter.

**Configuration:**
- `lineTerminator` - Suffix characters (e.g., `\r\n`, `\n`, `;`)

**Implementation:**
- File: `app/src/DataModel/Frame.cpp:37-63`
- Used in combination with frame delimiters for compatibility
- Common patterns:
  - `frameEnd: "*/", lineTerminator: "\r\n"` → Frame ends with `*/\r\n`
  - `frameEnd: ";", lineTerminator: "\n"` → Frame ends with `;\n`

#### 3. CSV Field Delimiters (Data-Level)

**Purpose:** Separate individual values within a CSV frame.

**Configuration:**
- **Default:** Comma `,` (hard-coded in C++ `parseCsvValues()`)
- **Custom:** Requires JavaScript parser in `frameParserCode`

**Implementation:**
- File: `app/src/DataModel/FrameBuilder.cpp:44-67`
- Function: `parseCsvValues(const QByteArray &data, QStringList &out, const int reserveHint)`
- CSV delimiters are processed AFTER frame extraction
- **Hard-coded to comma** - cannot be configured without JavaScript parser

**Default Comma Example (QuickPlot mode):**
```cpp
// QuickPlot mode: Line-based parsing (like Arduino Serial Plotter)
// NO frame delimiters - just line terminators (CR, LF, or CRLF)

Incoming stream: 1.23,4.56,7.89\n1.11,2.22,3.33\r\n
Extracted lines: "1.23,4.56,7.89" and "1.11,2.22,3.33"

// CSV parsing (parseCsvValues)
Values extracted: ["1.23", "4.56", "7.89"] and ["1.11", "2.22", "3.33"]
```

**Custom Semicolon Example (ProjectFile mode with JavaScript):**
```json
{
  "operationMode": 0,
  "frameStart": "/*",
  "frameEnd": "*/",
  "frameParserCode": "function parse(frame) { return frame.split(';'); }"
}
```

#### Operation Modes and Delimiter Behavior

| Mode | Value | Frame Delimiters | Line Terminators | CSV Delimiters | Custom Parser |
|------|-------|-----------------|------------------|----------------|---------------|
| **ProjectFile** | 0 | ✅ Configurable | ✅ Configurable | ✅ Via JavaScript | ✅ Supported |
| **DeviceSendsJSON** | 1 | ❌ Fixed `/*` `*/` | ✅ Any | N/A (JSON mode) | ❌ Ignored |
| **QuickPlot** | 2 | ❌ None (line-based) | ✅ CR/LF/CRLF only | ✅ Comma only | ❌ Ignored |

**Critical Rules:**
- **ProjectFile mode** (`operationMode: 0`) respects all custom delimiters and JavaScript parsers configured in the project JSON
- **DeviceSendsJSON mode** (`operationMode: 1`) uses fixed `/*` and `*/` frame delimiters, expects JSON payloads
- **QuickPlot mode** (`operationMode: 2`) works like Arduino Serial Plotter - NO frame delimiters, only line terminators (CR `\r`, LF `\n`, or CRLF `\r\n`), comma-separated values only

#### Performance Considerations

**Frame Delimiter Parsing:**
- KMP algorithm: O(n+m) complexity for delimiter detection
- Lock-free circular buffer with atomic operations
- Zero-copy pattern matching in `CircularBuffer::findPattern()`

**CSV Delimiter Parsing:**
- Hard-coded comma parsing: Single-pass O(n) with string trimming
- No heap allocation during parsing (uses QStringList with reserve)
- JavaScript custom parsers: QuickJS engine overhead (slower but flexible)

**When to Use Each Approach:**
- **Default comma CSV:** Best performance, use when possible
- **JavaScript parser:** Necessary for non-comma delimiters, more flexible but slower
- **Custom frame delimiters:** Use ProjectFile mode, configure in project JSON

### Threading Architecture

**Data Flow Threads:**

1. **I/O Driver Worker Threads** - One per active driver (UART/Network/BLE/etc.)
   - Processes raw hardware data on dedicated thread
   - Appends to lock-free `CircularBuffer` with atomic operations
   - Emits `dataReceived(ByteArrayPtr)` signal to main thread
   - Zero mutex contention with SPSC guarantee

2. **Main Thread** - Frame parsing and dashboard updates
   - `IO::Manager` coordinates frame extraction
   - `FrameReader` parses delimited frames using lock-free queue
   - `FrameBuilder` constructs `Frame` objects
   - `Dashboard` receives frames by const reference (zero-copy)
   - Updates UI at configurable rate (default 60 Hz)

3. **Export Worker Threads** - One per enabled export (CSV/MDF4/Plugins)
   - Consume `TimestampedFrame` objects from lock-free queues
   - Batch processing with dual-trigger flushing (time + threshold)
   - Blocking I/O isolated from main thread
   - Graceful shutdown with `Qt::BlockingQueuedConnection`

**Thread Safety Mechanisms:**
- **Lock-free queues**: `moodycamel::ReaderWriterQueue` (SPSC)
- **Atomic operations**: `std::atomic` with memory_order_acquire/release
- **Immutability**: Shared data via `shared_ptr<const T>`
- **Recreate-on-change**: FrameReader achieves thread safety by recreation, not locks

**Critical Path: Frame Reception → Dashboard Display**

```
Hardware Device
    ↓
[Worker Thread] IO::Driver (UART/Network/BLE/etc.)
    ↓ processData() → CircularBuffer with KMP pattern matching
[Signal] HAL_Driver::dataReceived(ByteArrayPtr)
    ↓
[Main Thread] IO::Manager
    ↓ frameReady(QByteArray)
IO::FrameReader::readFrames()
    ↓ Extract frames using start/end delimiters or fixed length
DataModel::FrameBuilder::hotpathRxFrame()
    ↓ Parse JSON/CSV/custom format + checksum validation
DataModel::Frame (updated in-place)
    ↓
┌──────────────────────────────────────────────────────────────┐
│  UI::Dashboard::hotpathRxFrame(const Frame&)                 │
│  - Receives frame by reference (zero-copy, no allocation)    │
│  - Only copies internally when structure changes (rare)      │
└──────────────────────────────────────────────────────────────┘
    ↓ (only when CSV/MDF4/Plugins enabled - [[unlikely]] path)
std::make_shared<TimestampedFrame>(frame)
    ↓ Single allocation: Frame embedded by value + steady_clock timestamp
    ↓ Nanosecond precision via std::chrono::steady_clock
┌───────────────────┬───────────────────┬───────────────────┐
│  CSV::Export      │  MDF4::Export     │  API::Server  │
│  (worker thread)  │  (worker thread)  │  (worker thread)  │
│                   │                   │                   │
│  Uses steady_clock│  Derives wall-    │  JSON serialize   │
│  for relative     │  clock from       │  & broadcast      │
│  timestamps (9    │  steady_clock     │                   │
│  decimal places)  │  baseline offset  │                   │
└───────────────────┴───────────────────┴───────────────────┘
```

### Performance Hotpaths

**Critical Path Optimization Summary:**

**Allocation Budget:**
- **Dashboard**: 0 allocations per frame (receives const reference)
- **Exports**: 1 `make_shared<TimestampedFrame>` per frame (only when enabled)
- **Timestamps**: `steady_clock::now()` provides nanosecond precision in single syscall

**Optimizations Applied:**

1. **Zero-Copy Dashboard Path**
   - Dashboard receives `const Frame&` reference
   - No heap allocation on hotpath
   - Only copies when frame structure changes (rare)

2. **Single-Allocation Exports** (when enabled - [[unlikely]] path)
   - `std::make_shared<TimestampedFrame>(frame)` - ONE allocation
   - Frame embedded by value (not nested shared_ptr)
   - Nanosecond timestamp via `steady_clock::now()`

3. **Lock-Free Data Structures**
   - CircularBuffer: Atomic head/tail pointers, SPSC guarantee
   - ReaderWriterQueue: Wait-free enqueue guarantee
   - No mutex contention on hotpath

4. **Cached Flags & References**
   - `m_timestampedFramesEnabled` cached via signal connections
   - Static singleton references in hotpath functions
   - Avoids repeated lookups in tight loops

5. **Branch Prediction Hints**
   - `[[likely]]` for common case (successful frame parsing)
   - `[[unlikely]]` for error cases, export enabled, empty data
   - Compiler can optimize code layout for better instruction cache usage

6. **C++20 Concepts**
   - Compile-time type checking for templates
   - Better error messages
   - Self-documenting template constraints
   - Zero runtime overhead

### Design Patterns

**1. Hardware Abstraction Layer (HAL)**
- Abstract base: `IO::HAL_Driver` (app/src/IO/HAL_Driver.h:95)
- Implementations: UART, Network, BluetoothLE, Audio, Modbus, CANBus
- Enables protocol-agnostic data acquisition
- All drivers emit `dataReceived(ByteArrayPtr)` signal

**2. Immutable Data Distribution**
- `ByteArrayPtr = shared_ptr<const QByteArray>` (app/src/IO/HAL_Driver.h:32)
- Zero-copy sharing across threads
- Thread-safe via atomic reference counting
- Factory functions with move semantics

**3. Lock-Free Producer-Consumer**
- Template: `FrameConsumer<T>` (app/src/DataModel/FrameConsumer.h:406)
- Dual-trigger flushing: time-based + threshold-based
- Implementations: CSV::Export, MDF4::Export, API::Server
- Graceful shutdown with blocking wait

**4. Immutability-Based Thread Safety (FrameReader)**
- Configuration set ONCE in constructor (app/src/IO/FrameReader.h:52)
- Instance recreated on settings change via `IO::Manager::resetFrameReader()`
- No locks needed - config is immutable during lifetime
- **DO NOT add mutexes** - this design is intentional

**5. Singleton Pattern**
- 36 singleton classes found in codebase
- Pattern: Meyer's singleton with deleted copy/move constructors
- Critical singletons: IO::Manager, FrameBuilder, Dashboard, CSV::Export, MDF4::Export
- **Known issue**: High coupling, testing difficulties (see below)

### Known Architectural Issues

**From app/README.md TODO List:**

**1. Singleton Overuse (36 classes)**
- High coupling between modules
- Testing difficult (global state)
- Initialization order dependencies unclear
- **Recommendation**: Implement dependency injection container
- **Impact**: Improved testability, clearer dependencies, safer lifecycle

**2. ProjectModel.cpp (4,062 lines!)**
- Monolithic class with too many responsibilities
- **Recommendation**: Split into 5 focused classes:
  - `ProjectLoader` - File I/O
  - `ProjectValidator` - JSON schema validation
  - `ProjectSerializer` - JSON serialization
  - `ItemModelBuilder` - QStandardItemModel construction
  - `ProjectController` - Coordination logic
- **Impact**: Easier to modify project schema, safer refactoring

**3. Large Widget Classes**
- Terminal.cpp (1,708 lines) - Should use Model-View-Controller split
- GPS.cpp (1,417 lines) - Extract MapRenderer and GPSDataModel
- Plot3D.cpp (1,521 lines) - Separate rendering engine
- **Impact**: Easier widget maintenance, reusable components

**4. No Test Infrastructure**
- Currently zero automated tests
- **Recommendation**: Add Google Test or Catch2 framework
- Target 70%+ code coverage for critical paths
- Critical testing targets: Frame parsing, checksums, circular buffer, JSON validation
- **Impact**: Safe refactoring, regression prevention

**Thread Safety Assumptions:**
- **FrameReader**: Immutable configuration (recreate on change - DO NOT add locks!)
- **CircularBuffer**: SPSC only (Single Producer Single Consumer)
- **Dashboard**: Receives frames on main thread only
- **Exports**: Lock-free enqueue from main, batch process on worker

**If you encounter crashes:**
- Check if FrameReader accessed from multiple threads concurrently
- Verify CircularBuffer used as SPSC (not MPMC)
- Ensure dashboard updates happen on main thread
- Look for violations of immutability assumptions

## Code Style

Uses clang-format with LLVM base style:
- 80 column limit
- 2-space indentation
- Braces on new lines (Allman style)
- `PointerBindsToType: false` (pointer binds to name)

Naming conventions (from .clang-tidy):
- Classes: `CamelCase`
- Functions: `camelCase`
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

## Performance Optimization Techniques

**Current Optimizations:**
- Lock-free circular buffers (SPSC) with atomic operations
- Zero-copy dashboard path via const references (no allocation per frame)
- Single-allocation `TimestampedFrame` with embedded Frame data (vs. nested shared_ptr)
- Nanosecond-precision timestamps via `std::chrono::steady_clock` (single syscall)
- Branch prediction hints (`[[likely]]`, `[[unlikely]]`)
- Compile-time checksums with `constexpr`
- KMP pattern matching algorithm (O(n+m))
- Static caching of singleton references in hotpaths
- Profile-Guided Optimization (PGO) support

**Timestamp Implementation:**
- Dashboard path: Zero allocation per frame (receives const reference)
- Export path: One `make_shared<TimestampedFrame>` allocation per frame when enabled
- Timestamps: `steady_clock` provides nanosecond precision
- CSV: 9 decimal places in timestamp column (relative time from first frame)
- MDF4: Wall-clock derived from monotonic baseline (avoids clock drift issues)

## Dual Licensing

Source files use SPDX headers indicating license:
- `GPL-3.0-only` - Open source only
- `LicenseRef-SerialStudio-Commercial` - Pro features only
- `GPL-3.0-only OR LicenseRef-SerialStudio-Commercial` - Dual-licensed

Commercial features (Pro) include: MQTT, Modbus TCP/RTU, CAN Bus (with DBC import), MDF4 playback/export, 3D visualization, audio input, and advanced plotting.

## Development Guidelines

**When working with this codebase:**

1. **Respect the threading model** - Do not add locks to FrameReader or CircularBuffer
2. **Follow the zero-copy pattern** - Pass large objects by const reference when possible
3. **Use C++20 concepts** - Add type constraints to template functions
4. **Avoid inline comments** - Use block comments and self-documenting code
5. **Test performance impact** - This is a high-performance application optimized for 192 KHz+ data rates
6. **Consider PGO builds** - Profile-Guided Optimization provides 10-20% performance gains
7. **Use branch prediction hints** - Add `[[likely]]` and `[[unlikely]]` to hotpaths
8. **Cache singleton references** - Use static references in tight loops
9. **Maintain SPDX headers** - Ensure dual-license compliance
10. **Document architectural decisions** - Update CLAUDE.md for significant changes

**Critical Files for Understanding:**
- `app/src/IO/CircularBuffer.h` - Lock-free SPSC buffer implementation
- `app/src/IO/FrameReader.h` - Immutability-based thread safety pattern
- `app/src/DataModel/FrameBuilder.cpp` - Hotpath optimization (line 817)
- `app/src/DataModel/FrameConsumer.h` - Lock-free worker pattern
- `app/src/Concepts.h` - C++20 type constraints
- `app/src/SerialStudio.h` - Central enums and constants
