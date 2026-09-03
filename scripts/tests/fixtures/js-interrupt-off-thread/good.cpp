#include <QObject>

/**
 * @brief Stops a runaway script through the watchdog that owns the flag.
 */
void Sample::stopScript()
{
  m_watchdog.disarm();
}
