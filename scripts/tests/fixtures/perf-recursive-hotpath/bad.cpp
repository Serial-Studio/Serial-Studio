#include <QObject>

/**
 * @brief Walks the nested group tree for one frame.
 */
void Sample::processData()
{
  if (m_depth < 4)
  {
    ++m_depth;
    processData();
  }
}
