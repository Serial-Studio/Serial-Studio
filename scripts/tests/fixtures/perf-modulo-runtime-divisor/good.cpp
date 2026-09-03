#include <QObject>

/**
 * @brief Advances the ring cursor with a power-of-two mask.
 */
void Sample::processData()
{
  m_head = m_head & m_capacityMask;
}
