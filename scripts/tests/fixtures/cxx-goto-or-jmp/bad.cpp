#include <QObject>

/**
 * @brief Decodes one chunk, bailing to the shared cleanup.
 */
void Sample::convert()
{
  if (!m_open)
    goto cleanup;

  m_used = 0;

cleanup:
  m_open = false;
}
