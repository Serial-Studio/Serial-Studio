# Serial Studio

<a href="#">
    <img width="192px" height="192px" src="doc/icon.svg" align="right" />
</a>

[![Github commits](https://img.shields.io/github/last-commit/Serial-Studio/Serial-Studio?style=for-the-badge&logo=github)](https://github.com/Serial-Studio/Serial-Studio/commits/master)
[![GitHub contributors](https://img.shields.io/github/contributors/Serial-Studio/Serial-Studio?style=for-the-badge&logo=github)](https://github.com/Serial-Studio/Serial-Studio/graphs/contributors)
[![PR's Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen?style=for-the-badge)](https://github.com/Serial-Studio/Serial-Studio/pull/new)
[![Github All Releases](https://img.shields.io/github/downloads/Serial-Studio/Serial-Studio/total.svg?style=for-the-badge&logo=github)](https://github.com/Serial-Studio/Serial-Studio/releases/)
[![Instagram](https://img.shields.io/badge/Instagram-E4405F?style=for-the-badge&logo=instagram&logoColor=white)](https://instagram.com/serialstudio.app)
[![Donate](https://img.shields.io/badge/PayPal-00457C?style=for-the-badge&logo=paypal&logoColor=white)](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE)

Serial Studio is a multi-platform, versatile data visualization tool designed for embedded engineers, students, hackers, and teachers. It allows users to visualize, analyze, and represent data from their projects and devices without needing custom, project-specific visualization software. Serial Studio offers a flexible solution that adapts to a wide range of use cases, making it ideal for both educational and professional environments.

The tool was born out of my experience in multiple CanSat-based competitions, where I often found myself developing new Ground Station Software for each project. Over time, I realized it would be more efficient and sustainable to maintain a single, flexible Ground Station Software that allows users to define how incoming data is processed and displayed.

Today, Serial Studio is a powerful and adaptable tool, suitable not only for CanSat competitions but for any data acquisition and visualization project. It supports data retrieval from a wide range of sources, including hardware and software serial ports, MQTT, Bluetooth Low Energy (BLE), and network sockets (TCP/UDP).

*Read this document in other languages*: [Español](doc/README_ES.md) [简体中文](doc/README_ZH.md) [Deutsch](doc/README_DE.md) [Русский](doc/README_RU.md) [Français](doc/README_FR.md)

![Software usage](doc/screenshot.png)

## Features

- **Cross-platform:** Compatible with Windows, macOS, and Linux.
- **CSV Export:** Easily saves received data in CSV files for further analysis or processing.
- **Support for multiple data sources:** Handles a wide variety of sources, including serial ports, MQTT, Bluetooth Low Energy (BLE), and network sockets (TCP/UDP).
- **Customizable visualization:** Allows users to define and display data using various widgets, configurable via the project editor to meet specific needs.
- **Customizable frame analysis:** Provides the option to modify a JavaScript function to interpret incoming data frames, enabling the preprocessing of raw sensor data and handling of complex binary formats.
- **MQTT publishing and receiving:** Sends and receives data over the internet, enabling real-time data visualization from anywhere in the world.

## Installation

You can download and install the latest version of Serial Studio for your preferred platform from [here](https://github.com/Serial-Studio/Serial-Studio/releases/latest).

Here’s an updated version of your Linux installation instructions with the additional details:

### Linux Installation

For GNU/Linux users, there are multiple ways to install and run the application:

#### 1. AppImage

Download the AppImage file and ensure it has the correct executable permissions before running:

```bash
chmod +x SerialStudio-3.0.5-Linux-x86_64.AppImage
./SerialStudio-3.0.5-Linux-x86_64.AppImage
```

*Note:* You may need to install libfuse2 for the AppImage to work. On Debian/Ubuntu-based systems, you can install it using:

```bash
sudo apt update
sudo apt install libfuse2
```

You can integrate the Serial Studio *AppImage* into your system more easily using [AppImageLauncher](https://github.com/TheAssassin/AppImageLauncher/)..

#### 2. DEB/RPM Packages (Experimental)

You can also install Serial Studio using DEB or RPM packages, which are currently in an experimental stage. Please report any issues you encounter.

For Debian-based distributions (e.g., Debian, Ubuntu, Linux Mint, etc):

```bash
sudo dpkg -i SerialStudio-3.0.5-Linux-x86_64.deb
```

For RPM-based distributions (e.g., Fedora, RHEL, OpenSUSE, etc):

```bash
sudo rpm -i SerialStudio-3.0.5-Linux-x86_64.rpm
```

## Development

### Requirements

To compile Serial Studio, the only required dependency is [Qt](http://www.qt.io/download-open-source/). The desktop application compiles with **Qt 6.8.0**.

If you're compiling on GNU/Linux, you’ll also need to install `libgl1-mesa-dev`:

```bash
sudo apt install libgl1-mesa-dev
```

Here’s the list of required Qt modules:

- Qt SVG
- Qt Quick
- Qt Widgets
- Qt Location
- Qt Bluetooth
- Qt Networking
- Qt Positioning
- Qt Serial Port
- Qt Print Support
- Qt Quick Widgets
- Qt Quick Controls 2

### Cloning the Repository

To clone the repository with the necessary submodules, run:

```bash
git clone https://github.com/Serial-Studio/Serial-Studio
cd Serial-Studio
```

### Compiling the Application

Once Qt is installed, you can compile the project by opening the **CMakeLists.txt** file in your preferred IDE or by using the command line:

```bash
mkdir build
cd build 
cmake ../ -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j 10
```

## Software Architecture

Below is a simplified diagram illustrating how the different Serial Studio modules interact. For more detailed information, check out the full DOXYGEN documentation [here](https://serial-studio.github.io/hackers/).

![Architecture](doc/architecture/architecture.png)

## License

This project is licensed under the MIT License. For more details, see the [LICENSE](LICENSE.md) file.

## Support & Tipping

If you find Serial Studio useful, consider supporting its development by [tipping through PayPal](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE).

Alternatively, if you ever find yourself in [Cancún, Mexico](https://en.wikipedia.org/wiki/Cancun) and want to buy me a drink in person, feel free to [send me a DM on Instagram](https://instagram.com/aspatru). I’d love to meet you!
