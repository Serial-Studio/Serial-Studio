QT       -= gui
TARGET    = gtest
TEMPLATE  = lib
DEFINES  += GTEST_LIBRARY

SOURCES += googletest/googletest/src/gtest-all.cc
SOURCES += googletest/googlemock/src/gmock-all.cc

INCLUDEPATH += \
	googletest/googletest/include \
	googletest/googletest \
	googletest/googlemock/include \
	googletest/googlemock
