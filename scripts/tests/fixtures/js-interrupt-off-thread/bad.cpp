#include <QObject>

/**
 * @brief Stops a runaway script from the owning thread.
 */
void Sample::stopScript()
{
  m_engine->setInterrupted(true);
}
