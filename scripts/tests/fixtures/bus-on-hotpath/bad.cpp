#include "Core/Bus/MessageBus.h"

/**
 * @brief Stages one parsed row into the pooled block.
 */
void Sample::stage(int row)
{
  m_rows[row] = 0;
  Core::Bus::MessageBus::instance()->publish<Core::Bus::DashboardStructureChanged>(row);
}
