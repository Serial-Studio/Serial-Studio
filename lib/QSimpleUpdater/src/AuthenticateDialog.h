/*
 * Copyright (c) 2014-2025 Alex Spataru <https://github.com/alex-spataru>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef AUTHENTICATEDIALOG_H
#define AUTHENTICATEDIALOG_H

#include <QDialog>

namespace Ui {
class AuthenticateDialog;
}

/**
 * @brief A simple dialog for HTTP basic authentication credentials.
 *
 * Provides username and password input fields used when the remote server
 * requires authentication to download the update file.
 */
class AuthenticateDialog : public QDialog {
  Q_OBJECT

public:
  explicit AuthenticateDialog(QWidget* parent = nullptr);
  ~AuthenticateDialog();

  void setUserName(const QString& userName);
  void setPassword(const QString& password);
  QString userName() const;
  QString password() const;

private:
  Ui::AuthenticateDialog* ui;
};

#endif  // AUTHENTICATEDIALOG_H
