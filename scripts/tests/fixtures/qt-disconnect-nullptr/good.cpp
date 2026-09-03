#include <QObject>

/**
 * @brief Detaches the one connection this object owns.
 */
void Sample::detach()
{
  disconnect(m_valueConnection);
}
