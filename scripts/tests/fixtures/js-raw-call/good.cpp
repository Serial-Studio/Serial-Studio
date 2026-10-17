// SPDX-FileCopyrightText: 2026 Alex Spataru <https://serial-studio.com/>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QJSValue>

/**
 * @brief Calls the parser through the watchdog.
 */
QJSValue Sample::run()
{
  return m_watchdog.call(m_parse, m_args);
}
