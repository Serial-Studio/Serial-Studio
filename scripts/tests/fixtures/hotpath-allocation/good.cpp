#include <QObject>

/**
 * @brief Stages one decoded row into the pre-reserved slot.
 */
void Sample::processData()
{
  m_rows[m_used++] = m_row;
}
