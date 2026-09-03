#include <QObject>

namespace {
constexpr int kBlockSampleCap = 64;
}

/**
 * @brief Resets the staging cursor.
 */
void Sample::reset()
{
  m_used = kBlockSampleCap;
}
