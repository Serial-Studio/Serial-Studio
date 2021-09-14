QT += widgets

HEADERS += mainwindow.h
SOURCES += mainwindow.cpp main.cpp
RESOURCES += mactouchbar.qrc

INCLUDEPATH = ../../src
LIBS += -F../../lib -framework KDMacTouchBar
QMAKE_LFLAGS = '-Wl,-rpath,\'$$OUT_PWD/../../lib\',-rpath,\'$$INSTALL_PREFIX/lib\''

target.path = "$${INSTALL_PREFIX}/examples/mactouchbar"
INSTALLS += target
