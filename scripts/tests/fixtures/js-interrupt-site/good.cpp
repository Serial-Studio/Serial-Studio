// SPDX-FileCopyrightText: 2026 Alex Spataru <https://serial-studio.com/>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QJSEngine>

/**
 * @brief Clears the interrupt flag.
 */
void Sample::resume()
{
  m_engine.setInterrupted(false);
}
