# Serial Studio Examples

This folder contains examples that show how to use Serial Studio with sensors, microcontrollers, and other programs. Each example includes source code, Serial Studio project files (`*.json` or `*.ssproj`), setup instructions, and screenshots.

**Legend:**
- 🟢 **Beginner:** Simple setup, minimal hardware
- 🟡 **Intermediate:** Requires specific hardware or moderate configuration
- 🔴 **Advanced:** Complex setup, multiple components, or mathematical concepts

## Examples Overview

### 1. HexadecimalADC
**Difficulty:** 🟢 Beginner | **Time:** ~10 minutes | **License:** GPL / Pro

Reads analog input data from an ADC and transmits it over serial in hexadecimal format.

**Contents:**
  - **HexadecimalADC.ino**: Arduino code for reading and transmitting ADC data
  - **HexadecimalADC.json**: Serial Studio project file for hexadecimal visualization
  - **README.md**: Setup and usage instructions
  - **Screenshot**: Example view in Serial Studio
  
### 2. LTE modem
**Difficulty:** 🟡 Intermediate | **Time:** ~20 minutes | **License:** GPL (Serial/UDP), Pro (MQTT)

Reads signal quality data from an LTE modem and transmits it over Virtual Serial Port, MQTT, or UDP Socket.

**Contents:**
  - **lte.json**: Serial Studio project file for LTE signal quality visualization
  - **lte_mqtt.py**: Python script for parsing data and sending over MQTT
  - **lte_serial.py**: Python script for parsing data and sending over Virtual Serial Port
  - **lte_udp.py**: Python script for parsing data and sending over UDP Socket
  - **README.md**: Setup and usage instructions
  - **Screenshot**: Example view in Serial Studio
 
### 3. Lorenz Attractor
**Difficulty:** 🔴 Advanced | **Time:** ~25 minutes | **License:** Pro (XY Plot required)

Simulates the Lorenz attractor (chaotic differential equations) and transmits x, y, z values over serial. Serial Studio visualizes these in real-time, producing the iconic butterfly-shaped pattern.

**Contents:**
  - **LorenzAttractor.ino**: Arduino code for simulating and transmitting Lorenz data
  - **LorenzAttractor.json**: Serial Studio project file with XY plot configuration
  - **README.md**: Comprehensive setup guide with mathematical background
  - **Screenshots**: Project setup and attractor visualization examples

### 4. MPU6050
**Difficulty:** 🟡 Intermediate | **Time:** ~20 minutes | **License:** GPL / Pro

Captures motion and orientation data from an MPU6050 accelerometer and gyroscope. Visualized using widgets like g-meter, gyroscope display, and attitude indicators.

**Contents:**
  - **MPU6050.ino**: Arduino code for capturing and transmitting MPU6050 data
  - **MPU6050.json**: Serial Studio project file for accelerometer, gyroscope, and temperature visualization
  - **README.md**: Detailed setup instructions with wiring diagrams
  - **Screenshots**: Project setup and data visualization examples

### 5. PulseSensor
**Difficulty:** 🟢 Beginner | **Time:** ~10 minutes | **License:** GPL / Pro

Filters and smooths pulse data from a heart rate sensor using Quick Plot mode. Demonstrates signal filtering and CSV data logging.

**Contents:**
  - **PulseSensor.ino**: Arduino code for filtering and transmitting pulse data
  - **README.md**: Step-by-step setup and visualization guide
  - **Screenshots**: CSV logging and real-time visualization examples

### 6. TinyGPS
**Difficulty:** 🟡 Intermediate | **Time:** ~20 minutes | **License:** GPL / Pro

Reads GPS data (latitude, longitude, altitude) from a GPS module and visualizes it on an interactive map in Serial Studio.

**Contents:**
  - **TinyGPS.ino**: Arduino code for capturing and transmitting GPS data
  - **TinyGPS.json**: Serial Studio project file with map widget configuration
  - **README.md**: Comprehensive setup instructions for GPS module configuration
  - **Screenshots**: Map visualization and project setup examples

### 7. UDP Function Generator
**Difficulty:** 🟡 Intermediate | **Time:** ~15 minutes | **License:** GPL / Pro

Generates real-time waveforms (sine, triangle, sawtooth, square) and transmits them over UDP socket. Ideal for signal analysis and stress-testing Serial Studio's performance with high-frequency data streams.

**Contents:**
  - **udp_function_generator.c**: C program for waveform generation and UDP transmission
  - **README.md**: Setup and usage instructions with configuration options
  - **Screenshot**: Example waveform visualization

**Key Features:**
  - Multiple waveform types with configurable frequency, phase, and interval
  - Network-based signal processing via UDP
  - Debug output and aliasing warnings
  - Performance testing capabilities

### 8. ISS Tracker
**Difficulty:** 🟢 Beginner | **Time:** ~10 minutes | **License:** GPL / Pro

Fetches real-time International Space Station position and velocity data from a public API and transmits it over UDP. No hardware required!

**Contents:**
  - **iss-tracker.py**: Python script that pulls telemetry from API and sends over UDP
  - **iss-tracker.ssproj**: Serial Studio project file with map, altitude, and speed widgets
  - **README.md**: Setup guide for running the tracker
  - **Screenshot**: ISS telemetry visualization example

**Features:**
  - Real-time position tracking on interactive map
  - Altitude monitoring with alarms
  - Orbital velocity gauge with color-coded ranges
  - Software-only example (no microcontroller needed)

### 9. Modbus PLC Simulator
**Difficulty:** 🟡 Intermediate | **Time:** ~15 minutes | **License:** Pro (Modbus TCP/RTU required)

Simulates a hydraulic test system using Modbus TCP. Shows how Serial Studio works with industrial equipment. Includes realistic data, automatic tests, and failure modes.

**Note:** Serial Studio supports both Modbus TCP and Modbus RTU. This example uses Modbus TCP.

**Contents:**
  - **plc_simulator.py**: Hydraulic power simulator with Modbus TCP server
  - **Modbus PLC Simulator.ssproj**: Serial Studio project file with dashboard
  - **README.md**: Setup guide
  - **Screenshot**: Dashboard example

**Features:**
  - 50HP motor with soft-start and pressure control
  - 9 Modbus registers (temperature, pressure, RPM, flow, vibration)
  - Realistic physics simulation
  - Automatic test sequences (startup, running, pressure test, failure, shutdown)
  - Custom JavaScript parser for Modbus data
  - No hardware needed - software only

### 10. CAN Bus Example
**Difficulty:** 🟡 Intermediate | **Time:** ~15 minutes | **License:** Pro (CAN Bus required)

Simulates a car's ECU (Engine Control Unit) and sends realistic vehicle data over CAN bus. Shows how to import DBC files to create dashboards automatically.

**Contents:**
  - **ecu_simulator.py**: Car simulator with multiple ECU systems
  - **example_vehicle.dbc**: CAN database file
  - **CAN Bus Example.ssproj**: Serial Studio project file
  - **README.md**: Setup guide
  - **Screenshot**: Dashboard example

**Features:**
  - Works without hardware using VirtualCAN
  - Supports real CAN hardware (PEAK, SocketCAN, Vector, etc.)
  - Automatic DBC file import
  - Smart widget selection (gauges, plots, bars, LEDs)
  - Realistic car data (RPM, speed, temperature, battery, etc.)
  - No hardware needed - software only

### 11. ANSI Color Test
**Difficulty:** 🟢 Beginner | **Time:** ~5 minutes | **License:** GPL / Pro

Comprehensive test suite demonstrating Serial Studio's full ANSI color and VT-100 terminal emulation support. Sends colorful test sequences over UDP to showcase 4-bit, 8-bit (256 colors), and 24-bit RGB (true color) capabilities.

**Contents:**
  - **test_ansi_colors.py**: Python script that sends ANSI color test sequences via UDP
  - **README.md**: Comprehensive guide with color code reference and examples

**Features:**
  - 4-bit standard and bright colors (foreground and background)
  - 8-bit 256-color palette (standard colors, RGB cube, grayscale ramp)
  - 24-bit RGB true color support (16.7 million colors)
  - VT-100 control sequences (cursor movement, bold text)
  - Creative effects (rainbow text, color gradients)
  - Colored timestamps synchronized with ANSI colors setting
  - No hardware needed - software only

### 12. API Client
**Difficulty:** 🟡 Intermediate | **Time:** ~10 minutes | **License:** GPL / Pro

Python client for the Serial Studio API that allows external programs to control and configure Serial Studio through a TCP connection. Features an interactive REPL for easy exploration.

**Contents:**
  - **test_api.py**: Full-featured API client with interactive REPL, CLI, and test suite
  - **README.md**: Complete API reference and usage guide
  - **API_REFERENCE.md**: Detailed command documentation

**Features:**
  - **Live Monitor**: Real-time dashboard showing connection status and configuration
  - **Interactive REPL**: Explore 116 API commands with an easy-to-use interface
  - **CLI Client**: Send single commands or batches from terminal
  - **Test Suite**: Automated testing for all API functionality
  - **All Drivers Supported**: UART, Network, BLE, Modbus (Pro), CAN Bus (Pro), MQTT (Pro)
  - **Export Control**: CSV and MDF4 (Pro) export management
  - **I/O Manager**: Connection control, data transmission, configuration
  - **Batch Commands**: Execute multiple commands from JSON files
  - **JSON Output**: Scriptable output for automation
  - **No Dependencies**: Uses only Python standard library
  - No hardware needed - software only

### 13. NI DAQmx Example
**Difficulty:** 🟡 Intermediate | **Time:** ~5 minutes | **License:** MIT

High-performance Python bridge that streams real-time data from NI DAQ devices to Serial Studio via UDP. Supports voltage and current measurements with minimal latency.

**Contents:**
  - **daqbridge.py**: Main acquisition script with configurable channels and UDP streaming
  - **README.md**: Complete setup guide and configuration reference

**Features:**
  - **Universal NI DAQ Support**: Works with USB-600x, USB-621x, PCIe-6xxx, CompactDAQ, and other DAQmx-compatible devices
  - **Voltage & Current**: Direct voltage or current measurement via shunt resistor
  - **Real-Time Streaming**: One UDP packet per sample for lowest latency
  - **Configurable Channels**: Individual voltage ranges, terminal configs per channel
  - **Custom Processing**: User-defined processing function for calibration, filtering, calculations
  - **CSV Output**: Clean comma-separated format with configurable precision
  - **Live Stats**: Real-time sample rate and packet count monitoring
  - **Clean Shutdown**: Proper signal handling for Ctrl+C
  - Requires NI-DAQmx driver and hardware

### 14. RC Plane Simulator
**Difficulty:** 🟢 Beginner | **Time:** ~5 minutes | **License:** GPL / Pro

Simulates a small RC plane flying a complete flight profile with realistic telemetry data. Perfect for testing and demonstrating multiple Serial Studio widgets simultaneously without hardware.

**Contents:**
  - **rc_plane_simulator.py**: Python script that simulates RC plane flight telemetry
  - **RC Plane Simulator.ssproj**: Serial Studio project file with dashboard widgets
  - **README.md**: Setup guide and flight profile documentation

**Features:**
  - **Complete Flight Profile**: 170-second simulation with 20+ maneuvers (takeoff, turns, loops, rolls, inverted flight, landing)
  - **Multi-Widget Testing**: Exercises gyroscope, accelerometer, GPS, compass, gauges, bars, and time plots
  - **Realistic Physics**: Body-frame accelerations, rotation rates, GPS coordinates, battery drain, RSSI simulation
  - **Configurable Options**: Single flight or continuous loop, adjustable update rate (10-50 Hz), custom UDP port
  - **CSV Format**: Simple `$...,;` delimited data over UDP
  - No hardware needed - software only

## Getting Started

To use these examples:

1. **Connect Hardware**: Follow the instructions in each example's README file to connect components.
2. **Upload Arduino Code**: Open the `.ino` file in Arduino IDE, upload it to your board, and check the baud rate and port settings.
3. **Configure Serial Studio**:
   - Open Serial Studio and load the JSON project file (if provided).
   - Follow the setup instructions in each README file.
4. **See Your Data**: Once connected, you will see live data in Serial Studio using different widgets.

## Requirements

- **Arduino IDE**: To upload code to your board.
- **Serial Studio**: Download from [serial-studio.com](https://serial-studio.com).
- **Libraries**: Some examples need extra libraries (like Adafruit MPU6050 or TinyGPS). Check each README file for details.

## More Information

For more help, visit the [Serial Studio wiki](https://github.com/Serial-Studio/Serial-Studio/wiki). Each example includes troubleshooting tips and setup instructions.
