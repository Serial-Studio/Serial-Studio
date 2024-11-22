#-------------------------------------------------------------------
# CMake build support courtesy of A.Gembe
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-------------------------------------------------------------------

# - Try to find QsLog
# Once done, this will define
#
#  QsLog_FOUND - system has QsLog
#  QsLog_INCLUDE_DIRS - the QsLog include directories 
#  QsLog_LIBRARIES - link these to use QsLog
#  QsLog_BINARY_REL
#  QsLog_BINARY_DBG

include(FindPkgMacros)
findpkg_begin(QsLog)

# Get path, convert backslashes as ${ENV_${var}}
getenv_path(QsLog_HOME)

# construct search paths
set(QsLog_PREFIX_PATH ${QsLog_HOME} ${ENV_QsLog_HOME})

create_search_paths(QsLog)

# redo search if prefix path changed
clear_if_changed(
    QsLog_PREFIX_PATH
    QsLog_LIBRARY_REL
    QsLog_LIBRARY_DBG
    QsLog_BINARY_REL
    QsLog_BINARY_DBG
    QsLog_INCLUDE_DIR
)

set(QsLog_LIBRARY_NAMES QsLog)
get_debug_names(QsLog_LIBRARY_NAMES)

use_pkgconfig(QsLog_PKGC QsLog)

find_path(QsLog_INCLUDE_DIR NAMES QsLog.h HINTS ${QsLog_INC_SEARCH_PATH} ${QsLog_PKGC_INCLUDE_DIRS} PATH_SUFFIXES QsLog)
find_library(QsLog_LIBRARY_REL NAMES ${QsLog_LIBRARY_NAMES} HINTS ${QsLog_LIB_SEARCH_PATH} ${QsLog_PKGC_LIBRARY_DIRS} PATH_SUFFIXES "" release relwithdebinfo minsizerel)
find_library(QsLog_LIBRARY_DBG NAMES ${QsLog_LIBRARY_NAMES_DBG} HINTS ${QsLog_LIB_SEARCH_PATH} ${QsLog_PKGC_LIBRARY_DIRS} PATH_SUFFIXES "" debug)
find_file(QsLog_BINARY_REL NAMES QsLog.dll HINTS ${QsLog_PREFIX_PATH}/bin PATH_SUFFIXES "" release relwithdebinfo minsizerel)
find_file(QsLog_BINARY_DBG NAMES QsLog_d.dll HINTS ${QsLog_PREFIX_PATH}/bin PATH_SUFFIXES "" debug)
make_library_set(QsLog_LIBRARY)

findpkg_finish(QsLog)
add_parent_dir(QsLog_INCLUDE_DIRS QsLog_INCLUDE_DIR)
