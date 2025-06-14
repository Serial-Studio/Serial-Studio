# ADC Noise Analysis + FFT Visualization with Serial Studio

## Overview

This project demonstrates how to read and transmit analog-to-digital converter (ADC) values from six unconnected analog pins (A0 to A5) on an Arduino and analyze signal noise using **Fast Fourier Transform (FFT)** in **Serial Studio**.

The Arduino reads raw ADC values from floating analog pins, scales them to 8-bit (0–255), adds a CRC-16-CCITT checksum, and transmits the binary data over the serial port. Serial Studio parses the data, verifies the checksum, and visualizes it in real time using the FFT tool.

### Compatibility

Works with any Arduino board that has analog input pins. No external components are required. Floating analog pins (A0–A5) will naturally generate noise that can be analyzed in the frequency domain.

![Serial Studio FFT](doc/screenshot.png)

## Hardware Setup

### Connections

No external wiring is needed. Leave analog pins A0 to A5 unconnected to observe ambient noise.

### Optional

You can connect analog sensors or signal generators to the pins for more controlled signal analysis.

## Step-by-Step Guide

### 1. Arduino Sketch ([`HexadecimalADC.ino`](HexadecimalADC.ino))

This sketch reads analog values from pins A0–A5, maps the 10-bit readings to 8-bit values, appends a CRC-16-CCITT checksum, and transmits a binary data frame over serial.

Each frame layout:
- Start delimiter: `0xC0 0xDE`
- Sensor values: 6 bytes (scaled to 0–255)
- End delimiter: `0xDE 0xC0`
- CRC-16-CCITT: 2 bytes (big-endian)

**Baud Rate:** 115200

### 2. Serial Studio Configuration

#### Frame Format
- **Mode**: `Binary (direct)`
- **Frame Delimiters**: 
  - Start: `0xC0 0xDE`
  - End: `0xDE 0xC0`
- **Checksum**: `CRC-16-CCITT`

#### Loading the Project
1. Download and install Serial Studio from the [official website](https://serial-studio.com/).
2. Launch the app and load `HexadecimalADC.json` using the **Project Editor**.

#### JavaScript Parser Function

In `Binary (direct)` mode, the frame is passed as a raw byte array. The parser below converts each value into a voltage (0–5V):

```javascript
/**
 * This function parses a binary data frame (passed as a byte array) and converts
 * each byte into a decimal value scaled from 0 to 5 volts.
 *
 * @param[in]  frame  A JS array of byte values (0–255), passed from C++.
 * @return     Array of floats representing the scaled voltage values.
 */
function parse(frame) {
    let dataArray = [];

    for (let i = 0; i < frame.length; ++i) {
        let byte = frame[i];
        dataArray.push(byte * 5.0 / 255);
    }

    return dataArray;
}
```

[More about custom frame parsers here](https://github.com/Serial-Studio/Serial-Studio/wiki/Project-Editor#frame-parser-function-view).

### 3. Analyzing Noise with FFT

1. Open Serial Studio and select the correct serial port.
2. Set the baud rate to **115200**.
3. Observe the voltage signals from the floating analog pins.
4. Use the **FFT widget** to visualize the frequency characteristics of the noise.

### Troubleshooting Tips

- **No data shown**: Verify baud rate and correct serial port.
- **Flat FFT spectrum**: Try touching the pins or adding a sensor to inject a signal.
- **Validation errors**: Ensure `CRC-16-CCITT` is selected and frame format matches what the Arduino sends.
