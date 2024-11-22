

INCLUDEPATH += $$PWD

SOURCES += \
    $$PWD/edbee/commands/commentcommand.cpp \
    $$PWD/edbee/commands/copycommand.cpp \
    $$PWD/edbee/commands/cutcommand.cpp \
    $$PWD/edbee/commands/debugcommand.cpp \
    $$PWD/edbee/commands/duplicatecommand.cpp \
    $$PWD/edbee/commands/findcommand.cpp \
    $$PWD/edbee/commands/movelinecommand.cpp \
    $$PWD/edbee/commands/newlinecommand.cpp \
    $$PWD/edbee/commands/pastecommand.cpp \
    $$PWD/edbee/commands/redocommand.cpp \
    $$PWD/edbee/commands/removecommand.cpp \
    $$PWD/edbee/commands/replaceselectioncommand.cpp \
    $$PWD/edbee/commands/selectioncommand.cpp \
    $$PWD/edbee/commands/tabcommand.cpp \
    $$PWD/edbee/commands/togglereadonlycommand.cpp \
    $$PWD/edbee/commands/undocommand.cpp \
    $$PWD/edbee/data/factorycommandmap.cpp \
    $$PWD/edbee/data/factorykeymap.cpp \
    $$PWD/edbee/edbee.cpp \
    $$PWD/edbee/io/baseplistparser.cpp \
    $$PWD/edbee/io/jsonparser.cpp \
    $$PWD/edbee/io/keymapparser.cpp \
    $$PWD/edbee/io/textdocumentserializer.cpp \
    $$PWD/edbee/io/tmlanguageparser.cpp \
    $$PWD/edbee/io/tmthemeparser.cpp \
    $$PWD/edbee/lexers/grammartextlexer.cpp \
    $$PWD/edbee/models/change.cpp \
    $$PWD/edbee/models/changes/abstractrangedchange.cpp \
    $$PWD/edbee/models/changes/linedatachange.cpp \
    $$PWD/edbee/models/changes/linedatalistchange.cpp \
    $$PWD/edbee/models/changes/mergablechangegroup.cpp \
    $$PWD/edbee/models/changes/selectionchange.cpp \
    $$PWD/edbee/models/changes/textchange.cpp \
    $$PWD/edbee/models/changes/textchangewithcaret.cpp \
    $$PWD/edbee/models/chardocument/chartextbuffer.cpp \
    $$PWD/edbee/models/chardocument/chartextdocument.cpp \
    $$PWD/edbee/models/dynamicvariables.cpp \
    $$PWD/edbee/models/textautocompleteprovider.cpp \
    $$PWD/edbee/models/textbuffer.cpp \
    $$PWD/edbee/models/textdocument.cpp \
    $$PWD/edbee/models/textdocumentfilter.cpp \
    $$PWD/edbee/models/textdocumentscopes.cpp \
    $$PWD/edbee/models/texteditorcommandmap.cpp \
    $$PWD/edbee/models/texteditorconfig.cpp \
    $$PWD/edbee/models/texteditorkeymap.cpp \
    $$PWD/edbee/models/textgrammar.cpp \
    $$PWD/edbee/models/textlexer.cpp \
    $$PWD/edbee/models/textlinedata.cpp \
    $$PWD/edbee/models/textrange.cpp \
    $$PWD/edbee/models/textsearcher.cpp \
    $$PWD/edbee/models/textundostack.cpp \
    $$PWD/edbee/texteditorcommand.cpp \
    $$PWD/edbee/texteditorcontroller.cpp \
    $$PWD/edbee/texteditorwidget.cpp \
    $$PWD/edbee/util/cascadingqvariantmap.cpp \
    $$PWD/edbee/util/gapvector.h \
    $$PWD/edbee/util/lineending.cpp \
    $$PWD/edbee/util/lineoffsetvector.cpp \
    $$PWD/edbee/util/mem/debug_allocs.cpp \
    $$PWD/edbee/util/mem/debug_new.cpp \
    $$PWD/edbee/util/rangelineiterator.cpp \
    $$PWD/edbee/util/rangesetlineiterator.cpp \
    $$PWD/edbee/util/regexp.cpp \
    $$PWD/edbee/util/simpleprofiler.cpp \
    $$PWD/edbee/util/test.cpp \
    $$PWD/edbee/util/textcodec.cpp \
    $$PWD/edbee/util/textcodecdetector.cpp \
    $$PWD/edbee/util/util.cpp \
    $$PWD/edbee/views/accessibletexteditorwidget.cpp \
    $$PWD/edbee/views/components/texteditorautocompletecomponent.cpp \
    $$PWD/edbee/views/components/texteditorcomponent.cpp \
    $$PWD/edbee/views/components/texteditorrenderer.cpp \
    $$PWD/edbee/views/components/textmargincomponent.cpp \
    $$PWD/edbee/views/textcaretcache.cpp \
    $$PWD/edbee/views/texteditorscrollarea.cpp \
    $$PWD/edbee/views/textlayout.cpp \
    $$PWD/edbee/views/textrenderer.cpp \
    $$PWD/edbee/views/textselection.cpp \
    $$PWD/edbee/views/texttheme.cpp

HEADERS += \
    $$PWD/edbee/commands/commentcommand.h \
    $$PWD/edbee/commands/copycommand.h \
    $$PWD/edbee/commands/cutcommand.h \
    $$PWD/edbee/commands/debugcommand.h \
    $$PWD/edbee/commands/duplicatecommand.h \
    $$PWD/edbee/commands/findcommand.h \
    $$PWD/edbee/commands/movelinecommand.h \
    $$PWD/edbee/commands/newlinecommand.h \
    $$PWD/edbee/commands/pastecommand.h \
    $$PWD/edbee/commands/redocommand.h \
    $$PWD/edbee/commands/removecommand.h \
    $$PWD/edbee/commands/replaceselectioncommand.h \
    $$PWD/edbee/commands/selectioncommand.h \
    $$PWD/edbee/commands/tabcommand.h \
    $$PWD/edbee/commands/togglereadonlycommand.h \
    $$PWD/edbee/commands/undocommand.h \
    $$PWD/edbee/data/factorycommandmap.h \
    $$PWD/edbee/data/factorykeymap.h \
    $$PWD/edbee/debug.h \
    $$PWD/edbee/edbee.h \
    $$PWD/edbee/exports.h \
    $$PWD/edbee/io/baseplistparser.h \
    $$PWD/edbee/io/jsonparser.h \
    $$PWD/edbee/io/keymapparser.h \
    $$PWD/edbee/io/textdocumentserializer.h \
    $$PWD/edbee/io/tmlanguageparser.h \
    $$PWD/edbee/io/tmthemeparser.h \
    $$PWD/edbee/lexers/grammartextlexer.h \
    $$PWD/edbee/models/change.h \
    $$PWD/edbee/models/changes/abstractrangedchange.h \
    $$PWD/edbee/models/changes/linedatachange.h \
    $$PWD/edbee/models/changes/linedatalistchange.h \
    $$PWD/edbee/models/changes/mergablechangegroup.h \
    $$PWD/edbee/models/changes/selectionchange.h \
    $$PWD/edbee/models/changes/textchange.h \
    $$PWD/edbee/models/changes/textchangewithcaret.h \
    $$PWD/edbee/models/chardocument/chartextbuffer.h \
    $$PWD/edbee/models/chardocument/chartextdocument.h \
    $$PWD/edbee/models/dynamicvariables.h \
    $$PWD/edbee/models/textautocompleteprovider.h \
    $$PWD/edbee/models/textbuffer.h \
    $$PWD/edbee/models/textdocument.h \
    $$PWD/edbee/models/textdocumentfilter.h \
    $$PWD/edbee/models/textdocumentscopes.h \
    $$PWD/edbee/models/texteditorcommandmap.h \
    $$PWD/edbee/models/texteditorconfig.h \
    $$PWD/edbee/models/texteditorkeymap.h \
    $$PWD/edbee/models/textgrammar.h \
    $$PWD/edbee/models/textlexer.h \
    $$PWD/edbee/models/textlinedata.h \
    $$PWD/edbee/models/textrange.h \
    $$PWD/edbee/models/textsearcher.h \
    $$PWD/edbee/models/textundostack.h \
    $$PWD/edbee/texteditorcommand.h \
    $$PWD/edbee/texteditorcontroller.h \
    $$PWD/edbee/texteditorwidget.h \
    $$PWD/edbee/util/cascadingqvariantmap.h \
    $$PWD/edbee/util/lineending.h \
    $$PWD/edbee/util/lineoffsetvector.h \
    $$PWD/edbee/util/logging.h \
    $$PWD/edbee/util/mem/debug_allocs.h \
    $$PWD/edbee/util/mem/debug_new.h \
    $$PWD/edbee/util/rangelineiterator.h \
    $$PWD/edbee/util/rangesetlineiterator.h \
    $$PWD/edbee/util/regexp.h \
    $$PWD/edbee/util/simpleprofiler.h \
    $$PWD/edbee/util/test.h \
    $$PWD/edbee/util/textcodec.h \
    $$PWD/edbee/util/textcodecdetector.h \
    $$PWD/edbee/util/util.h \
    $$PWD/edbee/views/accessibletexteditorwidget.h \
    $$PWD/edbee/views/components/texteditorautocompletecomponent.h \
    $$PWD/edbee/views/components/texteditorcomponent.h \
    $$PWD/edbee/views/components/texteditorrenderer.h \
    $$PWD/edbee/views/components/textmargincomponent.h \
    $$PWD/edbee/views/textcaretcache.h \
    $$PWD/edbee/views/texteditorscrollarea.h \
    $$PWD/edbee/views/textlayout.h \
    $$PWD/edbee/views/textrenderer.h \
    $$PWD/edbee/views/textselection.h \
    $$PWD/edbee/views/texttheme.h

## Extra dependencies
##====================
include($$PWD/../vendor/qslog/QsLog.pri)
include($$PWD/../vendor/onig/onig.pri)

## Files that can usefully listed inside Qt IDE (Qt Creator, as "Other files")
##============================================================================
DISTFILES += \
    $$PWD/../CHANGELOG.md \
    $$PWD/../LICENSE \
    $$PWD/../README.md \
    $$PWD/../.travis.yml \
    $$PWD/../.gitignore \
    $$PWD/../CMakeLists.txt \
    $$PWD/../CI/travis.install.sh \
    $$PWD/../CI/travis.osx.install.sh \
    $$PWD/../CI/travis.before_install.sh \
    $$PWD/../CI/travis.osx.before_install.sh \
    $$PWD/CMakeLists.txt \
    $$PWD/Doxyfile \
    $$PWD/doc/coding-style.md \
    $$PWD/doc/index.md \
    $$PWD/images/doc-logo.png
