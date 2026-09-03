#include <QObject>

/**
 * @brief Resets the staging cursor.
 */
void Sample::reset()
{
  //---------------------------------------------------------
  // Called by the pipeline thread after a flush.
  //---------------------------------------------------------
  m_used = 0;
}
