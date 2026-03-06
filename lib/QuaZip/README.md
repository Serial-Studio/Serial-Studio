# Quazip

[![CI](https://github.com/stachenov/quazip/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/stachenov/quazip/actions/workflows/ci.yml)
[![Conan Center](https://shields.io/conan/v/quazip)](https://conan.io/center/recipes/quazip)

QuaZip is the C++ wrapper for Gilles Vollant's ZIP/UNZIP package
(AKA Minizip) using Qt library.

If you need to write files to a ZIP archive or read files from one
using QIODevice API, QuaZip is exactly the kind of tool you need.

See [the documentation](https://stachenov.github.io/quazip/) for details.

Want to report a bug or ask for a feature? Open an [issue](https://github.com/stachenov/quazip/issues).

Want to fix a bug or implement a new feature? See [CONTRIBUTING.md](CONTRIBUTING.md).

You're a package maintainer and want to update to QuaZip 1.0? Read [the migration guide](https://github.com/stachenov/quazip/blob/master/QuaZip-1.x-migration.md).

Copyright notice:

Copyright (C) 2005-2020 Sergey A. Tachenov and contributors

Distributed under LGPL, full details in the COPYING file.

Original ZIP package is copyrighted by Gilles Vollant, see
quazip/(un)zip.h files for details, but basically it's the zlib license.

> [!IMPORTANT]
> This project is looking for additional maintainers and code reviewers. While the library is mostly feature complete, there are a couple of big ticket issues that need work.
> Code review is welcome for all pull requests. If you are well versed in Qt and C/C++ please start helping around and check [#185](https://github.com/stachenov/quazip/issues/185). 

# Build

## Dependencies
You need at least the following dependencies:
- zlib
- Qt6, Qt5 or Qt4 (searched in that order)

## Linux
```
sudo apt-get install zlib1g-dev libbz2-dev
cmake -B build
cmake --build build
```

## Windows

When using Qt online installer on Windows with MSVC, make sure to select the box for `MSVC 20XY 64-bit` and under additional libraries, select `Qt 5 Compatibility Module`.
Finally, add `C:\Qt\6.8.2\msvc20XY_64` to your PATH.

If you don't use a package manager you will have to add library and include directories to your PATH or specify them with `CMAKE_PREFIX_PATH`.
Qt is not installed as a dependency of either vcpkg or conan.

### x64
Using vcpkg
```
cmake --preset vcpkg
cmake --build build --config Release
```

Using conan v2
```
conan install . -of build -s build_type=Release -o *:shared=False --build=missing
cmake --preset conan
cmake --build build --config Release
```

### x86
Only Qt5 is tested on x86.

Using vcpkg
```
cmake --preset vcpkg_x86
cmake --build build --config Release
```

Using conan v2
```
conan install . -of build -s build_type=Release -s:h arch=x86 -o *:shared=False --build=missing
cmake --preset conan_x86
cmake --build build --config Release
```

## Additional build options
If you built Qt from source and installed it, you might need to tell CMake where to find it, for example: `-DCMAKE_PREFIX_PATH="/usr/local/Qt-6.8.2"`.  
Alternatively, if you did not install the source build it might look something like: `-DCMAKE_PREFIX_PATH="/home/you/qt-everywhere-src-6.8.2/qtbase/lib/cmake"`.  
Replace `qtbase` if you used a custom prefix at `configure` step.

Qt installed through Linux distribution packages or official Qt online installer should be detected automatically.

CMake is used to configure and build the project. A typical configure, build, install and clean is shown below.

```
cmake -B build -DQUAZIP_QT_MAJOR_VERSION=6 -DBUILD_SHARED_LIBS=ON -DQUAZIP_ENABLE_TESTS=ON
cmake --build build --config Release
sudo cmake --install build
cd build
ctest --verbose -C Release
cmake --build . --target clean
```

CMake options

| Option                   | Description                                                                                                         | Default |
|--------------------------|---------------------------------------------------------------------------------------------------------------------|---------|
| `QUAZIP_QT_MAJOR_VERSION`| Specifies which major Qt version should be searched for (6, 5 or 4). By default it tries to find the most recent.   |         |
| `BUILD_SHARED_LIBS`      | Build QuaZip as a shared library                                                                                    | `ON`    |
| `QUAZIP_INSTALL`         | Enable installation                                                                                                 | `ON`    |
| `QUAZIP_USE_QT_ZLIB`     | Use Qt's bundled zlib instead of system zlib (**not recommended**). Qt must be built with `-qt-zlib` and `-static`. Incompatible with `BUILD_SHARED_LIBS=ON`. | `OFF`   |
| `QUAZIP_ENABLE_TESTS`    | Build QuaZip tests                                                                                                  | `OFF`   |
| `QUAZIP_BZIP2`           | Enable BZIP2 compression                                                                                            | `ON`    |
| `QUAZIP_BZIP2_STDIO`     | Output BZIP2 errors to stdio when BZIP2 compression is enabled                                                      | `ON`    |

