#include <QObject>

/**
 * @brief Resets the staging cursor.
 */
void Sample::reset()
{
  //---------------------------------------------------------
  // TODO(0075 A2): the overflow path drops whole blocks; counted, not logged.
  //---------------------------------------------------------
  m_used = 0;
}
