# Serial Studio

<a href="https://github.com/Serial-Studio/Serial-Studio/tree/master/doc/assets">
    <img width="192px" height="192px" src="doc/icon.svg" align="right" />
</a>

[![Github commits](https://img.shields.io/github/last-commit/Serial-Studio/Serial-Studio?style=for-the-badge&logo=github)](https://github.com/Serial-Studio/Serial-Studio/commits/master)
[![GitHub contributors](https://img.shields.io/github/contributors/Serial-Studio/Serial-Studio?style=for-the-badge&logo=github)](https://github.com/Serial-Studio/Serial-Studio/graphs/contributors)
[![PR's Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen?style=for-the-badge)](https://github.com/Serial-Studio/Serial-Studio/pull/new)
[![Github All Releases](https://img.shields.io/github/downloads/Serial-Studio/Serial-Studio/total.svg?style=for-the-badge&logo=github)](https://github.com/Serial-Studio/Serial-Studio/releases/)
[![Instagram](https://img.shields.io/badge/Instagram-E4405F?style=for-the-badge&logo=instagram&logoColor=white)](https://instagram.com/serialstudio.app)
[![Donate](https://img.shields.io/badge/Donate-00457C?style=for-the-badge&logo=paypal&logoColor=white)](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE)

Serial Studio is a multi-platform, versatile data visualization tool designed for embedded engineers, students, hackers, and teachers. It allows users to visualize, analyze, and represent data from their projects and devices without needing custom, project-specific visualization software. Serial Studio offers a flexible solution that adapts to a wide range of use cases, making it ideal for both educational and professional environments.

The tool was born out of my experience in multiple CanSat-based competitions, where I often found myself developing new Ground Station Software for each project. Over time, I realized it would be more efficient and sustainable to maintain a single, flexible Ground Station Software that allows users to define how incoming data is processed and displayed.

Today, Serial Studio is a powerful and adaptable tool, suitable not only for CanSat competitions but for any data acquisition and visualization project. It supports data retrieval from a wide range of sources, including hardware and software serial ports, MQTT, Bluetooth Low Energy (BLE), and network sockets (TCP/UDP).

![Software usage](doc/screenshot.png)

## Features

- **Cross-platform:** Compatible with Windows, macOS, and Linux.
- **CSV Export:** Easily saves received data in CSV files for further analysis or processing.
- **Support for multiple data sources:** Handles a wide variety of sources, including serial ports, MQTT, Bluetooth Low Energy (BLE), and network sockets (TCP/UDP).
- **Customizable visualization:** Allows users to define and display data using various widgets, configurable via the project editor to meet specific needs.
- **Customizable frame analysis:** Provides the option to modify a JavaScript function to interpret incoming data frames, enabling the preprocessing of raw sensor data and handling of complex binary formats.
- **MQTT publishing and receiving:** Sends and receives data over the internet, enabling real-time data visualization from anywhere in the world.

Visit the **[Wiki](https://github.com/Serial-Studio/Serial-Studio/wiki)** for comprehensive guides, including:

- **Installation Instructions:** Set up Serial Studio on Windows, macOS, or Linux.  
- **Quick Start Guide:** Learn how to connect your device and visualize data in minutes.  
- **Advanced Topics:** Explore data flow, frame parsing, and building custom dashboards.
- **Examples:** Examples with code, projects, and explanations to help you learn Serial Studio.

### Building Serial Studio

To build Serial Studio from source, the only required dependency is [Qt](https://www.qt.io/download-open-source/) (preferably with all plugins and modules installed). The application is built using **Qt 6.8.1**.

If you're compiling on GNU/Linux, install the following additional packages:

```bash
sudo apt install libgl1-mesa-dev build-essential
```

Once Qt is installed, you can compile the project by opening `CMakeLists.txt` in your preferred IDE or using the terminal:

```
mkdir build
cd build
cmake ../ -DPRODUCTION_OPTIMIZATION=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

If you build Serial Studio using an open-source Qt installation, the resulting binary is automatically licensed under the terms of the GNU GPLv3. You are free to use and distribute that build as long as you comply with the GPL.

The GPLv3 build excludes MQTT, CAN/Modbus, 3D plotting, and other features available in with a Commercial License. Most of these features depend on Qt modules that are difficult to support with open-source builds.

## Support & Licensing
Serial Studio is developed and maintained by [Alex Spataru](https://github.com/alex-spataru).  
It is open source and community-driven, with commercial options available for users who need advanced features or business-friendly licensing.

If Serial Studio is useful to you, consider supporting its development in one of the following ways:

- [**Donate via PayPal**](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE) – Helps keep the project active and sustainable.
- [**Purchase a Commercial License**](https://serial-studio.com) – Required for commercial use of the official binary. Includes Pro features and priority support.

Commercial licenses directly fund continued development, bug fixes, and new features.

## License
Serial Studio is distributed under a dual-license model. For full licensing terms, see [LICENSE.md](LICENSE.md). 

#### Open Source (GPLv3)
The source code is licensed under the GNU General Public License v3 (GPLv3). You can use, modify, and redistribute the source code under GPL terms, as long as any derivative works are also released under GPLv3.

To use Serial Studio under GPLv3:
- You must compile it yourself using an open-source Qt installation, or
- Install it from a trusted package manager that distributes GPL-compliant builds.

The GPL license does not apply to the official precompiled binaries.

### Commercial License
All official binaries downloaded from [serial-studio.com](https://serial-studio.com/), GitHub Releases, or other channels maintained by the author are covered by a Commercial License.

A paid license is required to:
- Use the official binary in any commercial, enterprise, or proprietary environment.
- Access Pro features (MQTT, CANBus, Modbus, advanced plotting, etc.).
- Receive priority support.

Without a valid license, use of the binary is limited to personal and evaluation purposes only.
