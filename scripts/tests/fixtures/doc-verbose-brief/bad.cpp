#include <QObject>

/**
 * @brief Resets the staging cursor.
 *
 * @param cap the block sample cap
 * @return nothing
 */
void Sample::reset(int cap)
{
  m_used = cap;
}
