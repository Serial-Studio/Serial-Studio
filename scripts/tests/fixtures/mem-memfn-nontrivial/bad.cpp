#include <QObject>

/**
 * @brief Clears the staged label.
 */
void Sample::convert()
{
  QString label;
  memset(&label, 0, sizeof(label));
}
