/*
 * Copyright (c) 2014-2025 Alex Spataru <https://github.com/alex-spataru>
 *
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file for more details.
 */

#ifndef _WINDOW_H
#define _WINDOW_H

#include <QApplication>
#include <QMainWindow>

namespace Ui {
class Window;
}

class QSimpleUpdater;

/**
 * @brief Tutorial window demonstrating QSimpleUpdater usage.
 */
class Window : public QMainWindow {
  Q_OBJECT

public:
  explicit Window(QWidget* parent = nullptr);
  ~Window();

public slots:
  void resetFields();
  void checkForUpdates();
  void updateChangelog(const QString& url);
  void displayAppcast(const QString& url, const QByteArray& reply);

private:
  Ui::Window* m_ui;
  QSimpleUpdater* m_updater;
};

#endif
