#include <QObject>

/**
 * @brief Adopts the pooled block.
 */
void Sample::convert(const std::shared_ptr<Block> &block)
{
  m_block = block;
}
