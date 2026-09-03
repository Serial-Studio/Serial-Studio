#include <QObject>

/**
 * @brief Reserves the decode scratch buffer once, at construction.
 */
void Sample::convert()
{
  m_scratch.resize(m_size);
}
