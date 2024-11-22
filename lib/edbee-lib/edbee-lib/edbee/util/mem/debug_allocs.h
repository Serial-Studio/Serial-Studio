/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

/// Cross Platform Memory Leak Detection.
/// Original source from: http://www.gilgil.net by Gilbert Lee. All rights reserved
/// Altered to use QT API
#pragma once

#include "edbee/exports.h"



#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    #define EdbeeRecursiveMutex QMutex
#else
    #define EdbeeRecursiveMutex QRecursiveMutex
#endif

class EdbeeRecursiveMutex;


//#if !defined(__APPLE__)
//#include <malloc>
//#endif
#include <cstring> // for size_t
#include <map>

namespace edbee {

/// This structure is used to 'remember' what is allocated at which place
struct DebugAllocation
{
    void*       pointer;
    std::size_t size;
    char*       file;
    int         line;
//    MemoryLeakInfo() : p(0), size(0), file(0), line(0) {}
//    MemoryLeakInfo( void *p1, std::size_t size1, char *file1, int line1) : p(p1), size(size1), file(file1), line(line1) {}
    void clear() { pointer=0; size=0; file=0; line=0; }
};

/// This class is used to remember all memory leakds
class EDBEE_EXPORT DebugAllocationList {
public:

    DebugAllocationList();
    virtual ~DebugAllocationList();

    void clear();
    size_t size() { return allocationList_.size(); }
    inline bool isRunning() { return running_; }

    void pause( bool val )
    {
        if( started_ ) {
            running_ = !val;
        }
    }


    EdbeeRecursiveMutex* mutex();

public:
    void start( bool checkDelete );
    int  stop();

    DebugAllocation* find(void* p);
    DebugAllocation* add (void* p, size_t size, char* file, int line);
    bool del (void* p);

    bool checkDelete();

public:

    static DebugAllocationList* instance();


private:
    std::map< void *, DebugAllocation*>  allocationList_;       ///< The allocation
    bool checkDelete_;                                          ///< Should we check for invalid deletes?
    EdbeeRecursiveMutex* mutex_;                                ///< The current mutext
    bool running_;                                              ///< A we 'recording'
    bool started_;                                              ///< Paused?


    friend void* debug_malloc(size_t size, const char* file, const int line);
    friend void  debug_free  (void* p,     const char* file, const int line);


};


} // edbee
