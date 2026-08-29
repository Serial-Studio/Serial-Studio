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

#include "Misc/CLI/CliIndustrialConfig.h"

#ifdef BUILD_COMMERCIAL
#  include <QDebug>
#  include <QJsonArray>
#  include <QJsonObject>
#  include <QList>
#  include <QStringList>
#  include <utility>

#  include "IO/ConnectionManager.h"
#  include "IO/Drivers/CANBus.h"
#  include "IO/Drivers/EthernetIp.h"
#  include "IO/Drivers/Iec104.h"
#  include "IO/Drivers/Modbus.h"
#  include "IO/Drivers/OpcUa.h"
#  include "IO/Drivers/OpcUaWire.h"
#  include "IO/Drivers/S7.h"
#  include "IO/HAL_Driver.h"
#  include "Misc/CLI.h"
#  include "Misc/CLI/CliSpecParsers.h"
#  include "SerialStudio.h"
#endif

namespace Misc {
namespace CliIndustrialConfig {

#ifdef BUILD_COMMERCIAL

//---------------------------------------------------------------------------------------------------
// Modbus helpers
//---------------------------------------------------------------------------------------------------

/**
 * @brief Parses a Modbus register spec string and registers it with @p modbus.
 */
static void applyModbusRegister(IO::Drivers::Modbus* modbus, const QString& spec)
{
  quint8 registerType = 0;
  quint16 start       = 0;
  quint16 count       = 0;
  if (!CliSpecParsers::parseModbusRegisterSpec(spec, registerType, start, count))
    return;

  modbus->addRegisterGroup(registerType, start, count);
}

/**
 * @brief Applies the Modbus parity option to @p modbus.
 */
static void applyModbusParity(IO::Drivers::Modbus* modbus, const QString& parity)
{
  const int index = CliSpecParsers::modbusParityIndex(parity);
  if (index < 0) {
    qWarning() << "Invalid ModBus parity (none/even/odd/space/mark):" << parity;
    modbus->setParityIndex(0);
    return;
  }

  modbus->setParityIndex(static_cast<quint8>(index));
}

/**
 * @brief Applies the Modbus data-bits option to @p modbus.
 */
static void applyModbusDataBits(IO::Drivers::Modbus* modbus, const QString& dataBits)
{
  const int index = CliSpecParsers::modbusDataBitsIndex(dataBits);
  if (index < 0) {
    qWarning() << "Invalid ModBus data bits (5/6/7/8):" << dataBits;
    modbus->setDataBitsIndex(3);
    return;
  }

  modbus->setDataBitsIndex(static_cast<quint8>(index));
}

/**
 * @brief Applies the Modbus stop-bits option to @p modbus.
 */
static void applyModbusStopBits(IO::Drivers::Modbus* modbus, const QString& stopBits)
{
  const int index = CliSpecParsers::modbusStopBitsIndex(stopBits);
  if (index < 0) {
    qWarning() << "Invalid ModBus stop bits (1/1.5/2):" << stopBits;
    modbus->setStopBitsIndex(0);
    return;
  }

  modbus->setStopBitsIndex(static_cast<quint8>(index));
}

/**
 * @brief Applies the Modbus slave option to @p modbus.
 */
static void applyModbusSlave(IO::Drivers::Modbus* modbus,
                             const QCommandLineParser& parser,
                             const QCommandLineOption& opt)
{
  if (!parser.isSet(opt))
    return;

  bool ok                      = false;
  const unsigned int slave_val = parser.value(opt).toUInt(&ok);
  if (!ok || slave_val < 1 || slave_val > 247) {
    qWarning() << "Invalid ModBus slave address (1-247):" << parser.value(opt);
    return;
  }

  modbus->setSlaveAddress(static_cast<quint8>(slave_val));
}

/**
 * @brief Applies the Modbus poll-interval option to @p modbus.
 */
static void applyModbusPoll(IO::Drivers::Modbus* modbus,
                            const QCommandLineParser& parser,
                            const QCommandLineOption& opt)
{
  if (!parser.isSet(opt))
    return;

  bool ok                = false;
  const quint16 interval = parser.value(opt).toUInt(&ok);
  if (!ok || interval < 50 || interval > 60000) {
    qWarning() << "Invalid ModBus poll interval (50-60000 ms):" << parser.value(opt);
    return;
  }

  modbus->setPollInterval(interval);
}

/**
 * @brief Applies the Modbus serial baud-rate option to @p modbus.
 */
static void applyModbusBaud(IO::Drivers::Modbus* modbus,
                            const QCommandLineParser& parser,
                            const QCommandLineOption& opt)
{
  if (!parser.isSet(opt))
    return;

  bool ok               = false;
  const qint32 baudRate = parser.value(opt).toInt(&ok);
  if (!ok) {
    qWarning() << "Invalid ModBus baud rate:" << parser.value(opt);
    return;
  }

  modbus->setBaudRate(baudRate);
}

/**
 * @brief Applies all --modbus-register specs and warns if none are present.
 */
static void applyModbusRegisters(IO::Drivers::Modbus* modbus,
                                 const QCommandLineParser& parser,
                                 const QCommandLineOption& opt)
{
  modbus->clearRegisterGroups();
  if (!parser.isSet(opt)) {
    qWarning() << "No register groups specified. Use --modbus-register to add registers.";
    return;
  }

  const QStringList registerSpecs = parser.values(opt);
  for (const QString& spec : std::as_const(registerSpecs))
    applyModbusRegister(modbus, spec);
}

/**
 * @brief Configures and connects a Modbus RTU bus from CLI options.
 */
static void setupModbusRtuConnection(IO::ConnectionManager& cm,
                                     const QCommandLineParser& parser,
                                     const CliOptions& opts)
{
  const QString portPath = parser.value(opts.modbusRtuOpt);
  cm.setBusType(SerialStudio::BusType::ModBus);

  auto* modbus = cm.modbus();
  modbus->setProtocolIndex(0);

  const QStringList ports = modbus->serialPortList();
  const int portIndex     = ports.indexOf(portPath);
  if (portIndex < 0) {
    qWarning() << "ModBus serial port not found:" << portPath;
    qWarning() << "Available ports:" << ports.join(", ");
    return;
  }

  modbus->setSerialPortIndex(portIndex);
  applyModbusSlave(modbus, parser, opts.modbusSlaveOpt);
  applyModbusPoll(modbus, parser, opts.modbusPollOpt);
  applyModbusBaud(modbus, parser, opts.modbusBaudOpt);

  if (parser.isSet(opts.modbusParityOpt))
    applyModbusParity(modbus, parser.value(opts.modbusParityOpt).toLower());

  if (parser.isSet(opts.modbusDataBitsOpt))
    applyModbusDataBits(modbus, parser.value(opts.modbusDataBitsOpt));

  if (parser.isSet(opts.modbusStopBitsOpt))
    applyModbusStopBits(modbus, parser.value(opts.modbusStopBitsOpt));

  applyModbusRegisters(modbus, parser, opts.modbusRegisterOpt);
  cm.connectDevice();
}

/**
 * @brief Configures and connects a Modbus TCP bus from CLI options.
 */
static void setupModbusTcpConnection(IO::ConnectionManager& cm,
                                     const QCommandLineParser& parser,
                                     const CliOptions& opts)
{
  QString host;
  quint16 port = 502;
  if (!CliSpecParsers::parseModbusTcpAddress(parser.value(opts.modbusTcpOpt), host, port)) {
    qWarning() << "Invalid ModBus TCP address format. Expected: host[:port]";
    return;
  }

  cm.setBusType(SerialStudio::BusType::ModBus);

  auto* modbus = cm.modbus();
  modbus->setProtocolIndex(1);
  modbus->setHost(host);
  modbus->setPort(port);

  applyModbusSlave(modbus, parser, opts.modbusSlaveOpt);
  applyModbusPoll(modbus, parser, opts.modbusPollOpt);
  applyModbusRegisters(modbus, parser, opts.modbusRegisterOpt);
  cm.connectDevice();
}

//---------------------------------------------------------------------------------------------------
// OPC UA
//---------------------------------------------------------------------------------------------------

/**
 * @brief Parses nodeId[:type[:name]][:n=N][:unit=U] from the right, since node ids themselves
 *        may contain ':'. Returns an empty object when the spec is malformed or names an unknown
 *        type code.
 */
[[nodiscard]] static QJsonObject parseOpcUaTagSpec(const QString& spec)
{
  using IO::Drivers::OpcUaWire::Type;
  using IO::Drivers::OpcUaWire::typeFromCode;

  QString id   = spec;
  QString type = QStringLiteral("f64");
  QString name;
  QString unit;
  int arrayLen = 1;

  auto body = spec;
  for (int guard = 0; guard < 4; ++guard) {
    const auto tail = body.section(QLatin1Char(':'), -1);
    if (tail.startsWith(QLatin1String("n=")) && tail.mid(2).toInt() > 0) {
      arrayLen = tail.mid(2).toInt();
      body     = body.section(QLatin1Char(':'), 0, -2);
      continue;
    }

    if (tail.startsWith(QLatin1String("unit="))) {
      unit = tail.mid(5);
      body = body.section(QLatin1Char(':'), 0, -2);
      continue;
    }

    break;
  }

  id               = body;
  const auto parts = body.split(QLatin1Char(':'));
  if (parts.size() >= 3 && typeFromCode(parts.at(parts.size() - 2)) != Type::Invalid) {
    name = parts.last();
    type = parts.at(parts.size() - 2);
    id   = parts.mid(0, parts.size() - 2).join(QLatin1Char(':'));
  } else if (parts.size() >= 2 && typeFromCode(parts.last()) != Type::Invalid) {
    type = parts.last();
    id   = parts.mid(0, parts.size() - 1).join(QLatin1Char(':'));
  } else if (parts.size() >= 2) {
    return {};
  }

  if (id.isEmpty())
    return {};

  QJsonObject tag;
  tag[QStringLiteral("id")]   = id;
  tag[QStringLiteral("t")]    = type;
  tag[QStringLiteral("n")]    = arrayLen;
  tag[QStringLiteral("unit")] = unit;
  tag[QStringLiteral("name")] = name.isEmpty() ? id : name;
  return tag;
}

/**
 * @brief Configures and connects an OPC UA session from CLI options (policy None, tags by node id).
 */
static void setupOpcUaConnection(IO::ConnectionManager& cm,
                                 const QCommandLineParser& parser,
                                 const CliOptions& opts)
{
  const QString url = parser.value(opts.opcuaOpt);
  if (!url.startsWith(QLatin1String("opc.tcp://"))) {
    qWarning() << "Invalid OPC UA endpoint. Expected: opc.tcp://host[:port][/path]";
    return;
  }

  auto* opcua = cm.opcUa();
  cm.setBusType(SerialStudio::BusType::OpcUa);
  opcua->setEndpointUrl(url);

  if (parser.isSet(opts.opcuaUserOpt)) {
    opcua->setAuthMode(1);
    opcua->setUsername(parser.value(opts.opcuaUserOpt));
    if (parser.isSet(opts.opcuaPassOpt))
      opcua->setPassword(parser.value(opts.opcuaPassOpt));
  } else {
    opcua->setAuthMode(0);
  }

  if (parser.isSet(opts.opcuaIntervalOpt)) {
    bool ok            = false;
    const int interval = parser.value(opts.opcuaIntervalOpt).toInt(&ok);
    if (ok && interval >= 10 && interval <= 60000)
      opcua->setPublishingInterval(interval);
    else
      qWarning() << "Invalid OPC UA interval (10-60000 ms):" << parser.value(opts.opcuaIntervalOpt);
  }

  QJsonArray tags;
  const auto specs = parser.values(opts.opcuaTagOpt);
  for (const auto& spec : specs) {
    const auto tag = parseOpcUaTagSpec(spec);
    if (tag.isEmpty())
      qWarning() << "Invalid OPC UA tag spec (nodeId[:type[:name]], known type code):" << spec;
    else
      tags.append(tag);
  }

  if (tags.isEmpty()) {
    qWarning() << "No OPC UA tags given (--opcua-tag); nothing to subscribe to";
    return;
  }

  opcua->setTags(tags);

  if (!parser.isSet(opts.projectOpt) && !opcua->loadGeneratedProject())
    qWarning() << "Could not generate a project for the OPC UA tags";

  cm.connectDevice();
}

//---------------------------------------------------------------------------------------------------
// CAN bus
//---------------------------------------------------------------------------------------------------

/**
 * @brief Configures and connects a CAN bus from CLI options.
 */
static void setupCanbusConnection(IO::ConnectionManager& cm,
                                  const QCommandLineParser& parser,
                                  const CliOptions& opts)
{
  const QStringList parts = parser.value(opts.canbusOpt).split(':');
  if (parts.size() != 2) {
    qWarning() << "Invalid CAN bus format. Expected: plugin:interface";
    return;
  }

  const QString plugin        = parts[0].toLower();
  const QString interfaceName = parts[1];

  cm.setBusType(SerialStudio::BusType::CanBus);
  auto* canBus = cm.canBus();

  const QStringList availablePlugins = canBus->pluginList();
  const int pluginIndex              = availablePlugins.indexOf(plugin);
  if (pluginIndex < 0) {
    qWarning() << "CAN plugin" << plugin << "not found";
    qWarning() << "Available plugins:" << availablePlugins.join(", ");
    return;
  }

  canBus->setPluginIndex(pluginIndex);
  const QStringList availableInterfaces = canBus->interfaceList();
  const int interfaceIndex              = availableInterfaces.indexOf(interfaceName);

  if (interfaceIndex < 0) {
    qWarning() << "CAN interface" << interfaceName << "not found for plugin" << plugin;
    qWarning() << "Available interfaces:" << availableInterfaces.join(", ");
    return;
  }

  canBus->setInterfaceIndex(interfaceIndex);

  if (parser.isSet(opts.canbusBitrateOpt)) {
    bool ok               = false;
    const quint32 bitrate = parser.value(opts.canbusBitrateOpt).toUInt(&ok);
    if (ok && bitrate > 0)
      canBus->setBitrate(bitrate);
    else
      qWarning() << "Invalid CAN bus bitrate:" << parser.value(opts.canbusBitrateOpt);
  }

  if (parser.isSet(opts.canbusFdOpt))
    canBus->setCanFD(true);

  cm.connectDevice();
}

//---------------------------------------------------------------------------------------------------
// PLC helpers (spec 0073)
//---------------------------------------------------------------------------------------------------

/**
 * @brief Applies the shared endpoint options of a PLC bus: the poll interval. The range is
 *        rejected here (like --opcua-interval and Modbus) rather than left to the driver's qBound,
 *        which cannot tell a mistyped value from a deliberate one.
 */
static void applyPollInterval(IO::HAL_Driver* driver, const QString& value)
{
  bool ok            = false;
  const int interval = value.toInt(&ok);
  if (!ok || interval < 50 || interval > 60000) {
    qWarning() << "Invalid poll interval (50-60000 ms):" << value;
    return;
  }

  driver->setDriverProperty(QStringLiteral("pollInterval"), interval);
}

/**
 * @brief One validated S7 variable parsed from an --s7-variable argument.
 */
struct S7VarSpec {
  QString name;
  QString address;
};

/**
 * @brief One validated EtherNet/IP tag parsed from an --ethernetip-tag argument.
 */
struct EipTagSpec {
  int element;
  QString tag;
  QString type;
};

/**
 * @brief Splits an `address[:name]` variable spec. An S7 address may itself carry a `:TYPE`
 *        suffix, so the longest prefix the driver accepts wins and the remainder becomes the
 *        channel name; an empty return means no prefix parsed as an address at all.
 */
[[nodiscard]] static QString splitS7Spec(const IO::Drivers::S7& driver,
                                         const QString& spec,
                                         QString& name)
{
  name.clear();

  const int fields = static_cast<int>(spec.count(QLatin1Char(':'))) + 1;
  for (int last = fields - 1; last >= 0; --last) {
    const auto candidate = spec.section(QLatin1Char(':'), 0, last);
    if (!driver.validateAddress(candidate).isEmpty())
      continue;

    name = last + 1 < fields ? spec.section(QLatin1Char(':'), last + 1) : QString();
    return candidate;
  }

  return {};
}

/**
 * @brief Configures and connects a Siemens S7 controller from CLI options.
 */
static void setupS7Connection(IO::ConnectionManager& cm,
                              const QCommandLineParser& parser,
                              const CliOptions& opts)
{
  auto* s7 = cm.s7();
  cm.setBusType(SerialStudio::BusType::S7);
  s7->setHost(parser.value(opts.s7Opt));

  int rack = 0;
  if (CliSpecParsers::parseIntOption(parser, opts.s7RackOpt, 0, 7, QStringLiteral("S7 rack"), rack))
    s7->setRack(rack);

  int slot = 0;
  if (CliSpecParsers::parseIntOption(
        parser, opts.s7SlotOpt, 0, 31, QStringLiteral("S7 slot"), slot))
    s7->setSlot(slot);

  if (parser.isSet(opts.s7IntervalOpt))
    applyPollInterval(s7, parser.value(opts.s7IntervalOpt));

  QList<S7VarSpec> variables;
  const auto specs = parser.values(opts.s7VariableOpt);
  for (const auto& spec : specs) {
    QString name;
    const auto address = splitS7Spec(*s7, spec, name);
    if (address.isEmpty())
      qWarning() << "Invalid S7 variable spec:" << spec;
    else
      variables.append({name, address});
  }

  if (variables.isEmpty()) {
    qWarning() << "No S7 variables given (--s7-variable); nothing to poll";
    return;
  }

  s7->clearVariables();
  for (const auto& variable : variables)
    s7->addVariable(variable.name, variable.address);

  if (!parser.isSet(opts.projectOpt) && !s7->loadGeneratedProject())
    qWarning() << "Could not generate a project for the S7 variables";

  cm.connectDevice();
}

/**
 * @brief Parses every --ethernetip-tag argument into validated tag specs, warning per bad spec.
 */
[[nodiscard]] static QList<EipTagSpec> parseEipTagSpecs(const QStringList& specs)
{
  QList<EipTagSpec> tags;
  for (const auto& spec : specs) {
    const auto parts   = spec.split(QLatin1Char(':'));
    const auto address = parts.value(0);
    if (address.isEmpty()) {
      qWarning() << "Invalid EtherNet/IP tag spec (empty tag name):" << spec;
      continue;
    }

    int element = -1;
    if (parts.size() > 2) {
      bool ok = false;
      element = parts.at(2).toInt(&ok);
      if (!ok || element < 0) {
        qWarning() << "Invalid EtherNet/IP tag element (>= 0):" << spec;
        continue;
      }
    }

    tags.append({element, address, parts.size() > 1 ? parts.at(1) : QStringLiteral("f32")});
  }

  return tags;
}

/**
 * @brief Configures and connects an EtherNet/IP controller from CLI options.
 */
static void setupEthernetIpConnection(IO::ConnectionManager& cm,
                                      const QCommandLineParser& parser,
                                      const CliOptions& opts)
{
  auto* eip = cm.ethernetIp();
  cm.setBusType(SerialStudio::BusType::EthernetIp);
  eip->setHost(parser.value(opts.eipOpt));

  if (parser.isSet(opts.eipPathOpt))
    eip->setCipPath(parser.value(opts.eipPathOpt));

  if (parser.isSet(opts.eipPlcOpt)) {
    const auto families = eip->plcTypeList();
    const int index = static_cast<int>(families.indexOf(parser.value(opts.eipPlcOpt).toLower()));
    if (index < 0)
      qWarning() << "Unknown controller family:" << parser.value(opts.eipPlcOpt);
    else
      eip->setPlcTypeIndex(index);
  }

  if (parser.isSet(opts.eipIntervalOpt))
    applyPollInterval(eip, parser.value(opts.eipIntervalOpt));

  const QList<EipTagSpec> tags = parseEipTagSpecs(parser.values(opts.eipTagOpt));
  if (tags.isEmpty()) {
    qWarning() << "No EtherNet/IP tags given (--ethernetip-tag); nothing to poll";
    return;
  }

  eip->clearTags();
  for (const auto& tag : tags)
    eip->addTag(QString(), tag.tag, tag.type, tag.element);

  if (!parser.isSet(opts.projectOpt) && !eip->loadGeneratedProject())
    qWarning() << "Could not generate a project for the EtherNet/IP tags";

  cm.connectDevice();
}

/**
 * @brief Applies the IEC 60870-5-104 protocol windows and timers, warning where the station will
 *        clamp a pair the operator gave in the wrong order.
 */
static void applyIec104Timing(IO::Drivers::Iec104* iec104,
                              const QCommandLineParser& parser,
                              const CliOptions& opts)
{
  int windowK      = 0;
  const bool haveK = CliSpecParsers::parseIntOption(
    parser, opts.iec104KOpt, 1, 32767, QStringLiteral("IEC 60870-5-104 send window k"), windowK);
  if (haveK)
    iec104->setWindowK(windowK);

  int windowW      = 0;
  const bool haveW = CliSpecParsers::parseIntOption(
    parser, opts.iec104WOpt, 1, 32767, QStringLiteral("IEC 60870-5-104 ack window w"), windowW);
  if (haveW) {
    if (haveK && windowW > windowK)
      qWarning() << "IEC 60870-5-104 ack window w exceeds send window k; the station clamps w to k";

    iec104->setWindowW(windowW);
  }

  int t1            = 0;
  const bool haveT1 = CliSpecParsers::parseIntOption(
    parser, opts.iec104T1Opt, 1000, 255000, QStringLiteral("IEC 60870-5-104 t1 timeout (ms)"), t1);
  if (haveT1)
    iec104->setTimeoutT1(t1);

  int t2            = 0;
  const bool haveT2 = CliSpecParsers::parseIntOption(
    parser, opts.iec104T2Opt, 1000, 255000, QStringLiteral("IEC 60870-5-104 t2 timeout (ms)"), t2);
  if (haveT2) {
    if (haveT1 && t2 > t1)
      qWarning() << "IEC 60870-5-104 t2 timeout exceeds t1; the station clamps t2 to t1";

    iec104->setTimeoutT2(t2);
  }

  int t3 = 0;
  if (CliSpecParsers::parseIntOption(parser,
                                     opts.iec104T3Opt,
                                     1000,
                                     255000,
                                     QStringLiteral("IEC 60870-5-104 t3 timeout (ms)"),
                                     t3))
    iec104->setTimeoutT3(t3);
}

/**
 * @brief Configures and connects an IEC 60870-5-104 station from CLI options. The point table is
 *        DISCOVERED by the station interrogation, so a first headless run has nothing to generate
 *        a project from; the warning says so rather than failing silently.
 */
static void setupIec104Connection(IO::ConnectionManager& cm,
                                  const QCommandLineParser& parser,
                                  const CliOptions& opts)
{
  auto* iec104 = cm.iec104();
  cm.setBusType(SerialStudio::BusType::Iec104);
  iec104->setHost(parser.value(opts.iec104Opt));

  int value = 0;
  if (CliSpecParsers::parseIntOption(
        parser, opts.iec104PortOpt, 1, 65535, QStringLiteral("IEC 60870-5-104 port"), value))
    iec104->setPort(value);

  if (CliSpecParsers::parseIntOption(parser,
                                     opts.iec104CaOpt,
                                     0,
                                     65535,
                                     QStringLiteral("IEC 60870-5-104 common address"),
                                     value))
    iec104->setCommonAddress(value);

  applyIec104Timing(iec104, parser, opts);

  if (!parser.isSet(opts.projectOpt) && !iec104->loadGeneratedProject())
    qWarning() << "No IEC 60870-5-104 points are known yet; connect once so the station "
                  "interrogation can discover them, then re-run to generate a project";

  cm.connectDevice();
}

//---------------------------------------------------------------------------------------------------
// Dispatch
//---------------------------------------------------------------------------------------------------

/**
 * @brief Dispatches the first industrial-bus option that is set to its driver setup helper. The
 *        order matches the pre-0070 dispatcher exactly, so two mutually exclusive flags still
 *        resolve the way they always did.
 */
bool apply(IO::ConnectionManager& cm, const QCommandLineParser& parser, const CliOptions& opts)
{
  if (parser.isSet(opts.modbusRtuOpt)) {
    setupModbusRtuConnection(cm, parser, opts);
    return true;
  }

  if (parser.isSet(opts.modbusTcpOpt)) {
    setupModbusTcpConnection(cm, parser, opts);
    return true;
  }

  if (parser.isSet(opts.canbusOpt)) {
    setupCanbusConnection(cm, parser, opts);
    return true;
  }

  if (parser.isSet(opts.opcuaOpt)) {
    setupOpcUaConnection(cm, parser, opts);
    return true;
  }

  if (parser.isSet(opts.s7Opt)) {
    setupS7Connection(cm, parser, opts);
    return true;
  }

  if (parser.isSet(opts.eipOpt)) {
    setupEthernetIpConnection(cm, parser, opts);
    return true;
  }

  if (parser.isSet(opts.iec104Opt)) {
    setupIec104Connection(cm, parser, opts);
    return true;
  }

  return false;
}

#else

/**
 * @brief GPL build stub: no industrial bus option exists, so nothing ever claims the session.
 */
bool apply(IO::ConnectionManager&, const QCommandLineParser&, const CliOptions&)
{
  return false;
}

#endif

}  // namespace CliIndustrialConfig
}  // namespace Misc
