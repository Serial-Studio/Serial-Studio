#include <QObject>

/**
 * @brief Formats the status line in one pass.
 */
void Sample::convert()
{
  m_text = m_format.arg(m_name, m_value);
}
