#include <QObject>

/**
 * @brief Reports the current sample rate.
 */
void Sample::report(int rate)
{
  qDebug() << "rate" << rate;
}
