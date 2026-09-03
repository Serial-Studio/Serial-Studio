#include <QObject>

/**
 * @brief Formats the status line.
 */
void Sample::convert()
{
  m_text = m_format.arg(m_name).arg(m_value);
}
