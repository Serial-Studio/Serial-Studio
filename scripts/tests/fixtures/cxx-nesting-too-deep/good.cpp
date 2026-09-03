#include <QObject>

/**
 * @brief Records the first in-range sample.
 */
void Sample::convert()
{
  if (!m_open)
    return;

  for (int i = 0; i < 4; ++i)
  {
    if (m_values[i] <= 0 || m_values[i] >= 9)
      continue;

    m_used = i;
  }
}
