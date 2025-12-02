# Serial Studio Comprehensive Testing Plan

## Overview

This plan outlines the implementation of a comprehensive unit testing suite for Serial Studio using Qt Test framework with full coverage of core modules and Qt-dependent code.

## Framework Choice: Qt Test

**Rationale:**
- Native Qt integration with QTest macros
- Built-in support for Qt signals/slots testing
- QObject testing without additional mocking frameworks
- Seamless integration with Qt's build system (CMake)
- Support for GUI event simulation
- Already included with Qt 6.9.2 installation

## Project Structure

```
Serial-Studio/
├── tests/
│   ├── CMakeLists.txt                 # Main test configuration
│   ├── test_main.cpp                  # Common test runner setup
│   │
│   ├── JSON/
│   │   ├── test_frame.cpp             # Frame structure tests
│   │   ├── test_frame_serialization.cpp
│   │   ├── test_frame_deserialization.cpp
│   │   ├── test_frame_utilities.cpp
│   │   ├── test_frame_builder.cpp
│   │   └── test_project_model.cpp
│   │
│   ├── IO/
│   │   ├── test_checksum.cpp          # All checksum algorithms
│   │   ├── test_checksum_fuzz.cpp     # Fuzzy testing for checksums
│   │   ├── test_frame_reader.cpp
│   │   ├── test_circular_buffer.cpp
│   │   ├── test_fixed_queue.cpp
│   │   └── test_console.cpp
│   │
│   ├── Misc/
│   │   ├── test_utilities.cpp
│   │   ├── test_translator.cpp
│   │   └── test_theme_manager.cpp
│   │
│   ├── DSP/
│   │   └── test_dsp.cpp               # FFT and signal processing
│   │
│   ├── Licensing/
│   │   ├── test_simple_crypt.cpp
│   │   └── test_machine_id.cpp
│   │
│   ├── CSV/
│   │   └── test_export.cpp
│   │
│   └── fixtures/
│       ├── sample_frames.json         # Test data
│       ├── test_project.json
│       └── checksum_vectors.json      # Known-good test vectors
│
├── .github/workflows/
│   └── test.yml                       # CI/CD test workflow
│
└── CMakeLists.txt                     # Updated to include tests
```

## Test Coverage Plan

### 1. JSON Module (High Priority)

**JSON::Frame (test_frame.cpp)**
- Dataset construction and field validation
- Group construction with multiple datasets
- Action construction and timer modes
- Frame alignment verification (alignas(8))
- Edge cases: empty frames, large frames

**Frame Serialization (test_frame_serialization.cpp)**
- Dataset to JSON serialization
- Group to JSON serialization
- Action to JSON serialization
- Complete Frame to JSON serialization
- Min/max ordering guarantees
- QString simplification

**Frame Deserialization (test_frame_deserialization.cpp)**
- JSON to Dataset parsing
- JSON to Group parsing
- JSON to Action parsing
- Complete JSON to Frame parsing
- Legacy field support (single "alarm" field)
- Malformed JSON handling
- Missing field defaults

**Frame Utilities (test_frame_utilities.cpp)**
- `clear_frame()` functionality
- `compare_frames()` equivalence testing
- `finalize_frame()` post-processing
- `read_io_settings()` delimiter parsing
- `get_tx_bytes()` action byte generation

**FrameBuilder (test_frame_builder.cpp)**
- Operation mode switching
- JSON map loading
- Frame parsing and construction
- Signal emission verification
- Connection state handling
- Singleton pattern integrity

**ProjectModel (test_project_model.cpp)**
- Project configuration loading
- Project serialization
- Project validation
- Settings persistence

### 2. IO Module (High Priority)

**Checksum (test_checksum.cpp)**
- XOR-8 correctness with known vectors
- MOD-256 correctness
- CRC-8 correctness
- CRC-16 correctness
- CRC-16-MODBUS correctness
- CRC-16-CCITT correctness
- Fletcher-16 correctness
- CRC-32 correctness
- Adler-32 correctness
- Endianness handling (big/little)
- Empty input handling
- Algorithm name lookup
- Unknown algorithm handling

**Checksum Fuzzy Tests (test_checksum_fuzz.cpp)**
- Random input testing (various lengths)
- Boundary condition testing (0, 1, MAX_SIZE bytes)
- Deterministic output verification
- Performance regression detection
- Collision resistance (basic)

**FrameReader (test_frame_reader.cpp)**
- Start-delimited frame detection
- End-delimited frame detection
- Start-end delimited frame detection
- Checksum validation (all types)
- Incomplete frame handling
- Multiple frames in buffer
- Operation mode switching
- Quick plot mode
- Binary data handling

**CircularBuffer (test_circular_buffer.cpp)**
- Push/pop operations
- Wraparound behavior
- Overflow handling
- Empty/full state detection
- Thread safety (if applicable)

**FixedQueue (test_fixed_queue.cpp)**
- Enqueue/dequeue operations
- Fixed size constraints
- Overflow behavior
- Performance characteristics

**Console (test_console.cpp)**
- Data reception
- Data export
- Command handling
- History management

### 3. Misc Module (Medium Priority)

**Utilities (test_utilities.cpp)**
- File revelation (mock)
- HiDPI image path resolution
- Message box formatting (mock)
- Application reboot logic

**Translator (test_translator.cpp)**
- Language detection
- Translation loading
- Locale switching

**ThemeManager (test_theme_manager.cpp)**
- Theme loading
- Color palette generation
- Theme switching

### 4. DSP Module (Medium Priority)

**DSP (test_dsp.cpp)**
- FFT processing
- Windowing functions
- Frequency domain conversions
- Known signal testing (sine, square waves)

### 5. Licensing Module (Low Priority - Sensitive)

**SimpleCrypt (test_simple_crypt.cpp)**
- Encryption correctness
- Decryption correctness
- Round-trip integrity
- Key strength validation

**MachineID (test_machine_id.cpp)**
- ID generation determinism
- Platform-specific behavior

### 6. CSV Module (Low Priority)

**Export (test_export.cpp)**
- CSV formatting
- Data serialization
- File I/O (using temp files)

## Fuzzy Testing Strategy

Fuzzy tests will be integrated throughout:

1. **Checksum Fuzzing** (dedicated file)
   - Random data payloads (1-65535 bytes)
   - Stress test all algorithms
   - Measure performance consistency

2. **Frame Parsing Fuzzing** (within test_frame_reader.cpp)
   - Malformed JSON
   - Truncated frames
   - Invalid delimiters
   - Checksum corruption

3. **Input Validation Fuzzing** (various tests)
   - Extreme numeric values (NaN, Inf, MIN, MAX)
   - Unicode and special characters
   - Buffer overflow attempts

## CMake Integration

### Main CMakeLists.txt Update
```cmake
# Add option to enable/disable tests
option(BUILD_TESTS "Build unit tests" OFF)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
```

### tests/CMakeLists.txt Structure
```cmake
cmake_minimum_required(VERSION 3.20)
project(SerialStudioTests)

# Find Qt Test module
find_package(Qt6 REQUIRED COMPONENTS Test)

# Enable CTest
enable_testing()

# Common include directories
include_directories(
  ${CMAKE_SOURCE_DIR}/app/src
  ${CMAKE_SOURCE_DIR}/lib
)

# Helper function to create tests
function(add_serial_studio_test test_name)
  add_executable(${test_name} ${ARGN})
  target_link_libraries(${test_name}
    Qt6::Test
    Qt6::Core
    # Add other required Qt modules
  )
  add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

# Add all test executables
add_serial_studio_test(test_checksum
  IO/test_checksum.cpp
  ${CMAKE_SOURCE_DIR}/app/src/IO/Checksum.cpp
)

# ... more tests ...
```

## GitHub Actions Workflow

### .github/workflows/test.yml

```yaml
name: Unit Tests

on:
  push:
    branches: [ master, develop ]
  pull_request:
    branches: [ master, develop ]

jobs:
  test-linux:
    runs-on: ubuntu-22.04
    name: 🧪 Linux Tests

    steps:
    - name: 🧰 Checkout
      uses: actions/checkout@v4

    - name: ⚙️ Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y \
          build-essential \
          libgl1-mesa-dev \
          libxkbcommon-x11-0 \
          libxcb-icccm4 \
          libxcb-image0 \
          libxcb-keysyms1 \
          libxcb-randr0 \
          libxcb-render-util0 \
          libxcb-xinerama0 \
          libxcb-xfixes0 \
          xvfb

    - name: 📦 Install Qt 6.9.2
      uses: jurplel/install-qt-action@v3
      with:
        version: '6.9.2'
        modules: 'qt5compat qtserialport qtserialbus qtconnectivity qtmqtt qtgraphs'
        cache: true

    - name: 🔨 Configure CMake
      run: |
        mkdir build
        cd build
        cmake ../ \
          -DBUILD_TESTS=ON \
          -DBUILD_GPL3=ON \
          -DCMAKE_BUILD_TYPE=Debug

    - name: 🏗️ Build Tests
      run: |
        cd build
        cmake --build . --target all -j$(nproc)

    - name: 🧪 Run Tests
      run: |
        cd build
        xvfb-run ctest --output-on-failure --verbose

    - name: 📊 Upload Test Results
      if: always()
      uses: actions/upload-artifact@v3
      with:
        name: test-results-linux
        path: build/Testing/Temporary/LastTest.log
```

## Test Data and Fixtures

### fixtures/sample_frames.json
```json
{
  "simple_frame": {
    "title": "Test Frame",
    "groups": [
      {
        "title": "Sensor Group",
        "widget": "",
        "datasets": [
          {
            "title": "Temperature",
            "value": "25.5",
            "units": "°C",
            "index": 0,
            "graph": true,
            "min": 0,
            "max": 100
          }
        ]
      }
    ],
    "actions": []
  }
}
```

### fixtures/checksum_vectors.json
```json
{
  "XOR-8": [
    {"input": "48656C6C6F", "expected": "06"},
    {"input": "DEADBEEF", "expected": "00"}
  ],
  "CRC-16": [
    {"input": "313233343536373839", "expected": "29B1"}
  ]
}
```

## Implementation Order

1. **Phase 1: Foundation (Week 1)**
   - Set up CMake test infrastructure
   - Create test_main.cpp runner
   - Implement test_checksum.cpp (complete)
   - Implement test_frame.cpp (core structures)
   - Set up GitHub Actions workflow

2. **Phase 2: Core Logic (Week 1)**
   - Implement test_frame_serialization.cpp
   - Implement test_frame_deserialization.cpp
   - Implement test_frame_utilities.cpp
   - Implement test_circular_buffer.cpp
   - Implement test_fixed_queue.cpp

3. **Phase 3: Complex Modules (Week 2)**
   - Implement test_frame_reader.cpp
   - Implement test_frame_builder.cpp
   - Implement test_project_model.cpp
   - Add fuzzy tests (checksum, frame parsing)

4. **Phase 4: Remaining Modules (Week 2)**
   - Implement DSP tests
   - Implement Misc module tests
   - Implement CSV tests
   - Implement Licensing tests (if needed)

5. **Phase 5: Integration & Polish**
   - Code coverage analysis
   - CI/CD optimization
   - Documentation updates
   - PR preparation

## Success Criteria

- ✅ All tests compile without warnings
- ✅ All tests pass on Linux CI
- ✅ Code coverage > 60% for core modules (JSON, IO, Checksum)
- ✅ Fuzzy tests run without crashes
- ✅ GitHub Actions workflow runs automatically on PRs
- ✅ Test execution time < 60 seconds
- ✅ Zero memory leaks (verified with sanitizers)

## Notes

- Qt Test is selected for native Qt integration
- Tests will use temporary directories for file I/O
- QApplication mock required for GUI tests
- Comprehensive coverage includes ~25-30 test files
- Fuzzy tests integrated throughout for robustness
- CI runs on Linux (Ubuntu 22.04) with Qt 6.9.2
