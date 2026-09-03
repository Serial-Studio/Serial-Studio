#include <QObject>

/**
 * @brief Resolves the widget behind the generic item.
 */
void Sample::convert()
{
  m_plot = dynamic_cast<Plot *>(m_item);
}
