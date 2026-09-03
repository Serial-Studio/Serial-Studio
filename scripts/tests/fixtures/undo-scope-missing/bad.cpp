#include <QObject>

/**
 * @brief Renames the group at @p index.
 */
void DataModel::ProjectModel::renameGroup(int index)
{
  m_groups[index].title = m_pendingTitle;
  setModified(true);
}
