#include <QObject>

/**
 * @brief Pins the acquisition buffer so a page fault never stalls the reader.
 */
void Sample::pin(qsizetype bytes)
{
  m_buffer.resize(bytes);
  (void)lockMemoryResident(m_buffer.data(), bytes);
}
