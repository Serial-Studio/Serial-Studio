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

#pragma once

#include <QPlainTextEdit>
#include <UI/DeclarativeWidget.h>

namespace Widgets
{
class FormattedText
{
public:
    FormattedText() = default;
    FormattedText(const FormattedText &other) = default;
    FormattedText(const QString &txt, const QTextCharFormat &fmt = QTextCharFormat())
        : text(txt)
        , format(fmt)
    {
    }

    QString text;
    QTextCharFormat format;
};

class AnsiEscapeCodeHandler
{
public:
    QVector<FormattedText> parseText(const FormattedText &input);
    void setTextEdit(QPlainTextEdit *widget);
    void endFormatScope();

private:
    void setFormatScope(const QTextCharFormat &charFormat);

    QString m_pendingText;
    QPlainTextEdit *textEdit;
    QString m_alternateTerminator;
    QTextCharFormat m_previousFormat;
    bool m_previousFormatClosed = true;
    bool m_waitingForTerminator = false;
};

class Terminal : public UI::DeclarativeWidget
{
    // clang-format off
    Q_OBJECT
    Q_PROPERTY(QFont font
               READ font
               WRITE setFont
               NOTIFY fontChanged)
    Q_PROPERTY(QString text
               READ text
               WRITE setText
               NOTIFY textChanged)
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
               NOTIFY colorPaletteChanged)
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
    Q_PROPERTY(int scrollbarWidth
               READ scrollbarWidth
               WRITE setScrollbarWidth
               NOTIFY scrollbarWidthChanged)
    Q_PROPERTY(bool vt100emulation
               READ vt100emulation
               WRITE setVt100Emulation
               NOTIFY vt100EmulationChanged)
    // clang-format on

Q_SIGNALS:
    void textChanged();
    void fontChanged();
    void readOnlyChanged();
    void autoscrollChanged();
    void colorPaletteChanged();
    void wordWrapModeChanged();
    void copyAvailableChanged();
    void widgetEnabledChanged();
    void scrollbarWidthChanged();
    void centerOnScrollChanged();
    void vt100EmulationChanged();
    void placeholderTextChanged();
    void undoRedoEnabledChanged();
    void maximumBlockCountChanged();

public:
    Terminal(QQuickItem *parent = 0);

    QFont font() const;
    QString text() const;

    bool empty() const;
    bool readOnly() const;
    bool autoscroll() const;
    QPalette palette() const;
    int wordWrapMode() const;
    int scrollbarWidth() const;
    bool copyAvailable() const;
    bool widgetEnabled() const;
    bool centerOnScroll() const;
    bool vt100emulation() const;
    bool undoRedoEnabled() const;
    int maximumBlockCount() const;
    QString placeholderText() const;
    QTextDocument *document() const;

public Q_SLOTS:
    void copy();
    void clear();
    void selectAll();
    void clearSelection();
    void setReadOnly(const bool ro);
    void setFont(const QFont &font);
    void append(const QString &text);
    void setText(const QString &text);
    void insertText(const QString &text);
    void setWordWrapMode(const int mode);
    void setAutoscroll(const bool enabled);
    void setScrollbarWidth(const int width);
    void setPalette(const QPalette &palette);
    void setWidgetEnabled(const bool enabled);
    void setCenterOnScroll(const bool enabled);
    void setVt100Emulation(const bool enabled);
    void setUndoRedoEnabled(const bool enabled);
    void setPlaceholderText(const QString &text);
    void scrollToBottom(const bool repaint = false);
    void setMaximumBlockCount(const int maxBlockCount);

private Q_SLOTS:
    void repaint();
    void updateScrollbarVisibility();
    void setCopyAvailable(const bool yes);
    void addText(const QString &text, const bool enableVt100);

private:
    QString vt100Processing(const QString &data);
    void requestRepaint(const bool textChanged = false);

private:
    bool m_repaint;
    bool m_autoscroll;
    bool m_textChanged;
    bool m_emulateVt100;
    bool m_copyAvailable;

    QPlainTextEdit m_textEdit;
    AnsiEscapeCodeHandler m_escapeCodeHandler;
};
}
