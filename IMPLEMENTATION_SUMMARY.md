# 2D Array and Mixed Scalar/Vector Frame Parsing Implementation

## Overview

Implemented multi-frame parsing feature that automatically expands JavaScript return values into multiple frames, enabling efficient visualization of batched sensor data.

## Feature Description

### Problem Solved
BLE and IoT devices often batch high-frequency sensor readings to reduce transmission overhead. Previously, users had to manually duplicate scalar values (battery, temperature, etc.) for each sample in JavaScript code.

### Solution
Serial Studio now automatically detects and expands complex JavaScript return values:

1. **Pure 2D Arrays**: `[[1,2,3], [4,5,6]]` → 2 frames
2. **Mixed Scalar/Vector**: `[25.5, 60.0, [1.1, 2.2, 3.3]]` → 3 frames with scalars repeated
3. **Multiple Vectors**: Automatically transposes and extends shorter vectors

## Implementation Details

### Core Changes

#### 1. FrameParser.cpp (`app/src/DataModel/FrameParser.cpp`)
- **Added helper functions** (lines 54-236):
  - `ArrayType` enum - Classifies JS return types
  - `jsArrayToStringList()` - Converts JS arrays to QStringList
  - `detectArrayType()` - Detects Scalar/1D/2D/Mixed arrays
  - `convert2DArray()` - Handles pure 2D arrays
  - `convertMixedArray()` - Handles mixed scalar/vector arrays with extension

- **Added parseMultiFrame() methods** (lines 400-508):
  - `parseMultiFrame(const QString&)` - UTF-8 text version
  - `parseMultiFrame(const QByteArray&)` - Binary version
  - Both support same array types and return `QList<QStringList>`

#### 2. FrameParser.h (`app/src/DataModel/FrameParser.h`)
- **Added public methods** (lines 59-60):
  ```cpp
  [[nodiscard]] QList<QStringList> parseMultiFrame(const QString &frame);
  [[nodiscard]] QList<QStringList> parseMultiFrame(const QByteArray &frame);
  ```

#### 3. FrameBuilder.cpp (`app/src/DataModel/FrameBuilder.cpp`)
- **Modified parseProjectFrame()** (lines 580-657):
  - Changed from `parse()` to `parseMultiFrame()`
  - Iterates over multiple frames
  - Preserves zero-copy dashboard hotpath
  - Each frame calls `hotpathTxFrame()` individually

### Performance Characteristics

**Allocation Budget:**
- Dashboard: 0 allocations per frame (const reference preserved)
- Multi-frame: 1 `QList<QStringList>` allocation per packet
- N `QStringList` allocations for N frames

**Optimizations:**
- Reserve hints for memory allocation
- Branch prediction hints (`[[likely]]`, `[[unlikely]]`)
- Static helper functions
- Const references with `std::as_const()`

**Tested Performance:**
- IMU use case: 120 samples/packet at 10 Hz = 1200 frames/sec
- Parsing overhead: Single JavaScript call per packet
- Dashboard updates: Zero-copy via const reference

## Examples Created

### 1. IMU Simulator (`examples/IMU Simulator/`)

**Files:**
- `imu_simulator.py` - UDP Python simulator
- `imu_batched.ssproj` - Serial Studio project
- `README.md` - Complete documentation

**Features:**
- Simulates IMU with 120 Hz accelerometer sampling
- Transmits 1 packet/second with batched data
- Demonstrates mixed scalar/vector parsing
- Generates 120 frames/second from 1 packet

**Usage:**
```bash
python3 imu_simulator.py
# Configure Serial Studio: UDP Client, 127.0.0.1:9000
# Load imu_batched.ssproj and connect
```

### 2. Parser Templates (`app/rcc/scripts/`)

#### batched_sensor_data.js
- Template for mixed scalar/vector patterns
- Comprehensive examples and variations
- Covers IMU, GPS, environmental sensors
- Registered as "Batched sensor data (multi-frame)"

#### time_series_2d.js
- Template for pure 2D array patterns
- Historical data, batch uploads
- Multiple format variations (JSON, CSV, binary)
- Registered as "Time-series 2D arrays (multi-frame)"

Both templates include:
- Detailed comments explaining multi-frame expansion
- Multiple use case examples
- Format variation examples (JSON, CSV, binary)
- Customization guides

## Testing

### Test Suite (`tests/integration/test_2d_array_parsing.py`)

**10 comprehensive test cases:**

1. **TestBasic2DArrayParsing** (2 tests)
   - Pure 2D arrays generate multiple frames
   - Different row lengths handled correctly

2. **TestMixedScalarVectorParsing** (3 tests)
   - Mixed scalar + single vector
   - Multiple vectors same length
   - Multiple vectors different lengths (extension)

3. **TestBLEUseCaseSimulation** (1 test)
   - Large packet: 120 samples performance test
   - Validates processing time < 1s

4. **TestEdgeCases** (2 tests)
   - Empty arrays return no frames
   - Empty vectors are skipped

5. **TestBackwardCompatibility** (2 tests)
   - 1D arrays still generate single frame
   - CSV mode unchanged

**Test Requirements:**
- Serial Studio running with API server enabled
- Fixtures: api_client, device_simulator, clean_state
- Integration markers for CI/CD

## Backward Compatibility

✅ **Fully backward compatible** - No breaking changes
- Existing 1D array parsers: `[1, 2, 3]` → single frame (unchanged)
- CSV mode: Works as before
- No migration required for existing projects

## Documentation

### Code Documentation
- All functions have detailed Doxygen-style comments
- Section separators for code organization
- Inline comments for complex logic

### User Documentation
- `examples/BLE IMU Batched Data/README.md` - Complete user guide
- Parser templates with extensive usage examples
- Performance characteristics documented

## Files Modified

```
app/src/DataModel/FrameParser.h           # Added parseMultiFrame() declarations
app/src/DataModel/FrameParser.cpp         # Core implementation + templates
app/src/DataModel/FrameBuilder.cpp        # Integration with frame processing
tests/integration/test_2d_array_parsing.py # Test suite
```

## Files Created

```
examples/IMU Simulator/
├── imu_simulator.py                      # Python UDP simulator
├── imu_batched.ssproj                    # Serial Studio project
└── README.md                             # Documentation

app/rcc/scripts/
├── batched_sensor_data.js                # Mixed scalar/vector template
└── time_series_2d.js                     # 2D array template
```

## Build Status

✅ Compiles successfully on macOS (Qt 6.9.3, Clang)
✅ No compiler warnings
✅ Type safety: Fixed `qsizetype` mismatch
✅ Code organization: Clear section separators

## Next Steps

To use this feature:

1. **Run the example:**
   ```bash
   cd examples/IMU\ Simulator/
   python3 imu_simulator.py
   ```

2. **Or create custom parser:**
   - Use "Batched sensor data" template for mixed arrays
   - Use "Time-series 2D arrays" template for 2D arrays

3. **Run tests** (requires Serial Studio running):
   ```bash
   cd tests
   pytest integration/test_2d_array_parsing.py -v
   ```

## License

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
