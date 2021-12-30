INCLUDEPATH += $$PWD/headers
INCLUDEPATH += $$PWD/fftreal

HEADERS += \
    $$PWD/fftreal/Array.h \
    $$PWD/fftreal/Array.hpp \
    $$PWD/fftreal/DynArray.h \
    $$PWD/fftreal/DynArray.hpp \
    $$PWD/fftreal/FFTReal.h \
    $$PWD/fftreal/FFTReal.hpp \
    $$PWD/fftreal/FFTRealFixLen.h \
    $$PWD/fftreal/FFTRealFixLen.hpp \
    $$PWD/fftreal/FFTRealFixLenParam.h \
    $$PWD/fftreal/FFTRealPassDirect.h \
    $$PWD/fftreal/FFTRealPassDirect.hpp \
    $$PWD/fftreal/FFTRealPassInverse.h \
    $$PWD/fftreal/FFTRealPassInverse.hpp \
    $$PWD/fftreal/FFTRealSelect.h \
    $$PWD/fftreal/FFTRealSelect.hpp \
    $$PWD/fftreal/FFTRealUseTrigo.h \
    $$PWD/fftreal/FFTRealUseTrigo.hpp \
    $$PWD/fftreal/OscSinCos.h \
    $$PWD/fftreal/OscSinCos.hpp \
    $$PWD/fftreal/def.h \

HEADERS += \
    $$PWD/headers/qcomplexnumber.h \
    $$PWD/headers/qfouriercalculator.h \
    $$PWD/headers/qfourierfixedcalculator.h \
    $$PWD/headers/qfouriertransformer.h \
    $$PWD/headers/qfouriervariablecalculator.h \
    $$PWD/headers/qwindowfunction.h

SOURCES += \
    $$PWD/sources/qcomplexnumber.cpp \
    $$PWD/sources/qfouriercalculator.cpp \
    $$PWD/sources/qfourierfixedcalculator.cpp \
    $$PWD/sources/qfouriertransformer.cpp \
    $$PWD/sources/qfouriervariablecalculator.cpp \
    $$PWD/sources/qwindowfunction.cpp
