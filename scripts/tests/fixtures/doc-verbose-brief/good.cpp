#include <QObject>

/**
 * @brief Resets the staging cursor to @p cap.
 */
void Sample::reset(int cap)
{
  m_used = cap;
}
