#include <QObject>

class Block;

/**
 * @brief Binds the staging callback.
 */
void Sample::bind()
{
  void (Block::*handler)() = nullptr;
  m_handler = handler;
}
