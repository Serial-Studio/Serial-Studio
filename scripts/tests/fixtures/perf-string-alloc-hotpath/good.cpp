#include <QObject>

/**
 * @brief Hands the decoded label to the sink from its cached byte form.
 */
void Sample::processData()
{
  m_sink.write(m_labelUtf8);
}
