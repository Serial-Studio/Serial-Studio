set(QZTEST_SOURCES
        qztest.h
        testjlcompress.h
        testjlcp_compress.h
        testjlcp_extract.h
        testquachecksum32.h
        testquagzipfile.h
        testquaziodevice.h
        testquazip.h
        testquazipdir.h
        testquazipfile.h
        testquazipfileinfo.h
        testquazipnewinfo.h
        qztest.cpp
        testjlcompress.cpp
        testjlcp_compress.cpp
        testjlcp_extract.cpp
        testquachecksum32.cpp
        testquagzipfile.cpp
        testquaziodevice.cpp
        testquazip.cpp
        testquazipdir.cpp
        testquazipfile.cpp
        testquazipfileinfo.cpp
        testquazipnewinfo.cpp
)

add_executable(qztest ${QZTEST_SOURCES} qztest.qrc)
set_target_properties(qztest PROPERTIES AUTORCC ON)
target_include_directories(qztest PRIVATE ${QUAZIP_INC})

add_dependencies(qztest QuaZip::QuaZip)

target_link_libraries(qztest
    ${QUAZIP_TEST_QT_LIBRARIES}
    QuaZip::QuaZip
)

add_test(NAME qztest_all
	COMMAND qztest
	WORKING_DIRECTORY ${QUAZIP_BINARY_DIR}/quazip # preliminary hack to find the dll on windows
)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --verbose DEPENDS qztest)

if (WIN32 AND BUILD_SHARED_LIBS)
	message(STATUS "Setting up windeployqt")
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
	include(windeployqt)
	windeployqt(qztest)

    # This copies quazip1-qt6.dll .dll, zlib1.dll and bz2.dll.
    # For an unknown reason, bz2.dll is copied on second build only. :insert thinking emoji:
    add_custom_command(
        TARGET qztest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:qztest> $<TARGET_FILE_DIR:qztest>
        COMMAND_EXPAND_LISTS
    )
endif()