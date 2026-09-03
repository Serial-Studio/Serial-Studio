/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QAbstractListModel>
#  include <QHash>
#  include <QList>
#  include <QString>
#  include <QStringList>
#  include <QVariantList>
#  include <QVariantMap>

namespace Sessions {

/**
 * @brief The report dialog's dataset picker as a real model (spec 0075, G6): the folder / group /
 *        dataset tree flattened into rows, with the expansion state, the search filter and the
 *        tri-state check propagation. The roles carry the same names the delegate already binds,
 *        so the view is unchanged; the twenty-five JS row walks are gone.
 */
class ReportOptionsModel : public QAbstractListModel {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int count
             READ count
             NOTIFY countChanged)
  Q_PROPERTY(int visibleRowCount
             READ visibleRowCount
             NOTIFY countsChanged)
  Q_PROPERTY(int selectedDatasetCount
             READ selectedDatasetCount
             NOTIFY countsChanged)
  // clang-format on

signals:
  void countChanged();
  void countsChanged();

public:
  /**
   * @brief Row roles; the names match the properties the QML delegate binds.
   */
  enum Roles {
    KindRole = Qt::UserRole + 1,
    NodeIdRole,
    ParentIdRole,
    DepthRole,
    ExpandedRole,
    HasChildrenRole,
    RowVisibleRole,
    LabelRole,
    SourceLabelRole,
    CheckStateRole,
    UniqueIdRole,
    CheckedRole,
  };
  Q_ENUM(Roles)

  explicit ReportOptionsModel(QObject* parent = nullptr);

  ReportOptionsModel(ReportOptionsModel&&)                 = delete;
  ReportOptionsModel(const ReportOptionsModel&)            = delete;
  ReportOptionsModel& operator=(ReportOptionsModel&&)      = delete;
  ReportOptionsModel& operator=(const ReportOptionsModel&) = delete;

  [[nodiscard]] int count() const noexcept;
  [[nodiscard]] int visibleRowCount() const noexcept;
  [[nodiscard]] int selectedDatasetCount() const noexcept;

  [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

public slots:
  void clear();
  void build(const QVariantList& datasets, const QVariantMap& folderByGroup);
  void setSearch(const QString& query);
  void toggleExpanded(const int row);
  void setAllExpanded(const bool expanded);
  void setSubtreeChecked(const int row, const bool checked);

  [[nodiscard]] QVariantList selectedUniqueIds() const;

private:
  /**
   * @brief One flattened tree row: a folder, a group header or a dataset leaf.
   */
  struct Row {
    QString kind;
    QString nodeId;
    QString parentId;
    QString label;
    QString sourceLabel;
    int depth        = 0;
    int uniqueId     = -1;
    int checkState   = Qt::Checked;
    bool expanded    = false;
    bool hasChildren = false;
    bool rowVisible  = false;
    bool checked     = true;
  };

  /**
   * @brief One folder in the tree the group paths describe, before flattening.
   */
  struct FolderNode {
    QString fullPath;
    QStringList childOrder;
    QStringList groups;
  };

  void emitFolderNode(const QString& path, const int depth);
  void recomputeVisibility();
  void recomputeCollapsedVisibility();
  void recomputeSearchVisibility();
  void recomputeAncestors(const QString& parentId);
  void recomputeNode(const int headerIndex);
  void refreshSelectedCount();
  void applyRowChecked(const int row, const bool checked);
  void publishRowChange(const int row, const QList<int>& roles);

  [[nodiscard]] int indexOfNode(const QString& nodeId) const;

  QList<Row> m_rows;
  QHash<QString, int> m_indexByNode;
  QHash<QString, FolderNode> m_folders;
  QHash<QString, QVariantMap> m_groupsByKey;
  QHash<QString, QVariantList> m_itemsByKey;
  QString m_search;
  int m_visibleRowCount;
  int m_selectedDatasetCount;
};

}  // namespace Sessions

#endif  // BUILD_COMMERCIAL
