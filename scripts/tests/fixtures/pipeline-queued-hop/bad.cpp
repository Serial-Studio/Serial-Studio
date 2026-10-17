// SPDX-FileCopyrightText: 2026 Alex Spataru <https://serial-studio.com/>
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @brief Wires the parser to the builder across the queue.
 */
void Host::wire()
{
  connect(m_frameParser,
          &FrameParser::frameReady,
          m_frameBuilder,
          &FrameBuilder::onFrame,
          Qt::QueuedConnection);
}
