QT = core
TARGET = qtcsv
TEMPLATE = lib
VERSION = 1.5.0
win32:TARGET_EXT = .dll

# Uncomment this setting if you want to build static library
#CONFIG += staticlib

!msvc {
    # flags for gcc-like compiler
    CONFIG += warn_on
    QMAKE_CXXFLAGS_WARN_ON += -Werror -Wformat=2 -Wuninitialized -Winit-self \
            -Wswitch-enum -Wundef -Wpointer-arith \
            -Wdisabled-optimization -Wcast-align -Wcast-qual
}

CONFIG(staticlib): DEFINES += QTCSV_STATIC_LIB
DEFINES += QTCSV_LIBRARY

include(qtcsv.pri)

# Uncomment this settings if you want to manually set destination directory for
# compiled library
#CONFIG(release, debug|release): DESTDIR = $$PWD
#CONFIG(debug, debug|release): DESTDIR = $$PWD

DISTFILES += \
    CMakeLists.txt

OTHER_FILES += \
    appveyor.yml \
    .travis.yml

message(=== Configuration of qtcsv ===)
message(Qt version: $$[QT_VERSION])
message(Library version: $$VERSION)
message(Library files will be created in folder: $$OUT_PWD)

unix {
    # settings for command "make install"
    copy_lib_headers.path = /usr/local/include/qtcsv/
    copy_lib_headers.files = $$PWD/include/qtcsv/*.h

    target.path = /usr/local/lib
    INSTALLS += target copy_lib_headers

    message(--- Settings for command \"make install\")
    message(Library files will be copied to folder: $$target.path)
    message(Library headers will be copied to folder: $$copy_lib_headers.path)
}
