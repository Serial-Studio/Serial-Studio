#
# Serial Studio
# https://serial-studio.com/
#
# Copyright (C) 2020–2026 Alex Spataru
#
# This file is dual-licensed:
#
# - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
# - Under the Serial Studio Commercial License for builds that include
#   any Pro functionality.
#
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
#

cmake_minimum_required(VERSION 3.20)

#---------------------------------------------------------------------------------------------------
# open62541 1.5.7, single-file distribution
#---------------------------------------------------------------------------------------------------
#
# open62541.c / open62541.h are the amalgamation assets published with the v1.5.7 release, dropped
# in verbatim. They are compiled here as one static library instead of adding upstream's CMake
# project, which brings four git submodules (one of which, deps/ua-nodeset, cannot even be checked
# out on a stock Windows: its paths exceed MAX_PATH), a FindMbedTLS module that find_library()s
# archives that do not exist yet at configure time, and unconditional install(EXPORT)/export()
# rules that reject any link dependency outside their export set. None of that buys us anything
# for a client that is statically linked into one executable.
#
# The published amalgamation is generated with UA_ENABLE_ENCRYPTION_MBEDTLS already defined (see
# the UA_ENABLE_* block at the top of open62541.h), so the security policies come from lib/mbedtls
# rather than from any library resolved on the target machine. That is the whole point of spec
# 0067: the OPC UA driver's presence and its capabilities are properties of THIS build.
#
# Upstream sources carry the local patches recorded in PATCHES.md. To update: replace both files
# from the release assets of the new tag, re-apply PATCHES.md, and update SS_OPEN62541_VERSION
# below.
#
#---------------------------------------------------------------------------------------------------

set(SS_OPEN62541_VERSION "1.5.7" CACHE INTERNAL "Vendored open62541 release")

enable_language(C)

add_library(open62541 STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/open62541.c
  ${CMAKE_CURRENT_SOURCE_DIR}/open62541.h
)

#
# SYSTEM keeps open62541.h's own diagnostics out of the translation units that include it: it
# redefines _WINSOCKAPI_ (Qt already defines it on the command line) and has an unreferenced
# parameter in one of its inline helpers. Those are upstream's to care about, not ours, and they
# would otherwise appear on every file that touches the driver.
#
target_include_directories(open62541 SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(open62541 PUBLIC mbedtls mbedx509 mbedcrypto)

if(MSVC)
  target_compile_options(open62541 PRIVATE /w)
else()
  target_compile_options(open62541 PRIVATE -w)
endif()

if(WIN32)
  target_link_libraries(open62541 PUBLIC ws2_32 iphlpapi)
endif()

#
# open62541.h defines _XOPEN_SOURCE 600 before the first POSIX header. On Apple platforms that
# makes <sys/socket.h> and friends hide every BSD extension the stack uses (SO_REUSEPORT,
# INADDR_LOOPBACK, struct ip_mreq, NI_MAXHOST, DT_REG), so the amalgamation does not compile.
# _DARWIN_C_SOURCE restores them; it is PUBLIC because OpcUaSession.cpp includes the same header
# and would otherwise trip the identical hiding.
#
if(APPLE)
  target_compile_definitions(open62541 PUBLIC _DARWIN_C_SOURCE)
endif()

#
# Links the stack into an application target. The BUILD_COMMERCIAL guard is what lets a GPL
# unit-test configure build open62541 for tst_opcua_marshal without any of it reaching the GPL
# binary: the target exists, the executable simply never links it.
#
function(target_link_open62541 TARGET)
  if(NOT TARGET open62541 OR NOT BUILD_COMMERCIAL)
    return()
  endif()

  target_compile_definitions(${TARGET} PRIVATE SS_OPCUA_ACTIVE=1)
  target_link_libraries(${TARGET} PRIVATE open62541)
endfunction()
