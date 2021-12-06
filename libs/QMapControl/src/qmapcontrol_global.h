#include <QtGlobal>

#if !defined(QT_STATIC) && !defined(QMAPCONTROL_PROJECT_INCLUDE_SRC)
#  if defined(QMAPCONTROL_LIBRARY)
#    define QMAPCONTROL_EXPORT Q_DECL_EXPORT
#  else
#    define QMAPCONTROL_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define QMAPCONTROL_EXPORT
#endif
