CMAKE_MINIMUM_REQUIRED(VERSION 3.1...3.20)

IF(POLICY CMP0020)
  CMAKE_POLICY(SET CMP0020 NEW)
ENDIF()

PROJECT(edbee-test)

SET(SOURCES
  edbee/commands/replaceselectioncommandtest.cpp
  edbee/models/textrangetest.cpp
  edbee/models/textdocumenttest.cpp
  edbee/models/textbuffertest.cpp
  edbee/models/textlinedatatest.cpp
  edbee/util/gapvectortest.cpp
  edbee/util/lineoffsetvectortest.cpp
  main.cpp
  edbee/util/lineendingtest.cpp
  edbee/textdocumentserializertest.cpp
  edbee/io/tmlanguageparsertest.cpp
  edbee/util/regexptest.cpp
  edbee/models/textdocumentscopestest.cpp
  edbee/models/textundostacktest.cpp
  edbee/util/cascadingqvariantmaptest.cpp
  edbee/models/textsearchertest.cpp
  edbee/commands/duplicatecommandtest.cpp
  edbee/commands/newlinecommandtest.cpp
  edbee/util/utiltest.cpp
  edbee/lexers/grammartextlexertest.cpp
  edbee/commands/removecommandtest.cpp
  edbee/models/changes/linedatalistchangetest.cpp
  edbee/models/changes/textchangetest.cpp
  edbee/models/changes/mergablechangegrouptest.cpp
  edbee/util/rangesetlineiteratortest.cpp
  edbee/models/dynamicvariablestest.cpp
  edbee/util/rangelineiteratortest.cpp
  edbee/views/textthememanagertest.cpp
)

SET(HEADERS
  edbee/commands/replaceselectioncommandtest.h
  edbee/models/textrangetest.h
  edbee/models/textdocumenttest.h
  edbee/models/textbuffertest.h
  edbee/models/textlinedatatest.h
  edbee/util/gapvectortest.h
  edbee/util/lineoffsetvectortest.h
  edbee/util/lineendingtest.h
  edbee/textdocumentserializertest.h
  edbee/io/tmlanguageparsertest.h
  edbee/util/regexptest.h
  edbee/models/textdocumentscopestest.h
  edbee/models/textundostacktest.h
  edbee/util/cascadingqvariantmaptest.h
  edbee/models/textsearchertest.h
  edbee/commands/duplicatecommandtest.h
  edbee/commands/newlinecommandtest.h
  edbee/util/utiltest.h
  edbee/lexers/grammartextlexertest.h
  edbee/commands/removecommandtest.h
  edbee/models/changes/linedatalistchangetest.h
  edbee/models/changes/textchangetest.h
  edbee/models/changes/mergablechangegrouptest.h
  edbee/util/rangesetlineiteratortest.h
  edbee/models/dynamicvariablestest.h
  edbee/util/rangelineiteratortest.h
  edbee/views/textthememanagertest.h
)

if (BUILD_WITH_QT5)
  find_package(Qt5 REQUIRED COMPONENTS Core UiTools Widgets)
  set(QT_LIBS Qt5::Core Qt5::UiTools Qt5::Widgets)
else()
  find_package(Qt6 REQUIRED COMPONENTS Core UiTools Widgets)
  set(QT_LIBS Qt6::Core Qt6::UiTools Qt6::Widgets)
endif()

ADD_EXECUTABLE(edbee-test
  ${SOURCES} ${HEADERS} ${QS_LOG_SOURCES} ${QS_LOG_HEADERS}
)

TARGET_LINK_LIBRARIES(edbee-test edbee-lib ${QT_LIBS})

set_target_properties(edbee-test PROPERTIES AUTOMOC ON CXX_STANDARD 11)

