#include <QObject>

/**
 * @brief Stages one decoded row into the open block.
 */
void Sample::processData()
{
  m_rows.push_back(m_row);
}
