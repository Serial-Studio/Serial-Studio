#-------------------------------------------------
#
# Project created by QtCreator 2013-01-03T08:11:25
#
#-------------------------------------------------

QT += core gui widgets

# for the time being:
greaterThan(QT_MAJOR_VERSION,5): QT += core5compat


TARGET = edbee
TEMPLATE = lib
CONFIG += staticlib

# Define EDBEE_BEGUG to enable memory debugging
DEFINES += EDBEE_DEBUG

# DEFINE 'EDBEE_SANITIZE' to enable santitize bounds checks
EDBEE_SANITIZE = $$(EDBEE_SANITIZE)
!isEmpty( EDBEE_SANITIZE ) {
  warning('*** SANITIZE ENABLED! edbee-lib ***')
  QMAKE_CXXFLAGS+=-fsanitize=address -fsanitize=bounds -fsanitize-undefined-trap-on-error
  QMAKE_LFLAGS+=-fsanitize=address -fsanitize=bounds -fsanitize-undefined-trap-on-error
}

# This seems to be required for Windows
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
DEFINES += QT_NODLL

include(edbee-lib.pri)
