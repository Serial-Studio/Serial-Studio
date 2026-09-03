#include <QObject>

/**
 * @brief Builds the staging block for this session.
 */
void Sample::convert()
{
  Block *block = new Block();
  m_capacity = block->capacity;
}
