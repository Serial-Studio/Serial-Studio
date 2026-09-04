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

#ifdef BUILD_COMMERCIAL

#  include <QtGlobal>

#  include "MQTT/Publisher.h"

/**
 * @file mqtt_publisher_stub.cpp
 * @brief Link-only stand-ins for suites that compile DataTable.cpp under BUILD_COMMERCIAL.
 *
 * TableApiBridge::mqttPublish() is compiled whenever the tier is configured Pro, so the suite
 * needs MQTT::Publisher at link time even though no test ever routes a table write to a broker.
 * Linking the real Publisher.cpp instead would drag FrameBuilder, ProjectModel and the worker
 * thread into a suite that exercises none of them (the same closure the session_context_stub
 * comment describes). Reaching either symbol at runtime means a test walked onto the publish
 * path, which is a test-authoring bug, not a broker outage.
 */
MQTT::Publisher& MQTT::Publisher::instance()
{
  qFatal("MQTT::Publisher::instance() reached from a unit test without a composition root");
}

/**
 * @brief Fails loudly instead of silently swallowing a payload a test did not expect to send.
 */
qint64 MQTT::Publisher::mqttPublish(const QString&, const QByteArray&, int, bool)
{
  qFatal("MQTT::Publisher::mqttPublish() reached from a unit test without a broker");
}

#endif
