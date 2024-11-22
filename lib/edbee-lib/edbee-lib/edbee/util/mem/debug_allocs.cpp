/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include <QString>
#include <QMutex>

#include <stdlib.h>
#include <stdio.h>
#include <cstdlib>

#include "debug_allocs.h"

namespace edbee {


/// The global memory leak object
static DebugAllocationList allocationList;


/// returns the allocation list instance
DebugAllocationList* DebugAllocationList::instance()
{
    return &allocationList;
}


/// cnostructs the allocation list
DebugAllocationList::DebugAllocationList()
    : allocationList_()
    , mutex_(0)
    , running_(false)
    , started_(false)
{
#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    mutex_ = new EdbeeRecursiveMutex( QMutex::Recursive );
#else
    mutex_ = new EdbeeRecursiveMutex();
#endif
    start(false);
    checkDelete_ = false;
}


/// The allocation list destructor
DebugAllocationList::~DebugAllocationList()
{
    stop();          // check last memory leak
    //delete mutex_;   // < do not delete keep memory leak :S. When deleting this mutex app shutdown crashes
    mutex_ = 0;
    clear();
}


/// clears the debugging allocation list
void DebugAllocationList::clear()
{
    // iterate over the map's (key,val) pairs as usual
    for(std::map<void*,DebugAllocation*>::iterator iter = allocationList_.begin(); iter != allocationList_.end(); ++iter) {
        std::free( iter->second );
    }

    allocationList_.clear();
}


/// Returns the mutex for thread-safety
EdbeeRecursiveMutex* DebugAllocationList::mutex()
{
    return mutex_;
}


/// Starts the monitoring of allocations
/// @param checkDelete should the delete operation be checked?
void DebugAllocationList::start( bool checkDelete )
{
    clear();
    checkDelete_ = checkDelete;
    running_ = true;
    started_ = true;
}


/// Stops the monitoring of allocations
int DebugAllocationList::stop()
{
    started_ = false;
    running_ = false;
    checkDelete_ = false;
    int res = 0;

    for(std::map<void*,DebugAllocation*>::iterator iter = allocationList_.begin(); iter != allocationList_.end(); ++iter) {
        DebugAllocation *info = iter->second;
        if( info && info->pointer ) {
            printf( "Memory leak %p(%u) %s:%d\n", info->pointer, (unsigned int)info->size, info->file, info->line );
            res++;
        }
    }

    if( !res ) {
        //printf("memory ok :-D\n");
    }

    return res;
}


/// Finds the given pointer int the allocation list
/// @param p the pointer to search
/// @return the allocation object
DebugAllocation* DebugAllocationList::find(void* p)
{
    std::map< void *, DebugAllocation*>::iterator itr = allocationList_.find(p);
    if( itr != allocationList_.end() )
        return itr->second;
    return 0;
}


/// Adds the given pointer to the allocation list
/// @param p pointer
/// @param size the size of the pointer
/// @param file the source file of the allocation
/// @param line the line number of the allocation
DebugAllocation* DebugAllocationList::add(void* p, size_t size, char* file, int line)
{
    DebugAllocation* info = (DebugAllocation*)std::malloc( sizeof(DebugAllocation) );
    info->clear();
    info->pointer = p;
    info->size = size;
    info->file = file;
    info->line = line;
    allocationList_[p]=info;
    return info;
}


/// removes the given pointer from the allocation list
/// @param p the pointer
bool DebugAllocationList::del(void* p)
{
    std::map<void*,DebugAllocation*>::iterator iter = allocationList_.find(p);

    if( iter != allocationList_.end() ) {
        DebugAllocation *mi = iter->second; //take(p);
        allocationList_.erase(iter);
        if( !mi ) return false;
        std::free( mi );
    }
    return true;

}


/// returns the state of checkdelete
bool DebugAllocationList::checkDelete()
{
    return checkDelete_;
}

} // edbee
