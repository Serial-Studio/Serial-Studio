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

#pragma once

#include <functional>
#include <QString>

#include "API/CommandProtocol.h"
#include "API/Server/ConnectionState.h"

class QTcpSocket;

namespace API {

class MirrorPublisher;

/**
 * @brief The connection-scoped mirror verbs (spec 0040). They live outside CommandRegistry because
 *        they mutate per-socket state the registry has no access to, and outside Server because
 *        nothing here touches a socket, the worker thread, or the wire payloads: the publisher owns
 *        the snapshot encoding, these functions only negotiate one viewer's subscription.
 */
namespace MirrorCommands {

/**
 * @brief Applies a subscriber's per-frame broadcast decision to the worker thread.
 */
using StreamFramesSetter = std::function<void(const bool enabled)>;

/**
 * @brief Whether a command name is one of the connection-scoped mirror commands.
 */
[[nodiscard]] bool isMirrorCommand(const QString& command);

/**
 * @brief Subscribes this connection to the mirror and, by default, opts it out of the per-frame
 *        broadcast: at capture rates that stream would disconnect the viewer on the byte cap long
 *        before the network noticed, which is why subscribe is the first request a viewer sends.
 */
[[nodiscard]] CommandResponse subscribe(MirrorPublisher& publisher,
                                        QTcpSocket* socket,
                                        ConnectionState& state,
                                        const CommandRequest& request,
                                        const StreamFramesSetter& setStreamFrames);

/**
 * @brief Renegotiates this connection's mirror cadence. An out-of-range rate is refused rather
 *        than clamped: a silent clamp hides a misconfigured viewer.
 */
[[nodiscard]] CommandResponse setRate(MirrorPublisher& publisher,
                                      ConnectionState& state,
                                      const CommandRequest& request);

/**
 * @brief Stops this connection's mirror. The frame stream stays off: only mirror.subscribe ever
 *        changes that flag, so unsubscribing cannot reopen the firehose on a slow reader.
 */
[[nodiscard]] CommandResponse unsubscribe(MirrorPublisher& publisher,
                                          ConnectionState& state,
                                          const CommandRequest& request);

}  // namespace MirrorCommands
}  // namespace API
