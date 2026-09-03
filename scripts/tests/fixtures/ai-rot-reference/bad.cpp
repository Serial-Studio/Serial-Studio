#include <QObject>

/**
 * @brief Resets the staging cursor.
 */
void Sample::reset()
{
  //---------------------------------------------------------
  // This commit resets the cursor earlier than before.
  //---------------------------------------------------------
  m_used = 0;
}
