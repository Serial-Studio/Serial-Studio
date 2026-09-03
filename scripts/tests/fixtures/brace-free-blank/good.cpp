#include <QObject>

/**
 * @brief Clamps the staging cursor to the block cap.
 */
void Sample::convert()
{
  if (m_used > kCap)
    m_used = kCap;

  m_open = true;
}
