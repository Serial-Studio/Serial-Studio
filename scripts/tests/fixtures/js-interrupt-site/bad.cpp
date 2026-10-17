// SPDX-FileCopyrightText: 2026 Alex Spataru <https://serial-studio.com/>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QJSEngine>

/**
 * @brief Interrupts the engine by hand.
 */
void Sample::stop()
{
  m_engine.setInterrupted(true);
}
