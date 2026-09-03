#include <QObject>

/**
 * @brief Stages one decoded row into the open block.
 */
void Sample::processData()
{
  m_used.store(0, std::memory_order_relaxed);
}
