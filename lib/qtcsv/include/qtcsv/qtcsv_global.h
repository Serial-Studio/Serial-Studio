#ifndef QTCSV_GLOBAL_H
#define QTCSV_GLOBAL_H

#include <QtGlobal>

#if defined(QTCSV_BUILD_DLIB)
#  define QTCSVSHARED_EXPORT Q_DECL_EXPORT
#elif defined(QTCSV_USE_DLIB)
#  define QTCSVSHARED_EXPORT Q_DECL_IMPORT
#else
#  define QTCSVSHARED_EXPORT
#endif

#endif // QTCSV_GLOBAL_H
