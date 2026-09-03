#include <QObject>

/**
 * @brief Selects the serial port by name.
 */
void IO::Drivers::Sample::setPortName(const QString &name)
{
  m_portName = name;
  Q_EMIT portNameChanged();
}
