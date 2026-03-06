project(QuaZip_Library VERSION ${QUAZIP_LIB_VERSION})

include(GNUInstallDirs) # configurable CMAKE_INSTALL_*DIR

cmake_minimum_required(VERSION 3.13)

set(QUAZIP_HEADERS
        JlCompress.h
        ioapi.h
        minizip_crypt.h
        quaadler32.h
        quachecksum32.h
        quacrc32.h
        quagzipfile.h
        quaziodevice.h
        quazip.h
        quazip_global.h
        quazip_qt_compat.h
        quazipdir.h
        quazipfile.h
        quazipfileinfo.h
        quazipnewinfo.h
        unzip.h
        zip.h
   )

set(QUAZIP_SOURCES
        ${QUAZIP_HEADERS}
        unzip.c
        zip.c
        JlCompress.cpp
        qioapi.cpp
        quaadler32.cpp
        quachecksum32.cpp
        quacrc32.cpp
        quagzipfile.cpp
        quaziodevice.cpp
        quazip.cpp
        quazipdir.cpp
        quazipfile.cpp
        quazipfileinfo.cpp
        quazipnewinfo.cpp
   )

set(QUAZIP_INCLUDE_PATH ${QUAZIP_DIR_NAME}/quazip)
set(QUAZIP_INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake)
set(QUAZIP_PKGCONFIG_NAME quazip${QuaZip_VERSION_MAJOR}-qt${QUAZIP_QT_MAJOR_VERSION})

if(EMSCRIPTEN)
    set(BUILD_SHARED_LIBS OFF)
endif()

add_library(${QUAZIP_LIB_TARGET_NAME} ${QUAZIP_SOURCES})
add_library(QuaZip::QuaZip ALIAS ${QUAZIP_LIB_TARGET_NAME})

set_target_properties(${QUAZIP_LIB_TARGET_NAME} PROPERTIES
	VERSION ${QUAZIP_LIB_VERSION}
	SOVERSION ${QUAZIP_LIB_SOVERSION}
	OUTPUT_NAME ${QUAZIP_LIB_FILE_NAME}
	PUBLIC_HEADER "${QUAZIP_HEADERS}"
)
target_include_directories(${QUAZIP_LIB_TARGET_NAME} PRIVATE ${QUAZIP_INC})
target_include_directories(${QUAZIP_LIB_TARGET_NAME} PUBLIC
	$<BUILD_INTERFACE:${QUAZIP_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${QUAZIP_DIR_NAME}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${QUAZIP_INCLUDE_PATH}>
)
target_link_libraries(${QUAZIP_LIB_TARGET_NAME} ${QUAZIP_LIB_LIBRARIES} ${QUAZIP_LIB})
if(BUILD_SHARED_LIBS)
	target_compile_definitions(${QUAZIP_LIB_TARGET_NAME} PRIVATE QUAZIP_BUILD) # dllexport
else()
	target_compile_definitions(${QUAZIP_LIB_TARGET_NAME} PUBLIC QUAZIP_STATIC) # suppress dllimport
endif()
target_link_directories(${QUAZIP_LIB_TARGET_NAME} PUBLIC ${QUAZIP_LBD})

include(CMakePackageConfigHelpers)

if(QUAZIP_INSTALL)
    if(BUILD_SHARED_LIBS)
        set(Flavor Shared)
    else()
        set(Flavor Static)
    endif()
    set(QUAZIP_EXPORT_SET ${QUAZIP_PACKAGE_NAME}_${Flavor}Targets)
    write_basic_package_version_file(${PROJECT_BINARY_DIR}/${QUAZIP_PACKAGE_NAME}ConfigVersion.cmake
        COMPATIBILITY SameMajorVersion
    )
    configure_package_config_file(QuaZipConfig.cmake.in ${QUAZIP_PACKAGE_NAME}Config.cmake
        INSTALL_DESTINATION ${QUAZIP_INSTALL_CONFIGDIR}/${QUAZIP_DIR_NAME}
    )
    install(TARGETS ${QUAZIP_LIB_TARGET_NAME} ${QUAZIP_DEP}
        EXPORT ${QUAZIP_EXPORT_SET}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${QUAZIP_INCLUDE_PATH}
    )
    install(EXPORT ${QUAZIP_EXPORT_SET}
        NAMESPACE QuaZip::
        DESTINATION ${QUAZIP_INSTALL_CONFIGDIR}/${QUAZIP_DIR_NAME}
    )
    install(FILES ${PROJECT_BINARY_DIR}/${QUAZIP_PACKAGE_NAME}Config.cmake
                  ${PROJECT_BINARY_DIR}/${QUAZIP_PACKAGE_NAME}ConfigVersion.cmake
        DESTINATION ${QUAZIP_INSTALL_CONFIGDIR}/${QUAZIP_DIR_NAME}
    )

    configure_file(quazip.pc.cmakein ${QUAZIP_PKGCONFIG_NAME}.pc @ONLY)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${QUAZIP_PKGCONFIG_NAME}.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
endif()
