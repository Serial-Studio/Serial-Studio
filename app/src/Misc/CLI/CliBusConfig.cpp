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

#include "Misc/CLI/CliBusConfig.h"

#include <QDebug>
#include <QStringList>

#include "IO/ConnectionManager.h"
#include "IO/Drivers/Network.h"
#include "IO/Drivers/UART.h"
#include "Misc/CLI.h"
#include "Misc/CLI/CliIndustrialConfig.h"
#include "SerialStudio.h"

namespace Misc {
namespace CliBusConfig {

//---------------------------------------------------------------------------------------------------
// UART
//---------------------------------------------------------------------------------------------------

/**
 * @brief Configures and connects the UART bus from CLI options.
 */
static void setupUartConnection(IO::ConnectionManager& cm,
                                const QCommandLineParser& parser,
                                const CliOptions& opts)
{
  cm.setBusType(SerialStudio::BusType::UART);

  if (parser.isSet(opts.uartOpt)) {
    const QString device = parser.value(opts.uartOpt);
    cm.uart()->registerDevice(device);
  }

  if (parser.isSet(opts.baudOpt)) {
    bool ok               = false;
    const qint32 baudRate = parser.value(opts.baudOpt).toInt(&ok);
    if (!ok)
      qWarning() << "Invalid baud rate:" << parser.value(opts.baudOpt);
    else
      cm.uart()->setBaudRate(baudRate);
  }

  cm.connectDevice();
}

//---------------------------------------------------------------------------------------------------
// TCP / UDP
//---------------------------------------------------------------------------------------------------

/**
 * @brief Configures and connects a TCP socket from a CLI host:port string.
 */
static void setupTcpConnection(IO::ConnectionManager& cm, const QString& tcpAddress)
{
  const QStringList parts = tcpAddress.split(':');
  if (parts.size() != 2) {
    qWarning() << "Invalid TCP address format. Expected: host:port";
    return;
  }

  bool ok            = false;
  const quint16 port = parts[1].toUInt(&ok);
  if (!ok || port == 0) {
    qWarning() << "Invalid TCP port:" << parts[1];
    return;
  }

  cm.setBusType(SerialStudio::BusType::Network);
  cm.network()->setTcpSocket();
  cm.network()->setRemoteAddress(parts[0]);
  cm.network()->setTcpPort(port);
  cm.connectDevice();
}

/**
 * @brief Applies the optional --udp-remote spec to the active network driver.
 */
static void applyUdpRemote(IO::ConnectionManager& cm, const QString& udpRemote)
{
  const QStringList parts = udpRemote.split(':');
  if (parts.size() != 2) {
    qWarning() << "Invalid UDP address format. Expected: host:port";
    return;
  }

  bool ok                  = false;
  const quint16 remotePort = parts[1].toUInt(&ok);
  if (!ok || remotePort == 0) {
    qWarning() << "Invalid UDP remote port:" << parts[1];
    return;
  }

  cm.network()->setRemoteAddress(parts[0]);
  cm.network()->setUdpRemotePort(remotePort);
}

/**
 * @brief Configures and connects a UDP socket from CLI options.
 */
static void setupUdpConnection(IO::ConnectionManager& cm,
                               const QCommandLineParser& parser,
                               const CliOptions& opts)
{
  bool ok                 = false;
  const quint16 localPort = parser.value(opts.udpOpt).toUInt(&ok);
  if (!ok || localPort == 0) {
    qWarning() << "Invalid UDP local port:" << parser.value(opts.udpOpt);
    return;
  }

  cm.setBusType(SerialStudio::BusType::Network);
  cm.network()->setUdpSocket();
  cm.network()->setUdpLocalPort(localPort);

  if (parser.isSet(opts.udpMulticastOpt))
    cm.network()->setUdpMulticast(true);

  if (parser.isSet(opts.udpRemoteOpt))
    applyUdpRemote(cm, parser.value(opts.udpRemoteOpt));

  cm.connectDevice();
}

//---------------------------------------------------------------------------------------------------
// WebSocket / HTTP
//---------------------------------------------------------------------------------------------------

/**
 * @brief Configures and connects a WebSocket client from a CLI URL. Scheme validation is left to
 *        the driver so the CLI and the UI reject exactly the same set of URLs.
 */
static void setupWebSocketConnection(IO::ConnectionManager& cm,
                                     const QCommandLineParser& parser,
                                     const CliOptions& opts,
                                     const QString& url)
{
  cm.setBusType(SerialStudio::BusType::Network);

  auto* network = cm.network();
  network->setSocketType(IO::Drivers::Network::SocketType::WebSocket);
  network->setWebSocketUrl(url);

  if (parser.isSet(opts.insecureTlsOpt))
    network->setIgnoreTlsErrors(true);

  cm.connectDevice();
}

/**
 * @brief Configures and connects an HTTP polling source from CLI options.
 */
static void setupHttpConnection(IO::ConnectionManager& cm,
                                const QCommandLineParser& parser,
                                const CliOptions& opts,
                                const QString& url)
{
  cm.setBusType(SerialStudio::BusType::Network);

  auto* network = cm.network();
  network->setSocketType(IO::Drivers::Network::SocketType::Http);
  network->setHttpUrl(url);

  if (parser.isSet(opts.httpMethodOpt)) {
    const QString method = parser.value(opts.httpMethodOpt).toUpper();
    const int index      = network->httpMethods().indexOf(method);
    if (index < 0) {
      qWarning() << "Invalid HTTP method:" << method;
      return;
    }

    network->setHttpMethodIndex(index);
  }

  if (parser.isSet(opts.httpIntervalOpt))
    network->setHttpInterval(parser.value(opts.httpIntervalOpt).toInt());

  if (parser.isSet(opts.httpHeaderOpt))
    network->setHttpHeaders(parser.values(opts.httpHeaderOpt).join(QChar('\n')));

  if (parser.isSet(opts.insecureTlsOpt))
    network->setIgnoreTlsErrors(true);

  cm.connectDevice();
}

//---------------------------------------------------------------------------------------------------
// Dispatch
//---------------------------------------------------------------------------------------------------

/**
 * @brief Dispatches the first bus option that is set to its driver setup helper, transports first
 *        and the Pro industrial buses after, matching the pre-0070 dispatcher order exactly.
 */
void apply(IO::ConnectionManager& cm, const QCommandLineParser& parser, const CliOptions& opts)
{
  if (parser.isSet(opts.uartOpt) || parser.isSet(opts.baudOpt)) {
    setupUartConnection(cm, parser, opts);
    return;
  }

  if (parser.isSet(opts.tcpOpt)) {
    setupTcpConnection(cm, parser.value(opts.tcpOpt));
    return;
  }

  if (parser.isSet(opts.udpOpt)) {
    setupUdpConnection(cm, parser, opts);
    return;
  }

  if (parser.isSet(opts.wsOpt)) {
    setupWebSocketConnection(cm, parser, opts, parser.value(opts.wsOpt));
    return;
  }

  if (parser.isSet(opts.httpOpt)) {
    setupHttpConnection(cm, parser, opts, parser.value(opts.httpOpt));
    return;
  }

  static_cast<void>(CliIndustrialConfig::apply(cm, parser, opts));
}

}  // namespace CliBusConfig
}  // namespace Misc
