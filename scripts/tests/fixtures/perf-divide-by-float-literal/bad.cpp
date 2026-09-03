#include <QObject>

/**
 * @brief Converts the raw reading to volts.
 */
void Sample::convert()
{
  m_volts = m_raw / 2.5;
}
