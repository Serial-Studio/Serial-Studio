#include <QObject>

/**
 * @brief Builds the sample against the session.
 */
Sample::Sample(SessionContext& context)
  : m_context(context)
{
  m_dashboard = &Dashboard::instance();
}
