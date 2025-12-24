#include "platform.h"

#include <cstring>

#if (!_MSC_VER)
#include <strings.h>

#include <cerrno>
#endif

#include "mdf/mdfhelper.h"

namespace Platform {

int stricmp(const char *__s1, const char *__s2) {
#if (_MSC_VER)
  return _stricmp(__s1, __s2);
#else
  return strcasecmp(__s1, __s2);
#endif
}

int strnicmp(const char *__s1, const char *__s2, size_t __n) {
#if (_MSC_VER)
  return _strnicmp(__s1, __s2, __n);
#else
  return strncasecmp(__s1, __s2, __n);
#endif
}

void strerror(int __errnum, char *__buf, size_t __buflen) {
#if (_WIN32)
  strerror_s(__buf, __buflen, __errnum);
#elif (__APPLE__ || __FreeBSD__)
  int err = strerror_r(__errnum, __buf, __buflen);
  if (err == 0)
  {
    
  }
#elif (__CYGWIN__)
  strerror_r(__errnum, __buf, __buflen);
#else
  auto* dummy = strerror_r(__errnum, __buf, __buflen);
  if (dummy != nullptr) {
    strcpy(__buf, dummy);
  }
#endif
}

int64_t ftell64(std::FILE *__stream) {
#if (_MSC_VER)
  return _ftelli64(__stream);
#else
  return ftello(__stream);
#endif
}

int fseek64(std::FILE *__stream, int64_t __off, int __whence) {
#if (_MSC_VER)
  return _fseeki64(__stream, __off, __whence);
#else
  return fseeko(__stream, __off, __whence);
#endif
}

int fileopen(std::FILE **out, const char *__restrict __filename,
             const char *__restrict __modes) {
  *out = nullptr;
#if (_MSC_VER)
  return _wfopen_s(out, mdf::MdfHelper::Utf8ToUtf16(__filename).c_str(), mdf::MdfHelper::Utf8ToUtf16(__modes).c_str());
#else
  *out = fopen(__filename, __modes);
  if (!(*out)) {
    return errno;
  }
  return 0;
#endif
}

}  // namespace Platform

