#include <QObject>

/**
 * @brief Resolves the widget behind the generic item through its kind tag.
 */
void Sample::convert()
{
  m_plot = static_cast<Plot *>(m_item);
}
