#include <QObject>

/**
 * @brief Installs the project model into the session.
 */
void Sample::convert()
{
  m_context.adoptProjectModel(std::move(model));
}
