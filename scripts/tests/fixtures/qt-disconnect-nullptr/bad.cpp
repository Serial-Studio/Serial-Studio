#include <QObject>

/**
 * @brief Detaches every slot bound to the cached connection.
 */
void Sample::detach()
{
  disconnect(m_valueConnection, nullptr);
}
