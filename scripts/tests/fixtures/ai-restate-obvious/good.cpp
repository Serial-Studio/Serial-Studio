#include <QObject>

/**
 * @brief Resets the staging cursor.
 */
void Sample::reset()
{
  //---------------------------------------------------------
  // Zero is the only value the sink treats as empty.
  //---------------------------------------------------------
  m_used = 0;
}
