<a href="#">
    <img width="192px" height="192px" src="/doc/icon.svg" align="right" />
</a>

# Serial Studio

[![Github commits](https://img.shields.io/github/last-commit/Serial-Studio/Serial-Studio)](https://github.com/Serial-Studio/Serial-Studio/commits/master)
[![GitHub contributors](https://img.shields.io/github/contributors/Serial-Studio/Serial-Studio)](https://github.com/Serial-Studio/Serial-Studio/graphs/contributors)
[![PR's Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat)](https://github.com/Serial-Studio/Serial-Studio/pull/new)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/4b6f3ce14a684704980fea31d8c1632e)](https://www.codacy.com/gh/Serial-Studio/Serial-Studio/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Serial-Studio/Serial-Studio&amp;utm_campaign=Badge_Grade)
[![Github All Releases](https://img.shields.io/github/downloads/Serial-Studio/Serial-Studio/total.svg)](https://github.com/Serial-Studio/Serial-Studio/releases/)
[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-v1.4%20adopted-ff69b4.svg)](CODE_OF_CONDUCT.md)
[![Donate](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE)

Serial Studio — это многоплатформенный и универсальный инструмент для визуализации данных, разработанный для инженеров встраиваемых систем, студентов, хакеров и преподавателей. Он позволяет пользователям визуализировать, анализировать и отображать данные своих проектов и устройств без необходимости разработки специализированного ПО для каждого проекта. Serial Studio предлагает гибкое решение, которое адаптируется к широкому спектру случаев использования, что делает его идеальным как для образовательных, так и для профессиональных сред.

Этот инструмент был создан на основе моего опыта участия в нескольких соревнованиях CanSat, где мне часто приходилось разрабатывать новое ПО для наземной станции для каждого проекта. Со временем я понял, что было бы более эффективно и устойчиво поддерживать одну гибкую программу, которая позволяет пользователям определять, как обрабатывать и отображать входящие данные.

Сегодня Serial Studio — это мощный и адаптируемый инструмент, подходящий не только для соревнований CanSat, но и для любых проектов по сбору и визуализации данных. Он поддерживает получение данных из множества источников, включая последовательные порты, MQTT, Bluetooth Low Energy (BLE) и сетевые сокеты (TCP/UDP).

*Читать этот документ на других языках*: [Español](/doc/README_ES.md) [简体中文](/doc/README_ZH.md) [Deutsch](/doc/README_DE.md) [Русский](/doc/README_RU.md)

![Использование программы](/doc/screenshot.png)

## Особенности

- **Многоплатформенность:** Совместим с Windows, macOS и Linux.
- **Экспорт в CSV:** Легко сохраняет полученные данные в файлах CSV для последующего анализа или обработки.
- **Поддержка множества источников данных:** Поддерживает широкий спектр источников, включая последовательные порты, MQTT, Bluetooth Low Energy (BLE) и сетевые сокеты (TCP/UDP).
- **Настраиваемая визуализация:** Позволяет определять и визуализировать данные с помощью различных виджетов, которые можно настроить в редакторе проектов в соответствии с конкретными потребностями пользователя.
- **Настраиваемый анализ кадров:** Предоставляет возможность модифицировать функцию JavaScript для интерпретации входящих кадров данных, что упрощает предварительную обработку сырых данных с датчиков и работу с бинарными форматами.
- **Публикация и прием через MQTT:** Отправляет и принимает данные через Интернет, позволяя визуализировать данные в реальном времени из любой точки мира.

## Установка

Вы можете загрузить и установить последнюю версию Serial Studio для вашей предпочитаемой платформы [здесь](https://github.com/Serial-Studio/Serial-Studio/releases/latest).

### Установка на Linux

Для пользователей GNU/Linux, после загрузки AppImage, убедитесь, что у него установлены правильные разрешения на выполнение перед запуском приложения:

```bash
chmod +x SerialStudio-2.1.0-Linux.AppImage
./SerialStudio-2.1.0-Linux.AppImage
```

Кроме того, вы можете интегрировать Serial Studio в свою систему, используя [AppImageLauncher](https://github.com/TheAssassin/AppImageLauncher/).

### Предварительно собранные пакеты для Linux

Для пользователей Arch Linux, вы можете установить Serial Studio через AUR:

```bash
aur fetch serial-studio-git
aur build
sudo pacman -S serial-studio-git
```

**Примечание:** Рецепт пакета AUR может быть устаревшим, поэтому обязательно проверяйте наличие обновлений.

## Разработка

### Требования

Для компиляции Serial Studio единственная необходимая зависимость — это [Qt](http://www.qt.io/download-open-source/). Десктопное приложение компилируется с **Qt 6.7.1**.

Если вы компилируете на GNU/Linux, вам также нужно установить `libgl1-mesa-dev`:

```bash
sudo apt install libgl1-mesa-dev
```

Вот список необходимых модулей Qt:

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

### Клонирование репозитория

Чтобы клонировать репозиторий с необходимыми подмодулями, выполните:

```bash
git clone https://github.com/Serial-Studio/Serial-Studio
cd Serial-Studio
```

### Компиляция приложения

После установки Qt вы можете скомпилировать проект, открыв файл **CMakeLists.txt** в своем предпочтительном IDE или используя командную строку:

```bash
mkdir build
cd build 
cmake ../ -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j 10
```

## Архитектура ПО

Ниже приведена упрощенная диаграмма, иллюстрирующая, как взаимодействуют различные модули Serial Studio. Для получения более детальной информации ознакомьтесь с полной документацией DOXYGEN [здесь](https://serial-studio.github.io/hackers/).

![Архитектура](/doc/architecture/architecture.png)

## Лицензия

Этот проект лицензирован под лицензией MIT. Для получения более подробной информации см. файл [LICENSE](LICENSE.md).

## Поддержка и пожертвования

Если вы находите Serial Studio полезным, рассмотрите возможность поддержки его разработки с помощью [пожертвования через PayPal](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE).

Или, если вы когда-нибудь окажетесь в [Канкуне, Мексика](https://ru.wikipedia.org/wiki/Канкун) и захотите пригласить меня на пиво лично, не стесняйтесь [отправить мне сообщение в Instagram](https://instagram.com/aspatru). Я буду рад познакомиться!