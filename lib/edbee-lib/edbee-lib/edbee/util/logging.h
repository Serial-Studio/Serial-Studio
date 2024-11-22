/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include <QFileInfo>
#include "../vendor/qslog/QsLog.h"


#define qlog_trace() \
    if( QsLogging::Logger::instance().loggingLevel() > QsLogging::TraceLevel ){} \
    else  QsLogging::Logger::Helper(QsLogging::TraceLevel).stream() << QFileInfo(__FILE__).fileName() << '@' << __LINE__
#define qlog_debug() \
    if( QsLogging::Logger::instance().loggingLevel() > QsLogging::DebugLevel ){} \
    else QsLogging::Logger::Helper(QsLogging::DebugLevel).stream() << QFileInfo(__FILE__).fileName() << '@' << __LINE__
#define qlog_info()  \
    if( QsLogging::Logger::instance().loggingLevel() > QsLogging::InfoLevel ){} \
    else QsLogging::Logger::Helper(QsLogging::InfoLevel).stream() << QFileInfo( __FILE__).fileName() << '@' << __LINE__
#define qlog_warn()  \
    if( QsLogging::Logger::instance().loggingLevel() > QsLogging::WarnLevel ){} \
    else QsLogging::Logger::Helper(QsLogging::WarnLevel).stream() << QFileInfo(__FILE__).fileName() << '@' << __LINE__
#define qlog_error() \
    if( QsLogging::Logger::instance().loggingLevel() > QsLogging::ErrorLevel ){} \
    else QsLogging::Logger::Helper(QsLogging::ErrorLevel).stream() << QFileInfo(__FILE__).fileName() << '@' << __LINE__
#define qlog_fatal() \
    QsLogging::Logger::Helper(QsLogging::FatalLevel).stream() << QFileInfo(__FILE__).fileName() << '@' << __LINE__


/*
#define qlog_trace() \
    if( QsLogging::Logger::instance().loggingLevel() > QsLogging::TraceLevel ){} \
    else  QsLogging::Logger::Helper(QsLogging::TraceLevel).stream() << __FILE__ << '@' << __LINE__
#define qlog_debug() \
    if( QsLogging::Logger::instance().loggingLevel() > QsLogging::DebugLevel ){} \
    else QsLogging::Logger::Helper(QsLogging::DebugLevel).stream() << __FILE__ << '@' << __LINE__
#define qlog_info()  \
    if( QsLogging::Logger::instance().loggingLevel() > QsLogging::InfoLevel ){} \
    else QsLogging::Logger::Helper(QsLogging::InfoLevel).stream() << __FILE__ << '@' << __LINE__
#define qlog_warn()  \
    if( QsLogging::Logger::instance().loggingLevel() > QsLogging::WarnLevel ){} \
    else QsLogging::Logger::Helper(QsLogging::WarnLevel).stream() << __FILE__ << '@' << __LINE__
#define qlog_error() \
    if( QsLogging::Logger::instance().loggingLevel() > QsLogging::ErrorLevel ){} \
    else QsLogging::Logger::Helper(QsLogging::ErrorLevel).stream() << __FILE__ << '@' << __LINE__
#define qlog_fatal() \
    QsLogging::Logger::Helper(QsLogging::FatalLevel).stream() << __FILE__ << '@' << __LINE__
*/


#define qlog() \
    QsLogging::Logger::Helper(QsLogging::InfoLevel).stream()


