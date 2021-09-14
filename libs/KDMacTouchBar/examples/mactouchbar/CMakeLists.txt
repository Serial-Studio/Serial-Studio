include(${QT_USE_FILE})

set(CMAKE_AUTORCC ON)
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_executable(mactouchbar MACOSX_BUNDLE
               main.cpp
               mainwindow.cpp
               mainwindow.h
               mactouchbar.qrc)

include_directories(../../src)

install(TARGETS mactouchbar
        DESTINATION "${CMAKE_INSTALL_PREFIX}/examples/mactouchbar")

target_link_libraries(mactouchbar KDMacTouchBar)
