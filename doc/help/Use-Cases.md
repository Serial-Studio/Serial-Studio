# Serial Studio use cases

Your Arduino sketch prints one line per second:

```cpp
Serial.begin(9600);
// ...
Serial.print(temp);
Serial.print(",");
Serial.print(hum);
Serial.print(",");
Serial.println(pressure);
delay(1000);
```

The serial port carries `21.5,48.2,1012.3`, then `21.6,48.1,1012.3`, and so on. In Serial Studio, set **Operation Mode** to "Quick Plot (Auto-Detected Format)" in the **Device Setup** panel, pick the serial port and the baud rate from `Serial.begin()` (9600 here), and click **Connect**. Each value gets its own plot, a Multi-Plot overlays them, and a Data Grid shows the latest numbers. No project file, no script. The same dropdown has Console Only and **Parse via Project File**, which is where the Project Editor and your own dashboard layout live. The exact steps are in [Operation Modes](Operation-Modes.md).

The setups below start from that same line and show what changes when the link, the data or the dashboard gets bigger. Each one names its edition. They are sketches of working configurations, not validated or certified designs, and Serial Studio is not a medical device.

## Editions

UART, TCP/UDP, Bluetooth LE, the three operation modes, the Project Editor, frame parsers and CSV export are free. MQTT, Modbus, CAN Bus, OPC UA, Audio, Raw USB, HID, Process I/O, multi-device projects, output controls, MDF4, dataset transforms, computed datasets, Variables and the 3D Plot, Waterfall, Image View and Canvas widgets need Pro; WebSocket, HTTP and the API Server are free. The complete list is in [Pro vs Free](Pro-vs-Free.md). The official binary offers a 14-day trial the first time you use a Pro feature, through a Yes/No prompt; registering the trial needs an internet connection. After it expires, the same prompt offers activation or purchase.

## Over serial (UART)

### Joint angles from a robot arm

A 6-DOF arm on an Arduino Mega prints `10.5,42.0,-3.2,88.1,0.0,12.7` (six joint angles) at 50 Hz. In Quick Plot, that is six plots, a Multi-Plot and a Data Grid with nothing to configure. To add an XY plot of the end-effector or bars with threshold colors, use **Save as Project** on the dashboard and add the widgets in the [Project Editor](Project-Editor.md); bar colors come from alarm bands on the dataset.

Edition: Free (UART, Quick Plot, Parse via Project File).

### Classroom weather station

Fifteen groups build an Arduino Uno with a DHT22 and a BMP180 and print temperature, humidity and pressure as comma-separated values. Everyone starts in Quick Plot. A group that wants gauges and labels switches to "Parse via Project File" and builds a project in the Project Editor; one project file serves every lab station. CSV export gives the data to Python or a spreadsheet for the write-up.

Edition: Free.

### Greenhouse with a binary sensor frame

An Arduino Mega packs eight soil-moisture readings, temperature, humidity and a light level into a binary frame. A [frame parser](Frame-Parser.md) in JavaScript converts the bytes to values, and the dashboard shows the moisture as a Bar widget and the climate as plots. Switching the irrigation relay from the dashboard needs [output controls](Output-Controls.md) (Pro) with a transmit function; without them, the firmware decides when the relay switches. The relay does not care who asked.

Edition: Free for the dashboard (UART, frame parser); Pro for output controls.

### Radio telemetry from a rocket, balloon or UAV

A 433 MHz or 900 MHz radio module on the ground station shows up as a serial port. The airborne side sends altitude, acceleration, battery voltage and GPS coordinates as lines of text. Serial Studio plots them, draws the GPS Map track, and logs everything to CSV for the recovery team. Values such as a predicted landing point or remaining flight time come from the firmware, or from a [computed dataset](Derived-Values.md#computed-dataset) (Pro) that combines other datasets.

Edition: Free; Pro for computed datasets.

## Over Bluetooth LE

### Mobile robot with an ESP32

An ESP32 notifies one BLE characteristic with a line such as `lat,lon,battery,ax,ay,az` at 20 Hz. Serial Studio connects as the BLE central, subscribes to that characteristic, and shows the position on a GPS Map, the battery as a Gauge and the IMU on the Accelerometer and Gyroscope widgets. See [Bluetooth LE](Drivers-Bluetooth-LE.md) for the firmware side. When the robot leaves BLE range, an ESP32 publishing the same values to an MQTT broker lets a Pro dashboard keep watching.

Edition: Free over BLE; Pro for the MQTT fallback.

### Gait insoles

An Arduino Nano 33 BLE in each shoe sends four force-sensor readings per insole. The dashboard overlays left and right on Multi-Plots and exports CSV for later analysis. Stride metrics are calculated by the firmware or after export.

Edition: Free (Bluetooth LE).

## Over TCP and UDP

### Race car and competition robot

A Raspberry Pi with a CAN hat reads the car's bus and forwards a text line per sample, over a Wi-Fi hotspot, to the pit-lane laptop. A competition robot's RoboRIO or Jetson sends JSON the same way. In the **Device Setup** panel, choose the Network driver; Serial Studio listens on a TCP or UDP socket; a frame parser maps each JSON field to a dataset, and the dashboard shows tire temperatures, brake pressure, PID terms or battery current beside each other.

Edition: Free (TCP or UDP, frame parser). The Pi does the CAN decoding; to decode CAN in Serial Studio itself, see [ECU signals from a DBC file](#ecu-signals-from-a-dbc-file).

### Software-defined radio spectrum

GNU Radio computes magnitudes from an RTL-SDR dongle and sends them as UDP packets. The FFT Plot shows the spectrum; the Waterfall widget (Pro) adds the time axis for repeaters and satellite passes.

Edition: Free (UDP) for the FFT Plot; Pro for Waterfall.

## MQTT

### Twenty sensors in a forest

Each ESP32 with a BME280 publishes a reading to an MQTT broker over Wi-Fi or 4G. In the **Device Setup** panel, choose the MQTT driver; Serial Studio subscribes with a topic filter such as `sensors/+/data` through the [MQTT subscriber](Drivers-MQTT.md). The bytes do not say which node sent them, so each node includes its own id in the payload. A frame parser reads the id and fills that node's dataset group; with 20 nodes the project holds 20 groups. GPS Map widgets mark the node positions, and CSV export feeds the seasonal analysis.

The same pattern covers a home energy monitor: an ESP32 with current sensors publishes to a Mosquitto broker on a Raspberry Pi, and a power figure (volts times amps) is a computed dataset. An accumulated kWh needs a Variables table that remembers the running total between frames, or the firmware can send it.

Edition: Pro (MQTT, computed datasets, Variables).

## Industrial buses

### Modbus PLC monitoring

A line has several PLCs, and engineers need register values on one screen. Serial Studio polls as a Modbus master over Modbus TCP or Modbus RTU. In **Configure Register Groups…** you add one group per block of Holding Registers, Input Registers, Coils or Discrete Inputs, and each group can name its own slave address. Each poll produces a binary frame; set the source decoder to **Binary (Direct)** and use a frame parser to turn register bytes into datasets. See [Modbus](Drivers-Modbus.md). A water-quality probe with an RS485 output works the same way when it speaks Modbus RTU through a USB-RS485 adapter; if it prints text, use the UART driver instead.

Edition: Pro (Modbus).

### OPC UA tag browser

PLCs and gateways that already publish an OPC UA address space need no tag-list export. You enter `opc.tcp://host:port`, browse the address space and tick the tags to monitor, whole folders included, up to 2048 channels. Serial Studio generates the project from the selection and subscribes to the tags. Login is anonymous or username/password. See [OPC UA](Drivers-OPC-UA.md) for security policies and timestamps.

Edition: Pro (OPC UA).

## CAN and MDF4

### ECU signals from a DBC file

An ECU engineer imports a DBC file and the dashboard shows decoded signals by name, such as `Engine_RPM` and `Vehicle_Speed`, for every message ID at once. The adapter can be PEAK PCAN, SocketCAN on Linux, a Qt-supported Vector interface, or `virtualcan` for testing; CAN FD is available when the adapter supports it. Export the session to MDF4 for CANalyzer or CANape. See [CAN Bus](Drivers-CAN-Bus.md).

Edition: Pro (CAN Bus, DBC import, MDF4 export).

### Bluetooth OBD-II (ELM327) adapter

This is a different path from CAN Bus: pair it in the operating system so it appears as a COM port, connect with the free UART driver, and decode its AT-command replies in a frame parser.

Edition: Free (UART, frame parser).

### Replaying an MDF4 file

The **Open MDF4** toolbar button replays a `.mf4` file through the same pipeline as a live connection, at the original timing, with Play/Pause, frame stepping and a seek slider. A file recorded by Serial Studio replays with no setup. A file from another tool maps each channel group's data channels onto the loaded project's datasets in file order, so load a project whose datasets match the file's channel order. Playback shows one file at a time, and CSV and MDF4 export are off while a player is open. See [MDF4](MDF4.md).

Edition: Pro (MDF4 playback).

## Other Pro drivers and multi-device projects

### Simulation dashboard from a child process

A physics simulation prints `position,velocity,energy` to stdout. With the Process I/O driver in **Launch Process** mode, Serial Studio starts the program, you answer Yes to the launch-consent prompt, and the values plot live. **Named Pipe** mode reads from a FIFO instead; use Launch mode when the dashboard must write back to the process, for a parameter sweep. See [Process I/O](Drivers-Process-IO.md).

Edition: Pro (Process I/O).

### Gamepad plus robot feedback

The HID driver reads a USB gamepad's axes and buttons through hidapi. In a multi-device project, a second source receives the robot's IMU and motor values over BLE. Joystick axes become plots, buttons light up in an LED Panel, and one CSV export holds both sources for post-match review; its rows are sparse, with empty cells where a source had no sample.

Edition: Pro (HID, multi-device projects).

### Two drones on one dashboard

Drone A sends over a 900 MHz radio modem on UART, drone B over UDP on Wi-Fi. A multi-device project gives each its own source, parser and groups, so GPS, attitude and battery sit side by side. MDF4 export writes one file; each project group becomes a channel group named `SourceName / GroupName`. [Output controls](Output-Controls.md) in an Output Panel group per drone send commands: a slider whose transmit function builds `"THR " + value + "\r\n"`, a knob for yaw and a toggle for the camera. The throttle range, yaw range and command strings are example settings. A return-home command belongs on an Emergency Stop button style, which latches.

Edition: Pro (multi-device projects, output controls, MDF4 export).

### USB instruments beside serial sensors

Bench instruments that expose bulk USB endpoints but no serial port connect through the Raw USB driver (libusb). In a multi-device project they sit next to a serial-sensor source, and MDF4 export keeps both in one file.

Edition: Pro (Raw USB, multi-device projects, MDF4 export).

## More setups at a glance

| Setup | Link | What the dashboard shows | Edition |
|-------|------|--------------------------|---------|
| Pendulum lab with Arduino Nano and MPU6050 | UART | X/Y/Z acceleration plots, FFT Plot for the resonance peak | Free |
| Smart beehive: HX711 scale and DS18B20 on a LoRa node | LoRa gateway to serial | Weight and temperature trends | Free |
| Air-quality nodes: PMS5003 and GPS on an ESP32 | MQTT | Readings on a GPS Map | Pro (MQTT) |
| CNC spindle load and axis positions | Modbus RTU, or serial with a frame parser | Spindle RPM, load, X/Y/Z positions | Pro for Modbus; Free over serial |
| Single-sensor UART setups: conveyor encoder, MAX6675 reflow oven | UART | Belt speed and pulse count, or temperature against time with CSV for the process record | Free |
| EV battery pack: BMS cell voltages | UART | 96 cell voltages in a Bar widget, pack current and temperatures | Free |
| Antenna analyzer: NanoVNA | USB serial | SWR sweep plot; a Smith chart is a Canvas script you write, no template ships | Free; Pro for Canvas |
| APRS tracker: TNC with decoded packets | UART | Station positions on a GPS Map, text in the console | Free |
| Biosignals: AD8232 ECG, MAX30102 oximeter | UART or BLE | Waveform and FFT Plot; SpO2 and BPM computed by the firmware | Free |
| Solar array: INA219 on an ESP32 | UART or MQTT | Voltage, current, watts | Free; Pro for MQTT |
| Aquarium: pH, temperature and conductivity | UART | Gauges with alarm bands for out-of-range values | Free |
| HVAC ducts: DS18B20 on supply and return | Wi-Fi (TCP) | Supply, return and outdoor temperatures; the differential is a computed dataset | Free; Pro for computed datasets |
| Dyno: ELM327 OBD-II adapter | COM port via UART | RPM, throttle and coolant plots; an XY power curve needs both axes as datasets in the stream | Free |

## Custom visualizations (Canvas)

Canvas lets you draw a widget with a short JavaScript script, or start from a bundled template. In the [Project Editor](Project-Editor.md), click **Canvas** in the toolbar to add a Canvas group, add datasets, click **Edit Code**, then click **Template** in the code editor to pick one. The script defines `paint(ctx, w, h)` and, optionally, `onFrame()` once per dashboard tick; a 250 ms watchdog stops a call that does not return. Every Canvas entry below is Pro. See [Canvas Widget](Canvas-Widget.md) for the full list of templates.

### Phosphor-style oscilloscope

The built-in line plot has no persistence trace, no afterglow and no XY mode. The `Oscilloscope` template draws channels on a phosphor-green CRT background with a 10 x 8 graticule, a glow under each trace and a crisp line on top. The `XY scope (Lissajous)` template plots two synchronous channels against each other.

### Artificial horizon

A flat attitude indicator is simpler than the 3D Gyroscope widget. The `Artificial horizon` template binds two datasets, pitch and roll in degrees. Sky and ground rotate with roll and slide with pitch, a pitch ladder marks every 10°, and a fixed aircraft symbol stays in front. The source can be any stream that provides pitch and roll in degrees.

### Lab-equipment mimic

When the dashboard replaces an analog meter, operators should not relearn the panel. Start from the `Dial gauge` template, set the green, amber and red zone thresholds, and replace the face colors and tick marks. A smoothing filter in `onFrame()` makes the needle sweep instead of snapping to each new value.

### Audio VU meter and bargraphs

The `Audio VU meter` and `Bars with peak hold` templates draw segmented LED-style bars with peak markers: saturated segments are active, pastel segments are inactive, and a peak marker holds for a set interval before it decays. Pair them with the [Audio driver](Drivers-Audio.md) (Pro) to read a microphone or line input.

### Polar diagnostics for rotating machinery

The `Polar plot` template draws (angle, magnitude) pairs as rays on a polar grid with range rings and 30° spokes. Put it next to a Waterfall widget (Pro) for order-tracking spectra on the same dashboard.

## Starting your own

Print one line of numbers, run Quick Plot, and check that the values arrive. Then save the stream as a project, and add widgets, a frame parser or a different driver as the data needs them. The toolbar's **Examples** button downloads example projects from GitHub the first time you open it, so it needs an internet connection.

To add a use case to this page, open a GitHub issue or send a pull request. For help with a setup, see the [FAQ](FAQ.md), [Troubleshooting](Troubleshooting.md) or [GitHub Discussions](https://github.com/Serial-Studio/Serial-Studio/discussions).
