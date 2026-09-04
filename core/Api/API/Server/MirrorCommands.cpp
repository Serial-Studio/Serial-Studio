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

#include "API/Server/MirrorCommands.h"

#include "API/Mirror/MirrorProtocol.h"
#include "API/Mirror/MirrorPublisher.h"
#include "Core/SSAssert.h"

//--------------------------------------------------------------------------------------------------
// Command classification
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether a command name is one of the connection-scoped mirror commands.
 */
bool API::MirrorCommands::isMirrorCommand(const QString& command)
{
  return command == QLatin1String(Mirror::Command::Subscribe)
      || command == QLatin1String(Mirror::Command::SetRate)
      || command == QLatin1String(Mirror::Command::Unsubscribe);
}

//--------------------------------------------------------------------------------------------------
// Subscription verbs
//--------------------------------------------------------------------------------------------------

/**
 * @brief Subscribes this connection to the mirror and, by default, opts it out of the per-frame
 *        broadcast: at capture rates that stream would disconnect the viewer on the byte cap long
 *        before the network noticed, which is why subscribe is the first request a viewer sends.
 */
API::CommandResponse API::MirrorCommands::subscribe(MirrorPublisher& publisher,
                                                    QTcpSocket* socket,
                                                    ConnectionState& state,
                                                    const CommandRequest& request,
                                                    const StreamFramesSetter& setStreamFrames)
{
  SS_ASSERT(socket != nullptr,
            return CommandResponse::makeError(
              request.id, ErrorCode::ExecutionError, QStringLiteral("No connection")));
  SS_ASSERT_LOG(state.authenticated);

  const int version = request.params.value(QStringLiteral("wireVersion")).toInt(0);
  if (version != Mirror::kWireVersion) {
    return CommandResponse::makeError(
      request.id,
      QLatin1String(Mirror::ErrorCode::VersionMismatch),
      QStringLiteral("This instance speaks mirror wire version %1, the client asked for %2")
        .arg(QString::number(Mirror::kWireVersion), QString::number(version)));
  }

  const auto hzValue = request.params.value(QStringLiteral("hz"));
  const int hz       = hzValue.isUndefined() ? Mirror::kHzDefault : hzValue.toInt(0);
  if (hz < Mirror::kHzMin || hz > Mirror::kHzMax) {
    return CommandResponse::makeError(
      request.id,
      QLatin1String(Mirror::ErrorCode::RateOutOfRange),
      QStringLiteral("Mirror rate %1 Hz is outside the supported range").arg(QString::number(hz)));
  }

  const int precision = request.params.value(QStringLiteral("precision")).toInt(0);
  if (precision < Mirror::kPrecisionMin || precision > Mirror::kPrecisionMax) {
    return CommandResponse::makeError(
      request.id,
      ErrorCode::InvalidParam,
      QStringLiteral("Value precision %1 is outside 0 (full) to 17 significant digits")
        .arg(QString::number(precision)));
  }

  if (!publisher.subscribe(socket, state.sessionId, hz, precision)) {
    return CommandResponse::makeError(
      request.id,
      QLatin1String(Mirror::ErrorCode::ViewerLimit),
      QStringLiteral("This instance is not accepting more mirror viewers"));
  }

  state.mirrorSubscribed = true;
  state.mirrorHz         = hz;
  state.mirrorPrecision  = precision;
  setStreamFrames(request.params.value(QStringLiteral("frames")).toBool(false));

  auto result = publisher.info();
  result.insert(QStringLiteral("connectionId"), state.sessionId);
  result.insert(QStringLiteral("hz"), hz);
  result.insert(QStringLiteral("effectiveHz"), publisher.effectiveHz(hz));
  result.insert(QStringLiteral("frames"), state.streamFrames);
  result.insert(QStringLiteral("precision"), precision);
  return CommandResponse::makeSuccess(request.id, result);
}

/**
 * @brief Renegotiates this connection's mirror cadence. An out-of-range rate is refused rather
 *        than clamped: a silent clamp hides a misconfigured viewer.
 */
API::CommandResponse API::MirrorCommands::setRate(MirrorPublisher& publisher,
                                                  ConnectionState& state,
                                                  const CommandRequest& request)
{
  if (!state.mirrorSubscribed) {
    return CommandResponse::makeError(request.id,
                                      QLatin1String(Mirror::ErrorCode::NotSubscribed),
                                      QStringLiteral("This connection holds no mirror "
                                                     "subscription"));
  }

  const int hz = request.params.value(QStringLiteral("hz")).toInt(0);
  if (hz < Mirror::kHzMin || hz > Mirror::kHzMax) {
    return CommandResponse::makeError(
      request.id,
      QLatin1String(Mirror::ErrorCode::RateOutOfRange),
      QStringLiteral("Mirror rate %1 Hz is outside the supported range").arg(QString::number(hz)));
  }

  if (!publisher.setRate(state.sessionId, hz)) {
    return CommandResponse::makeError(request.id,
                                      QLatin1String(Mirror::ErrorCode::NotSubscribed),
                                      QStringLiteral("This connection holds no mirror "
                                                     "subscription"));
  }

  state.mirrorHz = hz;

  QJsonObject result;
  result.insert(QStringLiteral("hz"), hz);
  result.insert(QStringLiteral("effectiveHz"), publisher.effectiveHz(hz));
  return CommandResponse::makeSuccess(request.id, result);
}

/**
 * @brief Stops this connection's mirror. The frame stream stays off: only mirror.subscribe ever
 *        changes that flag, so unsubscribing cannot reopen the firehose on a slow reader.
 */
API::CommandResponse API::MirrorCommands::unsubscribe(MirrorPublisher& publisher,
                                                      ConnectionState& state,
                                                      const CommandRequest& request)
{
  if (!state.mirrorSubscribed) {
    return CommandResponse::makeError(request.id,
                                      QLatin1String(Mirror::ErrorCode::NotSubscribed),
                                      QStringLiteral("This connection holds no mirror "
                                                     "subscription"));
  }

  publisher.unsubscribe(state.sessionId);
  state.mirrorSubscribed = false;

  QJsonObject result;
  result.insert(QStringLiteral("mirrorSubscribed"), false);
  return CommandResponse::makeSuccess(request.id, result);
}
