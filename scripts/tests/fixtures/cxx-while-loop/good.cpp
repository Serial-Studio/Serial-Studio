#include <QObject>

/**
 * @brief Drains the staged rows, bounded by the block's sample cap.
 */
void Sample::convert()
{
  for (int i = 0; i < kBlockSampleCap && m_sink.accepts(); ++i)
  {
    m_sink.write(m_row);
  }
}
