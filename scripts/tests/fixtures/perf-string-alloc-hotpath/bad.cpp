#include <QObject>

/**
 * @brief Hands the decoded label to the sink.
 */
void Sample::processData()
{
  m_sink.write(m_label.toUtf8());
}
