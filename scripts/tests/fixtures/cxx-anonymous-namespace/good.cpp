#include <QObject>

namespace Staging {
constexpr int kBlockSampleCap = 64;
}

/**
 * @brief Resets the staging cursor.
 */
void Sample::reset()
{
  m_used = Staging::kBlockSampleCap;
}
