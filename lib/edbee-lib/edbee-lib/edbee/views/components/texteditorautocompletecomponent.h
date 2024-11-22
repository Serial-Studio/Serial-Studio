#pragma once

#include "edbee/exports.h"

#include <QModelIndex>
#include <QWidget>
#include <QLabel>
#include <QAbstractItemDelegate>
#include <QPainter>
#include <QScrollBar>
#include <QStylePainter>
#include <QPointer>
#include <QMenu>
#include <QTextDocument>

class QListWidget;
class QListWidgetItem;

namespace edbee {

class TextDocument;
class TextEditorController;
class TextEditorComponent;
class TextMarginComponent;
class TextEditorWidget;
class TextRange;

class FakeToolTip : public QWidget
{
    Q_OBJECT

public:
    explicit FakeToolTip(TextEditorController *controller, QWidget *parent = 0);
    void setText(const QString text);
    QTextDocument* tipText;
    TextEditorController* controller();

private:
    TextEditorController* controllerRef_;       ///< A reference to the controller

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);
};

// inspiration:
// http://doc.qt.io/qt-5/qtwidgets-tools-customcompleter-example.html

/// An autocomplete list
/// Which receives it's autocomplete list from the document
class EDBEE_EXPORT TextEditorAutoCompleteComponent : public QWidget
{
    Q_OBJECT
public:
    explicit TextEditorAutoCompleteComponent(TextEditorController* controller, TextEditorComponent *parent, TextMarginComponent *margin);

    TextEditorController* controller() const;

    QSize sizeHint() const;
protected:

    bool shouldDisplayAutoComplete(TextRange& range, QString& word);
    void hideInfoTip();
    bool fillAutoCompleteList(TextDocument *document, const TextRange &range, const QString& word );

    void positionWidgetForCaretOffset(int offset);
    bool eventFilter(QObject* obj, QEvent* event);

    void hideEvent(QHideEvent *event);
    //void focusOutEvent(QFocusEvent *event);
    //void moveEvent(QMoveEvent *event);

    void insertCurrentSelectedListItem();
signals:

public slots:
    void updateList();
    void backspacePressed();
    void textKeyPressed();
    void listItemClicked(QListWidgetItem*item);
    void listItemDoubleClicked(QListWidgetItem*item);
    void selectItemOnHover(QModelIndex modelIndex);
    void showInfoTip();

private:
    TextEditorController* controllerRef_;       ///< A reference to the controller
    QMenu* menuRef_;
    QListWidget* listWidgetRef_;                ///< The current autocomplete words
    TextEditorComponent* editorComponentRef_;   ///< Reference to the editor component
    TextMarginComponent* marginComponentRef_;   ///< Reference to the editor's margin
    bool eventBeingFiltered_;                   ///< Prevent endless double filter when forwarding event to list item
    QString currentWord_;                       ///< The current word beïng entered
    bool canceled_;                             ///< Has the autocomplete been canceled
    QPointer<FakeToolTip> infoTipRef_;
};

class AutoCompleteDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    AutoCompleteDelegate(TextEditorController *controller, QObject *parent = 0);

    TextEditorController* controller() const;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

public slots:

private:
    TextEditorController* controllerRef_;       ///< A reference to the controller
    int pixelSize;
};

}
