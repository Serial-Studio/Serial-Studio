#include <QObject>

/**
 * @brief Renames the group at @p index.
 */
void DataModel::ProjectModel::renameGroup(int index)
{
  ProjectUndoScope scope(this, tr("Rename Group"));
  m_groups[index].title = m_pendingTitle;
  setModified(true);
}
