# Arduino + Serial Studio Examples

This directory contains various examples demonstrating how to use Serial Studio to visualize data from sensors connected to an Arduino. Each example includes the Arduino code, a README file with setup instructions, and screenshots. Some examples also include Serial Studio project files (`*.json`) to simplify the visualization setup.

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
