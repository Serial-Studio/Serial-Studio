CONFIG += silent
CONFIG -= depend_includepath

QWT_CONFIG += QwtPlot
QWT_CONFIG += QwtPolar
QWT_CONFIG += QwtWidgets

c++11 {
    CONFIG += strict_c++
}

sanitize {
    CONFIG += sanitizer
    CONFIG += sanitize_address
    CONFIG *= sanitize_undefined
}

QMAKE_CXXFLAGS *= -fno-math-errno
QMAKE_CXXFLAGS *= -funsafe-math-optimizations

DEFINES += QWT_MOC_INCLUDE=1

include($$PWD/src/src.pri)
include($$PWD/classincludes/classincludes.pri)
