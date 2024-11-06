# Serial Studio

<a href="#">
    <img width="192px" height="192px" src="/doc/icon.svg" align="right" />
</a>

[![Dernier commit sur Github](https://img.shields.io/github/last-commit/Serial-Studio/Serial-Studio?style=for-the-badge&logo=github)](https://github.com/Serial-Studio/Serial-Studio/commits/master)
[![Contributeurs GitHub](https://img.shields.io/github/contributors/Serial-Studio/Serial-Studio?style=for-the-badge&logo=github)](https://github.com/Serial-Studio/Serial-Studio/graphs/contributors)
[![PR's Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen?style=for-the-badge)](https://github.com/Serial-Studio/Serial-Studio/pull/new)
[![Github All Releases](https://img.shields.io/github/downloads/Serial-Studio/Serial-Studio/total.svg?style=for-the-badge&logo=github)](https://github.com/Serial-Studio/Serial-Studio/releases/)
[![Instagram](https://img.shields.io/badge/Instagram-E4405F?style=for-the-badge&logo=instagram&logoColor=white)](https://instagram.com/serialstudio.app)
[![Faire un don](https://img.shields.io/badge/PayPal-00457C?style=for-the-badge&logo=paypal&logoColor=white)](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE)

Serial Studio est un outil de visualisation de données multiplateforme et polyvalent conçu pour les ingénieurs en électronique, les étudiants, les hackers et les enseignants. Il permet aux utilisateurs de visualiser, analyser et représenter les données de leurs projets et dispositifs sans avoir besoin d'un logiciel de visualisation spécifique au projet. Serial Studio offre une solution flexible qui s'adapte à une grande variété de cas d'utilisation, ce qui le rend idéal pour les environnements éducatifs et professionnels.

L'outil est né de mon expérience dans plusieurs compétitions basées sur CanSat, où je me retrouvais souvent à développer un nouveau logiciel de station au sol pour chaque projet. Au fil du temps, j'ai réalisé qu'il serait plus efficace et durable de maintenir un seul logiciel de station au sol flexible, permettant aux utilisateurs de définir comment les données entrantes sont traitées et affichées.

Aujourd'hui, Serial Studio est un outil puissant et adaptable, adapté non seulement aux compétitions CanSat, mais aussi à tout projet d'acquisition et de visualisation de données. Il prend en charge la récupération de données à partir d'une grande variété de sources, y compris les ports série matériel et logiciel, MQTT, Bluetooth Low Energy (BLE) et les sockets réseau (TCP/UDP).

*Lire ce document dans d'autres langues* : [Español](/doc/README_ES.md) [简体中文](/doc/README_ZH.md) [Deutsch](/doc/README_DE.md) [Русский](/doc/README_RU.md) [Français](/doc/README_FR.md)

![Utilisation du logiciel](/doc/screenshot.png)

## Fonctionnalités

- **Multiplateforme :** Compatible avec Windows, macOS et Linux.
- **Exportation CSV :** Enregistre facilement les données reçues dans des fichiers CSV pour une analyse ou un traitement ultérieur.
- **Support de multiples sources de données :** Gère une grande variété de sources, y compris les ports série, MQTT, Bluetooth Low Energy (BLE) et les sockets réseau (TCP/UDP).
- **Visualisation personnalisable :** Permet aux utilisateurs de définir et d'afficher les données à l'aide de différents widgets, configurables via l'éditeur de projet pour répondre aux besoins spécifiques.
- **Analyse de trame personnalisable :** Fournit la possibilité de modifier une fonction JavaScript pour interpréter les trames de données entrantes, permettant le prétraitement des données brutes des capteurs et la gestion des formats binaires complexes.
- **Publication et réception MQTT :** Envoie et reçoit des données via Internet, permettant la visualisation des données en temps réel de n'importe où dans le monde.

## Installation

Vous pouvez télécharger et installer la dernière version de Serial Studio pour votre plateforme préférée depuis [ici](https://github.com/Serial-Studio/Serial-Studio/releases/latest).


### Installation sur Linux

Pour les utilisateurs de GNU/Linux, il existe plusieurs façons d’installer et d’exécuter l’application :

#### 1. AppImage

Téléchargez le fichier AppImage et assurez-vous qu’il dispose des permissions d’exécution avant de le lancer :

```
chmod +x SerialStudio-3.0.5-Linux-x86_64.AppImage
./SerialStudio-3.0.5-Linux-x86_64.AppImage
```

*Remarque :* Vous pourriez avoir besoin d’installer libfuse2 pour que l’AppImage fonctionne. Sur les systèmes basés sur Debian/Ubuntu, vous pouvez l’installer avec :

```
sudo apt update
sudo apt install libfuse2
```

Vous pouvez intégrer l’AppImage de Serial Studio plus facilement dans votre système en utilisant [AppImageLauncher](https://github.com/TheAssassin/AppImageLauncher/)..

#### 2. Paquets DEB/RPM (expérimental)

Vous pouvez également installer Serial Studio avec des paquets DEB ou RPM, qui sont actuellement en phase expérimentale. Merci de signaler tout problème rencontré.

Pour les distributions basées sur Debian (par ex., Debian, Ubuntu, Linux Mint, etc.) :

```
sudo dpkg -i SerialStudio-3.0.5-Linux-x86_64.deb
```

Pour les distributions basées sur RPM (par ex., Fedora, RHEL, OpenSUSE, etc.) :

```
sudo rpm -i SerialStudio-3.0.5-Linux-x86_64.rpm
```

## Développement

### Prérequis

Pour compiler Serial Studio, la seule dépendance requise est [Qt](http://www.qt.io/download-open-source/). L'application de bureau est compilée avec **Qt 6.8.0**.

Si vous compilez sous GNU/Linux, vous devrez également installer `libgl1-mesa-dev` :

```bash
sudo apt install libgl1-mesa-dev
```

Voici la liste des modules Qt requis:

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

### Clonage du dépôt

Pour cloner le dépôt avec les sous-modules nécessaires, exécutez :

```bash
git clone https://github.com/Serial-Studio/Serial-Studio
cd Serial-Studio
```

### Compilation de l'application

Une fois Qt installé, vous pouvez compiler le projet en ouvrant le fichier **CMakeLists.txt** dans votre IDE préféré ou en utilisant la ligne de commande :

```bash
mkdir build
cd build 
cmake ../ -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j 10
```

## Architecture du logiciel

Voici un schéma simplifié illustrant comment les différents modules de Serial Studio interagissent. Pour plus d'informations détaillées, consultez la documentation complète DOXYGEN [ici](https://serial-studio.github.io/hackers/).

![Architecture](/doc/architecture/architecture.png)

## Licence

Ce projet est sous licence MIT. Pour plus de détails, consultez le fichier [LICENSE](/LICENSE.md).

## Support & Contribution

Si vous trouvez Serial Studio utile, pensez à soutenir son développement en [contribuant via PayPal](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE).

Sinon, si vous passez un jour à [Cancún, au Mexique](https://en.wikipedia.org/wiki/Cancun) et que vous souhaitez m'offrir un verre en personne, n'hésitez pas à [m'envoyer un message sur Instagram](https://instagram.com/aspatru). J'adorerais vous rencontrer !
