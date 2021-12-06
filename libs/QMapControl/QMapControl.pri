QT += gui
QT += network
QT += widgets

VERSION = 0.9.7.9

INCLUDEPATH += $$PWD/src
DEFINES += QMAPCONTROL_PROJECT_INCLUDE_SRC

HEADERS += $$PWD/src/curve.h \
           $$PWD/src/geometry.h \
           $$PWD/src/imagemanager.h \
           $$PWD/src/layer.h \
           $$PWD/src/layermanager.h \
           $$PWD/src/linestring.h \
           $$PWD/src/mapadapter.h \
           $$PWD/src/mapcontrol.h \
           $$PWD/src/mapnetwork.h \
           $$PWD/src/point.h \
           $$PWD/src/tilemapadapter.h \
           $$PWD/src/wmsmapadapter.h \
           $$PWD/src/circlepoint.h \
           $$PWD/src/imagepoint.h \
           $$PWD/src/gps_position.h \
           $$PWD/src/osmmapadapter.h \
           $$PWD/src/maplayer.h \
           $$PWD/src/geometrylayer.h \
           $$PWD/src/googlemapadapter.h \
           $$PWD/src/openaerialmapadapter.h \
           $$PWD/src/fixedimageoverlay.h \
           $$PWD/src/emptymapadapter.h \
           $$PWD/src/arrowpoint.h \
           $$PWD/src/invisiblepoint.h \
           $$PWD/src/qmapcontrol_global.h \
           $$PWD/src/bingapimapadapter.h \
           $$PWD/src/googleapimapadapter.h

SOURCES += $$PWD/src/curve.cpp \
           $$PWD/src/geometry.cpp \
           $$PWD/src/imagemanager.cpp \
           $$PWD/src/layer.cpp \
           $$PWD/src/layermanager.cpp \
           $$PWD/src/linestring.cpp \
           $$PWD/src/mapadapter.cpp \
           $$PWD/src/mapcontrol.cpp \
           $$PWD/src/mapnetwork.cpp \
           $$PWD/src/point.cpp \
           $$PWD/src/tilemapadapter.cpp \
           $$PWD/src/wmsmapadapter.cpp \
           $$PWD/src/circlepoint.cpp \
           $$PWD/src/imagepoint.cpp \
           $$PWD/src/gps_position.cpp \
           $$PWD/src/osmmapadapter.cpp \
           $$PWD/src/maplayer.cpp \
           $$PWD/src/geometrylayer.cpp \
           $$PWD/src/googlemapadapter.cpp \
           $$PWD/src/openaerialmapadapter.cpp \
           $$PWD/src/fixedimageoverlay.cpp \
           $$PWD/src/arrowpoint.cpp \
           $$PWD/src/invisiblepoint.cpp \
           $$PWD/src/emptymapadapter.cpp \
           $$PWD/src/bingapimapadapter.cpp \
           $$PWD/src/googleapimapadapter.cpp
