#include <QObject>

/**
 * @brief Applies the decoded value to the dataset at @p index.
 */
void Sample::apply(int index)
{
  Q_ASSERT(index >= 0);
  m_values[index] = 0;
}
