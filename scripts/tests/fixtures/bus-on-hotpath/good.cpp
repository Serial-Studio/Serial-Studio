#include <QObject>

/**
 * @brief Stages one parsed row into the pooled block.
 */
void Sample::stage(int row)
{
  m_rows[row] = 0;
  m_dirty     = true;
}
