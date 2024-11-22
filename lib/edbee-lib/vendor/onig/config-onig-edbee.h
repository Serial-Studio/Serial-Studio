#ifndef CONFIGONIGEDBEE_H
#define CONFIGONIGEDBEE_H

#ifdef __clang__
  //#pragma clang diagnostic ignored "-Wunused-variable"
  #pragma clang diagnostic ignored "-Wunused-parameter"
  #pragma clang diagnostic ignored "-Wmissing-field-initializers"

#else
  #ifdef  __GNUC__
  //#pragma GCC diagnostic ignored "-Wunused-variable"
  #pragma GCC diagnostic ignored "-Wunused-parameter"
  #pragma GCC diagnostic ignored "-Wmissing-field-initializers"
  #endif
#endif

#ifdef _MSC_VER
  // Visual studio directives
  // C4100: unreferenced formal parameter
  #pragma warning( disable : 4100 )
#endif

// TODO: Add more visual studio and other compiler directives

#endif // CONFIGONIGEDBEE_H
