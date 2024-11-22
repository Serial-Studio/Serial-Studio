#include "QsLogLevel.h"
#include <QString>
#include <QObject>
#include <cassert>

static const char TraceString[] = "TRACE";
static const char DebugString[] = "DEBUG";
static const char InfoString[]  = "INFO ";
static const char WarnString[]  = "WARN ";
static const char ErrorString[] = "ERROR";
static const char FatalString[] = "FATAL";

const char* QsLogging::LevelName(QsLogging::Level theLevel)
{
    switch (theLevel) {
        case QsLogging::TraceLevel:
            return TraceString;
        case QsLogging::DebugLevel:
            return DebugString;
        case QsLogging::InfoLevel:
            return InfoString;
        case QsLogging::WarnLevel:
            return WarnString;
        case QsLogging::ErrorLevel:
            return ErrorString;
        case QsLogging::FatalLevel:
            return FatalString;
        case QsLogging::OffLevel:
            return "";
        default: {
            Q_ASSERT(!"bad log level");
            return InfoString;
        }
    }
}

QString QsLogging::LocalizedLevelName(QsLogging::Level theLevel)
{
    switch (theLevel) {
    case QsLogging::TraceLevel:
        return QObject::tr("Trace");
    case QsLogging::DebugLevel:
        return QObject::tr("Debug");
    case QsLogging::InfoLevel:
        return QObject::tr("Info");
    case QsLogging::WarnLevel:
        return QObject::tr("Warning");
    case QsLogging::ErrorLevel:
        return QObject::tr("Error");
    case QsLogging::FatalLevel:
        return QObject::tr("Fatal");
    default:
        return QString();
    }
}
