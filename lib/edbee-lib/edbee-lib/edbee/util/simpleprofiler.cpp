/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "simpleprofiler.h"
#include <QDateTime>
#include "edbee/debug.h"

namespace edbee {

/// This method returns the profile instance
SimpleProfiler *SimpleProfiler::instance()
{
    static SimpleProfiler profiler;
    return &profiler;
}


/// the constructor for a profile stat issue
SimpleProfiler::ProfilerItem::ProfilerItem(const char *filename, int line, const char *function, const char* name )
    : filename_(filename)
    , line_(line)
    , function_(function)
    , name_(name)
    , callCount_(0)
    , duration_(0)
    , childDuration_(0)
{
}


SimpleProfiler::SimpleProfiler()
{
}


/// destroy the stuff
SimpleProfiler::~SimpleProfiler()
{
    qDeleteAll(statsMap_);
    statsMap_.clear();
    stack_.clear();
}


/// begin the profiling
void SimpleProfiler::begin(const char *file, int line, const char *function, const char* name )
{
    // build the key
    QString key = QStringLiteral("%1:%2").arg(file).arg(line);

    // fetch or create the item
    ProfilerItem *item = statsMap_.value(key,0);
    if( !item ) {
        item = new ProfilerItem(file,line,function,name);
        statsMap_.insert(key,item);
    }

    ProfileStackItem stackItem = { item, QDateTime::currentMSecsSinceEpoch() };
    stack_.push(stackItem);
}

/// ends profiling
void SimpleProfiler::end()
{
    Q_ASSERT( stack_.size() > 0 );
    ProfileStackItem stackItem = stack_.pop();
    stackItem.item->incCallCount();
    int duration = QDateTime::currentMSecsSinceEpoch() - stackItem.startTime;
    stackItem.item->addDuration( duration );
    if( !stack_.isEmpty() ) {
        stack_.top().item->addChildDuration( duration );
    }

}


static bool sortByDuration( const SimpleProfiler::ProfilerItem* a, const SimpleProfiler::ProfilerItem* b )
{
   return b->durationWithoutChilds() < a->durationWithoutChilds();
}

/// This method dumps the results to the output
void SimpleProfiler::dumpResults()
{
    QList<ProfilerItem*> items = statsMap_.values();
    if( items.length() > 0 ) {
        std::sort(items.begin(), items.end(), sortByDuration);

        qlog_info() << "";
        qlog_info() << "Profiler Results";
        qlog_info() << "================";

        qint64 totalDuration = 0;
        int totalCallCount = 0;
        int totalDurationWitoutChilds = 0;
        foreach( ProfilerItem* item, items ) {
            totalDuration  += item->duration();
            totalCallCount += item->callCount();
            totalDurationWitoutChilds += item->durationWithoutChilds();
        }

        foreach( ProfilerItem* item, items ) {
            double durationPercentage = totalDuration > 0 ? 100.0 * item->duration() / totalDuration : 100;
            double callCountPercentage = totalCallCount > 0 ? 100.0 * item->callCount() / totalCallCount : 100;
            double durationWithoutChildsPercenage = totalDurationWitoutChilds > 0 ? 100.0 * item->durationWithoutChilds() / totalDurationWitoutChilds : 100;

            QString line = QStringLiteral("%1x(%2%) %3ms(%4%) %5ms(%6%) |  %7:%8 %9")
               .arg(item->callCount(),8).arg( callCountPercentage, 6, 'f', 2 )
               .arg(item->duration(),6).arg( durationPercentage, 6, 'f', 2 )
               .arg(item->durationWithoutChilds(), 6 ).arg( durationWithoutChildsPercenage, 6, 'f', 2 )
               .arg(item->filename()).arg(item->line()).arg(item->name())
            ;
            qlog_info() << line;
        }

        qlog_info() << "";
        qlog_info() << "Total Duration: " << totalDuration << "ms";
        qlog_info() << "Total Calls   : " << totalCallCount;
        qlog_info() << "Total Items   : " << items.size() << "ms";

        if( !stack_.isEmpty() ) {
            qlog() << "** WARNING: The stack isn't empty!! ** ";
            foreach( ProfileStackItem stackItem, stack_) {
                qlog() << " * " << stackItem.item->function() << ":" << stackItem.item->line();
            }
        }
    }
}


} // edbee
