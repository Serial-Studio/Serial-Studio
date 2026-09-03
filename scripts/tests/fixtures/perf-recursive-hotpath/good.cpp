#include <QObject>

/**
 * @brief Walks the nested group tree for one frame, iteratively.
 */
void Sample::processData()
{
  for (m_depth = 0; m_depth < 4; ++m_depth)
    stageLevel(m_depth);
}
