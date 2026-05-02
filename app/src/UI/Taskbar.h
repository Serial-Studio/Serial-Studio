/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QObject>
#include <QQuickItem>
#include <QStandardItemModel>
#include <QVariantList>

#include "UI/WidgetRegistry.h"
#include "UI/WindowManager.h"

namespace UI {
/**
 * @brief QStandardItemModel used to represent dashboard widgets in a hierarchical UI.
 */
class TaskbarModel : public QStandardItemModel {
  Q_OBJECT

public:
  enum Roles {
    WindowIdRole = Qt::UserRole + 1,
    WidgetTypeRole,
    WidgetNameRole,
    WidgetIconRole,
    GroupIdRole,
    GroupNameRole,
    IsGroupRole,
    WindowStateRole,
    OverviewRole,
  };

  enum WindowState {
    WindowNormal    = 0,
    WindowMinimized = 1,
    WindowClosed    = 2
  };
  Q_ENUM(WindowState)

  explicit TaskbarModel(QObject* parent = nullptr);
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
};

/**
 * @brief Controller that manages dashboard window state and taskbar UI models.
 */
class Taskbar : public QQuickItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(TaskbarModel* fullModel
             READ fullModel
             NOTIFY fullModelChanged)
  Q_PROPERTY(TaskbarModel* taskbarButtons
             READ taskbarButtons
             NOTIFY taskbarButtonsChanged)
  Q_PROPERTY(int activeGroupId
             READ activeGroupId
             WRITE setActiveGroupId
             NOTIFY activeGroupIdChanged)
  Q_PROPERTY(int activeGroupIndex
             READ activeGroupIndex
             WRITE setActiveGroupIndex
             NOTIFY activeGroupIdChanged)
  Q_PROPERTY(QQuickItem* activeWindow
             READ activeWindow
             WRITE setActiveWindow
             NOTIFY activeWindowChanged)
  Q_PROPERTY(QVariantList groupModel
             READ groupModel
             NOTIFY fullModelChanged)
  Q_PROPERTY(QVariantList workspaceModel
             READ workspaceModel
             NOTIFY workspaceModelChanged)
  Q_PROPERTY(QString searchFilter
             READ searchFilter
             WRITE setSearchFilter
             NOTIFY searchFilterChanged)
  Q_PROPERTY(QVariantList searchResults
             READ searchResults
             NOTIFY searchResultsChanged)
  Q_PROPERTY(QVariantList allWidgets
             READ allWidgets
             NOTIFY fullModelChanged)
  Q_PROPERTY(UI::WindowManager* windowManager
             READ windowManager
             WRITE setWindowManager
             NOTIFY windowManagerChanged)
  Q_PROPERTY(bool hasMaximizedWindow
             READ hasMaximizedWindow
             NOTIFY statesChanged)
  // clang-format on

signals:
  void statesChanged();
  void fullModelChanged();
  void searchDismissed();
  void highlightWidget(int windowId);
  void searchFilterChanged();
  void searchResultsChanged();
  void activeWindowChanged();
  void windowStatesChanged();
  void activeGroupIdChanged();
  void windowManagerChanged();
  void workspaceModelChanged();
  void taskbarButtonsChanged();
  void registeredWindowsChanged();

public:
  Taskbar(QQuickItem* parent = nullptr);
  ~Taskbar();

  [[nodiscard]] int activeGroupId() const;
  [[nodiscard]] int activeGroupIndex() const;
  [[nodiscard]] QString searchFilter() const;
  [[nodiscard]] QVariantList groupModel() const;
  [[nodiscard]] QVariantList allWidgets() const;
  [[nodiscard]] QVariantList searchResults() const;
  [[nodiscard]] QVariantList workspaceModel() const;
  [[nodiscard]] QQuickItem* activeWindow() const;

  [[nodiscard]] TaskbarModel* fullModel() const;
  [[nodiscard]] TaskbarModel* taskbarButtons() const;
  [[nodiscard]] WindowManager* windowManager() const;

  [[nodiscard]] bool hasMaximizedWindow() const;

  Q_INVOKABLE [[nodiscard]] QQuickItem* windowData(const int id) const;
  Q_INVOKABLE [[nodiscard]] QVariantList workspaceWidgetIds(int workspaceId) const;
  Q_INVOKABLE [[nodiscard]] TaskbarModel::WindowState windowState(QQuickItem* window) const;
  Q_INVOKABLE [[nodiscard]] QQuickItem* nextActiveWindow(int delta) const;

public slots:
  void saveLayout();
  void dismissSearch();
  void setSearchFilter(const QString& filter);
  void setActiveGroupId(int groupId);
  void setActiveGroupIndex(int index);
  void showWindow(QQuickItem* window);
  void closeWindow(QQuickItem* window);
  void minimizeWindow(QQuickItem* window);
  void setActiveWindow(QQuickItem* window);
  void unregisterWindow(QQuickItem* window);
  void setWindowManager(UI::WindowManager* manager);
  void registerWindow(const int id, QQuickItem* window);
  void setWindowState(const int id, const UI::TaskbarModel::WindowState state);
  void navigateToWidget(int windowId, int groupId);
  void createWorkspace(const QString& name);
  void deleteWorkspace(int workspaceId);
  void renameWorkspace(int workspaceId, const QString& name);
  void addWidgetToActiveWorkspace(int windowId);
  void removeWidgetFromActiveWorkspace(int windowId);
  void setWorkspaceWidgets(int workspaceId, const QVariantList& windowIds);

private slots:
  void onTerminalToggled();
  void onNotificationLogToggled();
  void onRegistryCleared();
  void onBatchUpdateCompleted();
  void onWidgetCreated(UI::WidgetID id, const UI::WidgetInfo& info);
  void onWidgetDestroyed(UI::WidgetID id);

private:
  void rebuildModel();
  void connectToRegistry();
  void mapWidgetToWindow(UI::WidgetID wid, int windowId);
  void cloneSpecialOverviewRow(int widgetType);
  void populateTaskbarFromWorkspace(int groupId);
  void populateTaskbarFromGroup(int groupId);
  void removeWorkspaceTaskbarRow(int windowId);
  void removeOverviewByType(int widgetType);
  void selectGroupAfterRebuild();
  void appendGroupChildItem(QStandardItem* groupItem,
                            int groupId,
                            const QString& groupName,
                            int windowId,
                            SerialStudio::DashboardWidget widgetType,
                            int relativeIndex);
  void attachGroupItemToFullModel(QStandardItem* groupItem, int groupId, bool alreadyRegistered);
  void collectGroupWidgetIds(int groupId,
                             QList<int>& windowIds,
                             QList<int>& relativeIds,
                             QList<SerialStudio::DashboardWidget>& widgetTypes) const;
  void mapMainGroupWidgetId(SerialStudio::DashboardWidget groupType, int groupId, int mainWindowId);
  void buildOverviewGroupItem(QStandardItem* groupItem,
                              int groupId,
                              const QString& groupName,
                              SerialStudio::DashboardWidget groupType,
                              int mainWindowId,
                              bool alreadyRegistered);
  [[nodiscard]] QStandardItem* findItemByWindowId(int windowId,
                                                  QStandardItem* parentItem = nullptr,
                                                  int depth                 = 0) const;
  [[nodiscard]] QStandardItem* findItemByWidgetId(UI::WidgetID widgetId,
                                                  QStandardItem* parentItem = nullptr) const;
  [[nodiscard]] QStandardItem* findGroupItemByGroupId(int groupId) const;
  [[nodiscard]] QStandardItem* createItemFromWidgetInfo(const UI::WidgetInfo& info);
  [[nodiscard]] int findWindowIdByGroupAndIndex(int widgetType, int relativeIndex) const;
  [[nodiscard]] int relativeIndexForWindow(int windowId) const;

  int m_activeGroupId;
  bool m_rebuildInProgress;
  bool m_batchUpdateInProgress;
  bool m_restoringLayout;
  QString m_searchFilter;

  QQuickItem* m_activeWindow;
  UI::WindowManager* m_windowManager;
  QMap<QQuickItem*, int> m_windowIDs;
  QMap<QQuickItem*, QMetaObject::Connection> m_windowConnections;

  QMap<UI::WidgetID, int> m_widgetIdToWindowId;
  QMap<int, UI::WidgetID> m_windowIdToWidgetId;

  TaskbarModel* m_fullModel;
  TaskbarModel* m_taskbarButtons;
};

}  // namespace UI
