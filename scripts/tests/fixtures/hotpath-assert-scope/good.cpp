#include <QObject>

/**
 * @brief Applies the decoded value to the dataset at @p index.
 */
void Sample::convert(int index)
{
  SS_ASSERT(index >= 0, return);
  m_values[index] = 0;
}
