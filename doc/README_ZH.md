<a href="#">
    <img width="192px" height="192px" src="/doc/icon.svg" align="right" />
</a>

# Serial Studio

[![Build Status](https://github.com/Serial-Studio/Serial-Studio/workflows/Deploy/badge.svg)](https://github.com/Serial-Studio/Serial-Studio/actions/)
[![CodeQL](https://github.com/Serial-Studio/Serial-Studio/workflows/CodeQL/badge.svg)](https://github.com/Serial-Studio/Serial-Studio/actions?query=workflow%3ACodeQL)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/4b6f3ce14a684704980fea31d8c1632e)](https://www.codacy.com/gh/Serial-Studio/Serial-Studio/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Serial-Studio/Serial-Studio&amp;utm_campaign=Badge_Grade)
[![Github All Releases](https://img.shields.io/github/downloads/Serial-Studio/Serial-Studio/total.svg)](https://github.com/Serial-Studio/Serial-Studio/releases/)
[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-v1.4%20adopted-ff69b4.svg)](CODE_OF_CONDUCT.md)
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE)

Serial Studio是一个多平台，多用途的串行数据可视化的应用软件。本项目的目标是使嵌入式开发人员和制造商可以轻松地可视化的呈现和分析其项目和设备生成的数据，而无需为每个项目编写专用的计算机软件。

在我参加过的一些基于CanSat比赛的地面工作站的软件开发过程中，对这个项目的需求有所增加。为每个竞赛和项目开发和维护不同的GSS软件是不可持续的。正确的解决方案是拥有一个通用的地面工作站软件，并让每个CanSat定义如何使用可扩展的通信协议并将数据呈现给最终用户。

此外，本项目使用的方法可以扩展到涉及某种数据采集和测量的几乎任何类型的项目中。如果您想更深入地了解本项目为何存在以及其全部内容，请查看[此博客文章](https://www.alex-spataru.com/blog/introducing-serial-studio)。

**注意:**有关通信协议的说明在[Wiki中](https://github.com/Serial-Studio/Serial-Studio/wiki/Communication-Protocol)提供。

*其他语言请阅读*  :[English](../README.md)、 [Español](README_ES.md)、 [Deutsch](README_DE.md)

![Software usage](mockup.png)

## 编译说明

#### 要求

编译本项目的唯一要求是在你的电脑系统中安装 QT ， 本项目编译支持 **Qt 5.15***。

在GNU/Linux系统上，还需要安装`libgl1-mesa-dev`才能编译应用程序。

已使用的Qt模块的完整列表：

- Qt SVG
- Qt Quick
- Qt Widgets
- Qt Networking
- Qt Serial Port
- Qt Print Support
- Qt Quick Widgets
- Qt Quick Controls 2

#### 克隆

本仓库使用[`git submodule`](https://git-scm.com/book/en/v2/Git-Tools-Submodules)。为了克隆它，请在终端上执行以下命令：

	git clone https://github.com/Serial-Studio/Serial-Studio
	cd Serial-Studio
	git submodule init
	git submodule update

或者，只运行:

	git clone --recursive https://github.com/Serial-Studio/Serial-Studio

#### 编译

安装 Qt 后，在 Qt Creator 中打开 *Serial-Studio.pro*，然后单击 “运行” 按钮。

或者，您也可以使用以下命令：

	qmake
	make -j4

## Licence

本项目是根据MIT许可证发布的，更多信息，请查看[LICENSE](https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md)文件。



