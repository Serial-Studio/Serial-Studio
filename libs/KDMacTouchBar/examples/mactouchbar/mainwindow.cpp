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

#include "mainwindow.h"

#include <QtGui/QCloseEvent>

#include <QtWidgets/QAction>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QLabel>

#include "kdmactouchbar.h"

MainWindow::MainWindow()
{
    setWindowTitle("KDMacTouchBar Example");
    resize(400, 200);

    // attach a touchbar to this window
    KDMacTouchBar *touchBar = new KDMacTouchBar(this);

    QIcon qtIcon(QStringLiteral(":qtlogo.png"));

    // add item
    QAction *action = new QAction(qtIcon, tr("Qt with touchbar"));
    touchBar->addAction(action);
    connect(action, &QAction::triggered, this, &MainWindow::activated);

    // separator
    touchBar->addSeparator();
    touchBar->addSeparator()->setText(tr("More items:"));

    // and more items
    QAction *action1 = new QAction(tr("Item 1"));
    touchBar->addAction(action1);
    connect(action1, &QAction::triggered, this, &MainWindow::activated);

    QAction *action2 = new QAction(tr("Item 2"));
    touchBar->addAction(action2);
    connect(action2, &QAction::triggered, this, &MainWindow::activated);

    // make special escape button
    QAction *quit = new QAction(tr("Quit"));
    touchBar->setEscapeAction(quit);
    connect(quit, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::activated()
{
    QMessageBox::information(this, tr("Activated"), tr("You activated the touchbar action!"));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->setAccepted(QMessageBox::question(this, tr("Quit"), tr("Do yo really want to quit?")) == QMessageBox::Yes);
}
