#include <QObject>

/**
 * @brief Reserves the decode scratch buffer.
 */
void Sample::convert()
{
  m_scratch = malloc(m_size);
}
