#include <QObject>

/**
 * @brief Resets the staging cursor.
 */
void Sample::convert()
{
  // Closes the block.
  // Then rewinds the cursor.
  m_used = 0;
}
