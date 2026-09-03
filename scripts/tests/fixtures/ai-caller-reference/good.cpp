#include <QObject>

/**
 * @brief Resets the staging cursor.
 */
void Sample::reset()
{
  //---------------------------------------------------------
  // Pipeline-thread only: the cursor is not atomic.
  //---------------------------------------------------------
  m_used = 0;
}
