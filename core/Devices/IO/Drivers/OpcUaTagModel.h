/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
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

#include <memory>
#include <QAbstractItemModel>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QQueue>
#include <QString>
#include <QVariant>

#include "IO/Drivers/OpcUa.h"
#include "IO/Drivers/OpcUaSession.h"
#include "IO/Drivers/OpcUaTypes.h"
#include "IO/Drivers/OpcUaWire.h"

namespace IO {
namespace Drivers {

/**
 * @brief Lazily-browsed tree over an OPC UA address space for the tag picker (spec 0066 R6):
 *        one Browse per expansion and one batched Read per level, Variables expand too (struct
 *        members and properties live under them), and units resolve for ticked tags only.
 */
class OpcUaTagModel : public QAbstractItemModel {
  Q_OBJECT
  Q_PROPERTY(int selectedCount READ selectedCount NOTIFY selectionChanged)
  Q_PROPERTY(int selectedIndices READ selectedIndices NOTIFY selectionChanged)
  Q_PROPERTY(bool overSoftLimit READ overSoftLimit NOTIFY selectionChanged)
  Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

signals:
  void busyChanged();
  void selectionChanged();
  void browseError(const QString& nodeId, const QString& reason);
  void expandRequested(const QModelIndex& index);

public:
  enum Roles {
    NameRole = Qt::UserRole + 1,
    NodeIdRole,
    TypeCodeRole,
    AccessRole,
    CheckedRole,
    SelectableRole,
    FolderRole,
    ArrayLenRole,
    DescriptionRole,
    UnitRole,
  };
  Q_ENUM(Roles)

  explicit OpcUaTagModel(QObject* parent = nullptr);
  ~OpcUaTagModel() override;

  OpcUaTagModel(OpcUaTagModel&&)                 = delete;
  OpcUaTagModel(const OpcUaTagModel&)            = delete;
  OpcUaTagModel& operator=(OpcUaTagModel&&)      = delete;
  OpcUaTagModel& operator=(const OpcUaTagModel&) = delete;

  [[nodiscard]] QModelIndex index(int row,
                                  int column,
                                  const QModelIndex& parent = QModelIndex()) const override;
  [[nodiscard]] QModelIndex parent(const QModelIndex& child) const override;
  [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  [[nodiscard]] int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
  [[nodiscard]] bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
  [[nodiscard]] bool canFetchMore(const QModelIndex& parent) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
  void fetchMore(const QModelIndex& parent) override;
  bool setData(const QModelIndex& index, const QVariant& value, int role) override;

  [[nodiscard]] bool busy() const;
  [[nodiscard]] int selectedCount() const;
  [[nodiscard]] int selectedIndices() const;
  [[nodiscard]] bool overSoftLimit() const;
  [[nodiscard]] QList<OpcUaTag> selectedTags() const;
  [[nodiscard]] QJsonArray childrenJson(const QString& nodeId) const;
  [[nodiscard]] bool fetchNode(const QString& nodeId);
  [[nodiscard]] bool hasSeen(const QString& nodeId) const;

  [[nodiscard]] static OpcUaWire::Type wireTypeFromDataTypeId(const QString& dataTypeId) noexcept;
  [[nodiscard]] static OpcUaWire::Type wireTypeFromValue(const QVariant& value) noexcept;

public slots:
  void setSession(OpcUaSession* session);
  void clear();
  void preselect(const QList<OpcUaTag>& tags);
  void setChecked(const QModelIndex& index, const bool checked);
  void selectAllReadable();

private slots:
  void onAttributesRead(quint32 token,
                        const QList<OpcUaTypes::ReadRow>& rows,
                        OpcUaTypes::StatusCode status);
  void onBrowseReply(quint32 token,
                     const QString& nodeId,
                     const QList<OpcUaTypes::ReferenceRow>& children,
                     OpcUaTypes::StatusCode status);

private:
  /**
   * @brief Why a node is being browsed. One node can be browsed for all three reasons at once,
   *        so the session's reply token carries this back rather than the node id alone.
   */
  enum class Purpose : quint8 {
    Level,
    Probe,
    Units,
  };

  /**
   * @brief Read-reply routing tokens. A ticked variable's EngineeringUnits property is also an
   *        ordinary child row of that variable, so a reply keyed on node id alone cannot say
   *        whether it belongs to the level read or to the units lookup.
   */
  enum Token : quint32 {
    LevelRead = 1,
    UnitRead  = 2,
  };

  /**
   * @brief One address-space row; children are owned through the vector. Objects and Variables
   *        are both expandable until a browse proves a node has no children.
   */
  struct Node {
    Node* parent;
    int row;
    bool folder;
    bool expandable;
    bool fetched;
    bool fetching;
    bool probed;
    bool probing;
    bool attributesRead;
    bool checked;
    bool inherited;
    bool readable;
    bool euResolved;
    int arrayLen;
    double euMin;
    double euMax;
    OpcUaWire::Type type;
    QString nodeId;
    QString name;
    QString unit;
    QString description;
    QString path;
    std::vector<std::unique_ptr<Node>> children;

    Node()
      : parent(nullptr)
      , row(0)
      , folder(false)
      , expandable(false)
      , fetched(false)
      , fetching(false)
      , probed(false)
      , probing(false)
      , attributesRead(false)
      , checked(false)
      , inherited(false)
      , readable(false)
      , euResolved(false)
      , arrayLen(1)
      , euMin(0)
      , euMax(0)
      , type(OpcUaWire::Type::Invalid)
    {}
  };

  /**
   * @brief A browse still on the wire, and what its reply means.
   */
  struct PendingBrowse {
    Node* node;
    Purpose purpose;

    PendingBrowse() : node(nullptr), purpose(Purpose::Level) {}
  };

  /**
   * @brief A property whose Value is being read for a ticked variable: EngineeringUnits gives the
   *        unit string, EURange the display bounds.
   */
  struct UnitTarget {
    Node* node;
    bool range;

    UnitTarget() : node(nullptr), range(false) {}
  };

  [[nodiscard]] Node* nodeAt(const QModelIndex& index) const;
  [[nodiscard]] Node* findNode(Node* from, const QString& nodeId) const;
  [[nodiscard]] QModelIndex indexOf(Node* node) const;
  [[nodiscard]] bool selectable(const Node* node) const noexcept;
  [[nodiscard]] QString resolveNodeId(const OpcUaTypes::ReferenceRow& row) const;
  [[nodiscard]] std::unique_ptr<Node> makeChild(Node* parent,
                                                const OpcUaTypes::ReferenceRow& row) const;

  void browse(Node* node);
  void readLevel(Node* node);
  void expandPreselected(Node* node);
  void refreshAncestors(Node* node);
  [[nodiscard]] QString pathOf(const Node* node) const;
  [[nodiscard]] bool isPreselected(const QString& nodeId) const;
  void queueProbe(Node* node);
  void pumpProbeQueue();
  void queueUnits(Node* node);
  void pumpUnitQueue();
  void applyAttribute(Node* node,
                      OpcUaTypes::NodeAttribute attribute,
                      const QVariant& value,
                      OpcUaTypes::StatusCode status);
  void finishAttributes(Node* node);
  void onBrowseFinished(Node* node,
                        const QList<OpcUaTypes::ReferenceRow>& children,
                        OpcUaTypes::StatusCode status);
  void onProbeFinished(Node* node,
                       const QList<OpcUaTypes::ReferenceRow>& children,
                       OpcUaTypes::StatusCode status);
  void onUnitsBrowsed(Node* node, const QList<OpcUaTypes::ReferenceRow>& children);
  void applyUnitValue(const UnitTarget& target,
                      const QVariant& value,
                      OpcUaTypes::StatusCode status);
  [[nodiscard]] quint32 issueBrowse(Node* node, Purpose purpose);
  void applySelectAll(Node* node);
  void collectSelected(const Node* node, QList<OpcUaTag>& out) const;
  void countSelected(const Node* node, int& count, int& indices) const;
  void countSelectable(const Node* node, int& checked, int& selectable) const;
  void publishBusy();

  OpcUaSession* m_session;
  std::unique_ptr<Node> m_root;
  QList<OpcUaTag> m_preselected;
  QHash<QString, Node*> m_index;
  QHash<QString, Node*> m_pendingReads;
  QHash<quint32, PendingBrowse> m_browseTokens;
  QQueue<Node*> m_probeQueue;
  QQueue<Node*> m_unitQueue;
  QHash<QString, UnitTarget> m_unitTargets;
  quint32 m_nextToken;
  int m_unitsInFlight;
  int m_probesInFlight;
  int m_pendingBrowses;
  bool m_lastBusy;
};

}  // namespace Drivers
}  // namespace IO
