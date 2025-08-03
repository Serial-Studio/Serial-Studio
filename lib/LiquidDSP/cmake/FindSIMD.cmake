INCLUDE(CheckCSourceRuns)
INCLUDE(CheckCSourceCompiles)
INCLUDE(CheckCXXSourceRuns)

SET(SSE4_CODE "
  #include <immintrin.h>
  int main()
  {
    float _v[8] = { 0.f, 1.f, 2.f, 3.f,};
    float _h[8] = { 1.f,-1.f, 1.f,-1.f,};
    __m128 v = _mm_loadu_ps(_v);
    __m128 h = _mm_loadu_ps(_h);
    __m128 s = _mm_mul_ps(v, h);
    // unload packed array
    float w[4];
    _mm_storeu_ps(w, s);
    return (w[0]== 0.f && w[1]== -1.f && w[2]== 2.f && w[3]== -3.f) ? 0 : 1;
  }
")

# TODO: distinguish between AVX, FMA, AVX2
SET(AVX2_CODE "
  #include <immintrin.h>
  int main()
  {
    float _v[8] = { 0.f, 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f,};
    float _h[8] = { 1.f,-1.f, 1.f,-1.f, 1.f,-1.f, 1.f,-1.f,};
    __m256 v = _mm256_loadu_ps(_v);
    __m256 h = _mm256_loadu_ps(_h);
    __m256 s = _mm256_mul_ps(v, h);
    // unload packed array
    float w[4];
    _mm256_storeu_ps(w, s);
    return (w[ 0]== 0.f && w[ 1]== -1.f && w[ 2]== 2.f && w[ 3]== -3.f &&
            w[ 4]== 4.f && w[ 5]== -5.f && w[ 6]== 6.f && w[ 7]== -7.f) ? 0 : 1;
  }
")

SET(AVX512_CODE "
  #include <immintrin.h>
  int main()
  {
    float _v[16] = { 0.f, 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f,10.f,11.f,12.f,13.f,14.f,15.f,};
    float _h[16] = { 1.f,-1.f, 1.f,-1.f, 1.f,-1.f, 1.f,-1.f, 1.f,-1.f, 1.f,-1.f, 1.f,-1.f, 1.f,-1.f,};
    __m512 v = _mm512_loadu_ps(_v);
    __m512 h = _mm512_loadu_ps(_h);
    __m512 s = _mm512_mul_ps(v, h);
    // unload packed array
    float w[4];
    _mm512_storeu_ps(w, s);
    return (w[ 0]== 0.f && w[ 1]== -1.f && w[ 2]== 2.f && w[ 3]== -3.f &&
            w[ 4]== 4.f && w[ 5]== -5.f && w[ 6]== 6.f && w[ 7]== -7.f &&
            w[ 8]== 8.f && w[ 9]== -9.f && w[10]==10.f && w[11]==-11.f &&
            w[12]==12.f && w[13]==-13.f && w[14]==14.f && w[15]==-15.f) ? 0 : 1;
  }
")

SET(NEON_CODE "
  #include <arm_neon.h>
  int main()
  {
    float _v[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    float _h[4] = {2.0f, 4.0f, 6.0f, 8.0f};
    float32x4_t v = vld1q_f32(_v);
    float32x4_t h = vld1q_f32(_h);
    float32x4_t s = vmulq_f32(h,v);

    // unload packed array
    float w[4];
    vst1q_f32(w, s);
    return (w[0] == 2.0f && w[1] == 8.0f && w[2] == 18.0f && w[3] == 32.0f) ? 0 : -1;
  }
")

# NOTE: altivec is extremely old at this point; I'm not sure if this code is even correct
SET(ALTIVEC_CODE "
  int main()
  {
    vector float v = {1.0f, 2.0f, 3.0f, 4.0f};
    vector float h = {2.0f, 4.0f, 6.0f, 8.0f};
    vector float z = (vector float)(0);
    union { vector float r; float w[4]; } s;
    s.r = vec_madd(v, h, z);

    // unload packed array
    return (s.w[0]== 0.f && s.w[1]== -1.f && s.w[2]== 2.f && s.w[3]== -3.f) ? 0 : 1;
  }
")

MACRO(CHECK_SIMD lang type flags)
  SET(__FLAG_I 1)
  SET(CMAKE_REQUIRED_FLAGS_SAVE ${CMAKE_REQUIRED_FLAGS})
  FOREACH(__FLAG ${flags})
    #message("testing ${lang} ${type} ${__FLAG}")
    IF(NOT ${lang}_${type}_FOUND)
      SET(CMAKE_REQUIRED_FLAGS ${__FLAG})
      # TODO: check that program runs and returns proper exit code 0
      IF(lang STREQUAL "CXX")
        CHECK_CXX_SOURCE_RUNS("${${type}_CODE}" ${lang}_HAS_${type}_${__FLAG_I})
      ELSE()
        CHECK_C_SOURCE_RUNS("${${type}_CODE}" ${lang}_HAS_${type}_${__FLAG_I})
      ENDIF()
      IF(${lang}_HAS_${type}_${__FLAG_I})
        SET(${lang}_${type}_FOUND TRUE CACHE BOOL "${lang} ${type} support")
        SET(${lang}_${type}_FLAGS "${__FLAG}" CACHE STRING "${lang} ${type} flags")
      ENDIF()
      MATH(EXPR __FLAG_I "${__FLAG_I}+1")
    ENDIF()
  ENDFOREACH()
  SET(CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS_SAVE})

  IF(NOT ${lang}_${type}_FOUND)
    SET(${lang}_${type}_FOUND FALSE CACHE BOOL "${lang} ${type} support")
    SET(${lang}_${type}_FLAGS "" CACHE STRING "${lang} ${type} flags")
  ENDIF()

  SEPARATE_ARGUMENTS(${lang}_${type}_FLAGS)
  MARK_AS_ADVANCED(${lang}_${type}_FOUND ${lang}_${type}_FLAGS)

ENDMACRO()

# TODO: check msvc arch flags

CHECK_SIMD(C "SSE4" " ;-msse4.2;/arch:SSE")
CHECK_SIMD(C "NEON" " ;-ffast-math;/arch:armv8.0;/arch:armv9.0")
CHECK_SIMD(C "ALTIVEC" " ;-fno-common -faltivec;/arch:altivec")

CHECK_SIMD(CXX "SSE4" " ;-msse4.2;/arch:SSE")
CHECK_SIMD(CXX "NEON" " ;-ffast-math;/arch:armv8.0;/arch:armv9.0")
CHECK_SIMD(CXX "ALTIVEC" " ;-fno-common -faltivec;/arch:altivec")
