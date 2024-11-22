/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include <QApplication>
#include <QDebug>
#include <QList>
#include <QTimer>

#include "../vendor/qslog/QsLog.h"
#include "../vendor/qslog/QsLogDest.h"

#include "edbee/util/test.h"
#include "edbee/edbee.h"

#include "edbee/debug.h"


/// the main entry method of the test
int main(int argc, char* argv[])
{
    QApplication app( argc, argv);

    // add all tests that need to be run
    //==================================
    QList<QString> tests;

    tests.append( "edbee::TextChangeTest");
    tests.append( "edbee::MergableChangeGroupTest");
    tests.append( "edbee::LineOffsetVectorTest");
    tests.append( "edbee::LineDataListChangeTest");
    tests.append( "edbee::PlainTextDocumentTest");
    tests.append( "edbee::TextLineDataTest");
    tests.append( "edbee::TextRangeSetTest");
    tests.append( "edbee::TextUndoStackTest");

    tests.clear();      // when the tests lists is empty all tests are run

    // test initialization
    //=====================

    // make sure we see the QsLogging items
    QsLogging::Logger& logger = QsLogging::Logger::instance();
    static QsLogging::DestinationPtrU debugDestination( QsLogging::DestinationFactory::MakeDebugOutputDestination() );
    logger.addDestination(std::move(debugDestination));
    logger.setLoggingLevel(QsLogging::TraceLevel);


    // Load the grammars
    QString appDataPath;
    #ifdef Q_OS_MAC
        appDataPath = app.applicationDirPath() + "/../Resources/";
    #else
        appDataPath= app.applicationDirPath() + "/data/";
    #endif

    // configure the edbee component to use the default paths
    edbee::Edbee* tm = edbee::Edbee::instance();
    //tm->setKeyMapPath( QStringLiteral("%1%2").arg(appDataPath).arg("keymaps"));
    tm->setGrammarPath(  QStringLiteral("%1%2").arg(appDataPath).arg("syntaxfiles") );
    //tm->setThemePath( QStringLiteral("%1%2").arg(appDataPath).arg("themes") );
    tm->init();

    // next run all tests
    if( tests.isEmpty() ) {
        edbee::test::engine().runAll();

    // only run the selected tests
    } else {
        edbee::test::engine().startRun();
        foreach( QString test, tests ) {
            edbee::test::engine().run(test);
        }
        edbee::test::engine().endRun();
    }


    // shutdown edbee
    tm->shutdown();

    return 0;
}

