cmake_minimum_required (VERSION 3.0)

project (qtcsv VERSION 1.5.0 LANGUAGES CXX)

# set options
option(STATIC_LIB "build as static lib if ON, otherwise build shared lib" OFF)
option(USE_QT4 "builds against Qt4 if ON, otherwise builds against Qt5" OFF)
option(BUILD_TESTS "build tests" ON)

# find qt package
if(USE_QT4)
    find_package(Qt4 REQUIRED)
    set(QT_CORE_TARGET Qt4::QtCore)
else()
    # if cmake failed to find Qt5Core configuration file, set path manually:
    #list(APPEND CMAKE_PREFIX_PATH "/path/to/Qt/lib/cmake/Qt5Core/")

    find_package(Qt5Core REQUIRED)
    set(QT_CORE_TARGET Qt5::Core)
endif(USE_QT4)

# instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# set list of source files
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/sources/*.cpp)

# Show all files in QtCreator. Starting with CMake 3.7 server-mode is used
# and QtCreator will show the files properly in an extra <Headers> section.
if(CMAKE_VERSION VERSION_LESS "3.7.0")
    file(GLOB_RECURSE QTCSV_ALL_FILES "*")
    add_custom_target(show_all_files_in_${PROJECT_NAME} SOURCES ${QTCSV_ALL_FILES})
endif()

if(STATIC_LIB)
    add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
    #propogate the QTCSV_STATIC_LIB define for all qtcsv consuming stuff
    #necessary for correct QTCSVSHARED_EXPORT handling, espacially for windows (cross compilation) builds
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DQTCSV_STATIC_LIB)
else()
    add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR})
endif(STATIC_LIB)

#use the QTCSV_LIBRARY define only for qtcsv build time to use export statements instead of import statements
target_compile_definitions(${PROJECT_NAME} PRIVATE -DQTCSV_LIBRARY)

# include root project folder as private, because the source headers are included with source/*.h
target_include_directories(${PROJECT_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include> PRIVATE .)

# set compiler flags for the library target
if (CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror -Wformat=2
        -Wuninitialized -Winit-self -Wswitch-enum -Wundef
        -Wpointer-arith -Wdisabled-optimization -Wcast-align -Wcast-qual)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE ${QT_CORE_TARGET})

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Config
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(DIRECTORY include DESTINATION .)

# create and install cmake package files
install(EXPORT ${PROJECT_NAME}Config DESTINATION share/${PROJECT_NAME}/cmake)
export(TARGETS ${PROJECT_NAME} FILE ${PROJECT_NAME}Config.cmake)

if(BUILD_TESTS)
    add_subdirectory(tests)
endif(BUILD_TESTS)
