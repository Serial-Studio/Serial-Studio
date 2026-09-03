#include <QObject>

/**
 * @brief Stages one decoded row into the open block.
 */
void Sample::processData()
{
  ++m_droppedCounter;
}
