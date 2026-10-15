# Serial Studio examples

Example projects for [Serial Studio](https://serial-studio.com). Each folder has source code, a project file (`*.ssproj`), a README with setup instructions, and screenshots where applicable.

You can also browse and download examples directly from Serial Studio via the **Examples** button in the toolbar.

## Examples

| Example | Description | Difficulty | Pro |
|---------|-------------|:----------:|:---:|
| [ANSI Color Test](ANSI%20Color%20Test) | ANSI color and VT-100 terminal emulation test suite | Beginner | |
| [API Test](API%20Test) | Python API client with interactive REPL and test suite | Intermediate | Yes |
| [BLE Battery](BLE%20Battery) | BLE Battery Service level monitor | Beginner | |
| [Camera Telemetry](Camera%20Telemetry) | Live camera video plus image analytics over UDP | Intermediate | Yes |
| [CAN Bus Example](CAN%20Bus%20Example) | ECU simulator with DBC file import (needs the widget-extension example installed) | Intermediate | Yes |
| [csv2wav](csv2wav) | Convert Audio I/O recordings to WAV files | Intermediate | Yes |
| [Dual Drone Telemetry](Dual%20Drone%20Telemetry) | Multi-source two-drone simulator with synthetic camera feeds | Advanced | Yes |
| [EM Wave Simulator](EM%20Wave%20Simulator) | Propagating electromagnetic plane wave visualizer | Advanced | Yes |
| [EtherNet-IP Example](EtherNet-IP%20Example) | Allen-Bradley Logix tag polling over EtherNet/IP (no simulator; real hardware or emulator) | Advanced | Yes |
| [Graphical Ping](Graphical%20Ping) | Live network latency plot driven by the system `ping` command | Beginner | |
| [HexadecimalADC](HexadecimalADC) | Binary ADC data with CRC-16 and FFT analysis | Intermediate | |
| [Hydrogen](Hydrogen) | Hydrogen 1s orbital Monte Carlo 3D visualization | Advanced | Yes |
| [IEC 104 Example](IEC%20104%20Example) | IEC 60870-5-104 substation simulator with interrogation-driven point discovery | Intermediate | Yes |
| [IMU Simulator](IMU%20Simulator) | Batched multi-frame IMU data parsing demo | Intermediate | |
| [ISS Tracker](ISS%20Tracker) | Real-time International Space Station position tracker | Beginner | |
| [LorenzAttractor](LorenzAttractor) | Lorenz attractor chaotic system simulation | Advanced | Yes |
| [LTE modem](LTE%20modem) | LTE modem signal quality via serial, MQTT, or UDP | Intermediate | |
| [MCP Client](MCP%20Client) | Model Context Protocol client for AI integration | Advanced | |
| [Modbus PLC Simulator](Modbus%20PLC%20Simulator) | Hydraulic test stand Modbus TCP simulator | Intermediate | Yes |
| [MPU6050](MPU6050) | MPU6050 accelerometer and gyroscope visualization | Beginner | |
| [MQTT Subscriber Example](MQTT%20Subscriber%20Example) | Live Helsinki transit vehicle tracking over public MQTT | Intermediate | Yes |
| [NI DAQmx](NI%20DAQmx) | NI DAQ device data acquisition bridge | Intermediate | |
| [OBD-II](OBD-II) | OBD-II vehicle telemetry via ELM327 adapter | Intermediate | |
| [OPC UA PLC Simulator](OPC%20UA%20PLC%20Simulator) | Bottling-line OPC UA server (asyncua) with typed tags, arrays, and bad-status handling | Intermediate | Yes |
| [Project Tutorial](Project%20Tutorial) | Companion of the Project File tutorial: delimiters, the Built-In parser, a data grid and an accelerometer | Beginner | |
| [Protobuf Example](Protobuf%20Example) | Protocol Buffers schema import with binary auto-detect parsing | Intermediate | |
| [PulseSensor](PulseSensor) | Heart rate PPG sensor with signal filtering | Beginner | |
| [RC Plane Simulator](RC%20Plane%20Simulator) | RC plane flight telemetry simulator | Intermediate | |
| [S7 PLC Example](S7%20PLC%20Example) | Siemens S7comm PLC simulator over ISO-on-TCP with absolute addressing | Intermediate | Yes |
| [Sparkplug Example](Sparkplug%20Example) | Sparkplug edge node with birth certificates, aliases, and rebirth handling | Intermediate | Yes |
| [System Monitor](System%20Monitor) | Live CPU, memory, disk, and network dashboard | Intermediate | Yes |
| [Testo T549i](Testo%20T549i) | BLE pressure probe with Control Loop handshake and Lua unit transforms | Intermediate | |
| [TinyGPS](TinyGPS) | GPS location tracker with map widget | Beginner | |
| [UDP Function Generator](UDP%20Function%20Generator) | Multi-waveform signal generator over UDP | Beginner | |
| [Vibration Test Rig](Vibration%20Test%20Rig) | Motor RPM sweep driving a Campbell-mode waterfall and two scripted Canvas widgets | Intermediate | Yes |
| [widget-extension](widget-extension) | Level Bar widget extension template (info.json + QML) for a custom dataset widget | Intermediate | |

## Suggested learning order

### Beginner

1. [Project Tutorial](Project%20Tutorial): delimiters, the Built-In parser, a Data Grid and an Accelerometer group.
2. [UDP Function Generator](UDP%20Function%20Generator): Quick Plot over a UDP source.
3. [PulseSensor](PulseSensor): Quick Plot from an Arduino and CSV export.
4. [MPU6050](MPU6050): Accelerometer and Gyroscope groups.
5. [TinyGPS](TinyGPS): a GPS Map group.
6. [Graphical Ping](Graphical%20Ping): a Control Loop that launches a helper script.
7. [BLE Battery](BLE%20Battery): a Bluetooth Low Energy source and a gauge.
8. [ISS Tracker](ISS%20Tracker): a Control Loop that polls a web API, with a map and a web view.
9. [ANSI Color Test](ANSI%20Color%20Test): Console rendering of ANSI and VT-100 sequences.

### Intermediate

1. [HexadecimalADC](HexadecimalADC): binary frames, CRC-16 and timed polling actions.
2. [IMU Simulator](IMU%20Simulator): multi-frame parsing of batched packets.
3. [RC Plane Simulator](RC%20Plane%20Simulator): several widget types from one UDP stream.
4. [Protobuf Example](Protobuf%20Example): schema import with a generated Lua parser.
5. [OBD-II](OBD-II): request and response polling with computed datasets.
6. [Testo T549i](Testo%20T549i): a BLE Control Loop handshake and Lua transforms.
7. [LTE modem](LTE%20modem): the same data over a virtual serial port, MQTT or UDP.
8. [NI DAQmx](NI%20DAQmx): a Python bridge for DAQ hardware.
9. [widget-extension](widget-extension): a custom widget package.
10. [API Test](API%20Test): remote control through the API server.
11. [csv2wav](csv2wav): checking the pipeline through the Audio driver.
12. [System Monitor](System%20Monitor): the Process I/O driver.
13. [Camera Telemetry](Camera%20Telemetry): JPEG image streaming.
14. [Vibration Test Rig](Vibration%20Test%20Rig): a Waterfall widget and Canvas widgets.
15. [MQTT Subscriber Example](MQTT%20Subscriber%20Example): an MQTT source on a public feed.
16. [Modbus PLC Simulator](Modbus%20PLC%20Simulator): a Modbus TCP source.
17. [CAN Bus Example](CAN%20Bus%20Example): CAN Bus with DBC import.
18. [OPC UA PLC Simulator](OPC%20UA%20PLC%20Simulator): an OPC UA client source.
19. [S7 PLC Example](S7%20PLC%20Example): an S7comm source.
20. [IEC 104 Example](IEC%20104%20Example): an IEC 60870-5-104 source.
21. [Sparkplug Example](Sparkplug%20Example): Sparkplug B over MQTT.

### Advanced

1. [LorenzAttractor](LorenzAttractor): a Control Loop that generates all data, with Plot 3D.
2. [Hydrogen](Hydrogen): a Plot 3D point cloud over UDP.
3. [EM Wave Simulator](EM%20Wave%20Simulator): Plot 3D and MultiPlot for a wave packet.
4. [Dual Drone Telemetry](Dual%20Drone%20Telemetry): two sources at once, with images, maps and output controls.
5. [EtherNet-IP Example](EtherNet-IP%20Example): Logix tag polling against real hardware or an emulator.
6. [MCP Client](MCP%20Client): AI assistant control through the Model Context Protocol.

## Getting started

1. Open the example's folder and read the README for hardware and software requirements.
2. Upload the firmware (if applicable) or run the companion script.
3. Open Serial Studio, load the `.ssproj` project file (if provided), and connect.

Several examples (e.g. RC Plane Simulator, Modbus PLC Simulator, CAN Bus Example) auto-launch
their companion Python script from the project's Control Loop as soon as the connection opens,
so step 2 is a no-op for those — make sure Python 3 and any extra libraries are installed
first. If a required package (e.g. `pymodbus` for Modbus PLC Simulator, `opencv-python` for
Camera Telemetry) is missing, the auto-launched script can fail silently in the background with
no dashboard data and no obvious error in Serial Studio itself; check the example's README for
its exact dependencies if nothing shows up after connecting.

## Requirements

- **Serial Studio.** Download from [serial-studio.com](https://serial-studio.com).
- **Arduino IDE** for examples that use Arduino sketches.
- **Python 3** for examples that use Python scripts.
- Check each example's README for extra libraries or hardware.
