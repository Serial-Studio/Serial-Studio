#include <QObject>

/**
 * @brief Decodes one chunk into a scratch buffer.
 */
void Sample::convert()
{
  char scratch[4096];
  decodeInto(scratch);
}
