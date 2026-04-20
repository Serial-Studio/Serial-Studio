# Serial Studio examples

Example projects for [Serial Studio](https://serial-studio.com). Each folder has source code, a project file (`*.ssproj`), a README with setup instructions, and screenshots where applicable.

You can also browse and download examples directly from Serial Studio via **Help > Examples Browser**.

## Examples

| Example | Description | Difficulty | Pro |
|---------|-------------|:----------:|:---:|
| [ANSI Color Test](ANSI%20Color%20Test) | ANSI color and VT-100 terminal emulation test suite | Beginner | |
| [API Test](API%20Test) | Python API client with interactive REPL and test suite | Intermediate | Yes |
| [BLE Battery](BLE%20Battery) | BLE Battery Service level monitor | Beginner | |
| [Camera Telemetry](Camera%20Telemetry) | Live camera video plus image analytics over UDP | Intermediate | |
| [CAN Bus Example](CAN%20Bus%20Example) | ECU simulator with DBC file import | Intermediate | |
| [csv2wav](csv2wav) | Convert Audio I/O recordings to WAV files | Intermediate | |
| [Dual Drone Telemetry](Dual%20Drone%20Telemetry) | Multi-source two-drone simulator with synthetic camera feeds | Advanced | Yes |
| [EM Wave Simulator](EM%20Wave%20Simulator) | Propagating electromagnetic plane wave visualizer | Advanced | |
| [HexadecimalADC](HexadecimalADC) | Binary ADC data with CRC-16 and FFT analysis | Intermediate | Yes |
| [Hydrogen](Hydrogen) | Hydrogen 1s orbital Monte Carlo 3D visualization | Advanced | |
| [IMU Simulator](IMU%20Simulator) | Batched multi-frame IMU data parsing demo | Intermediate | |
| [ISS Tracker](ISS%20Tracker) | Real-time International Space Station position tracker | Beginner | Yes |
| [LorenzAttractor](LorenzAttractor) | Lorenz attractor chaotic system simulation | Advanced | |
| [LTE modem](LTE%20modem) | LTE modem signal quality via serial, MQTT, or UDP | Intermediate | |
| [MCP Client](MCP%20Client) | Model Context Protocol client for AI integration | Advanced | |
| [Modbus PLC Simulator](Modbus%20PLC%20Simulator) | Hydraulic test stand Modbus TCP simulator | Intermediate | Yes |
| [MPU6050](MPU6050) | MPU6050 accelerometer and gyroscope visualization | Beginner | |
| [NI DAQmx](NI%20DAQmx) | NI DAQ device data acquisition bridge | Intermediate | |
| [PulseSensor](PulseSensor) | Heart rate PPG sensor with signal filtering | Beginner | |
| [RC Plane Simulator](RC%20Plane%20Simulator) | RC plane flight telemetry simulator | Intermediate | |
| [System Monitor](System%20Monitor) | Live CPU, memory, disk, and network dashboard | Intermediate | Yes |
| [TinyGPS](TinyGPS) | GPS location tracker with map widget | Beginner | |
| [UDP Function Generator](UDP%20Function%20Generator) | Multi-waveform signal generator over UDP | Beginner | |

## Getting started

1. Open the example's folder and read the README for hardware and software requirements.
2. Upload the firmware (if applicable) or run the companion script.
3. Open Serial Studio, load the `.ssproj` project file (if provided), and connect.

## Requirements

- **Serial Studio.** Download from [serial-studio.com](https://serial-studio.com).
- **Arduino IDE** for examples that use Arduino sketches.
- **Python 3** for examples that use Python scripts.
- Check each example's README for extra libraries or hardware.
