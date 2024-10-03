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

Serial Studio es una herramienta de visualización de datos multiplataforma y versátil, diseñada para ingenieros de sistemas embebidos, estudiantes, hackers y profesores. Permite a los usuarios visualizar, analizar y representar los datos de sus proyectos y dispositivos sin necesidad de desarrollar software de visualización específico para cada proyecto. Serial Studio ofrece una solución flexible que se adapta a una amplia gama de casos de uso, haciéndola ideal tanto para entornos educativos como profesionales.

La herramienta nació de mi experiencia en múltiples competencias basadas en CanSat, donde a menudo me encontraba desarrollando nuevo software para la estación terrena para cada proyecto. Con el tiempo, me di cuenta de que sería más eficiente y sostenible mantener un único programa que sea flexible, que permitiera a los usuarios definir cómo se procesan y muestran los datos entrantes.

Hoy en día, Serial Studio es una herramienta poderosa y adaptable, adecuada no solo para competencias de CanSat, sino para cualquier proyecto de adquisición y visualización de datos. Soporta la recuperación de datos desde una amplia gama de fuentes, incluyendo puertos seriales, MQTT, Bluetooth Low Energy (BLE), y sockets de red (TCP/UDP).

*Lee este documento en otros idiomas*: [Español](/doc/README_ES.md) [简体中文](/doc/README_ZH.md) [Deutsch](/doc/README_DE.md) [Русский](/doc/README_RU.md) [Français](/doc/README_FR.md)

![Uso del Software](/doc/screenshot.png)

## Características

- **Multiplataforma:** Compatible con Windows, macOS y Linux.
- **Exportación a CSV:** Guarda de manera sencilla los datos recibidos en archivos CSV para su posterior análisis o procesamiento.
- **Compatibilidad con múltiples fuentes de datos:** Soporta una amplia variedad de fuentes, incluyendo puertos seriales, MQTT, Bluetooth Low Energy (BLE) y sockets de red (TCP/UDP).
- **Visualización personalizable:** Permite definir y visualizar datos mediante diversos widgets, configurables a través del editor de proyectos para adaptarse a las necesidades específicas del usuario.
- **Análisis de tramas personalizable:** Ofrece la posibilidad de modificar una función en JavaScript para interpretar las tramas de datos entrantes, facilitando el preprocesamiento de datos crudos de sensores y el manejo de formatos binarios complejos.
- **Publicación y recepción mediante MQTT:** Envía y recibe datos a traves de la Internet, lo que habilita la visualización en tiempo real de información desde cualquier lugar del mundo.

## Instalación

Puedes descargar e instalar la última versión de Serial Studio para tu plataforma preferida desde [aquí](https://github.com/Serial-Studio/Serial-Studio/releases/latest).

### Instalación en Linux

Para usuarios de GNU/Linux, después de descargar el AppImage, asegúrate de que tenga los permisos de ejecución correctos antes de ejecutar la aplicación:

```bash
chmod +x SerialStudio-2.1.0-Linux.AppImage
./SerialStudio-2.1.0-Linux.AppImage
```

Alternativamente, puedes integrar Serial Studio en tu sistema utilizando [AppImageLauncher](https://github.com/TheAssassin/AppImageLauncher/).

### Paquetes Linux Precompilados

Para los usuarios de Arch Linux, puedes instalar Serial Studio usando el AUR:

```bash
aur fetch serial-studio-git
aur build
sudo pacman -S serial-studio-git
```

**Nota:** La receta del paquete AUR puede estar desactualizada, así que asegúrate de verificar actualizaciones.

## Desarrollo

### Requisitos

Para compilar Serial Studio, la única dependencia requerida es [Qt](http://www.qt.io/download-open-source/). La aplicación de escritorio se compila con **Qt 6.7.1**.

Si compilas en GNU/Linux, también necesitarás instalar `libgl1-mesa-dev`:

```bash
sudo apt install libgl1-mesa-dev
```

Aquí tienes la lista de módulos de Qt necesarios:

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

### Clonar el Repositorio

Para clonar el repositorio con los submódulos necesarios, ejecuta:

```bash
git clone https://github.com/Serial-Studio/Serial-Studio
cd Serial-Studio
```

### Compilar la Aplicación

Una vez que Qt esté instalado, puedes compilar el proyecto abriendo el archivo **CMakeLists.txt** en tu IDE preferido o usando la línea de comandos:

```bash
mkdir build
cd build 
cmake ../ -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j 10
```

## Arquitectura del Software

A continuación, se muestra un diagrama simplificado que ilustra cómo interactúan los diferentes módulos de Serial Studio. Para más información detallada, consulta la documentación completa de DOXYGEN [aquí](https://serial-studio.github.io/hackers/).

![Arquitectura](/doc/architecture/architecture.png)

## Licencia

Este proyecto está licenciado bajo la Licencia MIT. Para más detalles, consulta el archivo [LICENSE](LICENSE.md).

## Soporte y Donaciones

Si encuentras Serial Studio útil, considera apoyar su desarrollo mediante una [donación a través de PayPal](https://www.paypal.com/donate?hosted_button_id=XN68J47QJKYDE).

Alternativamente, si alguna vez te encuentras en [Cancún, México](https://es.wikipedia.org/wiki/Canc%C3%BAn) y quieres invitarme a tomar una cerveza en persona, no dudes en [enviarme un mensaje en Instagram](https://instagram.com/aspatru). Me encantaría conocerte!
