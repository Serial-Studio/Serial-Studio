set(PROJECTNAME QsLogUnitTest)
project(${PROJECTNAME})

if(BUILD_WITH_QT5)
    find_package(Qt5 REQUIRED COMPONENTS Test)
    set(QT_LIBRARIES Qt5::Core Qt5::Test)
else()
    find_package(Qt6 REQUIRED COMPONENTS Test)
    set(QT_LIBRARIES Qt6::Core Qt6::Test)
endif()

include_directories(${QsLogUnitTest_SOURCE_DIR} ${QsLog_SOURCE_DIR})

if(QS_LOG_IS_SHARED_LIBRARY)
	# Ugh, ugly hack
	add_definitions(-UQSLOG_IS_SHARED_LIBRARY)
	add_definitions(-DQSLOG_IS_SHARED_LIBRARY_IMPORT)
endif()

set(HEADER_FILES
	TestLog.h
	QtTestUtil/QtTestUtil.h
	QtTestUtil/TestRegistration.h
	QtTestUtil/TestRegistry.h
	)

set(SOURCE_FILES
	TestLog.cpp
	QtTestUtil/SimpleChecker.cpp
	QtTestUtil/TestRegistry.cpp
	)

add_executable(${PROJECTNAME} ${SOURCE_FILES} ${HEADER_FILES})

set_target_properties(${PROJECTNAME} PROPERTIES AUTOMOC ON)

target_link_libraries(${PROJECTNAME} QsLog ${QT_LIBRARIES})
