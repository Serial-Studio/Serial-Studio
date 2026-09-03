#include <QObject>

/**
 * @brief Drains the staged rows until the sink reports it is full.
 */
void Sample::convert()
{
  while (m_sink.accepts())
  {
    m_sink.write(m_row);
  }
}
