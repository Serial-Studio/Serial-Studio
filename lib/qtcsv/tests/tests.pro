QT += testlib
QT -= gui

TARGET = qtcsv_tests
CONFIG += console testcase
CONFIG -= app_bundle

TEMPLATE = app

!msvc {
    # flags for gcc-like compiler
    CONFIG += warn_on
    QMAKE_CXXFLAGS_WARN_ON += -Werror -Wformat=2 -Wuninitialized -Winit-self \
            -Wmissing-include-dirs -Wswitch-enum -Wundef -Wpointer-arith \
            -Wdisabled-optimization -Wcast-align -Wcast-qual
}

# set where linker could find qtcsv library. By default we expect
# that library is located in the same directory as the qtcsv_tests binary.
QTCSV_LOCATION = $$OUT_PWD
LIBS += -L$$QTCSV_LOCATION -lqtcsv

# Uncomment this settings if you want to manually set destination directory for
# compiled binary
#CONFIG(release, debug|release): DESTDIR = $$PWD/../
#CONFIG(debug, debug|release): DESTDIR = $$PWD/../

INCLUDEPATH += $$PWD/../include

SOURCES += \
    tst_testmain.cpp \
    teststringdata.cpp \
    testvariantdata.cpp \
    testreader.cpp \
    testwriter.cpp

HEADERS += \
    teststringdata.h \
    testvariantdata.h \
    testreader.h \
    testwriter.h

DISTFILES += \
    CMakeLists.txt

!equals(PWD, $$OUT_PWD){
    # Copy 'data' folder with test files to the destination directory
    win32 {
        COPY_FROM_PATH=$$shell_path($$PWD/data)
        COPY_TO_PATH=$$shell_path($$OUT_PWD/data)

        # on windows we should create "/data" directory before coping of files
        createdir.commands = $(MKDIR) $$COPY_TO_PATH
        first.depends = $(first) createdir
    }
    else {
        COPY_FROM_PATH=$$PWD/data
        COPY_TO_PATH=$$OUT_PWD
    }

    copydata.commands = $(COPY_DIR) $$COPY_FROM_PATH $$COPY_TO_PATH
    first.depends = $(first) copydata

    export(first.depends)
    win32: export(createdir.commands)
    export(copydata.commands)

    win32: QMAKE_EXTRA_TARGETS += first createdir copydata
    else: QMAKE_EXTRA_TARGETS += first copydata
}

message(=== Configuration of qtcsv_tests ===)
message(Qt version: $$[QT_VERSION])
message(qtcsv_tests binary will be created in folder: $$OUT_PWD)
message(Expected location of qtcsv library: $$QTCSV_LOCATION)
message(Expected location of \"data\" folder with test files: $$OUT_PWD)
