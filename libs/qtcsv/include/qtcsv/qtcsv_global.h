#ifndef QTCSV_GLOBAL_H
#define QTCSV_GLOBAL_H

#include <QtGlobal>

#ifdef QTCSV_STATIC_LIB
#  define QTCSVSHARED_EXPORT
#else
#  if defined(QTCSV_LIBRARY)
#    define QTCSVSHARED_EXPORT Q_DECL_EXPORT
#  else
#    define QTCSVSHARED_EXPORT Q_DECL_IMPORT
#  endif
#endif

#endif // QTCSV_GLOBAL_H
