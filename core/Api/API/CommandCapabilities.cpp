/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "API/CommandCapabilities.h"

#include <algorithm>
#include <iterator>

#include "Core/DataModel/FrameKeys.h"
#include "Core/SerialStudio.h"

using namespace API::CommandCapabilities;

//--------------------------------------------------------------------------------------------------
// Classification (both tables sorted for binary search; together they hold every command once)
//--------------------------------------------------------------------------------------------------

// clang-format off
static constexpr Entry kCapable[] = {
  {QLatin1StringView("assistant.project.bulkApply"), Dispatches, NoCapability},
  {QLatin1StringView("assistant.restore"), LoadsProject, NoCapability},
  {QLatin1StringView("assistant.script.apply"), RunsInertCode | Dispatches, NoCapability},
  {QLatin1StringView("assistant.script.dryRun"), RunsInertCode | Dispatches, NoCapability},
  {QLatin1StringView("assistant.workspace.addTile"), Dispatches, NoCapability},
  {QLatin1StringView("console.send"), WritesDevice, NoCapability},
  {QLatin1StringView("controlScript.dryRun"), RunsInertCode, NoCapability},
  {QLatin1StringView("controlScript.set"), InstallsScript, NoCapability},
  {QLatin1StringView("controlScript.setCode"), InstallsScript, NoCapability},
  {QLatin1StringView("extensions.addRepository"), InstallsScript, NoCapability},
  {QLatin1StringView("extensions.install"), InstallsScript, NoCapability},
  {QLatin1StringView("io.ble.writeCharacteristic"), WritesDevice, NoCapability},
  {QLatin1StringView("io.connect"), StartsProgram, NoCapability},
  {QLatin1StringView("io.process.setArguments"), ChoosesProgram, NoCapability},
  {QLatin1StringView("io.process.setExecutable"), ChoosesProgram, NoCapability},
  {QLatin1StringView("io.process.setMode"), ChoosesProgram, NoCapability},
  {QLatin1StringView("io.process.setWorkingDir"), ChoosesProgram, NoCapability},
  {QLatin1StringView("io.setBusType"), NoCapability, ChoosesProgram},
  {QLatin1StringView("io.writeData"), WritesDevice, NoCapability},
  {QLatin1StringView("project.action.duplicate"), WritesDevice, NoCapability},
  {QLatin1StringView("project.action.update"), NoCapability, WritesDevice},
  {QLatin1StringView("project.batch"), Dispatches, NoCapability},
  {QLatin1StringView("project.dataset.setTransformCode"), InstallsScript, NoCapability},
  {QLatin1StringView("project.dataset.transform.dryRun"), RunsInertCode, NoCapability},
  {QLatin1StringView("project.dataset.update"), NoCapability, InstallsScript},
  {QLatin1StringView("project.dryRun.endToEnd"), RunsInertCode, NoCapability},
  {QLatin1StringView("project.frameParser.dryCompile"), RunsInertCode, NoCapability},
  {QLatin1StringView("project.frameParser.dryRun"), RunsInertCode, NoCapability},
  {QLatin1StringView("project.frameParser.setCode"), InstallsScript, NoCapability},
  {QLatin1StringView("project.group.update"), NoCapability, InstallsScript},
  {QLatin1StringView("project.loadJson"), LoadsProject, NoCapability},
  {QLatin1StringView("project.open"), LoadsProject, NoCapability},
  {QLatin1StringView("project.outputWidget.dryRun"), RunsInertCode, NoCapability},
  {QLatin1StringView("project.outputWidget.update"), NoCapability, InstallsScript},
  {QLatin1StringView("project.painter.dryRun"), RunsInertCode, NoCapability},
  {QLatin1StringView("project.painter.setCode"), InstallsScript, NoCapability},
  {QLatin1StringView("project.source.setFrameParserCode"), InstallsScript, NoCapability},
  {QLatin1StringView("project.source.setProperties"), NoCapability, ChoosesProgram},
  {QLatin1StringView("project.source.setProperty"), NoCapability, ChoosesProgram},
  {QLatin1StringView("project.source.update"), NoCapability, ChoosesProgram},
  {QLatin1StringView("project.template.apply"), LoadsProject, NoCapability},
  {QLatin1StringView("project.transformLibrary.dryRun"), RunsInertCode, NoCapability},
  {QLatin1StringView("project.transformLibrary.set"), InstallsScript, NoCapability},
  {QLatin1StringView("project.validate"), RunsInertCode, NoCapability},
  {QLatin1StringView("sessions.regress"), LoadsProject, NoCapability},
  {QLatin1StringView("sessions.replay"), LoadsProject, NoCapability},
  {QLatin1StringView("sessions.verify"), LoadsProject, NoCapability},
  {QLatin1StringView("system.exec"), ControlScriptOnly, NoCapability},
  {QLatin1StringView("system.kill"), ControlScriptOnly, NoCapability},
  {QLatin1StringView("system.runningProcesses"), ControlScriptOnly, NoCapability},
};

static constexpr QLatin1StringView kPlain[] = {
  QLatin1StringView("alarms.acknowledge"),
  QLatin1StringView("alarms.clear"),
  QLatin1StringView("alarms.getProjectSounds"),
  QLatin1StringView("alarms.reset"),
  QLatin1StringView("alarms.setMuted"),
  QLatin1StringView("alarms.setProjectSounds"),
  QLatin1StringView("alarms.silence"),
  QLatin1StringView("alarms.state"),
  QLatin1StringView("alarms.test"),
  QLatin1StringView("api.getCommands"),
  QLatin1StringView("assistant.checkpoint"),
  QLatin1StringView("assistant.dataset.resolve"),
  QLatin1StringView("assistant.listCheckpoints"),
  QLatin1StringView("assistant.memory.propose"),
  QLatin1StringView("assistant.snapshot"),
  QLatin1StringView("assistant.workspace.plan"),
  QLatin1StringView("assistant.workspace.resolve"),
  QLatin1StringView("console.clear"),
  QLatin1StringView("console.getConfig"),
  QLatin1StringView("console.setAnsiColorsEnabled"),
  QLatin1StringView("console.setChecksumMethod"),
  QLatin1StringView("console.setCollapseDuplicates"),
  QLatin1StringView("console.setDataMode"),
  QLatin1StringView("console.setDisplayMode"),
  QLatin1StringView("console.setEcho"),
  QLatin1StringView("console.setEncoding"),
  QLatin1StringView("console.setFontFamily"),
  QLatin1StringView("console.setFontSize"),
  QLatin1StringView("console.setLineEnding"),
  QLatin1StringView("console.setSearchCaseSensitive"),
  QLatin1StringView("console.setShowTimestamp"),
  QLatin1StringView("console.setVt100Emulation"),
  QLatin1StringView("consoleExport.close"),
  QLatin1StringView("consoleExport.getStatus"),
  QLatin1StringView("consoleExport.setEnabled"),
  QLatin1StringView("controlScript.get"),
  QLatin1StringView("controlScript.getCode"),
  QLatin1StringView("controlScript.getStatus"),
  QLatin1StringView("csvExport.close"),
  QLatin1StringView("csvExport.getStatus"),
  QLatin1StringView("csvExport.setEnabled"),
  QLatin1StringView("csvExport.setInterval"),
  QLatin1StringView("csvPlayer.close"),
  QLatin1StringView("csvPlayer.getStatus"),
  QLatin1StringView("csvPlayer.open"),
  QLatin1StringView("csvPlayer.setPaused"),
  QLatin1StringView("csvPlayer.setProgress"),
  QLatin1StringView("csvPlayer.step"),
  QLatin1StringView("dashboard.getData"),
  QLatin1StringView("dashboard.getFps"),
  QLatin1StringView("dashboard.getOperationMode"),
  QLatin1StringView("dashboard.getStatus"),
  QLatin1StringView("dashboard.getTimeRange"),
  QLatin1StringView("dashboard.reprocess"),
  QLatin1StringView("dashboard.setFps"),
  QLatin1StringView("dashboard.setOperationMode"),
  QLatin1StringView("dashboard.setTimeRange"),
  QLatin1StringView("dashboard.tailFrames"),
  QLatin1StringView("dashboard.tick"),
  QLatin1StringView("diagnostics.run"),
  QLatin1StringView("diagnostics.status"),
  QLatin1StringView("extensions.getInfo"),
  QLatin1StringView("extensions.list"),
  QLatin1StringView("extensions.listRepositories"),
  QLatin1StringView("extensions.loadState"),
  QLatin1StringView("extensions.refresh"),
  QLatin1StringView("extensions.removeRepository"),
  QLatin1StringView("extensions.saveState"),
  QLatin1StringView("extensions.uninstall"),
  QLatin1StringView("influx.getStatus"),
  QLatin1StringView("influx.setConfig"),
  QLatin1StringView("influx.setEnabled"),
  QLatin1StringView("io.audio.getConfig"),
  QLatin1StringView("io.audio.listInputDevices"),
  QLatin1StringView("io.audio.listInputFormats"),
  QLatin1StringView("io.audio.listOutputDevices"),
  QLatin1StringView("io.audio.listOutputFormats"),
  QLatin1StringView("io.audio.listSampleRates"),
  QLatin1StringView("io.audio.setInputChannelConfig"),
  QLatin1StringView("io.audio.setInputDevice"),
  QLatin1StringView("io.audio.setInputSampleFormat"),
  QLatin1StringView("io.audio.setNormalization"),
  QLatin1StringView("io.audio.setOutputChannelConfig"),
  QLatin1StringView("io.audio.setOutputDevice"),
  QLatin1StringView("io.audio.setOutputSampleFormat"),
  QLatin1StringView("io.audio.setSampleRate"),
  QLatin1StringView("io.ble.getConfig"),
  QLatin1StringView("io.ble.getStatus"),
  QLatin1StringView("io.ble.listCharacteristics"),
  QLatin1StringView("io.ble.listDevices"),
  QLatin1StringView("io.ble.listServices"),
  QLatin1StringView("io.ble.selectDevice"),
  QLatin1StringView("io.ble.selectService"),
  QLatin1StringView("io.ble.selectServiceByUuid"),
  QLatin1StringView("io.ble.setCharacteristicIndex"),
  QLatin1StringView("io.ble.setNotifyCharacteristic"),
  QLatin1StringView("io.ble.startDiscovery"),
  QLatin1StringView("io.canbus.getConfig"),
  QLatin1StringView("io.canbus.getInterfaceError"),
  QLatin1StringView("io.canbus.listBitrates"),
  QLatin1StringView("io.canbus.listInterfaces"),
  QLatin1StringView("io.canbus.listPlugins"),
  QLatin1StringView("io.canbus.setBitrate"),
  QLatin1StringView("io.canbus.setCanFd"),
  QLatin1StringView("io.canbus.setDataBitrate"),
  QLatin1StringView("io.canbus.setInterfaceIndex"),
  QLatin1StringView("io.canbus.setListenOnly"),
  QLatin1StringView("io.canbus.setLoopback"),
  QLatin1StringView("io.canbus.setPluginIndex"),
  QLatin1StringView("io.disconnect"),
  QLatin1StringView("io.eip.addTag"),
  QLatin1StringView("io.eip.clearTags"),
  QLatin1StringView("io.eip.generateProject"),
  QLatin1StringView("io.eip.getConfig"),
  QLatin1StringView("io.eip.getStatus"),
  QLatin1StringView("io.eip.removeTag"),
  QLatin1StringView("io.eip.setProperty"),
  QLatin1StringView("io.getLatestFrame"),
  QLatin1StringView("io.getStatus"),
  QLatin1StringView("io.hid.getConfig"),
  QLatin1StringView("io.hid.listDevices"),
  QLatin1StringView("io.hid.setDeviceIndex"),
  QLatin1StringView("io.iec104.clearPoints"),
  QLatin1StringView("io.iec104.generateProject"),
  QLatin1StringView("io.iec104.getConfig"),
  QLatin1StringView("io.iec104.getPoints"),
  QLatin1StringView("io.iec104.getStatus"),
  QLatin1StringView("io.iec104.setProperty"),
  QLatin1StringView("io.listBuses"),
  QLatin1StringView("io.modbus.addRegisterGroup"),
  QLatin1StringView("io.modbus.clearRegisterGroups"),
  QLatin1StringView("io.modbus.getConfig"),
  QLatin1StringView("io.modbus.listBaudRates"),
  QLatin1StringView("io.modbus.listDataBits"),
  QLatin1StringView("io.modbus.listParities"),
  QLatin1StringView("io.modbus.listProtocols"),
  QLatin1StringView("io.modbus.listRegisterGroups"),
  QLatin1StringView("io.modbus.listRegisterTypes"),
  QLatin1StringView("io.modbus.listSerialPorts"),
  QLatin1StringView("io.modbus.listStopBits"),
  QLatin1StringView("io.modbus.removeRegisterGroup"),
  QLatin1StringView("io.modbus.setBaudRate"),
  QLatin1StringView("io.modbus.setDataBitsIndex"),
  QLatin1StringView("io.modbus.setHost"),
  QLatin1StringView("io.modbus.setParityIndex"),
  QLatin1StringView("io.modbus.setPollInterval"),
  QLatin1StringView("io.modbus.setPort"),
  QLatin1StringView("io.modbus.setProtocolIndex"),
  QLatin1StringView("io.modbus.setSerialPortIndex"),
  QLatin1StringView("io.modbus.setSlaveAddress"),
  QLatin1StringView("io.modbus.setStopBitsIndex"),
  QLatin1StringView("io.network.getConfig"),
  QLatin1StringView("io.network.getStatus"),
  QLatin1StringView("io.network.listSocketTypes"),
  QLatin1StringView("io.network.lookup"),
  QLatin1StringView("io.network.setHttpBody"),
  QLatin1StringView("io.network.setHttpHeaders"),
  QLatin1StringView("io.network.setHttpInterval"),
  QLatin1StringView("io.network.setHttpMethod"),
  QLatin1StringView("io.network.setHttpUrl"),
  QLatin1StringView("io.network.setIgnoreTlsErrors"),
  QLatin1StringView("io.network.setRemoteAddress"),
  QLatin1StringView("io.network.setSocketType"),
  QLatin1StringView("io.network.setTcpPort"),
  QLatin1StringView("io.network.setUdpLocalPort"),
  QLatin1StringView("io.network.setUdpMulticast"),
  QLatin1StringView("io.network.setUdpRemotePort"),
  QLatin1StringView("io.network.setWebSocketUrl"),
  QLatin1StringView("io.opcua.addTag"),
  QLatin1StringView("io.opcua.browse"),
  QLatin1StringView("io.opcua.clearTags"),
  QLatin1StringView("io.opcua.discoverEndpoints"),
  QLatin1StringView("io.opcua.exportCertificate"),
  QLatin1StringView("io.opcua.generateProject"),
  QLatin1StringView("io.opcua.getCertificate"),
  QLatin1StringView("io.opcua.getConfig"),
  QLatin1StringView("io.opcua.getStatus"),
  QLatin1StringView("io.opcua.listEndpoints"),
  QLatin1StringView("io.opcua.listTags"),
  QLatin1StringView("io.opcua.listTrusted"),
  QLatin1StringView("io.opcua.regenerateCertificate"),
  QLatin1StringView("io.opcua.removeTag"),
  QLatin1StringView("io.opcua.revokeTrust"),
  QLatin1StringView("io.opcua.setAuthMode"),
  QLatin1StringView("io.opcua.setEndpointIndex"),
  QLatin1StringView("io.opcua.setEndpointUrl"),
  QLatin1StringView("io.opcua.setIdentityType"),
  QLatin1StringView("io.opcua.setPassword"),
  QLatin1StringView("io.opcua.setPublishingInterval"),
  QLatin1StringView("io.opcua.setSecurityMode"),
  QLatin1StringView("io.opcua.setSecurityPolicy"),
  QLatin1StringView("io.opcua.setTags"),
  QLatin1StringView("io.opcua.setUserCertificate"),
  QLatin1StringView("io.opcua.setUsername"),
  QLatin1StringView("io.opcua.startBrowse"),
  QLatin1StringView("io.opcua.stopBrowse"),
  QLatin1StringView("io.opcua.trustServer"),
  QLatin1StringView("io.process.getConfig"),
  QLatin1StringView("io.process.listRunning"),
  QLatin1StringView("io.process.setPipePath"),
  QLatin1StringView("io.s7.addVariable"),
  QLatin1StringView("io.s7.clearVariables"),
  QLatin1StringView("io.s7.generateProject"),
  QLatin1StringView("io.s7.getConfig"),
  QLatin1StringView("io.s7.getStatus"),
  QLatin1StringView("io.s7.removeVariable"),
  QLatin1StringView("io.s7.setProperty"),
  QLatin1StringView("io.setPaused"),
  QLatin1StringView("io.uart.getConfig"),
  QLatin1StringView("io.uart.listBaudRates"),
  QLatin1StringView("io.uart.listPorts"),
  QLatin1StringView("io.uart.setAutoReconnect"),
  QLatin1StringView("io.uart.setBaudRate"),
  QLatin1StringView("io.uart.setDataBits"),
  QLatin1StringView("io.uart.setDevice"),
  QLatin1StringView("io.uart.setDtrEnabled"),
  QLatin1StringView("io.uart.setFlowControl"),
  QLatin1StringView("io.uart.setParity"),
  QLatin1StringView("io.uart.setPortIndex"),
  QLatin1StringView("io.uart.setStopBits"),
  QLatin1StringView("io.usb.getConfig"),
  QLatin1StringView("io.usb.listDevices"),
  QLatin1StringView("io.usb.setDeviceIndex"),
  QLatin1StringView("io.usb.setInEndpointIndex"),
  QLatin1StringView("io.usb.setIsoPacketSize"),
  QLatin1StringView("io.usb.setOutEndpointIndex"),
  QLatin1StringView("io.usb.setTransferMode"),
  QLatin1StringView("licensing.activate"),
  QLatin1StringView("licensing.activateOffline"),
  QLatin1StringView("licensing.deactivate"),
  QLatin1StringView("licensing.enableTrial"),
  QLatin1StringView("licensing.getGuardStatus"),
  QLatin1StringView("licensing.getStatus"),
  QLatin1StringView("licensing.getTrialStatus"),
  QLatin1StringView("licensing.setLicense"),
  QLatin1StringView("licensing.validate"),
  QLatin1StringView("mdf4Export.close"),
  QLatin1StringView("mdf4Export.getStatus"),
  QLatin1StringView("mdf4Export.setEnabled"),
  QLatin1StringView("mdf4Player.close"),
  QLatin1StringView("mdf4Player.getStatus"),
  QLatin1StringView("mdf4Player.open"),
  QLatin1StringView("mdf4Player.setPaused"),
  QLatin1StringView("mdf4Player.setProgress"),
  QLatin1StringView("mdf4Player.step"),
  QLatin1StringView("mirror.getInfo"),
  QLatin1StringView("mirror.getStructure"),
  QLatin1StringView("notifications.clearAll"),
  QLatin1StringView("notifications.clearChannel"),
  QLatin1StringView("notifications.getUnreadCount"),
  QLatin1StringView("notifications.list"),
  QLatin1StringView("notifications.listChannels"),
  QLatin1StringView("notifications.markRead"),
  QLatin1StringView("notifications.post"),
  QLatin1StringView("notifications.resolve"),
  QLatin1StringView("problems.list"),
  QLatin1StringView("problems.listCheckers"),
  QLatin1StringView("problems.run"),
  QLatin1StringView("project.action.add"),
  QLatin1StringView("project.action.delete"),
  QLatin1StringView("project.action.list"),
  QLatin1StringView("project.activate"),
  QLatin1StringView("project.dashboard.getTimeRange"),
  QLatin1StringView("project.dashboard.getWidgetTitles"),
  QLatin1StringView("project.dashboard.setTimeRange"),
  QLatin1StringView("project.dashboard.setWidgetFreezeTitle"),
  QLatin1StringView("project.dashboard.setWidgetTitle"),
  QLatin1StringView("project.dataTable.add"),
  QLatin1StringView("project.dataTable.addRegister"),
  QLatin1StringView("project.dataTable.delete"),
  QLatin1StringView("project.dataTable.deleteRegister"),
  QLatin1StringView("project.dataTable.get"),
  QLatin1StringView("project.dataTable.getValue"),
  QLatin1StringView("project.dataTable.getValueH"),
  QLatin1StringView("project.dataTable.handle"),
  QLatin1StringView("project.dataTable.list"),
  QLatin1StringView("project.dataTable.rename"),
  QLatin1StringView("project.dataTable.setValue"),
  QLatin1StringView("project.dataTable.setValueH"),
  QLatin1StringView("project.dataTable.updateRegister"),
  QLatin1StringView("project.dataset.add"),
  QLatin1StringView("project.dataset.addMany"),
  QLatin1StringView("project.dataset.delete"),
  QLatin1StringView("project.dataset.duplicate"),
  QLatin1StringView("project.dataset.getAlarmBands"),
  QLatin1StringView("project.dataset.getByPath"),
  QLatin1StringView("project.dataset.getByTitle"),
  QLatin1StringView("project.dataset.getByUniqueId"),
  QLatin1StringView("project.dataset.getExecutionOrder"),
  QLatin1StringView("project.dataset.getFFTMarkers"),
  QLatin1StringView("project.dataset.list"),
  QLatin1StringView("project.dataset.move"),
  QLatin1StringView("project.dataset.setAlarmBands"),
  QLatin1StringView("project.dataset.setFFTMarkers"),
  QLatin1StringView("project.dataset.setOption"),
  QLatin1StringView("project.dataset.setOptions"),
  QLatin1StringView("project.dataset.setVirtual"),
  QLatin1StringView("project.exportJson"),
  QLatin1StringView("project.frameParser.getCode"),
  QLatin1StringView("project.frameParser.getConfig"),
  QLatin1StringView("project.frameParser.getLanguage"),
  QLatin1StringView("project.frameParser.getTemplate"),
  QLatin1StringView("project.frameParser.getTemplateSchema"),
  QLatin1StringView("project.frameParser.listTemplates"),
  QLatin1StringView("project.frameParser.setLanguage"),
  QLatin1StringView("project.frameParser.setTemplate"),
  QLatin1StringView("project.frameParser.update"),
  QLatin1StringView("project.getStatus"),
  QLatin1StringView("project.group.add"),
  QLatin1StringView("project.group.delete"),
  QLatin1StringView("project.group.duplicate"),
  QLatin1StringView("project.group.get"),
  QLatin1StringView("project.group.list"),
  QLatin1StringView("project.group.move"),
  QLatin1StringView("project.mqtt.publisher.getConfig"),
  QLatin1StringView("project.mqtt.publisher.getStatus"),
  QLatin1StringView("project.mqtt.publisher.setConfig"),
  QLatin1StringView("project.mqtt.subscriber.getConfig"),
  QLatin1StringView("project.mqtt.subscriber.getStatus"),
  QLatin1StringView("project.mqtt.subscriber.setConfig"),
  QLatin1StringView("project.new"),
  QLatin1StringView("project.outputWidget.add"),
  QLatin1StringView("project.outputWidget.delete"),
  QLatin1StringView("project.outputWidget.duplicate"),
  QLatin1StringView("project.outputWidget.get"),
  QLatin1StringView("project.painter.getCode"),
  QLatin1StringView("project.redo"),
  QLatin1StringView("project.save"),
  QLatin1StringView("project.search"),
  QLatin1StringView("project.setTitle"),
  QLatin1StringView("project.snapshot"),
  QLatin1StringView("project.source.add"),
  QLatin1StringView("project.source.delete"),
  QLatin1StringView("project.source.getConfig"),
  QLatin1StringView("project.source.getFrameParserCode"),
  QLatin1StringView("project.source.list"),
  QLatin1StringView("project.template.list"),
  QLatin1StringView("project.transformLibrary.get"),
  QLatin1StringView("project.undo"),
  QLatin1StringView("project.workspace.add"),
  QLatin1StringView("project.workspace.addWidget"),
  QLatin1StringView("project.workspace.autoGenerate"),
  QLatin1StringView("project.workspace.cleanup"),
  QLatin1StringView("project.workspace.clearAll"),
  QLatin1StringView("project.workspace.delete"),
  QLatin1StringView("project.workspace.get"),
  QLatin1StringView("project.workspace.getCustomizeMode"),
  QLatin1StringView("project.workspace.list"),
  QLatin1StringView("project.workspace.profile.add"),
  QLatin1StringView("project.workspace.profile.list"),
  QLatin1StringView("project.workspace.profile.remove"),
  QLatin1StringView("project.workspace.profile.select"),
  QLatin1StringView("project.workspace.profile.update"),
  QLatin1StringView("project.workspace.removeWidget"),
  QLatin1StringView("project.workspace.rename"),
  QLatin1StringView("project.workspace.reorder"),
  QLatin1StringView("project.workspace.setCustomizeMode"),
  QLatin1StringView("project.workspace.update"),
  QLatin1StringView("project.workspace.validate"),
  QLatin1StringView("scripts.get"),
  QLatin1StringView("scripts.list"),
  QLatin1StringView("sessions.addTag"),
  QLatin1StringView("sessions.assignTag"),
  QLatin1StringView("sessions.close"),
  QLatin1StringView("sessions.delete"),
  QLatin1StringView("sessions.deleteTag"),
  QLatin1StringView("sessions.exportToCsv"),
  QLatin1StringView("sessions.get"),
  QLatin1StringView("sessions.getDbPath"),
  QLatin1StringView("sessions.getRegression"),
  QLatin1StringView("sessions.getStatus"),
  QLatin1StringView("sessions.getVerification"),
  QLatin1StringView("sessions.list"),
  QLatin1StringView("sessions.listTags"),
  QLatin1StringView("sessions.openDatabase"),
  QLatin1StringView("sessions.removeTag"),
  QLatin1StringView("sessions.renameTag"),
  QLatin1StringView("sessions.setExportEnabled"),
  QLatin1StringView("sessions.setNotes"),
  QLatin1StringView("stream.getInfo"),
  QLatin1StringView("stream.getSources"),
  QLatin1StringView("system.projectDir"),
  QLatin1StringView("ui.window.getLayout"),
  QLatin1StringView("ui.window.getStatus"),
  QLatin1StringView("ui.window.getWidgetSettings"),
  QLatin1StringView("ui.window.listGroups"),
  QLatin1StringView("ui.window.listWindowStates"),
  QLatin1StringView("ui.window.loadLayout"),
  QLatin1StringView("ui.window.saveLayout"),
  QLatin1StringView("ui.window.setActiveGroup"),
  QLatin1StringView("ui.window.setActiveGroupIndex"),
  QLatin1StringView("ui.window.setAutoLayout"),
  QLatin1StringView("ui.window.setLayout"),
  QLatin1StringView("ui.window.setLayoutPattern"),
  QLatin1StringView("ui.window.setWidgetSetting"),
  QLatin1StringView("ui.window.setWindowState"),
};
// clang-format on

//--------------------------------------------------------------------------------------------------
// Lookup helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief The capable entry for @p command, or null when it has no capability or is unknown.
 */
static const Entry* findCapable(const QStringView command) noexcept
{
  const auto begin = std::begin(kCapable);
  const auto end   = std::end(kCapable);
  const auto it    = std::lower_bound(
    begin, end, command, [](const Entry& a, QStringView b) { return a.name.compare(b) < 0; });

  if (it == end || it->name != command)
    return nullptr;

  return it;
}

/**
 * @brief Whether @p command is classified as able to do none of the gated things.
 */
static bool isPlain(const QStringView command) noexcept
{
  const auto begin = std::begin(kPlain);
  const auto end   = std::end(kPlain);
  const auto it    = std::lower_bound(
    begin, end, command, [](QLatin1StringView a, QStringView b) { return a.compare(b) < 0; });

  return it != end && *it == command;
}

//--------------------------------------------------------------------------------------------------
// Argument refinements
//--------------------------------------------------------------------------------------------------

/**
 * @brief The Process driver keys that pick what a source launches; pipePath only names a pipe.
 */
static bool isProgramKey(const QString& key)
{
  return key == QLatin1StringView("executable") || key == QLatin1StringView("arguments")
      || key == QLatin1StringView("workingDir") || key == QLatin1StringView("mode");
}

/**
 * @brief Whether a settings batch for a source carries any key that picks its program.
 */
static bool settingsChooseProgram(const QJsonObject& params)
{
  const auto settings = params.value(QStringLiteral("settings")).toObject();
  for (auto it = settings.constBegin(); it != settings.constEnd(); ++it)
    if (isProgramKey(it.key()))
      return true;

  return false;
}

/**
 * @brief Whether a busType parameter selects the Process driver, which starts whatever program
 *        it saved last. A GPL build has no Process driver.
 */
static bool selectsProcessBus(const QJsonObject& params)
{
#ifdef BUILD_COMMERCIAL
  constexpr int kProcess = static_cast<int>(SerialStudio::BusType::Process);
  return params.contains(Keys::BusType) && params.value(Keys::BusType).toInt(-1) == kProcess;
#else
  (void)params;
  return false;
#endif
}

/**
 * @brief Whether an action update changes the bytes, target or arming of an action:
 *        auto-execute and timers write later with no further prompt.
 */
static bool actionUpdateWritesDevice(const QJsonObject& params)
{
  if (params.value(QStringLiteral("autoExecuteOnConnect")).toBool())
    return true;

  if (params.value(QStringLiteral("timerMode")).toInt() != 0)
    return true;

  static constexpr QLatin1StringView kTxParams[] = {
    QLatin1StringView("txData"),
    QLatin1StringView("binaryData"),
    QLatin1StringView("checksum"),
    QLatin1StringView("eolSequence"),
    QLatin1StringView("txEncoding"),
    Keys::SourceId,
    QLatin1StringView("repeatCount"),
    QLatin1StringView("timerIntervalMs"),
  };

  for (const auto key : kTxParams)
    if (params.contains(key))
      return true;

  return false;
}

//--------------------------------------------------------------------------------------------------
// Public API
//--------------------------------------------------------------------------------------------------

/**
 * @brief The command's capabilities for these arguments, or nothing when it is unclassified.
 *        Arguments only ever add to what a command always has, never take away.
 */
std::optional<Capabilities> API::CommandCapabilities::of(const QStringView command,
                                                         const QJsonObject& params)
{
  if (const auto* entry = findCapable(command)) {
    auto capabilities = entry->always;
    if (entry->withArguments.toInt() != 0 && argumentsAddCapabilities(command, params))
      capabilities |= entry->withArguments;

    return capabilities;
  }

  if (isPlain(command))
    return Capabilities(NoCapability);

  return std::nullopt;
}

/**
 * @brief Whether these arguments trigger a command's argument-dependent capabilities: code
 *        fields on the generic updaters, transmit fields on an action, a program key on a
 *        source, or a switch to the Process bus.
 */
bool API::CommandCapabilities::argumentsAddCapabilities(const QStringView command,
                                                        const QJsonObject& params)
{
  if (command == u"project.dataset.update")
    return params.contains(QStringLiteral("transformCode"));

  if (command == u"project.group.update")
    return params.contains(QStringLiteral("painterCode"));

  if (command == u"project.outputWidget.update")
    return params.contains(QStringLiteral("transmitFunction"));

  if (command == u"project.action.update")
    return actionUpdateWritesDevice(params);

  if (command == u"project.source.setProperty")
    return isProgramKey(params.value(QStringLiteral("key")).toString());

  if (command == u"project.source.setProperties")
    return settingsChooseProgram(params);

  if (command == u"io.setBusType" || command == u"project.source.update")
    return selectsProcessBus(params);

  return false;
}

/**
 * @brief The commands with a capability, sorted, for the registry sweep.
 */
std::span<const Entry> API::CommandCapabilities::capableCommands() noexcept
{
  return std::span<const Entry>(kCapable);
}

/**
 * @brief The commands with no capability, sorted, for the registry sweep.
 */
std::span<const QLatin1StringView> API::CommandCapabilities::plainCommands() noexcept
{
  return std::span<const QLatin1StringView>(kPlain);
}
