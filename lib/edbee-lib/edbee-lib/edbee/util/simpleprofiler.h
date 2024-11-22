/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

//#include "config.h"

#include <QString>
#include <QStack>
#include <QMap>


namespace edbee {

#ifdef CONFIG_PROFILE

#define PROF_BEGIN \
    SimpleProfiler::instance()->begin( __FILE__, __LINE__, __func__, 0 );

#define PROF_END \
    SimpleProfiler::instance()->end();

#define PROF_BEGIN_NAMED(name) \
    SimpleProfiler::instance()->begin( __FILE__, __LINE__, __func__, name );

#else
#define PROF_BEGIN
#define PROF_END
#define PROF_BEGIN_NAMED(name)
#endif


/// __FILE__ / __LINE__ / __FUNCTION__ (or __func__ )

/// A simple profiler class that can be used to profile certain parts of the code.
/// I've introduced this class as a poor-mans profiler. Because currently valgrand and other profilers
/// don't seem to run smoothly on my mac. They crash ..
class EDBEE_EXPORT SimpleProfiler {

public:

    static SimpleProfiler* instance();


    /// the class to 'record a singlel item
    class ProfilerItem {
    public:
        ProfilerItem( const char* filename, int line, const char* function, const char* name );
        const char* filename() const { return filename_; }
        int line() const { return line_; }
        const char* function() const { return function_; }
        const char* name() const { return name_; }
        int callCount() const { return callCount_; }
        qint64 duration() const { return duration_; }
        qint64 childDuration() const { return childDuration_; }
        qint64 durationWithoutChilds() const { return duration_ - childDuration_; }

        void incCallCount() { ++callCount_; }
        void addDuration( qint64 duration ) { duration_ += duration; }
        void addChildDuration( qint64 duration ) { childDuration_ += duration; }

    protected:
        const char* filename_;      ///< The filename
        int line_;                  ///< The line
        const char* function_;      ///< The function/method name
        const char* name_;          ///< The custom name

        int callCount_;             ///< The total number of calls
        qint64 duration_;           ///< The total duration
        qint64 childDuration_;      ///< Duration of child-items (items called by this item)
    };

    /// The current stats items
    struct ProfileStackItem {
        ProfilerItem* item;
        qint64 startTime;
    };



    SimpleProfiler();
    virtual ~SimpleProfiler();


    void begin( const char* file, int line, const char* function, const char* name );
    void end();

    void dumpResults();


protected:

    QMap<QString,ProfilerItem*> statsMap_;   ///< The statistics
    QStack<ProfileStackItem> stack_;         ///< The current items being processed
};



} // edbee
