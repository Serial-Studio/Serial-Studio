# Should be called before PROJECT.
CMAKE_MINIMUM_REQUIRED(VERSION 3.1...3.20)

IF(POLICY CMP0020)
  CMAKE_POLICY(SET CMP0020 NEW)
ENDIF()

# hack?? https://stackoverflow.com/questions/31561309/cmake-warnings-under-os-x-macosx-rpath-is-not-specified-for-the-following-targe
set(CMAKE_MACOSX_RPATH 1)

PROJECT(edbee-lib)

add_subdirectory(vendor/qslog/ qslog)

SET(SOURCES
   edbee/commands/commentcommand.cpp
   edbee/commands/copycommand.cpp
   edbee/commands/cutcommand.cpp
   edbee/commands/debugcommand.cpp
   edbee/commands/duplicatecommand.cpp
   edbee/commands/findcommand.cpp
   edbee/commands/movelinecommand.cpp
   edbee/commands/newlinecommand.cpp
   edbee/commands/pastecommand.cpp
   edbee/commands/redocommand.cpp
   edbee/commands/removecommand.cpp
   edbee/commands/replaceselectioncommand.cpp
   edbee/commands/selectioncommand.cpp
   edbee/commands/tabcommand.cpp
   edbee/commands/togglereadonlycommand.cpp
   edbee/commands/undocommand.cpp
   edbee/data/factorycommandmap.cpp
   edbee/data/factorykeymap.cpp
   edbee/edbee.cpp
   edbee/io/baseplistparser.cpp
   edbee/io/jsonparser.cpp
   edbee/io/keymapparser.cpp
   edbee/io/textdocumentserializer.cpp
   edbee/io/tmlanguageparser.cpp
   edbee/io/tmthemeparser.cpp
   edbee/lexers/grammartextlexer.cpp
   edbee/models/change.cpp
   edbee/models/changes/abstractrangedchange.cpp
   edbee/models/changes/linedatachange.cpp
   edbee/models/changes/linedatalistchange.cpp
   edbee/models/changes/mergablechangegroup.cpp
   edbee/models/changes/selectionchange.cpp
   edbee/models/changes/textchange.cpp
   edbee/models/changes/textchangewithcaret.cpp
   edbee/models/chardocument/chartextbuffer.cpp
   edbee/models/chardocument/chartextdocument.cpp
   edbee/models/dynamicvariables.cpp
   edbee/models/textautocompleteprovider.cpp
   edbee/models/textbuffer.cpp
   edbee/models/textdocument.cpp
   edbee/models/textdocumentfilter.cpp
   edbee/models/textdocumentscopes.cpp
   edbee/models/texteditorcommandmap.cpp
   edbee/models/texteditorconfig.cpp
   edbee/models/texteditorkeymap.cpp
   edbee/models/textgrammar.cpp
   edbee/models/textlexer.cpp
   edbee/models/textlinedata.cpp
   edbee/models/textrange.cpp
   edbee/models/textsearcher.cpp
   edbee/models/textundostack.cpp
   edbee/texteditorcommand.cpp
   edbee/texteditorcontroller.cpp
   edbee/texteditorwidget.cpp
   edbee/util/cascadingqvariantmap.cpp
   edbee/util/gapvector.h
   edbee/util/lineending.cpp
   edbee/util/lineoffsetvector.cpp
   edbee/util/mem/debug_allocs.cpp
   edbee/util/mem/debug_new.cpp
   edbee/util/rangelineiterator.cpp
   edbee/util/rangesetlineiterator.cpp
   edbee/util/regexp.cpp
   edbee/util/simpleprofiler.cpp
   edbee/util/test.cpp
   edbee/util/textcodec.cpp
   edbee/util/textcodecdetector.cpp
   edbee/util/util.cpp
   edbee/views/accessibletexteditorwidget.cpp
   edbee/views/components/texteditorautocompletecomponent.cpp
   edbee/views/components/texteditorcomponent.cpp
   edbee/views/components/texteditorrenderer.cpp
   edbee/views/components/textmargincomponent.cpp
   edbee/views/textcaretcache.cpp
   edbee/views/texteditorscrollarea.cpp
   edbee/views/textlayout.cpp
   edbee/views/textrenderer.cpp
   edbee/views/textselection.cpp
   edbee/views/texttheme.cpp
)

SET(HEADERS
   edbee/commands/commentcommand.h
   edbee/commands/copycommand.h
   edbee/commands/cutcommand.h
   edbee/commands/debugcommand.h
   edbee/commands/duplicatecommand.h
   edbee/commands/findcommand.h
   edbee/commands/movelinecommand.h
   edbee/commands/newlinecommand.h
   edbee/commands/pastecommand.h
   edbee/commands/redocommand.h
   edbee/commands/removecommand.h
   edbee/commands/replaceselectioncommand.h
   edbee/commands/selectioncommand.h
   edbee/commands/tabcommand.h
   edbee/commands/togglereadonlycommand.h
   edbee/commands/undocommand.h
   edbee/data/factorycommandmap.h
   edbee/data/factorykeymap.h
   edbee/debug.h
   edbee/edbee.h
   edbee/io/baseplistparser.h
   edbee/io/jsonparser.h
   edbee/io/keymapparser.h
   edbee/io/textdocumentserializer.h
   edbee/io/tmlanguageparser.h
   edbee/io/tmthemeparser.h
   edbee/lexers/grammartextlexer.h
   edbee/models/change.h
   edbee/models/changes/abstractrangedchange.h
   edbee/models/changes/linedatachange.h
   edbee/models/changes/linedatalistchange.h
   edbee/models/changes/mergablechangegroup.h
   edbee/models/changes/selectionchange.h
   edbee/models/changes/textchange.h
   edbee/models/changes/textchangewithcaret.h
   edbee/models/chardocument/chartextbuffer.h
   edbee/models/chardocument/chartextdocument.h
   edbee/models/dynamicvariables.h
   edbee/models/textautocompleteprovider.h
   edbee/models/textbuffer.h
   edbee/models/textdocument.h
   edbee/models/textdocumentfilter.h
   edbee/models/textdocumentscopes.h
   edbee/models/texteditorcommandmap.h
   edbee/models/texteditorconfig.h
   edbee/models/texteditorkeymap.h
   edbee/models/textgrammar.h
   edbee/models/textlexer.h
   edbee/models/textlinedata.h
   edbee/models/textrange.h
   edbee/models/textsearcher.h
   edbee/models/textundostack.h
   edbee/texteditorcommand.h
   edbee/texteditorcontroller.h
   edbee/texteditorwidget.h
   edbee/util/cascadingqvariantmap.h
   edbee/util/lineending.h
   edbee/util/lineoffsetvector.h
   edbee/util/logging.h
   edbee/util/mem/debug_allocs.h
   edbee/util/mem/debug_new.h
   edbee/util/rangelineiterator.h
   edbee/util/rangesetlineiterator.h
   edbee/util/regexp.h
   edbee/util/simpleprofiler.h
   edbee/util/test.h
   edbee/util/textcodec.h
   edbee/util/textcodecdetector.h
   edbee/util/util.h
   edbee/views/accessibletexteditorwidget.h
   edbee/views/components/texteditorautocompletecomponent.h
   edbee/views/components/texteditorcomponent.h
   edbee/views/components/texteditorrenderer.h
   edbee/views/components/textmargincomponent.h
   edbee/views/textcaretcache.h
   edbee/views/texteditorscrollarea.h
   edbee/views/textlayout.h
   edbee/views/textrenderer.h
   edbee/views/textselection.h
   edbee/views/texttheme.h
)

add_subdirectory(vendor/onig/ onig)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Core5Compat)
set(QT_LIBS Qt6::Core Qt6::Widgets Qt6::Core5Compat)

ADD_LIBRARY(edbee-lib STATIC
  ${SOURCES} ${HEADERS} ${ONIG_SOURCES}
  ${ONIG_HEADERS})

target_link_libraries(edbee-lib QsLog ${QT_LIBS})

set_target_properties(edbee-lib PROPERTIES AUTOMOC ON CXX_STANDARD 11)

TARGET_INCLUDE_DIRECTORIES(edbee-lib PUBLIC
  ${QS_LOG_INCLUDE_DIRS}
  ${ONIG_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
)
