#include <QObject>

/**
 * @brief Stages one decoded row into the open block.
 */
void Sample::processData()
{
  QMutexLocker locker(&m_mutex);
  m_used = 0;
}
