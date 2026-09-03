#include <QObject>

/**
 * @brief Tears down the worker's signals before it is joined.
 */
void Sample::detach()
{
  disconnect(m_worker, nullptr, this, nullptr);
}
