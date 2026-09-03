#include <QObject>

constexpr int kBlockSampleCap = 64;

/**
 * @brief Resets the staging cursor.
 */
void Sample::reset()
{
  m_used = 0;
}

/**
 * @brief Caps the staging cursor.
 */
void Sample::cap()
{
  m_used = kBlockSampleCap;
}
