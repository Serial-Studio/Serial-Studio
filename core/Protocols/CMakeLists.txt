#
# Serial Studio
# https://serial-studio.com/
#
# Copyright (C) 2020–2025 Alex Spataru
#
# This file is dual-licensed:
#
# - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
# - Under the Serial Studio Commercial License for builds that include
#   any Pro functionality.
#
# You must comply with the terms of one of these licenses, depending
# on your use case.
#
# For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
# For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
#
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial

#---------------------------------------------------------------------------------------------------
# SerialStudioProtocols -- the wire-format layer (spec 0076)
#---------------------------------------------------------------------------------------------------
#
# Pure codecs: bytes in, typed results out. No sockets, no dialogs, no settings, no singletons --
# which is why every one of them is already covered by a unit or fuzz target that now links this
# archive instead of recompiling the same translation unit.
#
# Headers ride along with the sources so AUTOMOC sees the Q_OBJECT classes of the file-transfer
# protocols, and for the same reason none of them may stay in the executable's HEADERS list.
#
# Qt6::Core is the whole link set: the CAN, S7, IEC 104, Sparkplug and Modbus codecs speak bytes,
# not QtSerialBus, QtMqtt or QtNetwork -- those live with the drivers, in app/src.
#
#---------------------------------------------------------------------------------------------------

qt_add_library(
  SerialStudioProtocols STATIC

  FileTransfer/CRC.h
  FileTransfer/Protocol.h
  FileTransfer/XMODEM.h
  FileTransfer/XMODEM.cpp
  FileTransfer/YMODEM.h
  FileTransfer/YMODEM.cpp
  FileTransfer/ZMODEM.h
  FileTransfer/ZMODEM.cpp
)

add_library(SerialStudio::Protocols ALIAS SerialStudioProtocols)

#---------------------------------------------------------------------------------------------------
# Pro codecs
#---------------------------------------------------------------------------------------------------
#
# The open62541 precedent (lib/CMakeLists.txt): built on a GPL configure too when SS_BUILD_TESTS is
# on, because the unit and fuzz tier that proves these codecs is a GPL configure in CI. Building
# the objects does not put them in the GPL application -- a static archive contributes only the
# objects something references, and nothing in a GPL build references these symbols.
#
#---------------------------------------------------------------------------------------------------

if(BUILD_COMMERCIAL OR SS_BUILD_TESTS)
  target_sources(
    SerialStudioProtocols PRIVATE

    CAN/CanReassembly.h
    CAN/CanReassembly.cpp
    CAN/GsUsbProtocol.h
    S7/IsoTsap.h
    S7/IsoTsap.cpp
    S7/S7Pdu.h
    S7/S7Pdu.cpp
    S7/S7Address.h
    S7/S7Address.cpp
    Iec104/Apci.h
    Iec104/Apci.cpp
    Iec104/Asdu.h
    Iec104/Asdu.cpp
    Sparkplug/SparkplugPayload.h
    Sparkplug/SparkplugPayload.cpp
    Modbus/ModbusRtuCodec.h
    Modbus/ModbusRtuCodec.cpp
  )
endif()

target_include_directories(SerialStudioProtocols PUBLIC ${CMAKE_SOURCE_DIR}/core)
target_link_libraries(SerialStudioProtocols PUBLIC SerialStudio::Core)

if(COMMAND serial_studio_harden)
  serial_studio_harden(SerialStudioProtocols)
endif()
