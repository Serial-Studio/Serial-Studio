#include <QObject>

/**
 * @brief Resets the staging cursor.
 */
void Sample::reset()
{
  //---------------------------------------------------------
  // We need to reset the cursor before staging.
  //---------------------------------------------------------
  m_used = 0;
}
