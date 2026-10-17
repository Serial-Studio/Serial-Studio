// SPDX-FileCopyrightText: 2026 Alex Spataru <https://serial-studio.com/>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QMutex>

/**
 * @brief Counts frames under a lock.
 */
int Reader::count()
{
  QMutexLocker guard(&m_lock);
  return m_count;
}
