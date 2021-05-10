<a href="#">
    <img width="192px" height="192px" src="doc/icon.png" align="right" />
</a>

# Serial Studio

[![Build Status](https://github.com/Serial-Studio/Serial-Studio/workflows/Build/badge.svg)](https://github.com/Serial-Studio/Serial-Studio/actions/)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/4b6f3ce14a684704980fea31d8c1632e)](https://www.codacy.com/gh/Serial-Studio/Serial-Studio/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Serial-Studio/Serial-Studio&amp;utm_campaign=Badge_Grade)
[![Github All Releases](https://img.shields.io/github/downloads/Serial-Studio/Serial-Studio/total.svg)](https://github.com/Serial-Studio/Serial-Studio/releases/)
[![GitHub Latest Release](https://img.shields.io/github/downloads/Serial-Studio/Serial-Studio/latest/total.svg)](https://github.com/Serial-Studio/Serial-Studio/releases/latest)
[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-v1.4%20adopted-ff69b4.svg)](CODE_OF_CONDUCT.md)

Serial Sutdio est un programme de visualisation de données séries multiplateforme et multifonction. Le but de ce projet est de permettre aux développeurs de systèmes embarqués et aux bidouilleurs de simplement visualiser, présenter et analyser les données générées par leurs projets et leurs dispositifs, sans avoir à développer un logiciel spécialisé pour chaque projet.

Le besoin pour ce projet a émergé pendant le développement de programmes de station au sol (GSS) pour plusieurs compétitions basées sur CanSat auxquelles j'ai participées. C'est simplement non durable de développer et maintenir différents programmes GSS pour chaque compétition et chaque projet. La solution intelligente est d'avoir un programme GSS commun qui laisse à chaque CanSat le choix de définir comment les données sont présentées à l'utilisateur en utilisant un protocole de communication extensible.

En plus, cette approche peut être adaptée à n'importe quel type de projet qui requiert une sorte d'acquisition et de mesure. Pour une explication en profondeur de la raison d'être de ce projet, rendez vous sur [ce post](https://www.alex-spataru.com/blog/introducing-serial-studio).

Serial Studio a commencé par recevoir des données depuis un port série matériel, mais peut désormais recevoir des données depuis MQTT et depuis des websockets (TCP/UDP).

**NOTE :** Les informations concernant le protocole de communication sont détaillées dans le [wiki](https://github.com/Serial-Studio/Serial-Studio/wiki/Communication-Protocol).

*Lisez ceci dans d'autres langues*: [English](README.md) [Español](README_ES.md) [简体中文](README_ZH.md) [Deutsch](README_DE.md)

![Usage du logiciel](doc/app-usage.gif)

## Instructions de compilation

#### Prérequis

Le seul prérequis pour compiler l'application est d'avoir [Qt](http://www.qt.io/download-open-source/) installé sur votre système. L'application de bureau se compile avec Qt 5.15 ou plus récent. Vous allez aussi avoir besoin d'installer les modules Qt non-LGPL suivants :

- Qt Charts

Sur les systèmes GNU/Linux, vous devrez installer `libgl1-mesa-dev`pour compiler l'application.

Liste complète des modules Qt utilisés :


- Qt SQL
- Qt Quick
- Qt Widgets
- Qt Charts
- Qt Serial Port
- Qt Quick Controls
- Qt Quick Controls 2
- Qt Graphical Effects

#### Clonage du dépot

Ce dépot utilise [`git submodule`](https://git-scm.com/book/en/v2/Git-Tools-Submodules). Pour cloner le dépot, exécutez les commandes suivantes dans votre terminal :

	git clone https://github.com/Serial-Studio/Serial-Studio
	cd Serial-Studio
	git submodule init
	git submodule update

Un autre moyen est de simplement exécuter la commande suivante :

	git clone --recursive https://github.com/Serial-Studio/Serial-Studio

#### Compilation de l'application

Lorsque vous avez installé Qt, ouvrez *Serial-Studio.pro* dans Qt Creator et cliquez sur le bouton "Exécuter".

Une autre manière de faire est d'exécuter les commandes suivantes :

	qmake
	make -j4

### Paquets précompilés

Les utilisateurs d'Arch Linux peuvent installer [serial-studio-git](https://aur.archlinux.org/packages/serial-studio-git/) depuis aur, c.à.d avec [aurutils](https://aur.archlinux.org/packages/aurutils/) :

```bash
aur fetch serial-studio-git
aur build
sudo pacman -S serial-studio-git
```

## Licence

Ce projet est publié sous la licence MIT, pour plus d'informations, voir le fichier [LICENSE](LICENSE.md).



