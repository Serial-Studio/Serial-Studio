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

#include "AuthenticateDialog.h"

#include "ui_AuthenticateDialog.h"

/**
 * @brief Constructs the authentication dialog.
 * @param parent Optional parent widget.
 */
AuthenticateDialog::AuthenticateDialog(QWidget* parent)
  : QDialog(parent), ui(new Ui::AuthenticateDialog)
{
  ui->setupUi(this);
}

/**
 * @brief Destroys the dialog and frees the UI.
 */
AuthenticateDialog::~AuthenticateDialog()
{
  delete ui;
}

/**
 * @brief Sets the username field to the given @a userName.
 */
void AuthenticateDialog::setUserName(const QString& userName)
{
  ui->userLE->setText(userName);
}

/**
 * @brief Sets the password field to the given @a password.
 */
void AuthenticateDialog::setPassword(const QString& password)
{
  ui->passwordLE->setText(password);
}

/**
 * @brief Returns the current username entered by the user.
 */
QString AuthenticateDialog::userName() const
{
  return ui->userLE->text();
}

/**
 * @brief Returns the current password entered by the user.
 */
QString AuthenticateDialog::password() const
{
  return ui->passwordLE->text();
}

#if QSU_INCLUDE_MOC
#  include "moc_AuthenticateDialog.cpp"
#endif
