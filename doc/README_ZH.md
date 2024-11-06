<a href="#">
    <img width="192px" height="192px" src="/doc/icon.svg" align="right" />
</a>

# Serial Studio

[![Github commits](https://img.shields.io/github/last-commit/Serial-Studio/Serial-Studio?style=for-the-badge&logo=github)](https://github.com/Serial-Studio/Serial-Studio/commits/master)
[![GitHub contributors](https://img.shields.io/github/contributors/Serial-Studio/Serial-Studio?style=for-the-badge&logo=github)](https://github.com/Serial-Studio/Serial-Studio/graphs/contributors)
[![PR's Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen?style=for-the-badge)](https://github.com/Serial-Studio/Serial-Studio/pull/new)
[![Github All Releases](https://img.shields.io/github/downloads/Serial-Studio/Serial-Studio/total.svg?style=for-the-badge&logo=github)](https://github.com/Serial-Studio/Serial-Studio/releases/)
[![Instagram](https://img.shields.io/badge/Instagram-E4405F?style=for-the-badge&logo=instagram&logoColor=white)](https://instagram.com/serialstudio.app)
[![Donate](https://img.shields.io/badge/PayPal-00457C?style=for-the-badge&logo=paypal&logoColor=white)](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE)

Serial Studio 是一个跨平台、功能多样的数据可视化工具，专为嵌入式系统工程师、学生、黑客和教师设计。它允许用户可视化、分析和展示他们项目和设备中的数据，而无需为每个项目开发特定的可视化软件。**Serial Studio** 提供了一种灵活的解决方案，适应各种使用场景，非常适合教育和专业环境。

这个工具源于我在多个 CanSat 比赛中的经验，那时我经常需要为每个项目开发新的地面站软件。随着时间的推移，我意识到维护一个灵活的通用软件，让用户可以定义如何处理和显示传入的数据，会更加高效和可持续。

如今，Serial Studio 是一个强大且灵活的工具，不仅适用于 CanSat 竞赛，也适用于任何数据采集和可视化项目。它支持从多种来源获取数据，包括串行端口、MQTT、蓝牙低功耗 (BLE) 和网络套接字 (TCP/UDP)。

*阅读其他语言版本的文档*: [Español](/doc/README_ES.md) [简体中文](/doc/README_ZH.md) [Deutsch](/doc/README_DE.md) [Русский](/doc/README_RU.md) [Français](/doc/README_FR.md)

![软件使用](/doc/screenshot.png)

## 功能

- **跨平台：** 兼容 Windows、macOS 和 Linux。
- **CSV 导出：** 轻松将接收到的数据保存为 CSV 文件，以便进一步分析或处理。
- **多数据源支持：** 支持多种来源，包括串行端口、MQTT、蓝牙低功耗 (BLE) 和网络套接字 (TCP/UDP)。
- **可自定义可视化：** 通过项目编辑器定义和显示数据，使用各种可配置的小部件以满足用户的具体需求。
- **可自定义帧解析：** 提供修改 JavaScript 函数的选项，以解析传入的数据帧，方便处理原始传感器数据并支持复杂的二进制格式。
- **通过 MQTT 发布和接收：** 通过互联网发送和接收数据，支持从世界任何地方实时可视化数据。

## 安装

你可以从 [这里](https://github.com/Serial-Studio/Serial-Studio/releases/latest) 下载并安装适用于你首选平台的最新版本 **Serial Studio**。

以下是您的 Linux 安装说明的中文翻译：

### Linux 安装

对于 GNU/Linux 用户，有多种方式可以安装和运行该应用程序：

#### 1. AppImage

下载 AppImage 文件，并确保它具有正确的执行权限，然后运行：

```
chmod +x SerialStudio-3.0.5-Linux-x86_64.AppImage
./SerialStudio-3.0.5-Linux-x86_64.AppImage
```

注意： 可能需要安装 libfuse2 以确保 AppImage 能够正常工作。在基于 Debian/Ubuntu 的系统上，可以使用以下命令进行安装：

```
sudo apt update
sudo apt install libfuse2
```

您还可以使用 [AppImageLauncher](https://github.com/TheAssassin/AppImageLauncher/) 更轻松地将 Serial Studio 的 AppImage 集成到您的系统中。

#### 2. DEB/RPM 包（实验性）

您还可以通过 DEB 或 RPM 包安装 Serial Studio，目前这些包处于实验阶段。如果遇到任何问题，请反馈。

对于基于 Debian 的发行版（例如 Debian、Ubuntu、Linux Mint 等）：

```
sudo dpkg -i SerialStudio-3.0.5-Linux-x86_64.deb
```

对于基于 RPM 的发行版（例如 Fedora、RHEL、OpenSUSE 等）：

```
sudo rpm -i SerialStudio-3.0.5-Linux-x86_64.rpm
```

## 开发

### 要求

编译 Serial Studio 的唯一必要依赖项是 [Qt](http://www.qt.io/download-open-source/)。桌面应用程序使用 **Qt 6.8.0** 进行编译。

如果你在 GNU/Linux 上进行编译，你还需要安装 `libgl1-mesa-dev`：

```bash
sudo apt install libgl1-mesa-dev
```

以下是需要的 Qt 模块列表：

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

### 克隆仓库

要克隆包含必要子模块的仓库，请执行以下命令：

```bash
git clone https://github.com/Serial-Studio/Serial-Studio
cd Serial-Studio
```

### 编译应用程序

安装 Qt 后，你可以通过在你喜欢的 IDE 中打开 **CMakeLists.txt** 文件或使用命令行编译项目：

```bash
mkdir build
cd build 
cmake ../ -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j 10
```

## 软件架构

下面是一个简化的示意图，展示了 Serial Studio 各个模块如何交互。有关更详细的信息，请查阅完整的 DOXYGEN 文档 [这里](https://serial-studio.github.io/hackers/)。

![架构](/doc/architecture/architecture.png)

## 许可证

此项目根据 MIT 许可证授权。更多详细信息，请参阅 [LICENSE](/LICENSE.md) 文件。

## 支持与捐赠

如果你觉得 Serial Studio 有用，欢迎通过 [PayPal 捐赠](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE) 支持其开发。

或者，如果你有机会来到 [墨西哥坎昆](https://zh.wikipedia.org/wiki/坎昆)，并想亲自请我喝一杯，随时可以通过 [Instagram 给我发消息](https://instagram.com/aspatru)。我很乐意认识你！
