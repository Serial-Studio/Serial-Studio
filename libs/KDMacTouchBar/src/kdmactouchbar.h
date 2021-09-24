/****************************************************************************
** Copyright (C) 2019-2021 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** All rights reserved.
**
** This file is part of the KD MacTouchBar library.
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** You may even contact us at info@kdab.com for different licensing options.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#ifndef KDMACTOUCHBAR_H
#define KDMACTOUCHBAR_H

#include "kdmactouchbar_global.h"

#include <QWidget>
#include <QAction>

QT_BEGIN_NAMESPACE

class QDialogButtonBox;
class QMessageBox;
class QTabBar;


class KDMACTOUCHBAR_EXPORT KDMacTouchBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QAction *principialAction READ principialAction WRITE setPrincipialAction)
    Q_PROPERTY(QAction *escapeAction READ escapeAction WRITE setEscapeAction)
    Q_PROPERTY(TouchButtonStyle touchButtonStyle READ touchButtonStyle WRITE setTouchButtonStyle)
public:
    explicit KDMacTouchBar(QWidget *parent = nullptr);
    explicit KDMacTouchBar(QMessageBox *messageBox);
    ~KDMacTouchBar();

    enum TouchButtonStyle
    {
        IconOnly,
        TextOnly,
        TextBesideIcon
    };

    static void setAutomaticallyCreateMessageBoxTouchBar(bool automatic);
    static bool isAutomacicallyCreatingMessageBoxTouchBar();

    QAction *addSeparator();
    QAction *addTabBar(QTabBar *tabBar);
    void removeTabBar(QTabBar *tabBar);

    QAction *addMessageBox(QMessageBox *messageBox);
    void removeMessageBox(QMessageBox *messageBox);

    QAction *addDialogButtonBox(QDialogButtonBox *buttonBox);
    void removeDialogButtonBox(QDialogButtonBox *buttonBox);

    void setPrincipialAction(QAction *action);
    QAction *principialAction() const;

    void setEscapeAction(QAction *action);
    QAction *escapeAction() const;

    void setTouchButtonStyle(TouchButtonStyle touchButtonStyle);
    TouchButtonStyle touchButtonStyle() const;

    void clear();

protected:
    bool event(QEvent *event);

private:
    class Private;
    Private *const d;
};

QT_END_NAMESPACE

#endif
