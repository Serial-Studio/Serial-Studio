# ADC Noise Analysis + FFT Visualization with Serial Studio

## Overview

This project demonstrates how to read and transmit analog-to-digital converter (ADC) values from six unconnected analog pins (A0 to A5) on an Arduino and analyze the noise in the signal using **Fast Fourier Transform (FFT)** for frequency domain visualization in **Serial Studio**.

The Arduino reads the raw ADC values from the unconnected pins (floating values), scales them to 8-bit (0-255) binary, and transmits them over the serial port. The data is parsed and visualized in Serial Studio, where the **FFT** feature is used to observe the frequency characteristics of the noise.

### Compatibility
This project works with any Arduino board that has analog input pins. Although no external sensors are required, the analog pins **A0 to A5** will generate fluctuating noise due to their unconnected state, which can be analyzed using the FFT.

![Serial Studio FFT](doc/screenshot.png)

## Hardware Setup

### Connections

No external hardware is required for this project, as the analog pins (A0 to A5) are left **unconnected** to measure floating ADC values.

### Optional
If you'd like to connect external components to generate specific signals (e.g., noise, sine waves, etc.), you can connect these to the analog pins for more controlled analysis.

## Step-by-Step Guide

### 1. Arduino Sketch ([`HexadecimalADC.ino`](HexadecimalADC.ino))

This Arduino sketch reads analog values from six floating pins (A0 to A5), scales the 10-bit readings (0-1023) to 8-bit (0-255), and transmits the binary data over the serial port in frames. The data is processed by Serial Studio for FFT visualization.

- **Analog Reading**: Data is read from analog pins A0 to A5.
- **Data Conversion**: The 10-bit ADC values are mapped to 8-bit (0-255) and sent over the serial port.
- **Serial Output**: Data is framed by a start (`$`) and end (`;`) delimiter, and transmitted in binary format for visualization.

The serial output can be analyzed in **Serial Studio** using the **FFT tool** to observe the frequency spectrum of the noise or any connected signals.

The program runs at a baud rate of **115200**, ensuring smooth and high-speed data transmission.

### 2. Serial Studio Configuration

To visualize the data, Serial Studio needs to be properly configured to parse the incoming hexadecimal data:

1. **Download and Install Serial Studio**:
Visit [Serial Studio's official website](https://serial-studio.github.io/) to download and install the software.

2. **Open Serial Studio and Import `HexadecimalADC.json`**:
Launch Serial Studio and load the `HexadecimalADC.json` file included in this project using the **Project Editor**. This file contains all necessary configurations for interpreting the data transmitted by the Arduino.

#### About the JavaScript Parser Function

Since the data is transmitted in hexadecimal format, a custom JavaScript parser function is required to convert the incoming frame data into an array of decimal values that can be plotted or analyzed in Serial Studio.

Here’s the custom JavaScript parser function used by this project:

```javascript
/**
 * This function parses a binary data frame (represented as a hexadecimal string),
 * and converts it into an array of decimal values (0-255).
 *
 * @param[in]  frame  The latest received frame as a hexadecimal string.
 * @return     Array of integers containing the parsed frame elements.
 */
function parse(frame) {
    let dataArray = [];
    for (let i = 0; i < frame.length; i += 2) {
        let hexByte = frame.substring(i, i + 2);
        let decimalValue = parseInt(hexByte, 16);
        dataArray.push(decimalValue);
    }

    return dataArray;
}
```

This function reads the binary data sent from the Arduino, converts each 2-character hexadecimal byte into a decimal value (0-255), and returns an array of values that are displayed on the Serial Studio dashboard or used for FFT analysis.

You can read more about the frame parsing function [here](https://github.com/Serial-Studio/Serial-Studio/wiki/Project-Editor#frame-parser-function-view).

### 3. Analyzing Noise with FFT

Once you have configured Serial Studio and uploaded the Arduino sketch, follow these steps to analyze the noise data using FFT:

- **Connect to the Arduino**: Ensure that Serial Studio is connected to the correct serial port and that the baud rate is set to **115200**.
- **Visualize the Data**: The analog values from the unconnected ADC pins will be plotted in real time, showing random floating values due to the noise.

### Troubleshooting Tips

- **No Data in FFT Plot**: Verify that the Arduino is properly connected, and the serial port and baud rate match the configuration in Serial Studio.
- **Flat FFT Spectrum**: Unconnected ADC pins usually generate low-amplitude noise. If the FFT spectrum appears flat, try adjusting the sample rate or adding external components (such as signal generators) to visualize more pronounced frequency components.
- **Inconsistent Data**: If the data appears inconsistent, ensure that the **Hexadecimal Conversion** method in the Project Editor is selected, and the JavaScript parser is functioning correctly in Serial Studio.
