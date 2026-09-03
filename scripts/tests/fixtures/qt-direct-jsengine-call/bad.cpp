#include <QObject>

/**
 * @brief Runs the user's parse function over one frame.
 */
void Sample::runParser()
{
  parseFunction.call(m_args);
}
