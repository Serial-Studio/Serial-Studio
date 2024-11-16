# Serial Studio Examples

This directory contains various examples demonstrating how to use Serial Studio to visualize data from sensors connected to a microcontroller or another program. Each example includes an Arduino implementation (if required for the project), a README file with setup instructions, and screenshots. 

Some examples also include Serial Studio project files (`*.json`) to simplify the visualization setup.

## Examples Overview

### 1. HexadecimalADC
- **Description**: This example reads analog input data from an ADC and transmits it over serial in hexadecimal format.
- **Contents**:
  - **HexadecimalADC.ino**: Arduino code for reading and transmitting ADC data.
  - **HexadecimalADC.json**: Serial Studio project file for visualizing the ADC output in hexadecimal format.
  - **README.md**: Setup and usage instructions.
  - **Screenshot**: Example view in Serial Studio.
  
### 2. MPU6050
- **Description**: This example captures motion and orientation data from an MPU6050 accelerometer and gyroscope. Processed data is sent to Serial Studio for real-time visualization on widgets like a g-meter or attitude indicator.
- **Contents**:
  - **MPU6050.ino**: Arduino code for capturing and transmitting MPU6050 data.
  - **MPU6050.json**: Serial Studio project file for visualizing accelerometer, gyroscope and temperature data from the MPU6050 module.
  - **README.md**: Detailed setup instructions, including Serial Studio configuration.
  - **Screenshots**: `project-setup.png` and `screenshot.png` provide visual references for Serial Studio setup and data visualization.

### 3. PulseSensor
- **Description**: This example filters and smooths pulse data from a heart rate sensor and visualizes it in Serial Studio using **quick plot mode**. The filtered pulse signal is transmitted for live monitoring and CSV logging.
- **Contents**:
  - **PulseSensor.ino**: Arduino code for filtering and transmitting pulse data.
  - **README.md**: Step-by-step guide for setup and visualization in Serial Studio.
  - **Screenshots**: `csv.png` and `screenshot.png` for reference in CSV logging and Serial Studio visualization.

### 4. TinyGPS
- **Description**: This example reads GPS data (latitude, longitude, and altitude) from a GPS module and visualizes it on a map in Serial Studio.
- **Contents**:
  - **TinyGPS.ino**: Arduino code for capturing and transmitting GPS data.
  - **TinyGPS.json**: Serial Studio project file to visualize GPS data on a map.
  - **README.md**: Comprehensive setup instructions for GPS configuration, including Serial Studio setup.
  - **Screenshots**: `project-setup.png` and `screenshot.png` for guidance on map visualization in Serial Studio.

### 5. UDP Function Generator

- **Description**: This example generates real-time waveforms (sine, triangle, sawtooth, and square) and transmits them over an UDP socket locally. It is designed to generate data that can be visualized in **Serial Studio**, where you can observe and analyze the generated signals in real-time. The program is versatile and can also be used to stress-test Serial Studio's performance under continuous, high-frequency data streams.
- **Contents**:
  - **udp_function_generator.c**: The main C program that generates waveforms and sends them via UDP.
  - **README.md**: Detailed setup and usage instructions for configuring and running the program with Serial Studio.
  - **Screenshots**: Includes `serial-studio-setup.png` for configuration and `waveform-visualization.png` showcasing real-time waveform plots in Serial Studio.
- **Key Features**:
  - Generates multiple waveform types: sine, triangle, sawtooth, and square.
  - Configurable waveform properties: frequency, phase, and transmission interval.
  - Sends waveform data over UDP, making it ideal for network-based signal processing.
  - Option to print generated data for debugging and analysis.
  - Warns about high frequencies that may cause aliasing or distortion.
- :warning: Using sub-millisecond intervals is likely to overload Serial Studio's event system, potentially causing crashes and/or hangs. If you encounter this issue, consider running Serial Studio with a debugger and sharing your findings to help improve and address this limitation in future releases. Your feedback is invaluable in making Serial Studio more robust!

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
