#include <QObject>

/**
 * @brief Matches the frame header against the shared, prebuilt pattern.
 */
void Sample::convert()
{
  m_matched = kHeaderPattern.match(m_text).hasMatch();
}
