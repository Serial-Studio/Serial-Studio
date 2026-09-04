/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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

#include <QObject>
#include <QVariantList>
#include <QVariantMap>

namespace API {

class CommandHandler;
class CommandRegistry;

/**
 * @brief QML-instantiable bridge behind the API Terminal window: runs one command line through
 *        the in-process handler and dumps the live command catalog for discovery (spec 0045).
 */
class TerminalBridge : public QObject {
  // clang-format off
  Q_OBJECT
  // clang-format on

public:
  explicit TerminalBridge(QObject* parent = nullptr);
  TerminalBridge(TerminalBridge&&)                 = delete;
  TerminalBridge(const TerminalBridge&)            = delete;
  TerminalBridge& operator=(TerminalBridge&&)      = delete;
  TerminalBridge& operator=(const TerminalBridge&) = delete;

  Q_INVOKABLE [[nodiscard]] QVariantMap run(const QString& input);
  Q_INVOKABLE [[nodiscard]] QVariantList catalog() const;

private:
  CommandHandler& m_handler;
  CommandRegistry& m_registry;
};

}  // namespace API
