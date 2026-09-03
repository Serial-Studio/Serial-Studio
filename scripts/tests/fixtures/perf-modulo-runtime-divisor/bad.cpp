#include <QObject>

/**
 * @brief Advances the ring cursor.
 */
void Sample::processData()
{
  m_head = m_head % m_capacity;
}
