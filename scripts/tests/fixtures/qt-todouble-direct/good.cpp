#include <QObject>

/**
 * @brief Parses one numeric field out of the frame.
 */
void Sample::field(const QString &text)
{
  m_value = SerialStudio::toDouble(text);
}
