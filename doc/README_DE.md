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

Serial Studio ist ein plattformübergreifendes und vielseitiges Datenvisualisierungstool, das für Ingenieure eingebetteter Systeme, Studenten, Hacker und Lehrer entwickelt wurde. Es ermöglicht Nutzern, die Daten ihrer Projekte und Geräte zu visualisieren, zu analysieren und darzustellen, ohne für jedes Projekt eine spezifische Visualisierungssoftware entwickeln zu müssen. Serial Studio bietet eine flexible Lösung, die sich an eine Vielzahl von Anwendungsfällen anpasst und sowohl für Bildungs- als auch für professionelle Umgebungen ideal ist.

Das Tool entstand aus meinen Erfahrungen bei mehreren CanSat-Wettbewerben, bei denen ich oft neue Bodenstationssoftware für jedes Projekt entwickeln musste. Mit der Zeit stellte ich fest, dass es effizienter und nachhaltiger wäre, ein einziges, flexibles Programm zu pflegen, das es Nutzern ermöglicht, zu definieren, wie eingehende Daten verarbeitet und angezeigt werden.

Heute ist Serial Studio ein leistungsstarkes und anpassungsfähiges Tool, das nicht nur für CanSat-Wettbewerbe, sondern auch für jedes Projekt zur Datenerfassung und -visualisierung geeignet ist. Es unterstützt die Datenabfrage aus einer Vielzahl von Quellen, darunter serielle Schnittstellen, MQTT, Bluetooth Low Energy (BLE) und Netzwerksockets (TCP/UDP).

*Lies dieses Dokument in anderen Sprachen*: [Español](/doc/README_ES.md) [简体中文](/doc/README_ZH.md) [Deutsch](/doc/README_DE.md) [Русский](/doc/README_RU.md) [Français](/doc/README_FR.md)

![Software-Benutzung](/doc/screenshot.png)

## Funktionen

- **Plattformübergreifend:** Kompatibel mit Windows, macOS und Linux.
- **CSV-Export:** Speichert empfangene Daten mühelos in CSV-Dateien zur späteren Analyse oder Weiterverarbeitung.
- **Unterstützung für mehrere Datenquellen:** Unterstützt eine Vielzahl von Quellen, darunter serielle Schnittstellen, MQTT, Bluetooth Low Energy (BLE) und Netzwerksockets (TCP/UDP).
- **Anpassbare Visualisierung:** Ermöglicht das Definieren und Anzeigen von Daten mit verschiedenen Widgets, die über den Projekt-Editor an spezifische Benutzeranforderungen angepasst werden können.
- **Anpassbare Frame-Analyse:** Bietet die Möglichkeit, eine JavaScript-Funktion zu modifizieren, um eingehende Datenrahmen zu interpretieren, was die Vorverarbeitung von Rohsensordaten und die Handhabung komplexer Binärformate erleichtert.
- **MQTT-Publishing und -Empfang:** Sendet und empfängt Daten über das Internet und ermöglicht so die Echtzeit-Visualisierung von Daten von überall auf der Welt.

## Installation

Du kannst die neueste Version von Serial Studio für deine bevorzugte Plattform [hier](https://github.com/Serial-Studio/Serial-Studio/releases/latest) herunterladen und installieren.

### Linux-Installation

Für GNU/Linux-Benutzer gibt es mehrere Möglichkeiten, die Anwendung zu installieren und auszuführen:

#### 1. AppImage

Laden Sie die AppImage-Datei herunter und stellen Sie sicher, dass sie über die korrekten Ausführungsrechte verfügt, bevor Sie sie starten:

```bash
chmod +x SerialStudio-3.0.5-Linux-x86_64.AppImage
./SerialStudio-3.0.5-Linux-x86_64.AppImage
```

*Hinweis:* Möglicherweise müssen Sie libfuse2 installieren, damit das AppImage funktioniert. Auf Debian-/Ubuntu-basierten Systemen können Sie dies folgendermaßen tun:

```bash
sudo apt update
sudo apt install libfuse2
```

Sie können das AppImage von Serial Studio einfacher in Ihr System integrieren, indem Sie [AppImageLauncher](https://github.com/TheAssassin/AppImageLauncher/) verwenden.

#### 2. DEB/RPM-Pakete (experimentell)

Sie können Serial Studio auch mit DEB- oder RPM-Paketen installieren, die sich derzeit im experimentellen Stadium befinden. Bitte melden Sie eventuelle Probleme.

Für Debian-basierte Distributionen (z. B. Debian, Ubuntu, Linux Mint usw.):

```bash
sudo dpkg -i SerialStudio-3.0.5-Linux-x86_64.deb
```

Für RPM-basierte Distributionen (z. B. Fedora, RHEL, OpenSUSE usw.):

```bash
sudo rpm -i SerialStudio-3.0.5-Linux-x86_64.rpm
```

## Entwicklung

### Anforderungen

Zum Kompilieren von Serial Studio ist die einzige erforderliche Abhängigkeit [Qt](http://www.qt.io/download-open-source/). Die Desktop-Anwendung wird mit **Qt 6.8.0** kompiliert.

Wenn du unter GNU/Linux kompilierst, musst du auch `libgl1-mesa-dev` installieren:

```bash
sudo apt install libgl1-mesa-dev
```

Hier ist eine Liste der benötigten Qt-Module:

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

### Repository klonen

Um das Repository mit den erforderlichen Submodulen zu klonen, führe aus:

```bash
git clone https://github.com/Serial-Studio/Serial-Studio
cd Serial-Studio
```

### Anwendung kompilieren

Nachdem Qt installiert ist, kannst du das Projekt kompilieren, indem du die **CMakeLists.txt**-Datei in deiner bevorzugten IDE öffnest oder die Befehlszeile verwendest:

```bash
mkdir build
cd build 
cmake ../ -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j 10
```

## Software-Architektur

Im Folgenden siehst du ein vereinfachtes Diagramm, das zeigt, wie die verschiedenen Serial Studio-Module interagieren. Für detailliertere Informationen, siehe die vollständige DOXYGEN-Dokumentation [hier](https://serial-studio.github.io/hackers/).

![Architektur](/doc/architecture/architecture.png)

## Lizenz

Dieses Projekt ist unter der MIT-Lizenz lizenziert. Weitere Details findest du in der Datei [LICENSE](/LICENSE.md).

## Unterstützung und Spenden

Wenn du Serial Studio nützlich findest, erwäge bitte, die Entwicklung durch eine [Spende via PayPal](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE) zu unterstützen.

Alternativ, wenn du jemals in [Cancún, Mexiko](https://de.wikipedia.org/wiki/Canc%C3%BAn) bist und mich auf ein Bier persönlich einladen möchtest, zögere nicht, mir [eine Nachricht auf Instagram zu senden](https://instagram.com/aspatru). Ich würde mich freuen, dich kennenzulernen!
