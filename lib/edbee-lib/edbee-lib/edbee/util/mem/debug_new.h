/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */


#pragma once

#include <stdlib.h>
#include <stdio.h>
//#ifdef HAVE_MALLOC_H
//#include <malloc.h>
//#endif
//#include "memoryleak.h"

// #if defined(QT_DEBUG) && !defined(__MINGW32__) && defined(EDBEE_DEBUG)
#if defined(EDBEE_DEBUG)
    #define EDBEE_DEBUG_NEW_ACTIVE

    void* debug_malloc      (size_t size, const char* file, const int line);
    void  debug_free        (void* p,     const char* file, const int line);
    void* operator new      (size_t size, const char* file, const int line);
    void  operator delete   (void* p,     const char* file, const int line);
    void  operator delete   (void* p) throw();
    void* operator new[]    (size_t size, const char* file, const int line);
    void  operator delete[] (void* p,     const char* file, const int line);
    void  operator delete[] (void* p) throw();

    #define debug_new new(__FILE__, __LINE__)
    #define new       new(__FILE__, __LINE__)
    #define malloc(A) debug_malloc((A), __FILE__, __LINE__)
    #define free(A)   debug_free((A), __FILE__, __LINE__)
#endif


namespace edbee {
    void pause_memleak_detection(bool value);
} // edbee

