#include <QObject>

/**
 * @brief Applies the decoded value to the dataset at @p index.
 */
void Sample::convert(int index)
{
  SS_ASSERT_HOTPATH(index >= 0);
  m_values[index] = 0;
}
