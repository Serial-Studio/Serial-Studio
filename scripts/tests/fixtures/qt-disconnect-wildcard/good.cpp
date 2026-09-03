#include <QObject>

/**
 * @brief Tears down the worker's signals before it is joined.
 */
void Sample::detach()
{
  disconnect(m_workerFrameConnection);
  disconnect(m_workerErrorConnection);
}
