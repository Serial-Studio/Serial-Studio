#include <QObject>

/**
 * @brief Announces a new value to every connected slot.
 */
void Sample::notify()
{
  Q_EMIT valueChanged();
}
