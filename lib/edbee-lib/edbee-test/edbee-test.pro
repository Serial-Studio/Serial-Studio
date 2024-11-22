
QT  += core gui
QT  -= sql
QT  += widgets
greaterThan(QT_MAJOR_VERSION,5): QT += core5compat

TARGET = edbee-test
TEMPLATE = app

# DEFINE 'EDBEE_SANITIZE' to enable santitize bounds checks
EDBEE_SANITIZE = $$(EDBEE_SANITIZE)
!isEmpty( EDBEE_SANITIZE ) {
  warning('*** SANITIZE ENABLED! edbee-test ***')
  QMAKE_CXXFLAGS+=-fsanitize=address -fsanitize=bounds -fsanitize-undefined-trap-on-error
  QMAKE_LFLAGS+=-fsanitize=address -fsanitize=bounds -fsanitize-undefined-trap-on-error
}

# This seems to be required for Windows
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
DEFINES += QT_NODLL


# The test sources
SOURCES += \
	edbee/commands/replaceselectioncommandtest.cpp \
	edbee/models/textrangetest.cpp \
    edbee/models/textdocumenttest.cpp \
    edbee/models/textbuffertest.cpp \
	edbee/models/textlinedatatest.cpp \
	edbee/util/gapvectortest.cpp \
	edbee/util/lineoffsetvectortest.cpp \
	main.cpp \
    edbee/util/lineendingtest.cpp \
    edbee/textdocumentserializertest.cpp \
    edbee/io/tmlanguageparsertest.cpp \
    edbee/util/regexptest.cpp \
    edbee/models/textdocumentscopestest.cpp \
    edbee/models/textundostacktest.cpp \
    edbee/util/cascadingqvariantmaptest.cpp \
    edbee/models/textsearchertest.cpp \
    edbee/commands/duplicatecommandtest.cpp \
    edbee/commands/newlinecommandtest.cpp \
    edbee/util/utiltest.cpp \
    edbee/lexers/grammartextlexertest.cpp \
    edbee/commands/removecommandtest.cpp \
    edbee/models/changes/linedatalistchangetest.cpp \
    edbee/models/changes/textchangetest.cpp \
    edbee/models/changes/mergablechangegrouptest.cpp \
    edbee/util/rangesetlineiteratortest.cpp \
    edbee/models/dynamicvariablestest.cpp \
    edbee/util/rangelineiteratortest.cpp \
    edbee/views/textthememanagertest.cpp

HEADERS += \
	edbee/commands/replaceselectioncommandtest.h \
	edbee/models/textrangetest.h \
    edbee/models/textdocumenttest.h \
    edbee/models/textbuffertest.h \
	edbee/models/textlinedatatest.h \
	edbee/util/gapvectortest.h \
	edbee/util/lineoffsetvectortest.h \
    edbee/util/lineendingtest.h \
    edbee/textdocumentserializertest.h \
    edbee/io/tmlanguageparsertest.h \
    edbee/util/regexptest.h \
    edbee/models/textdocumentscopestest.h \
    edbee/models/textundostacktest.h \
    edbee/util/cascadingqvariantmaptest.h \
    edbee/models/textsearchertest.h \
    edbee/commands/duplicatecommandtest.h \
    edbee/commands/newlinecommandtest.h \
    edbee/util/utiltest.h \
    edbee/lexers/grammartextlexertest.h \
    edbee/commands/removecommandtest.h \
    edbee/models/changes/linedatalistchangetest.h \
    edbee/models/changes/textchangetest.h \
    edbee/models/changes/mergablechangegrouptest.h \
    edbee/util/rangesetlineiteratortest.h \
    edbee/models/dynamicvariablestest.h \
    edbee/util/rangelineiteratortest.h \
    edbee/views/textthememanagertest.h

##OTHER_FILES += ../edbee-data/config/*
##OTHER_FILES += ../edbee-data/keymaps/*
#OTHER_FILES += ../edbee-data/syntaxfiles/*
##OTHER_FILES += ../edbee-data/themes/*

## Extra data files
##==================

## Install all app data files to the application bundle
## TODO: We need to find a way to copy these files next to the exe file on windows (And later we need to check linux)
#APP_DATA_FILES.files = $$files(../edbee-data/*)
#APP_DATA_FILES.path = Contents/Resources

#QMAKE_BUNDLE_DATA += APP_DATA_FILES

#INCLUDEPATH += $$PWD/../edbee-lib

## Extra dependencies
##====================
include(../vendor/qslog/QsLog.pri)


## edbee-lib dependency
##=======================

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../edbee-lib/release/ -ledbee
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../edbee-lib/debug/ -ledbee
else:unix:!symbian: LIBS += -L$$OUT_PWD/../edbee-lib/ -ledbee

INCLUDEPATH += $$PWD/../edbee-lib
DEPENDPATH += $$PWD/../edbee-lib

win32-msvc*:LIBNAME=edbee.lib
else:LIBNAME=libedbee.a

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../edbee-lib/release/$$LIBNAME
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../edbee-lib/debug/$$LIBNAME
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../edbee-lib/$$LIBNAME

