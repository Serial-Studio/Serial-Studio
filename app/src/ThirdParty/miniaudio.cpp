#define MINIAUDIO_IMPLEMENTATION

#ifdef _WIN32
#  define MA_COINIT_VALUE 2 /* COINIT_APARTMENTTHREADED — match Qt's STA model */
#endif

#include "ThirdParty/miniaudio.h"
