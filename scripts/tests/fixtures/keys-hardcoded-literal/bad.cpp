#include <QObject>

/**
 * @brief Writes the source's bus type into the project JSON.
 */
void Sample::convert()
{
  QJsonObject obj;
  obj["busType"] = m_busType;
}
