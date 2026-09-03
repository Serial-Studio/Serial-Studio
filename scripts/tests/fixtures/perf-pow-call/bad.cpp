#include <QObject>

/**
 * @brief Squares the decoded magnitude.
 */
void Sample::convert()
{
  m_square = std::pow(m_value, 2.0);
}
