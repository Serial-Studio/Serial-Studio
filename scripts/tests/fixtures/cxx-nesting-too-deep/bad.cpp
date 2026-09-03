#include <QObject>

/**
 * @brief Records the first in-range sample.
 */
void Sample::convert()
{
  if (m_open)
  {
    for (int i = 0; i < 4; ++i)
    {
      if (m_values[i] > 0)
      {
        if (m_values[i] < 9)
        {
          m_used = i;
        }
      }
    }
  }
}
