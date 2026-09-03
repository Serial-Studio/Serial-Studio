#include <QObject>

/**
 * @brief Gates the Pro widget behind the licence.
 */
void Sample::convert()
{
  if (m_licensing.isActivated())
    m_widget.show();
}
