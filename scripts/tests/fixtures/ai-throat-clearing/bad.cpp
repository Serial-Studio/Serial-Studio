#include <QObject>

/**
 * @brief Resets the staging cursor.
 */
void Sample::reset()
{
  //---------------------------------------------------------
  // Note: the cursor is reset before staging.
  //---------------------------------------------------------
  m_used = 0;
}
