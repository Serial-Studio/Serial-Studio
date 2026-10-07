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

#include "Core/SerialStudio.h"
#include "IO/ConnectionManager/DeviceTableQuery.h"

class QObject;

namespace IO {

class ConnectFanOut;

/**
 * @brief The licensing verdict on a connect and on a live session (specs 0092, 0094): refuses what
 *        the entitlement does not cover and raises the Pro intent, ends a live session whose
 *        project gains Pro content past its admission (driver reopens included), and holds the
 *        device rebuild a licensing transition asks for while a session it cannot affect is live.
 */
class EntitlementGate {
public:
  /**
   * @brief The facade actions the gate drives, bound once by the facade's constructor.
   */
  struct Actions {
    std::function<void()> rebuild;
    std::function<void()> disconnect;
    std::function<void()> connect;
  };

  EntitlementGate(const DeviceTableQuery& query,
                  const ConnectFanOut& fanOut,
                  const SerialStudio::BusType& busType,
                  const SerialStudio::OperationMode& mode);

  EntitlementGate(EntitlementGate&&)                 = delete;
  EntitlementGate(const EntitlementGate&)            = delete;
  EntitlementGate& operator=(EntitlementGate&&)      = delete;
  EntitlementGate& operator=(const EntitlementGate&) = delete;

  [[nodiscard]] bool deferRebuild();
  [[nodiscard]] bool sessionLive() const;
  [[nodiscard]] bool refuseRecovery() const;
  [[nodiscard]] bool refuseConnect(bool raiseIntent);
  [[nodiscard]] DeviceTableQuery::ConnectRefusal pendingRefusal() const;
  [[nodiscard]] static QString refusalReason(DeviceTableQuery::ConnectRefusal refusal);

  void noteRebuild();
  void observeContent();
  void queueReconnect();
  void releaseDeferredRebuild();
  void bind(QObject* context, Actions actions);

private:
  [[nodiscard]] bool connected() const;
  [[nodiscard]] bool exceedsAdmitted(int transforms, int tables) const noexcept;
  void endGrownSession(bool raiseIntent);

  QObject* m_context;
  Actions m_actions;
  const ConnectFanOut& m_fanOut;
  const DeviceTableQuery& m_query;
  const SerialStudio::BusType& m_busType;
  const SerialStudio::OperationMode& m_mode;
  bool m_builtEntitled;
  bool m_rebuildDeferred;
  int m_admittedTables;
  int m_admittedTransforms;
};

}  // namespace IO
