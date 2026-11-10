# Serial Studio vs alternatives

You have an Arduino printing values with `Serial.println` and the Arduino Serial Plotter has run out of room. This page says where Serial Studio fits and where another tool is the better choice.

## Try it with the sketch you already have

A sketch that prints two comma-separated values per line works as is, with no project file, in the free edition (this is Quick Plot):

```cpp
Serial.println(String(t) + "," + String(h));
```

1. Open Serial Studio. In the Device Setup panel on the right edge of the window, set **Connection** to **UART/COM**, then pick the port and the baud rate from your sketch's `Serial.begin()`.
2. Set **Operation Mode** to **Quick Plot (Auto-Detected Format)**.
3. Click **Connect** in the toolbar.

Each value on the line gets its own live plot, and the sketch stays unchanged. [Getting Started](Getting-Started.md) walks through the same steps with screenshots.

## At a glance

Serial Studio is a desktop app that reads serial, network and Bluetooth LE data and draws dashboards from it. Simple cases need no script or server.

The free edition covers Serial/UART, network (TCP, UDP, WebSocket, HTTP) and Bluetooth LE, Quick Plot, Project File mode, all standard widgets and CSV export. Every other protocol (MQTT, Modbus, CAN Bus, OPC UA, S7, EtherNet/IP, IEC 104, Audio Input, raw USB, HID, Process I/O) is Pro, as are the 3D Plot, Waterfall, Image View and Canvas widgets. [Pro vs Free](Pro-vs-Free.md) has the full list, and the official binary's licence terms.

| Tool                      | What it is                                           | Cost model                         |
|---------------------------|------------------------------------------------------|------------------------------------|
| **Serial Studio**         | Telemetry dashboards from a GUI                      | GPL-3.0 source; Pro is paid        |
| Arduino Serial Plotter    | Line plots inside the Arduino IDE                    | Included with the IDE              |
| Processing                | Creative coding environment                          | Open source                        |
| Python + PySerial         | Your own script and plotting library                 | Open source                        |
| MATLAB, LabVIEW           | Commercial numerical and test-and-measurement suites | Commercial licences                |
| Grafana + Telegraf        | Web dashboards over a time-series database           | Open source; Grafana Cloud is paid |
| TeraTerm, PuTTY, CoolTerm | Terminal emulators                                   | Free                               |

## Serial Studio vs Arduino Serial Plotter

The Arduino Serial Plotter is the plotter built into the Arduino IDE. For a quick look at one or two values while you write a sketch, it is already open and needs nothing.

Serial Studio takes over when you want a separate scale per value, CSV export or gauges. Quick Plot gives each comma-separated value its own plot with its own scale, and CSV export is free. A project file, built in the [Project Editor](Project-Editor.md), adds gauges, bars, FFT and GPS maps, and a frame parser (JavaScript or Lua, turning a line of bytes into named values) handles binary protocols and checksums.

For a weather station, a project file with four values per line, such as `21.5,48,1013.2,270`, can show a gauge for temperature, a bar for humidity, a plot for pressure and a compass for wind direction, as described in [Widget Reference](Widget-Reference.md).

## Serial Studio vs Processing

Processing is a creative coding environment for interactive art and custom visualizations. If you want pixel-level control, or you are teaching programming through visual feedback, use it. Serial Studio fits when you want standard telemetry widgets configured in a GUI, and when teammates who do not write code need to open the dashboard.

## Serial Studio vs MATLAB and LabVIEW

Pick MATLAB when the work is analysis rather than monitoring: signal processing, statistics, Simulink models, publication-quality static plots, or an existing MATLAB codebase. Pick LabVIEW for National Instruments hardware, state machines and large test-and-measurement applications. Pick Serial Studio when you need to see telemetry live, without writing acquisition code or learning a visual programming language. MATLAB and LabVIEW are paid products; for Serial Studio's own cost, commercial use of the official binary needs Pro and a GPL build from source is free (see [Pro vs Free](Pro-vs-Free.md)).

## Serial Studio refresh rate and throughput

Serial Studio dashboards redraw at 60 Hz by default. Set **UI Refresh Rate (Hz)** under **Preferences → Plotting** (1 to 240) to change it; every incoming frame is still processed. The frame pipeline is tested in CI at 256,000 frames per second; actual throughput depends on your machine and driver.

## Serial Studio vs Python + PySerial

Python offers full control: pandas, numpy, custom algorithms, batch processing and automation. If Python is already part of the project, it is the natural tool for analysis.

Serial Studio is a standalone app with no Python install and no plotting code to maintain. It reads the port, parses frames, draws widgets and exports CSV. A common split is to monitor live in Serial Studio, export CSV, and analyze it in Python.

## Serial Studio vs Grafana + Telegraf

Grafana serves web dashboards over a time-series database such as InfluxDB or Prometheus, usually fed by a collector like Telegraf. It is the right tool for servers and infrastructure, shared team dashboards and historical analysis over weeks or months.

Serial Studio reads a serial port or BLE device directly. There is no database, collector or web server to set up: you install one app and connect. Grafana needs a database to read from and something to put serial data into it. To use both, Serial Studio Pro exports to InfluxDB 2.x, which Grafana can query directly; see [InfluxDB](InfluxDB.md).

## Serial Studio vs terminal emulators

TeraTerm and PuTTY are terminal emulators for serial, Telnet and SSH sessions, and CoolTerm is a serial-only terminal. They are the right choice for AT commands, a U-Boot or shell console, or SSH to a server.

Serial Studio turns the same bytes into plots, gauges, maps and FFT, logs them to CSV, and accepts custom frame parsing in JavaScript or Lua. File transfer over XMODEM, YMODEM or ZMODEM is a Pro feature; see [File Transmission](File-Transmission.md).

## Serial Studio vs a custom web dashboard

Node.js with Chart.js, or Python with Plotly Dash, makes sense when you need custom branding, integration with existing web infrastructure, or access from many browsers. You write and host the code.

Serial Studio is a desktop app, so there is no server to maintain and it works offline (the Examples button downloads from GitHub on first open). It ships a fixed widget set, extended in Pro by the scripted [Canvas Widget](Canvas-Widget.md) and by installable widget [Extensions](Extensions.md). To watch a dashboard from another machine, [Remote Dashboard](Remote-Dashboard.md) mirrors it read-only into a second Serial Studio, not a browser.

## Migrating

From the Arduino Serial Plotter, start with [Try it with the sketch you already have](#try-it-with-the-sketch-you-already-have). From Processing or a Python plotting script, replace the visualization part with a Serial Studio project file. Keep the script for custom analysis and feed it from the CSV export.
