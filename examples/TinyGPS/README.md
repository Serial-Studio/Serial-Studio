# TinyGPS + Serial Studio example

## Overview

This project shows how to use Serial Studio to visualize GPS location data from a GPS module (GY-NEO6MV2) connected to an Arduino. The Arduino reads GPS data (latitude, longitude, altitude) and sends it to Serial Studio over serial for display on a map.

![GPS data in Serial Studio](doc/screenshot.png)

**Compatibility.** Works with any GPS module that outputs NMEA sentences at 9600 baud. The sketch ships configured for an Uno or Nano; switching to a Mega 2560 (or any board with a spare hardware serial port) is a two-line change in the sketch.

## Hardware setup

You need a GY-NEO6MV2 GPS module and an Arduino. As shipped, the sketch reads the GPS through `SoftwareSerial` on an Uno or Nano: connect the TX pin of the GPS module to Arduino pin 2 (the sketch's software-serial RX pin). The GPS module must run at 9600 baud, which matches the default in the Arduino sketch.

On a Mega 2560 (or any board with a spare hardware serial port), use `Serial1` instead: connect the GPS module's TX pin to the Mega's `RX1` pin and swap these two lines in the sketch, uncommenting the first and removing the second:

```cpp
//HardwareSerial &gpsSerial = Serial1;
SoftwareSerial gpsSerial(2, 3); // For Arduino UNO/Nano
```

## Step by step

### 1. Arduino sketch (`TinyGPS.ino`)

Install the **TinyGPS** library:

- Open the Arduino IDE, go to **Sketch > Include Library > Manage Libraries**, search for "TinyGPS", and install it.

Once installed, open `TinyGPS.ino`, wire the GPS module per the hardware setup, and upload. The GPS data is transmitted with these delimiters:

- **Start delimiter (`$`).** Marks the start of a frame.
- **End delimiter (`;`).** Marks the end of the frame.
- **Separator (`,`).** Separates fields (latitude, longitude, altitude).

The frame looks like:

```
$latitude,longitude,altitude;
```

### 2. Serial Studio configuration

Serial Studio needs to be configured to parse the GPS data:

1. **Download and install Serial Studio.** Visit [Serial Studio's website](https://serial-studio.github.io/) to download and install.
2. **Open Serial Studio and import `TinyGPS.ssproj`.** Launch Serial Studio and load the `TinyGPS.ssproj` file from this project in the Project Editor. It has everything you need to interpret the Arduino's data.

#### Building from scratch

If you want to configure it yourself:

- **Frame start sequence:** `$`.
- **Frame end sequence:** `;`.
- **Data separator:** `,`.

Then click the **Map** button in Serial Studio to create a GPS map widget automatically. The widget uses the latitude and longitude to show the current position, and altitude is logged into the CSV.

![Serial Studio project setup](doc/project-setup.png)

### 3. API keys for maps

To get the most out of the mapping features, you may need API keys for some map styles:

- **Thunderforest API key.** Needed for some map types. Get one from [Thunderforest](https://www.thunderforest.com/docs/apikeys/).
- **MapTiler API key.** Needed for satellite maps. Get one from [MapTiler](https://www.maptiler.com/cloud/). The satellite map may take a bit to download and cache on first use.

### 4. Viewing GPS data

Once Serial Studio is configured:

- The bundled `TinyGPS.ssproj` configures a UART source at 115200 baud (matching the Arduino sketch). Pick the Arduino's serial port and connect.
- You'll see the real-time GPS position on the map widget. Altitude can be logged to a CSV file for later analysis.

### Troubleshooting

- **No data on the map.** Make sure the GPS module has a clear view of the sky to get a satellite fix. Check that the correct port and baud rate are selected in Serial Studio.
- **Incorrect parsing.** Double-check that the start sequence (`$`), end sequence (`;`), and separator (`,`) are set correctly in the project settings.
