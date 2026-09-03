#include <QObject>

class Block
{
public:
  void reset();
};

/**
 * @brief Binds the staging callback.
 */
void Sample::bind()
{
  void (Block::*handler)() = nullptr;
  m_handler = handler;
}
