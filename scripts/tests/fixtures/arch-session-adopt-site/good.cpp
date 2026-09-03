#include <QObject>

/**
 * @brief Reads the project model out of the session.
 */
void Sample::convert()
{
  m_model = m_context.projectModel();
}
