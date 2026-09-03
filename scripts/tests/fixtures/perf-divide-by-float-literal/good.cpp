#include <QObject>

/**
 * @brief Converts the raw reading to volts through a precomputed reciprocal.
 */
void Sample::convert()
{
  m_volts = m_raw * kInverseFullScale;
}
