#include <QObject>

/**
 * @brief Matches the frame header.
 */
void Sample::convert()
{
  m_matched = QRegularExpression(m_pattern).match(m_text).hasMatch();
}
