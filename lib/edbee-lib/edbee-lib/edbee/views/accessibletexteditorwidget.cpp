#include "accessibletexteditorwidget.h"

#include <QWidget>
#include <QAccessibleInterface>

#include "edbee/models/changes/textchange.h"
#include "edbee/models/textdocument.h"
#include "edbee/models/textrange.h"
#include "edbee/texteditorwidget.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/views/textselection.h"
#include "edbee/views/textrenderer.h"
#include "edbee/views/components/texteditorcomponent.h"

#include "edbee/debug.h"

/// The direct usage of the TextEditorWidget doesn't work.
/// This works for Windows Reader and Mac OS X Speaker, but doesn't seem to work for NVDA
/// The TexteditorComponent is the component that has the true focus
#define VIA_EDITOR_COMPONENT

/// QT Acessibility has an issue with reporting blank lines between elements lines.
/// defining 'WINDOWS_EMPTY_LINE_READING_ERROR_FIX' adds a \r before a newline.
/// Which is a workaround for this issue.
/// (It does some offset-length and string magic when this is enabled)
#if defined(Q_OS_WIN32)
#define WINDOWS_END_LINE_READ_ERROR_FIX
#define WINDOWS_LAST_LINE_ERROR_FIX
#define WINDOWS_EMPTY_LINE_FIX "\r\n"
#endif

namespace edbee {

/// Workaround for the strang line-reading on windows
/// It virtually replaces every <newline> with <space><newline>
static int D2V(TextEditorWidget* widget, int offset)
{
#ifdef WINDOWS_END_LINE_READ_ERROR_FIX
    int line = widget->textDocument()->lineFromOffset(offset);
    return offset + line;
#else
    Q_UNUSED(widget)
    return offset;
#endif
}

/// Converts the virtual-offset to the document-offset
static int V2D(TextEditorWidget* widget, int vOffset)
{
#ifdef WINDOWS_END_LINE_READ_ERROR_FIX
    TextDocument* doc = widget->textDocument();
    int displacement = 0;
    for( int i=0, cnt = doc->lineCount(); i < cnt; ++i ) {
        int lineVOffset = doc->offsetFromLine(i) + i;

        if( vOffset < lineVOffset) {
            return vOffset - displacement; // remove the newline count
        }
        displacement = i;
    }
    return vOffset - displacement;
#else
    Q_UNUSED(widget)
    return vOffset;
#endif
}

/// returns the virtual length of the textdocument
static int VLEN(TextEditorWidget* widget)
{
#if ! defined(WINDOWS_END_LINE_READ_ERROR_FIX)
    return widget->textDocument()->length();
#elif defined(WINDOWS_LAST_LINE_ERROR_FIX)
    return widget->textDocument()->length() + widget->textDocument()->lineCount();
#else
    return widget->textDocument()->length() + widget->textDocument()->lineCount() - 1;
#endif
}

/// Converts the given text to a virtual text
/// It prepends every newline with a space
static const QString VTEXT(QString str)
{
#ifdef WINDOWS_END_LINE_READ_ERROR_FIX
    QString result = str.replace(QString("\n"), QString(WINDOWS_EMPTY_LINE_FIX));
    return result;
#else
    return str;
#endif
}

/// Converts the given text to a virtual text
/// It prepends every newline with a space
static const QString VTEXT(TextDocument* doc)
{
#if ! defined(WINDOWS_END_LINE_READ_ERROR_FIX)
    return doc->text();
#elif defined(WINDOWS_LAST_LINE_ERROR_FIX)
    QString result = VTEXT(doc->text());
    return result + WINDOWS_EMPTY_LINE_FIX;
#else
    return VTEXT(doc->text());
#endif
}


/// Return a part of text, translating the virtual characters
/// It prepends every newline with a space
/// It assumes an extra space + newline is placed at the end of the document
static const QString VTEXT_PART(TextDocument* doc, int offset, int length)
{
#if ! defined(WINDOWS_END_LINE_READ_ERROR_FIX)
    if(length < 0) {
        return QString();
    }

    return doc->textPart(offset, qMin(length, doc->length() - offset));
#else
    int docLength = doc->length();
#if defined(WINDOWS_LAST_LINE_ERROR_FIX)
        int endOffset = qMin(offset + length, docLength + 2);
#endif // WINDOWS_LAST_LINE_ERROR_FIX - First use
    //qDebug() << "VTEXT_PART: " << offset << ", " << length << " :: docLength: " << docLength << ", endOffset: " << endOffset;

    QString txt;
    if( offset < docLength ) {
        int len = offset + length > docLength ? docLength - offset : length;
        //qDebug() << " - A  offset:" << offset <<", len: " << len;
        txt = VTEXT(doc->textPart(offset, len));
    }

#if defined(WINDOWS_LAST_LINE_ERROR_FIX)
    //qDebug() << " - B  endOffset: " << endOffset << " >= " << docLength;
    if( endOffset >= docLength ) {
        txt.append(WINDOWS_EMPTY_LINE_FIX);
    }
#endif // WINDOWS_LAST_LINE_ERROR_FIX - Second use
    //qDebug() << " => " << txt;
    return txt;
#endif // WINDOWS_END_LINE_READ_ERROR_FIX
}


AccessibleTextEditorWidget::AccessibleTextEditorWidget(TextEditorWidget* widget)
#ifdef VIA_EDITOR_COMPONENT
    : QAccessibleWidget(widget->textEditorComponent(), QAccessible::EditableText)
#else
    : QAccessibleWidget(widget, QAccessible::EditableText)
#endif
    , textWidgetRef_(widget)
{
}

AccessibleTextEditorWidget::~AccessibleTextEditorWidget()
{
}

/// Construct the AccessibleTextEditor interface for the given widget
QAccessibleInterface *AccessibleTextEditorWidget::factory(const QString &className, QObject *object)
{
    // edbee::TextMarginComponent, edbee::TextEditorScrollArea, edbee::TextEditorComponent
#ifdef VIA_EDITOR_COMPONENT
    if (className == QLatin1String("edbee::TextEditorComponent") && object && object->isWidgetType()) {
        return new AccessibleTextEditorWidget(static_cast<edbee::TextEditorComponent *>(object)->controller()->widget());
    }
#else
    if (className == QLatin1String("edbee::TextEditorWidget") && object && object->isWidgetType()) {
        return new AccessibleTextEditorWidget(static_cast<edbee::TextEditorWidget *>(object));
    }

#endif

    return nullptr;
}

/// Returns the widget that should be used for accessibility events
QWidget* AccessibleTextEditorWidget::eventWidgetForTextEditor(TextEditorWidget* widget)
{
#ifdef VIA_EDITOR_COMPONENT
    return widget->textEditorComponent();
#else
    return widget;
#endif
}

/// Notifies a text-selection change event
void AccessibleTextEditorWidget::notifyTextSelectionEvent(TextEditorWidget *widget, TextSelection *selection)
{
    QWidget* eventWidget = eventWidgetForTextEditor(widget);
    for(int i=0, cnt = selection->rangeCount(); i < cnt; ++i) {
        TextRange range = selection->range(i);


        QAccessibleTextSelectionEvent ev(eventWidget, D2V(widget, range.min()), D2V(widget, range.max()));
        ev.setCursorPosition(D2V(widget, range.caret()));

//        qDebug() << " !!updateAccessibility: QAccessibleTextSelectionEvent: " << range.min()<< ", " << range.max() << ", " << range.caret();

        QAccessible::updateAccessibility(&ev);

        // also send a caret event for range 0 without a selection
        if( i ==0 && !range.hasSelection()) {
            const QAccessibleTextInterface* ti = QAccessible::queryAccessibleInterface(eventWidget)->textInterface();
            QAccessibleTextCursorEvent event(eventWidget, ti->cursorPosition());
            QAccessible::updateAccessibility(&event);
        }

    }
}

/// Notifies a text change event happens
void AccessibleTextEditorWidget::notifyTextChangeEvent(TextEditorWidget *widget, TextBufferChange *change, QString oldDocText)
{
    QWidget* eventWidget = eventWidgetForTextEditor(widget);

    QString oldText = VTEXT(oldDocText);
    QString newText = VTEXT(QString(change->newText()));

    QAccessibleTextUpdateEvent ev(eventWidget, D2V(widget, change->offset()), oldText, newText);
    // TODO: When a caret is included, (Inherited change, use this caret position)

//    qDebug() << "-- change: length: " << change->length()
//             << ", newTextLength: " << change->newTextLength()
//             << ", offset :" << change->offset()
//             << ", newText: " << QString(change->newText())
//             << ", CONTENT: " << widget->textDocument()->text();
//    qDebug() << "!! updateAccessibility: QAccessibleTextUpdateEvent: " << change->offset()<< ", oldText: " << oldText << ", newText: " << newText;

    QAccessible::updateAccessibility(&ev);


}

void *AccessibleTextEditorWidget::interface_cast(QAccessible::InterfaceType t)
{
    if (t == QAccessible::TextInterface) {
        return static_cast<QAccessibleTextInterface*>(this);
    }
    return QAccessibleWidget::interface_cast(t);
}


QAccessible::State AccessibleTextEditorWidget::state() const
{
    QAccessible::State s = QAccessibleWidget::state();
    s.selectableText = true;
    s.multiLine = true;
    s.focusable = true;
    s.readOnly = false;
    s.editable = true;

    return s;
}



/// Returns a selection. The size of the selection is returned in startOffset and endOffset.
/// If there is no selection both startOffset and endOffset are nullptr.
///
/// The accessibility APIs support multiple selections. For most widgets though, only one selection
/// is supported with selectionIndex equal to 0.
void AccessibleTextEditorWidget::selection(int selectionIndex, int *startOffset, int *endOffset) const
{
    if(selectionIndex >= textSelection()->rangeCount()) {
        *startOffset = 0;
        *endOffset = 0;
    }

    TextRange& range = textSelection()->range(selectionIndex);
    *startOffset = D2V(textWidget(), range.min());
    *endOffset = D2V(textWidget(), range.max());
    //qDebug() << " selection >> " << selectionIndex << ", " << range.min() << " =>" << *startOffset << ", " << range.max() << " => " << *endOffset;
}

/// Returns the number of selections in this text.
int AccessibleTextEditorWidget::selectionCount() const
{
    if( !textSelection()->hasSelection()) return 0; // no selection (only caret)
    return textSelection()->rangeCount();
}

/// Select the text from startOffset to endOffset. The startOffset is the first character that will be selected.
/// The endOffset is the first character that will not be selected.
///
/// When the object supports multiple selections (e.g. in a word processor), this adds a new selection,
/// otherwise it replaces the previous selection.
///
/// The selection will be endOffset - startOffset characters long.
void AccessibleTextEditorWidget::addSelection(int startOffset, int endOffset)
{
    TextSelection selection = *textSelection();
    selection.addRange(V2D(textWidget(), startOffset), V2D(textWidget(), endOffset));
    controller()->changeAndGiveTextSelection(&selection);
}

/// Clears the selection with index selectionIndex.
void AccessibleTextEditorWidget::removeSelection(int selectionIndex)
{
    TextSelection selection = *textSelection();
    selection.removeRange(selectionIndex);
    controller()->changeAndGiveTextSelection(&selection);
}

/// Set the selection selectionIndex to the range from startOffset to endOffset.
void AccessibleTextEditorWidget::setSelection(int selectionIndex, int startOffset, int endOffset)
{
    TextSelection selection = *textSelection();
    selection.setRange(V2D(textWidget(), startOffset), V2D(textWidget(), endOffset), selectionIndex);
    controller()->changeAndGiveTextSelection(&selection);
}

/// Returns the current cursor position.
int AccessibleTextEditorWidget::cursorPosition() const
{
    int caret = D2V(textWidget(), textSelection()->range(0).caret());
    return caret;
}

/// Move the cursor position
void AccessibleTextEditorWidget::setCursorPosition(int position)
{
    //qDebug() << "AccessibleTextEditorWidget::setCursorPosition: " << position << " => <" << V2D(textWidget(), position);
    controller()->moveCaretToOffset(V2D(textWidget(), position), false);
}

QString AccessibleTextEditorWidget::text(QAccessible::Text t) const
{
    if (t != QAccessible::Value) {
        return QAccessibleWidget::text(t);
    }
    return VTEXT(textWidget()->textDocument());
}

/// Returns the text from startOffset to endOffset. The startOffset is the first character that will be returned.
/// The endOffset is the first character that will not be returned.
QString AccessibleTextEditorWidget::text(int vStartOffset, int vEndOffset) const
{
    int startOffset = V2D(textWidget(), vStartOffset);
    int endOffset = V2D(textWidget(), vEndOffset);

    //qDebug() << "text: " << VTEXT_PART(textWidget()->textDocument(), startOffset, endOffset - startOffset);
    //qDebug() << "      - vStartOffset " << vStartOffset << ", vEndOffset " << vEndOffset ;
    //qDebug() << "      - startOffset " << startOffset << ", endOffset: " << endOffset << ", len: " << endOffset-startOffset << " (docLength: " << textWidget()->textDocument()->length() <<")";
    return VTEXT_PART(textWidget()->textDocument(), startOffset, endOffset - startOffset);
}

/// Returns the length of the text (total size including spaces).
int AccessibleTextEditorWidget::characterCount() const
{
    //qDebug() << " characterCount >> " << VLEN(textWidget());
    return VLEN(textWidget());
}


/// Returns the position and size of the character at position offset in screen coordinates.
QRect AccessibleTextEditorWidget::characterRect(int vOffset) const
{
    TextEditorComponent* comp = textWidget()->textEditorComponent();
    int offset = V2D(textWidget(), vOffset);

    // workaround for newline char rect (is at the wrong location)
    // Very dirty workaround, it's dependent on the selection
#ifdef WINDOWS_END_LINE_READ_ERROR_FIX
        if(offset > 0 && offset <= textDocument()->length()){
            TextRange& range = textSelection()->range(0);
            if( range.hasSelection()) {
                if( offset == range.max() || textDocument()->charAt(offset) == '\n') {
                    offset -= 1;
                }
            }
        }

#endif

    int xPos = this->renderer()->xPosForOffset(offset);
    int yPos = this->renderer()->yPosForOffset(offset);
    QPoint point(xPos, yPos);
    QPoint pointScreen = comp->mapToGlobal(point);

    //qDebug() << " characterRect >>" << vOffset << " => " << offset << ": " << pointScreen;
    return QRect(pointScreen.x(), pointScreen.y(), renderer()->emWidth(), renderer()->lineHeight());
}


/// Returns the offset of the character at the point in screen coordinates.
int AccessibleTextEditorWidget::offsetAtPoint(const QPoint &point) const
{
    int line = renderer()->rawLineIndexForYpos(point.y());
    int col = renderer()->columnIndexForXpos(line, point.x());
    // qDebug() << " offsetAtPoint >>" << point << ": " << line << ", " << col;

   return D2V(textWidget(), textDocument()->offsetFromLineAndColumn(line, col));
}

/// Ensures that the text between startIndex and endIndex is visible.
void AccessibleTextEditorWidget::scrollToSubstring(int startIndex, int endIndex)
{
    Q_UNUSED(endIndex)
    // qDebug() << " scrollToSubstring >>" << startIndex << ", " << endIndex;
    controller()->scrollOffsetVisible(V2D(textWidget(), startIndex));
}


/// Returns the text attributes at the position offset.
/// In addition the range of the attributes is returned in startOffset and endOffset.
QString AccessibleTextEditorWidget::attributes(int offset, int *startOffset, int *endOffset) const
{
    Q_UNUSED(offset)
    Q_UNUSED(startOffset)
    Q_UNUSED(endOffset)
    // qDebug() << " attributes >>" << offset << ", " << *startOffset << ", " <<  *endOffset;
    return QString();
}

/// Returns the text item of type boundaryType that is right after offset offset and sets startOffset
/// and endOffset values to the start and end positions of that item; returns an empty string if there
/// is no such an item. Sets startOffset and endOffset values to -1 on error.
///
/// This default implementation is provided for small text edits. A word processor or text editor should
/// provide their own efficient implementations. This function makes no distinction between paragraphs and lines.
///
/// Note: this function can not take the cursor position into account. By convention an offset of -2 means
/// that this function should use the cursor position as offset. Thus an offset of -2 must be converted
/// to the cursor position before calling this function.
/// An offset of -1 is used for the text length and custom implementations of this function have to return the
/// result as if the length was passed in as offset.
QString AccessibleTextEditorWidget::textAfterOffset(int vOffset, QAccessible::TextBoundaryType boundaryType, int *startOffset, int *endOffset) const
{
    if(boundaryType == QAccessible::LineBoundary && vOffset >= 0) {
        int offset = V2D(textWidget(), vOffset);

        int line = textDocument()->lineFromOffset(offset);
        // qDebug() << " => " << textDocument()->line(line);
        int start = textDocument()->offsetFromLine(line);
        int end = textDocument()->offsetFromLine(line+1);

        QString str = VTEXT_PART(textDocument(), start, end - start);
        *startOffset = D2V(textWidget(), start);
        *endOffset = *startOffset + str.length(); //D2V(textWidget(), end);


//        qDebug() << "textAfterOffset: vOffset: " << vOffset << " => " << offset
//                 << ", startOffset: " << *startOffset << " (start: " << start << ")"
//                 << ", endOffset: " << *endOffset << " (end: " << end << ")"
//                 <<  " => " << str;
        return str;
    }

    QString str = QAccessibleTextInterface::textAfterOffset(vOffset, boundaryType, startOffset, endOffset);
//    qDebug() << "?? textAfterOffset: vOffset: " << vOffset << ", boundaryType: " << boundaryType << " => " << str << " (" << *startOffset << ", " << *endOffset <<")";
    return str;
}


/// Returns the text item of type boundaryType at offset offset and sets startOffset and endOffset
/// values to the start and end positions of that item; returns an empty string if there is no such an item.
/// Sets startOffset and endOffset values to -1 on error.
///
/// This default implementation is provided for small text edits. A word processor or text editor should
/// provide their own efficient implementations. This function makes no distinction between paragraphs and lines.
///
/// Note: this function can not take the cursor position into account. By convention an offset of -2 means that
/// this function should use the cursor position as offset. Thus an offset of -2 must be converted to the cursor
/// position before calling this function. An offset of -1 is used for the text length and custom implementations
/// of this function have to return the result as if the length was passed in as offset.
QString AccessibleTextEditorWidget::textAtOffset(int vOffset, QAccessible::TextBoundaryType boundaryType, int *startOffset, int *endOffset) const
{
    //  QAccessible::CharBoundary	0	Use individual characters as boundary.
    //  QAccessible::WordBoundary	1	Use words as boundaries.
    //  QAccessible::SentenceBoundary	2	Use sentences as boundary.
    //  QAccessible::ParagraphBoundary	3	Use paragraphs as boundary.
    //  QAccessible::LineBoundary	4	Use newlines as boundary.
    //  QAccessible::NoBoundary	5	No boundary (use the whole text).


    if(boundaryType == QAccessible::LineBoundary && vOffset >= 0) {
        int offset = V2D(textWidget(), vOffset);
        int line = textDocument()->lineFromOffset(offset);

        int start = textDocument()->offsetFromLine(line);
        int end = textDocument()->offsetFromLine(line+1);


        QString str = VTEXT_PART(textDocument(), start, end - start);
        *startOffset = D2V(textWidget(), start);
        *endOffset = (*startOffset) + str.length(); //D2V(textWidget(), end);


//        qDebug() << "textAtOffset: vOffset: " << vOffset << " => " << offset
//                 << ", startOffset: " << *startOffset << " (start: " << start << ")"
//                 << ", endOffset: " << *endOffset << " (end: " << end << ")"
//                 <<  " => " << str << " (" << str.length() << ")";
       return str;
    }

    QString str = QAccessibleTextInterface::textAtOffset(vOffset, boundaryType, startOffset, endOffset);
//    qDebug() << "?? textAtOffset: offset: " << vOffset << ", boundaryType: " << boundaryType << " => " << str << " (" << *startOffset << ", " << *endOffset <<")";
    return str;
}

/// Returns the text item of type boundaryType that is close to offset offset and sets startOffset and endOffset values
/// to the start and end positions of that item; returns an empty string if there is no such an item.
/// Sets startOffset and endOffset values to -1 on error.
///
/// This default implementation is provided for small text edits. A word processor or text editor should provide their
///  own efficient implementations. This function makes no distinction between paragraphs and lines.
///
/// Note: this function can not take the cursor position into account. By convention an offset of -2 means that this
/// function should use the cursor position as offset. Thus an offset of -2 must be converted to the cursor position
/// before calling this function. An offset of -1 is used for the text length and custom implementations of this function
/// have to return the result as if the length was passed in as offset.
QString AccessibleTextEditorWidget::textBeforeOffset(int offset, QAccessible::TextBoundaryType boundaryType, int *startOffset, int *endOffset) const
{
    // qDebug() << "?? textBeforeOffset: offset: " << offset << ", boundaryType: " << boundaryType;
    return QAccessibleTextInterface::textBeforeOffset(offset, boundaryType, startOffset, endOffset);
}

QAccessibleInterface *AccessibleTextEditorWidget::focusChild() const
{
    QAccessibleInterface* child = QAccessibleWidget::focusChild();
    return child;
}

/// Returns the rectangle for the editor widget
/// It returns the location of the textWidget (even when the TextComponent has got focus)
QRect AccessibleTextEditorWidget::rect() const
{
    QRect widgetRect = textWidget()->rect();
    QRect focusRect(textWidget()->mapToGlobal(widgetRect.topLeft()), widgetRect.size());
    return focusRect;
}

TextDocument* AccessibleTextEditorWidget::textDocument() const
{
    return textWidget()->textDocument();
}

TextSelection *AccessibleTextEditorWidget::textSelection() const
{
    return textWidget()->textSelection();
}

TextEditorController* AccessibleTextEditorWidget::controller() const
{
    return textWidget()->controller();
}

TextRenderer *AccessibleTextEditorWidget::renderer() const
{
    return textWidget()->textRenderer();
}

TextEditorWidget *AccessibleTextEditorWidget::textWidget() const
{
    return textWidgetRef_;
}

} // namespace Edbee
