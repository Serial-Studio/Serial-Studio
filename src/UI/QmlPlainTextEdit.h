/*
 * Copyright (c) 2020-2021 Alex Spataru <https://github.com/alex-spataru>
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

#ifndef UI_QML_PLAINTEXTEDIT_H
#define UI_QML_PLAINTEXTEDIT_H

#include <QPainter>
#include <QPlainTextEdit>
#include <QQuickPaintedItem>

namespace UI
{
class QmlPlainTextEdit : public QQuickPaintedItem
{
    // clang-format off
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QFont font
               READ font
               WRITE setFont
               NOTIFY fontChanged)
    Q_PROPERTY(QString text
               READ text
               WRITE setText
               NOTIFY textChanged)
    Q_PROPERTY(QColor color
               READ color
               WRITE setColor
               NOTIFY colorChanged)
    Q_PROPERTY(bool autoscroll
               READ autoscroll
               WRITE setAutoscroll
               NOTIFY autoscrollChanged)
    Q_PROPERTY(QString placeholderText
               READ placeholderText
               WRITE setPlaceholderText
               NOTIFY placeholderTextChanged)
    Q_PROPERTY(bool undoRedoEnabled
               READ undoRedoEnabled
               WRITE setUndoRedoEnabled
               NOTIFY undoRedoEnabledChanged)
    Q_PROPERTY(int wordWrapMode
               READ wordWrapMode
               WRITE setWordWrapMode
               NOTIFY wordWrapModeChanged)
    Q_PROPERTY(bool readOnly
               READ readOnly
               WRITE setReadOnly
               NOTIFY readOnlyChanged)
    Q_PROPERTY(bool centerOnScroll
               READ centerOnScroll
               WRITE setCenterOnScroll
               NOTIFY centerOnScrollChanged)
    Q_PROPERTY(bool widgetEnabled
               READ widgetEnabled
               WRITE setWidgetEnabled
               NOTIFY widgetEnabledChanged)
    Q_PROPERTY(QPalette palette
               READ palette
               WRITE setPalette
               NOTIFY paletteChanged)
    Q_PROPERTY(int maximumBlockCount
               READ maximumBlockCount
               WRITE setMaximumBlockCount
               NOTIFY maximumBlockCountChanged)
    Q_PROPERTY(bool copyAvailable
               READ copyAvailable
               NOTIFY copyAvailableChanged)
    Q_PROPERTY(bool empty
               READ empty
               NOTIFY textChanged)
    // clang-format on

signals:
    void textChanged();
    void fontChanged();
    void colorChanged();
    void paletteChanged();
    void readOnlyChanged();
    void autoscrollChanged();
    void wordWrapModeChanged();
    void copyAvailableChanged();
    void widgetEnabledChanged();
    void centerOnScrollChanged();
    void placeholderTextChanged();
    void undoRedoEnabledChanged();
    void maximumBlockCountChanged();

public:
    QmlPlainTextEdit(QQuickItem *parent = 0);
    ~QmlPlainTextEdit();

    virtual bool event(QEvent *event) override;
    virtual void paint(QPainter *painter) override;
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

    QFont font() const;
    QColor color() const;
    QString text() const;

    bool empty() const;
    bool readOnly() const;
    bool autoscroll() const;
    QPalette palette() const;
    int wordWrapMode() const;
    bool copyAvailable() const;
    bool widgetEnabled() const;
    bool centerOnScroll() const;
    bool undoRedoEnabled() const;
    int maximumBlockCount() const;
    QString placeholderText() const;

public slots:
    void copy();
    void clear();
    void selectAll();
    void clearSelection();
    void setReadOnly(const bool ro);
    void setFont(const QFont &font);
    void append(const QString &text);
    void setText(const QString &text);
    void setColor(const QColor &color);
    void insertText(const QString &text);
    void setWordWrapMode(const int mode);
    void setAutoscroll(const bool enabled);
    void setPalette(const QPalette &palette);
    void setWidgetEnabled(const bool enabled);
    void setCenterOnScroll(const bool enabled);
    void setUndoRedoEnabled(const bool enabled);
    void setPlaceholderText(const QString &text);
    void scrollToBottom(const bool repaint = true);
    void setMaximumBlockCount(const int maxBlockCount);

private slots:
    void updateWidgetSize();
    void setCopyAvailable(const bool yes);

protected:
    void processMouseEvents(QMouseEvent *event);
    void processWheelEvents(QWheelEvent *event);

private:
    QColor m_color;
    bool m_autoscroll;
    bool m_copyAvailable;
    QPlainTextEdit *m_textEdit;
};
}

#endif
