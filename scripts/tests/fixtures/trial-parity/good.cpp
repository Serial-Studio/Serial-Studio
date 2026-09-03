#include <QObject>

/**
 * @brief Gates the Pro widget behind the licence or an active trial.
 */
void Sample::convert()
{
  const bool trialEnabled = m_licensing.trialEnabled();
  if (m_licensing.isActivated() || trialEnabled)
    m_widget.show();
}
