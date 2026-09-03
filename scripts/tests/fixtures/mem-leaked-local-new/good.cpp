#include <QObject>

/**
 * @brief Builds the staging block for this session.
 */
void Sample::convert()
{
  auto block = std::make_unique<Block>();
  m_capacity = block->capacity;
}
