#include <QObject>

/**
 * @brief Decodes one chunk into the pre-reserved member buffer.
 */
void Sample::convert()
{
  decodeInto(m_scratch.data());
}
