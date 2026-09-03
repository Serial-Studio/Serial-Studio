#include <QObject>

/**
 * @brief Announces a new value to every connected slot.
 */
void Sample::notify()
{
  emit valueChanged();
}
