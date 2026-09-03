#include <QObject>

/**
 * @brief Builds the staging block for this session, parented to this.
 */
void Sample::convert()
{
  m_block = new Block(this);
}
