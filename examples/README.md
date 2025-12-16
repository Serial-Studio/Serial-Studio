# Serial Studio Examples

This directory contains various examples demonstrating how to use Serial Studio to visualize data from sensors, microcontrollers, and external programs. Each example includes complete source code, Serial Studio project files (`*.json` or `*.ssproj`), setup instructions, and screenshots.

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
**Difficulty:** 🟡 Intermediate | **Time:** ~15 minutes | **License:** Pro (Modbus required)

Physics-based hydraulic test stand simulator acting as a Modbus TCP server. Demonstrates Serial Studio's industrial protocol support with realistic telemetry, automatic failure modes, and recovery sequences.

**Contents:**
  - **plc_simulator.py**: Hydraulic power unit simulator with Modbus TCP server
  - **Modbus PLC Simulator.ssproj**: Serial Studio project file with industrial dashboard
  - **README.md**: Comprehensive setup and configuration guide
  - **Screenshot**: Industrial dashboard visualization example

**Features:**
  - 50HP motor with VFD soft-start and PID pressure control
  - 9 Modbus holding registers (temperature, pressure, RPM, flow, vibration)
  - Realistic physics (thermodynamics, pump dynamics, ISO 10816 vibration)
  - Automatic test sequences (startup, running, pressure test, failure, shutdown)
  - Custom JavaScript frame parser for Modbus protocol
  - Software-only example (no hardware needed)

## Getting Started

To use these examples:

1. **Hardware Setup**: Connect the necessary components as described in each example's README file.
2. **Arduino Code**: Open the Arduino `.ino` file in the Arduino IDE, upload it to your board, and ensure the correct baud rate and port settings are configured.
3. **Serial Studio Configuration**: 
   - Launch Serial Studio and import the provided JSON project file, if available.
   - Follow the configuration instructions in each example's README to set up data parsing and visualization widgets.
4. **Visualize Data**: Once connected, view live data in Serial Studio through various widgets and mapping features.

## Requirements

- **Arduino IDE**: To compile and upload `.ino` files.
- **Serial Studio**: For real-time data visualization. Download it from [Serial Studio's website](https://serial-studio.github.io/).
- **Libraries**: Some examples require additional libraries (e.g., Adafruit MPU6050 or TinyGPS). Refer to individual README files for specific library requirements.

## Additional Resources

For more details on Serial Studio, visit the [Serial Studio wiki](https://github.com/Serial-Studio/Serial-Studio/wiki). Each example README also includes troubleshooting tips and step-by-step instructions for setup and visualization.
