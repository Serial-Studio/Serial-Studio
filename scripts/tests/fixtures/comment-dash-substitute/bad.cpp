#include <QObject>

/**
 * @brief Resets the staging cursor.
 */
void Sample::reset()
{
  //---------------------------------------------------------
  // Cursor reset -- it precedes staging.
  //---------------------------------------------------------
  m_used = 0;
}
