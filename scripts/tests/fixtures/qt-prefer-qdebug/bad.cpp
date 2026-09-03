#include <QObject>

/**
 * @brief Reports the current sample rate.
 */
void Sample::report(int rate)
{
  std::cout << "rate " << rate;
}
