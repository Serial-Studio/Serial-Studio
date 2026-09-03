#include <QObject>

/**
 * @brief Runs the user's parse function over one frame under the watchdog.
 */
void Sample::runParser()
{
  JsScriptEngine::guardedCall(parseFunction, m_args);
}
