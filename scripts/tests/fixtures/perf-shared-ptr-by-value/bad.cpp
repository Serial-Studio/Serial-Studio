#include <QObject>

/**
 * @brief Adopts the pooled block.
 */
void Sample::convert(std::shared_ptr<Block> block)
{
  m_block = block;
}
