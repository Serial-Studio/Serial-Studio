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

**Serial Studio** is an open source, cross-platform telemetry dashboard and real-time data visualization tool. It supports input from serial ports, Bluetooth Low Energy (BLE), MQTT, and TCP/UDP sockets, allowing data acquisition from embedded devices, external software, and networked services.

Serial Studio runs on Windows, macOS, and Linux. It is suited for telemetry monitoring, sensor data analysis, and real-time debugging in educational, hobbyist, and professional environments.

![Software usage](doc/screenshot.png)

## Download
Serial Studio is available as source code and official precompiled binaries for Windows, macOS, and Linux.

- [Latest Stable Release](https://github.com/Serial-Studio/Serial-Studio/releases/latest)
- [Pre-release Builds](https://github.com/Serial-Studio/Serial-Studio/releases/continuous)

#### Microsoft Windows:
Requires the [Microsoft Visual C++ Redistributable (x64)](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170#latest-microsoft-visual-c-redistributable-version). On first launch, Windows may show a warning about an unknown developer, click _"More Info → Run Anyway"_ to continue.

#### Linux:
Provided as an [AppImage](https://appimage.org/). Make it executable and run it:

```bash
chmod +x SerialStudio-3.0.6-Linux-x86_64.AppImage
./SerialStudio-3.0.6-Linux-x86_64.AppImage
```

Some systems may require `libfuse2` to run AppImages:

```bash
sudo apt install libfuse2
```

**Recommendation:** Use [AppImageLauncher](https://github.com/TheAssassin/AppImageLauncher) to integrate Serial Studio with your desktop environment.

##### Raspberry Pi / ARM64:
An AppImage build for ARM64 is also available for devices like the Raspberry Pi. _Mileage may vary_ depending on your hardware and GPU drivers, as Serial Studio relies heavily on GPU-accelerated operations to render the user interface. A 64-bit Linux OS and `libfuse2` are required.

#### macOS: 
Distributed as a universal DMG. Open the DMG file and drag **Serial Studio** into the **Applications** folder.
Alternatively, you can try installing via Homebrew:

```bash
brew install --cask serial-studio
```

**Note:** The Homebrew cask is community-maintained. It’s available, but not officially developed or tested by me.

## Features

#### Operation Modes:
- **Project File Mode (recommended):** Uses local JSON files created with the **Project Editor** to define the dashboard layout and data mapping.
- **Quick Plot Mode:** Automatically plots comma-separated values with no configuration.
- **Device-defined Mode:** Dashboards are fully defined by incoming JSON data from the device.

#### Core Capabilities:
- **Cross-platform:** Runs on Windows, macOS, and Linux.
- **CSV export:** Save received data for offline analysis or processing.
- **Multiple data sources:** Supports serial ports, MQTT, BLE, and network sockets (TCP/UDP).
- **Customizable visualization:** Build dashboards using various widgets via the integrated project editor.
- **Advanced frame decoding:** Use a custom JavaScript function to preprocess raw data or handle complex binary formats.
- **MQTT support:** Publish and receive data over the internet for remote visualization.

## Documentation

Refer to the [Wiki](https://github.com/Serial-Studio/Serial-Studio/wiki) for complete guides and examples:

- **Installation:** Instructions for Windows, macOS, and Linux.
- **Quick Start:** Connect a device and visualize data in minutes.
- **Advanced Usage:** Learn about data flow, frame parsing, and dashboard customization.
- **Examples:** Sample code and projects to accelerate learning.

## Building Serial Studio

The only required dependency to build Serial Studio from source is [Qt](https://www.qt.io/download-open-source/), preferably with all modules and plugins installed. The project is built using **Qt 6.9.1**.

#### Additional Requirements for Linux

If you’re compiling on Linux, install the following packages:

```bash
sudo apt install libgl1-mesa-dev build-essential
```

#### Build Instructions

Once Qt is installed, you can compile the project by opening `CMakeLists.txt` in your preferred IDE or using the terminal:

```bash
mkdir build
cd build
cmake ../ -DPRODUCTION_OPTIMIZATION=ON -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

If you build Serial Studio using an open-source Qt installation, the resulting binary is licensed under the terms of the GNU GPLv3. You are free to use and distribute that build as long as you comply with the license.

**Note:** GPL builds exclude certain features such as MQTT, 3D plotting, and others that depend on proprietary Qt modules not available in open-source distributions.

## Support & Licensing
Serial Studio is developed and maintained by [Alex Spataru](https://github.com/alex-spataru).  
It is open source and community-driven, with commercial options available for users who need advanced features or business-friendly licensing.

If Serial Studio is useful to you, consider supporting its development in one of the following ways:

- [**Donate via PayPal**](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE): Helps keep the project active and sustainable.
- [**Purchase a Commercial License**](https://serial-studio.com): Required for commercial use of the official binary. Includes Pro features and priority support.

Commercial licenses directly fund continued development, bug fixes, and new features.

## License
Serial Studio is distributed under a dual-license model. For full licensing terms, see [LICENSE.md](LICENSE.md). 

#### Open Source (GPLv3)

The source code is licensed under the GNU General Public License v3 (GPLv3). You are free to use, modify, and redistribute it under GPL terms, provided any derivative works are also licensed under GPLv3.

To use Serial Studio under the GPL:
- You must compile it yourself using an open-source Qt installation, or
- Use a GPL-compliant build distributed by a trusted package manager.

**Note:** The GPL license applies only to builds made from source using open-source Qt. It does not cover official binaries.

### Commercial License
All official binaries downloaded from [serial-studio.com](https://serial-studio.com/), GitHub Releases, or other channels maintained by the author are covered by a Commercial License.

A paid license is required to:
- Use the official binary in any commercial, enterprise, or proprietary environment.
- Access Pro features (MQTT, XY plotting, 3D plots, etc.).
- Receive priority support.

Without a valid license, use of the binary is limited to personal and evaluation purposes only.
