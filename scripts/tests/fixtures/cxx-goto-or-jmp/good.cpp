#include <QObject>

/**
 * @brief Decodes one chunk, bailing to the shared cleanup.
 */
void Sample::convert()
{
  if (m_open)
    m_used = 0;

  m_open = false;
}
