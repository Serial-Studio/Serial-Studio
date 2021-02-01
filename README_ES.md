<a href="#">
    <img width="192px" height="192px" src="doc/icon.png" align="right" />
</a>

# Serial Studio

[![Build Status](https://github.com/Serial-Studio/Serial-Studio/workflows/Build/badge.svg)](https://github.com/Serial-Studio/Serial-Studio/actions/)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/4b6f3ce14a684704980fea31d8c1632e)](https://www.codacy.com/gh/Serial-Studio/Serial-Studio/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Serial-Studio/Serial-Studio&amp;utm_campaign=Badge_Grade)
[![Github All Releases](https://img.shields.io/github/downloads/Serial-Studio/Serial-Studio/total.svg)](https://github.com/Serial-Studio/Serial-Studio/releases/)
[![Contributor Covenant](https://img.shields.io/badge/Contributor%20Covenant-v1.4%20adopted-ff69b4.svg)](CODE_OF_CONDUCT.md)

Serial Studio es un programa de visualización de datos de puertos serial multiplataforma y multipropósito. El objetivo de este proyecto es permitir a los desarrolladores y creadores de sistemas embebidos visualizar, presentar y analizar fácilmente los datos generados por sus proyectos y dispositivos, sin la necesidad de escribir software especializado para cada proyecto.

La necesidad de este proyecto surgió durante el desarrollo del software de estación terrena para varios concursos de CanSat. Simplemente no es sostenible desarrollar y mantener diferentes programas para cada competencia. La solución inteligente es tener un software de estación terrena común y permitir que cada CanSat defina cómo se presentan los datos al usuario final mediante un protocolo de comunicación extensible.

Además, este enfoque se puede extender a casi cualquier tipo de proyecto que implique algún tipo de adquisición y medición de datos. Si desea una explicación más detallada de por qué existe este proyecto y de qué se trata, consulte [esta publicación de blog](https://www.alex-spataru.com/blog/introducing-serial-studio) (en inglés).

**NOTA:** en el [wiki](https://github.com/Serial-Studio/Serial-Studio/wiki/Protocolo-de-Comunicación) se muestra más información acerca del protocolo de comunicación utilizado por Serial Studio.

*Lea esto en otros idiomas:* [English](README.md) [简体中文](README_ZH.md) [Deutsch](README_DE.md)

![Software usage](doc/app-usage.gif)

## Instrucciones de complicación

#### Requisitos

El único requisito para compilar la aplicación es tener [Qt](http://www.qt.io/download-open-source/) instalado en su sistema. La aplicación se compilará con Qt 5.15 o superior. También necesitará tener instalados los siguientes módulos de Qt:

- Qt Charts

En sistemas GNU/Linux, también necesitará instalar `libgl1-mesa-dev` para compilar la aplicación.

Lista completa de módulos Qt usados:

- Qt SQL
- Qt Quick
- Qt Widgets
- Qt Charts
- Qt Serial Port
- Qt Quick Controls 2
- Qt Graphical Effects

#### Clonado

Este repositorio hace uso de [`git submodule`] (https://git-scm.com/book/en/v2/Git-Tools-Submodules). Para clonarlo, ejecute estos comandos en su Terminal:

	git clone https://github.com/Serial-Studio/Serial-Studio
	cd Serial-Studio
	git submodule init
	git submodule update
	
Alternativamente, simplemente ejecute:

	git clone --recursive https://github.com/Serial-Studio/Serial-Studio
    
#### Compilación

Una vez que haya instalado Qt, abra *Serial-Studio.pro* en Qt Creator y haga clic en el botón de "Ejecutar".

Alternativamente, también puede utilizar los siguientes comandos:

	qmake
	make -j4

## Licencia

Este proyecto se publica bajo la licencia MIT, para obtener más información, consulte el archivo [LICENSE](LICENSE.md).

