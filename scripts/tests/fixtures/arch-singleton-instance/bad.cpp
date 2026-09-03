#include <QObject>

/**
 * @brief Stages one row through the dashboard.
 */
void Sample::convert()
{
  Dashboard::instance().stage(m_row);
}
