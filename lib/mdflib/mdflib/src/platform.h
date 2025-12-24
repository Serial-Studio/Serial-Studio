#ifndef PLATFORM_H
#define PLATFORM_H

// Platform independent functions

#include <cstddef>
#include <cstdint>
#include <cstdio>

#ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL

#if defined(__cpp_lib_filesystem)
#define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#elif defined(__cpp_lib_experimental_filesystem)
#define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#elif !defined(__has_include)
#define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#elif __has_include(<filesystem>)

#ifdef _MSC_VER
#if __has_include(<yvals_core.h>)
#include <yvals_core.h>
#if defined(_HAS_CXX17) && _HAS_CXX17
#define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#endif
#endif
#ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL
#define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#endif
#else  // #ifdef _MSC_VER
#define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 0
#endif

#elif __has_include(<experimental/filesystem>)
#define INCLUDE_STD_FILESYSTEM_EXPERIMENTAL 1
#else
#error Could not find system header "<filesystem>" or "<experimental/filesystem>"
#endif
#endif  // #ifndef INCLUDE_STD_FILESYSTEM_EXPERIMENTAL

namespace Platform {
int stricmp(const char *__s1, const char *__s2);
int strnicmp(const char *__s1, const char *__s2, size_t __n);
void strerror(int __errnum, char *__buf, size_t __buflen);

int64_t ftell64(std::FILE *__stream);
int fseek64(FILE *__stream, int64_t __off, int __whence);
int fileopen(FILE **out, const char *__restrict __filename,
             const char *__restrict __modes);

}  // namespace Platform

#endif  // PLATFORM_H
