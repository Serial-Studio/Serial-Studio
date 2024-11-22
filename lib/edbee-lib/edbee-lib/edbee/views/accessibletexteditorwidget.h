#pragma once

#include "edbee/exports.h"

#include <QAccessibleTextInterface>
#include <QAccessibleWidget>
#include <QAccessibleTextInterface>
#include <QAccessibleEditableTextInterface>


namespace edbee {

class TextBufferChange;
class TextDocument;
class TextEditorController;
class TextEditorWidget;
class TextRenderer;
class TextSelection;

class EDBEE_EXPORT AccessibleTextEditorWidget : public QAccessibleWidget, public QAccessibleTextInterface
{
public:
    explicit AccessibleTextEditorWidget(TextEditorWidget* widget);
    virtual ~AccessibleTextEditorWidget();


public:
    static QAccessibleInterface* factory(const QString& className, QObject * object);
    static QWidget* eventWidgetForTextEditor(TextEditorWidget* widget);

    static void notifyTextSelectionEvent(TextEditorWidget* widget, TextSelection* selection);
    static void notifyTextChangeEvent(TextEditorWidget* widget, TextBufferChange* change, QString oldText);


    void* interface_cast(QAccessible::InterfaceType t) override;
    QAccessible::State state() const override;

    // QAccessibleTextInterface interface
public:
    virtual void selection(int selectionIndex, int *startOffset, int *endOffset) const override;
    virtual int selectionCount() const override;
    virtual void addSelection(int startOffset, int endOffset) override;
    virtual void removeSelection(int selectionIndex) override;
    virtual void setSelection(int selectionIndex, int startOffset, int endOffset) override;
    virtual int cursorPosition() const override;
    virtual void setCursorPosition(int position) override;
    virtual QString text(QAccessible::Text t) const override;
    virtual QString text(int startOffset, int endOffset) const override;
    virtual int characterCount() const override;
    virtual QRect characterRect(int offset) const override;
    virtual int offsetAtPoint(const QPoint &point) const override;
    virtual void scrollToSubstring(int startIndex, int endIndex) override;
    virtual QString attributes(int offset, int *startOffset, int *endOffset) const override;

    virtual QString textAfterOffset(int offset, QAccessible::TextBoundaryType boundaryType, int *startOffset, int *endOffset) const override;
    virtual QString textAtOffset(int offset, QAccessible::TextBoundaryType boundaryType, int *startOffset, int *endOffset) const override;
    virtual QString textBeforeOffset(int offset, QAccessible::TextBoundaryType boundaryType, int *startOffset, int *endOffset) const override;

    virtual QAccessibleInterface* focusChild() const override;
    virtual QRect rect() const override;

protected:

    TextDocument* textDocument() const;
    TextSelection* textSelection() const;
    TextEditorController* controller() const;
    TextRenderer* renderer() const;
    TextEditorWidget* textWidget() const;

private:
    TextEditorWidget* textWidgetRef_;
};

} // Edbee
