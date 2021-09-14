include(${QT_USE_FILE})

set(HEADERS kdmactouchbar.h
            kdmactouchbar_global.h)

add_definitions(-DKDMACTOUCHBAR_BUILD_KDMACTOUCHBAR_LIB -DQT_NO_CAST_TO_ASCII -DQT_ASCII_CAST_WARNING)

add_library(KDMacTouchBar SHARED
            kdmactouchbar.mm
            ${HEADERS})

install(TARGETS KDMacTouchBar
        LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")

target_link_libraries(KDMacTouchBar ${QT_LIBRARIES} "-framework Cocoa")

set_target_properties(KDMacTouchBar PROPERTIES
    FRAMEWORK TRUE
    FRAMEWORK_VERSION 1
    MACOSX_FRAMEWORK_IDENTIFIER com.kdab.KDMacTouchBar
    PUBLIC_HEADER "${HEADERS}"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
    MACOSX_RPATH TRUE
)
