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
# SerialStudioDevices -- the driver layer (spec 0076)
#---------------------------------------------------------------------------------------------------
#
# Every data source and the machinery that opens one: the HAL drivers, DeviceManager,
# ConnectionManager and its sub-objects, the async TCP dial, the file-transmission facade, and the
# MQTT publisher stack.
#
# The pipeline-side pieces of IO/ (FrameReader, PipelineHost, StreamWorker, FrameConfig.h) are NOT
# here: they live in SerialStudio::Pipeline, because they run on the processing thread and know
# nothing about a bus.
#
# Headers ride along with their sources so AUTOMOC sees the Q_OBJECT classes; the same entries were
# removed from the executable's HEADERS list, or moc would run twice and the link would fail on
# duplicate staticMetaObject symbols.
#
# A strict layer since spec 0077: it includes only itself and Core and Protocols, and the include
# roots and links below say exactly that. scripts/layer-verify.py fails on an include, an
# include root or a link outside that graph.
#
#---------------------------------------------------------------------------------------------------

qt_add_library(
  SerialStudioDevices STATIC

  IO/Drivers/Network.cpp
  IO/Drivers/Network/NetworkTcp.cpp
  IO/Drivers/Network/NetworkUdp.cpp
  IO/Drivers/Network/NetworkWebSocket.cpp
  IO/Drivers/Network/NetworkHttp.cpp
  IO/Drivers/UART.cpp
  IO/Drivers/BluetoothLE.cpp
  IO/Drivers/BluetoothLE/BleUuids.cpp
  IO/Drivers/SerialPortIdentity.cpp
  IO/DeviceManager.cpp
  IO/Drivers/GeneratedProjectRequest.cpp
  IO/ConnectionManager.cpp
  IO/ConnectionManager/DriverUiRegistry.cpp
  IO/ConnectionManager/ConnectFanOut.cpp
  IO/ConnectionManager/ReplyCapture.cpp
  IO/ConnectionManager/DeviceIoRouter.cpp
  IO/ConnectionManager/DeviceTableQuery.cpp
  IO/ConnectionManager/DriverFactory.cpp
  IO/ConnectionManager/StreamConfigBuilder.cpp
  IO/ConnectionManager/UiDriverSync.cpp
  IO/ConnectionManager/BusBridge.cpp
  IO/AsyncTcpDial.cpp
  IO/FileTransmission.cpp
  IO/Drivers/UART.h
  IO/Drivers/UART/UartPolicy.h
  IO/Drivers/Network.h
  IO/Drivers/BluetoothLE.h
  IO/DeviceManager.h
  IO/Drivers/GeneratedProjectRequest.h
  IO/ConnectionManager.h
  IO/ConnectionManager/DriverUiRegistry.h
  IO/ConnectionManager/ConnectFanOut.h
  IO/ConnectionManager/ReplyCapture.h
  IO/ConnectionManager/DeviceIoRouter.h
  IO/ConnectionManager/DeviceTableQuery.h
  IO/ConnectionManager/DriverFactory.h
  IO/ConnectionManager/StreamConfigBuilder.h
  IO/ConnectionManager/UiDriverSync.h
  IO/ConnectionManager/BusBridge.h
  IO/AsyncTcpDial.h
  IO/FileTransmission.h
  IO/Drivers/BluetoothLE/BleUuids.h
  IO/Drivers/SerialPortIdentity.h
)

add_library(SerialStudio::Devices ALIAS SerialStudioDevices)

#---------------------------------------------------------------------------------------------------
# Pro drivers
#---------------------------------------------------------------------------------------------------

if(BUILD_COMMERCIAL)
  target_sources(
    SerialStudioDevices PRIVATE

    IO/Drivers/Audio.h
    IO/Drivers/Audio/AudioDeviceCatalog.h
    IO/Drivers/Audio/AudioPcm.h
    IO/Drivers/Audio/PlaybackRing.h
    ThirdParty/miniaudio.h
    IO/Drivers/CANBus.h
    IO/Drivers/CANBus/CanBackends.h
    IO/Drivers/CANBus/GsUsbCanBackend.h
    IO/Drivers/CANBus/SlcanBackend.h
    IO/Drivers/CANBus/SeeedCanBackend.h
    IO/Drivers/CANBus/SerialCanBackendBase.h
    IO/Drivers/Modbus.h
    IO/Drivers/Modbus/ModbusRegisterGroups.h
  IO/Drivers/Modbus/ModbusWritePayload.h
    IO/Drivers/Modbus/ModbusProjectGenerator.h
    IO/Drivers/HID.h
    IO/Drivers/USB.h
    IO/Drivers/USB/UsbHex.h
    IO/Drivers/USB/UsbTransferPump.h
    IO/Drivers/Process.h
    IO/Drivers/MQTT.h
    IO/Drivers/MQTT/SparkplugSession.h
    IO/Drivers/OpcUa.h
    IO/Drivers/OpcUa/OpcUaBrowser.h
    IO/Drivers/OpcUa/OpcUaCertificateStore.h
    IO/Drivers/OpcUa/OpcUaEndpointSelection.h
    IO/Drivers/OpcUa/OpcUaFrameAssembler.h
    IO/Drivers/OpcUa/OpcUaProjectBuilder.h
    IO/Drivers/OpcUa/OpcUaSubscriptions.h
    IO/Drivers/OpcUa/OpcUaTag.h
    IO/Drivers/OpcUaTypes.h
    IO/Drivers/OpcUaMarshal.h
    IO/Drivers/OpcUaSession.h
    IO/Drivers/OpcUaSecurity.h
    IO/Drivers/OpcUaTagModel.h
    IO/Drivers/PolledPlcWorkerBase.h
    IO/Drivers/S7.h
    IO/Drivers/EthernetIp.h
    IO/Drivers/Iec104.h
    IO/Drivers/Audio.cpp
    IO/Drivers/Audio/AudioDeviceCatalog.cpp
    IO/Drivers/Audio/AudioPcm.cpp
    ThirdParty/miniaudio.cpp
    IO/Drivers/CANBus.cpp
    IO/Drivers/CANBus/CanBackends.cpp
    IO/Drivers/CANBus/GsUsbCanBackend.cpp
    IO/Drivers/CANBus/SlcanBackend.cpp
    IO/Drivers/CANBus/SeeedCanBackend.cpp
    IO/Drivers/CANBus/SerialCanBackendBase.cpp
    IO/Drivers/Modbus.cpp
    IO/Drivers/Modbus/ModbusRegisterGroups.cpp
  IO/Drivers/Modbus/ModbusWritePayload.cpp
    IO/Drivers/Modbus/ModbusProjectGenerator.cpp
    IO/Drivers/HID.cpp
    IO/Drivers/USB.cpp
    IO/Drivers/USB/UsbHex.cpp
    IO/Drivers/USB/UsbTransferPump.cpp
    IO/Drivers/Process.cpp
    IO/Drivers/MQTT.cpp
    IO/Drivers/MQTT/MQTTSparkplug.cpp
    IO/Drivers/MQTT/SparkplugSession.cpp
    IO/Drivers/OpcUa.cpp
    IO/Drivers/OpcUa/OpcUaBrowser.cpp
    IO/Drivers/OpcUa/OpcUaCertificateStore.cpp
    IO/Drivers/OpcUa/OpcUaEndpointSelection.cpp
    IO/Drivers/OpcUa/OpcUaFrameAssembler.cpp
    IO/Drivers/OpcUa/OpcUaProjectBuilder.cpp
    IO/Drivers/OpcUa/OpcUaSubscriptions.cpp
    IO/Drivers/OpcUa/OpcUaTag.cpp
    IO/Drivers/OpcUaMarshal.cpp
    IO/Drivers/OpcUaSession.cpp
    IO/Drivers/OpcUaSecurity.cpp
    IO/Drivers/OpcUaTagModel.cpp
    IO/Drivers/PolledPlcWorkerBase.cpp
    IO/Drivers/S7.cpp
    IO/Drivers/EthernetIp.cpp
    IO/Drivers/Iec104.cpp
  )
endif()

#---------------------------------------------------------------------------------------------------
# Include roots -- the layer graph
#---------------------------------------------------------------------------------------------------
#
# This partition keeps its own relative include root plus the roots of the layers below it that
# it may include (spec 0077). Nothing from app/src and nothing sideways: scripts/layer-verify.py
# fails on an include outside the graph and on a root or a link this list does not justify. The
# commercial token's generated guards header arrives through SerialStudio::Core.
#
#---------------------------------------------------------------------------------------------------

target_include_directories(
  SerialStudioDevices PUBLIC

  ${CMAKE_SOURCE_DIR}/core/Devices
)

#---------------------------------------------------------------------------------------------------
# Link set
#---------------------------------------------------------------------------------------------------
#
# luajit and QCodeEditor are the MQTT publisher script and its embedded editor; open62541 and
# libplctag are the OPC UA and EtherNet/IP stacks, linked through the same helpers the executable
# calls (both are no-ops on a configure that carries no such stack, and both add the SS_*_ACTIVE
# define the drivers compile against).
#
#---------------------------------------------------------------------------------------------------

target_link_libraries(
  SerialStudioDevices PUBLIC

  ${QT_LIBS}

  SerialStudio::Core
  SerialStudio::Protocols

  luajit
  QCodeEditor
)

target_link_open62541(SerialStudioDevices)
target_link_libplctag(SerialStudioDevices)

if(BUILD_COMMERCIAL)
  target_link_libraries(SerialStudioDevices PUBLIC usb-1.0)
  target_link_libraries(SerialStudioDevices PUBLIC hidapi::hidapi)
endif()

# The Network driver's TCP reachability probe calls socket/connect/WSAPoll/getaddrinfo directly
# rather than through QTcpSocket, so it needs ws2_32 explicitly: Qt keeps it as a private
# dependency and does not propagate it to consumers.
if(WIN32)
  target_link_libraries(SerialStudioDevices PRIVATE ws2_32)
endif()

# IO/Drivers/Audio.h and its helpers include ThirdParty/miniaudio.h, whose struct layouts depend on
# the MA_* feature switches: every archive that sees the header must configure it identically or
# two of them disagree about the size of a ma_device (the function lives in core/CMakeLists.txt so
# Pipeline, added earlier, can call it too). This is the only target that compiles miniaudio's
# implementation, so the vendored .cpp's warning suppressions and the ALSA link live here as well.
if(BUILD_COMMERCIAL)
  ss_apply_miniaudio_definitions(SerialStudioDevices)

  if(MSVC)
    set_source_files_properties(ThirdParty/miniaudio.cpp
      PROPERTIES COMPILE_OPTIONS "/wd4505"
    )
  else()
    set_source_files_properties(ThirdParty/miniaudio.cpp
      PROPERTIES COMPILE_OPTIONS "-Wno-unused-function"
    )
  endif()

  if(LINUX)
    target_link_libraries(SerialStudioDevices PRIVATE asound)
  endif()
endif()

if(MSVC)
  target_compile_options(SerialStudioDevices PRIVATE /W4)
endif()

if(BUILD_COMMERCIAL AND PRODUCTION_OPTIMIZATION)
  set_target_properties(SerialStudioDevices PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
  )
endif()

if(SS_UNITY_BUILD)
  set_target_properties(SerialStudioDevices PROPERTIES
    UNITY_BUILD ON
    UNITY_BUILD_BATCH_SIZE ${SS_UNITY_BATCH_SIZE}
  )

  if(MSVC)
    target_compile_options(SerialStudioDevices PRIVATE /bigobj)
  endif()

  # ~90k-line vendored TU full of file-local symbols. Its per-source COMPILE_OPTIONS already opt it
  # out of unity; the pin keeps a warning-flag cleanup from silently pulling it into a batch.
  if(BUILD_COMMERCIAL)
    set_source_files_properties(ThirdParty/miniaudio.cpp
      PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON
    )
  endif()
endif()

if(COMMAND serial_studio_harden)
  serial_studio_harden(SerialStudioDevices)
endif()
