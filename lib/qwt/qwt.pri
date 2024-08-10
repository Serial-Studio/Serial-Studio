QWT_CONFIG += QwtPlot
QWT_CONFIG += QwtPolar
QWT_CONFIG += QwtWidgets

DEFINES += QWT_MOC_INCLUDE=1

include($$PWD/src/src.pri)
include($$PWD/classincludes/classincludes.pri)
