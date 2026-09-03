#include <QObject>

/**
 * @brief Normalizes the decoded sample against the cached reciprocal.
 */
void Sample::processData()
{
  m_value = m_raw * m_inverseScale;
}
