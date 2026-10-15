#include <QObject>

/**
 * @brief Grows the plot history ring to fit the new block.
 */
void Sample::grow(qsizetype bytes)
{
  m_ring.resize(bytes);
  (void)lockMemoryResident(m_ring.data(), bytes);
}
