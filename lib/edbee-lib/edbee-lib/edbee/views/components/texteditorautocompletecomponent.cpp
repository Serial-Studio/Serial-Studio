#include "texteditorautocompletecomponent.h"

#include <QApplication>
#include <QKeyEvent>
#include <QLayout>
#include <QHBoxLayout>
#include <QWidgetAction>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTextEdit>
#include <QtGui>
#include <QTime>

#include "edbee/edbee.h"
#include "edbee/models/textautocompleteprovider.h"
#include "edbee/models/textdocument.h"
#include "edbee/models/texteditorconfig.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/views/components/texteditorcomponent.h"
#include "edbee/views/components/textmargincomponent.h"
#include "edbee/views/textrenderer.h"
#include "edbee/views/textselection.h"
#include "edbee/views/texttheme.h"

#include "edbee/debug.h"

namespace edbee {

TextEditorAutoCompleteComponent::TextEditorAutoCompleteComponent(TextEditorController *controller, TextEditorComponent *parent, TextMarginComponent *margin)
    : QWidget(parent)
    , controllerRef_(controller)
    , menuRef_(nullptr)
    , editorComponentRef_(parent)
    , marginComponentRef_(margin)
    , eventBeingFiltered_(false)
    , canceled_(false)
    , infoTipRef_(nullptr)
{
    /// initialize the widget
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::TopToBottom);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    this->setFocusPolicy(Qt::NoFocus);
    this->setAttribute(Qt::WA_ShowWithoutActivating);

    menuRef_ = new QMenu(this);
    menuRef_->setFocusPolicy(Qt::NoFocus);
    menuRef_->setAttribute(Qt::WA_ShowWithoutActivating);

    listWidgetRef_ = new QListWidget(menuRef_);
    listWidgetRef_->setFocusPolicy(Qt::NoFocus);
    listWidgetRef_->setAttribute(Qt::WA_ShowWithoutActivating);

    listWidgetRef_->installEventFilter(this);

    menuRef_->installEventFilter(this);
    menuRef_->setStyleSheet("QMenu { border: 1px solid black; }");
    listWidgetRef_->setObjectName("listWidgetRef");
    setLayout(layout);
    resize(0, 0);

    QWidgetAction *wAction = new QWidgetAction(menuRef_);
    wAction->setDefaultWidget(listWidgetRef_);
    menuRef_->addAction(wAction);

    listWidgetRef_->setFocus();

    infoTipRef_ = new FakeToolTip(controllerRef_, this);

    QPalette p = listWidgetRef_->palette();
    p.setColor(QPalette::Highlight, p.color(QPalette::Highlight));  // prevents the non-focus gray color
    p.setColor(QPalette::HighlightedText, p.color(QPalette::HighlightedText));
    p.setColor(QPalette::Base, QColor(37, 37, 38));
    p.setColor(QPalette::Text, Qt::white);

    listWidgetRef_->setAttribute(Qt::WA_NoMousePropagation,true);   // do not wheel
    listWidgetRef_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    listWidgetRef_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listWidgetRef_->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    AutoCompleteDelegate *acDel = new AutoCompleteDelegate(controllerRef_, this);
    listWidgetRef_->setItemDelegate(acDel);

    // make clicking in the list word
    listWidgetRef_->setMouseTracking(true);
    connect( listWidgetRef_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(listItemClicked(QListWidgetItem*)));
    connect( listWidgetRef_, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(listItemDoubleClicked(QListWidgetItem*)));
    connect( listWidgetRef_, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(showInfoTip()));
}


/// Returns the current text editor controller
TextEditorController *TextEditorAutoCompleteComponent::controller() const
{
    return controllerRef_;
}

QSize TextEditorAutoCompleteComponent::sizeHint() const
{
    if(!listWidgetRef_) return QSize();

    return QSize(850, ( qMin(listWidgetRef_->count(), 10) * 15 ) + 5);
}

/// This method check if the autocomplete should be shown
/// this method ALSO sets the word that's display
bool TextEditorAutoCompleteComponent::shouldDisplayAutoComplete(TextRange& range, QString& word )
{
    // when it's a selection (which shouldn't be posssible, but still check it)
    if( range.hasSelection()) return false;

    // when the character AFTER the current is a identifier we should NOT open it
    TextDocument* doc = controller()->textDocument();
    QChar nextChar = doc->charAtOrNull(range.caret());
    if( nextChar.isLetterOrNumber() ) return false;

    // expand the given range to word
    TextRange wordRange = range;
    wordRange.expandToWord(doc, doc->config()->whitespaces(), doc->config()->charGroups() );
    wordRange.maxVar() = range.max(); // next go past the right caret!
    word = doc->textPart(wordRange.min(), wordRange.length()).trimmed();      // workaround for space select bug! #61

    if(word.isEmpty()) {
        canceled_ = false;
        return false;
    }

    // canceled state, hides the autocomplete
    if( canceled_ ) { return false; }


    // else we can should
    return true;
}

void TextEditorAutoCompleteComponent::showInfoTip()
{
    if( !listWidgetRef_->isVisible() )
        return;

    const QModelIndex &current = listWidgetRef_->currentIndex();
    if( !current.isValid() )
        return;

    QString infoTip = current.data(Qt::UserRole).toString();
    if( infoTip.isEmpty() ) {
//        infoTip = "No tooltip data found!";
    }

    if( infoTipRef_.isNull() ) {
        infoTipRef_ = new FakeToolTip(controllerRef_, this);
    }

    infoTipRef_->setText(infoTip);

    const QFont font = controller()->textDocument()->config()->font();
    infoTipRef_->tipText->setDefaultFont(font);

    QFontMetrics fm(font);
    int maxWidth = 0;

    //for( QListWidgetItem *item : listWidgetRef_->findItems("*", Qt::MatchWildcard)) {
    for( int i=0; i<listWidgetRef_->count(); i++ ){
        QListWidgetItem *item = listWidgetRef_->item(i);
        QString sLabel = item->data(Qt::DisplayRole).toString();
        QString sDetail = item->data(Qt::UserRole).toString();
        QString sType = "";
        int widthMod = 4;
        if( listWidgetRef_->count() > 10 ){
            widthMod = listWidgetRef_->verticalScrollBar()->width();
            sLabel = sLabel + " ";
        }
        if( sDetail.contains(" = ") ){
            sType = QString("%1").arg(sDetail.split(" = ").value(0));
            int width = fm.horizontalAdvance(QString("%1   %2").arg(sLabel).arg(sType)) + widthMod;
            maxWidth = qMax(width, maxWidth);
        } else {
            int width = fm.horizontalAdvance(QString("%1").arg(sLabel)) + widthMod;
            maxWidth = qMax(width, maxWidth);
        }
    }

    int spacing = 0;
    menuRef_->resize(QSize(maxWidth + spacing + 2, ( qMin(listWidgetRef_->count(), 10) * fm.height() ) + 6 ));
    listWidgetRef_->resize(QSize(maxWidth + spacing, ( qMin(listWidgetRef_->count(), 10) * fm.height() + 4 )));
//    menuRef_->resize(QSize(maxWidth + spacing + 2, ( qMin(listWidgetRef_->count(), 10) * fm.height() ) ));
//    listWidgetRef_->resize(QSize(maxWidth + spacing, ( qMin(listWidgetRef_->count(), 10) * fm.height() )));


    QRect r = listWidgetRef_->visualItemRect(listWidgetRef_->currentItem());
    int xOffset;
    if( listWidgetRef_->count() > 10 )
        xOffset = 22+1;
    else
        xOffset = 5+1;

    //fetch the current selection
    TextRange range = controller()->textSelection()->range(0);

    QSize tipSize = infoTipRef_->tipText->documentLayout()->documentSize().toSize();

    infoTipRef_->resize(tipSize.width(), tipSize.height() - 4);

    //position the list
    positionWidgetForCaretOffset( qMax(0,range.caret() - currentWord_.length()) );

    QPoint newLoc(listWidgetRef_->parentWidget()->mapToGlobal(r.topRight()).x() + xOffset, listWidgetRef_->parentWidget()->mapToGlobal(r.topRight()).y() + 1);

    QRect screen = QApplication::primaryScreen()->availableGeometry();

    if( newLoc.x() + infoTipRef_->width() > screen.x() + screen.width() && (menuRef_->x() - infoTipRef_->width() - 1) >= 0 ){
        newLoc.setX(menuRef_->x() - infoTipRef_->width() - 1);
    }

    if(!infoTip.isEmpty()) {
      infoTipRef_->repaint();
      infoTipRef_->move(newLoc);
      infoTipRef_->show();
      infoTipRef_->raise();
    }
}

void TextEditorAutoCompleteComponent::hideInfoTip()
{
    if ( !infoTipRef_.isNull() )
        infoTipRef_->hide();
}

/// Fills the autocomplete list
bool TextEditorAutoCompleteComponent::fillAutoCompleteList(TextDocument* document, const TextRange& range, const QString& word)
{
    listWidgetRef_->clear();
    QList<TextAutoCompleteItem*> items = document->autoCompleteProviderList()->findAutoCompleteItemsForRange(document,range,word);
    if( items.length() > 0 ) {
        foreach( TextAutoCompleteItem* item, items ) {
            QListWidgetItem *wItem = new QListWidgetItem();
            wItem->setData(Qt::DisplayRole, item->label());
            wItem->setData(Qt::DecorationRole, item->kind());
            wItem->setData(Qt::UserRole, item->detail());
            wItem->setData(Qt::WhatsThisRole, item->documentation());
            listWidgetRef_->addItem(wItem);
        }
        listWidgetRef_->setCurrentIndex(listWidgetRef_->model()->index(0,0) );
    }
    else
    {
        hide();
    }

    QFont font = controller()->textDocument()->config()->font();
    QFontMetrics fm(font);

    setFixedHeight( ( qMin(listWidgetRef_->count(), 10) * fm.height() ) + 4 );

    if( items.length() > 10 ) {
        listWidgetRef_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    } else {
        listWidgetRef_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    return items.length() > 0;
}


/// positions the widget so it's visible.
void TextEditorAutoCompleteComponent::positionWidgetForCaretOffset(int offset)
{
    // find the caret position
    TextRenderer* renderer = controller()->textRenderer();
    int y = renderer->yPosForOffset(offset) - controller()->textRenderer()->viewportY(); //offset the y position based on how far scrolled down the editor is
    int x = renderer->xPosForOffset(offset) - controller()->textRenderer()->viewportX() + marginComponentRef_->widthHint() - 3; //adjusts x position based on scrollbars, line-number, and position in line
    QPoint newLoc = editorComponentRef_->parentWidget()->parentWidget()->mapToGlobal(QPoint(x, y));

    //We want to constrain the list to only show within the available screen space
#if QT_VERSION >= 0x050a00
    QScreen* pScreen = qApp->screenAt(newLoc);
    /* From the Qt documentation:
     * "The available geometry is the geometry excluding window manager reserved
     * areas such as task bars and system menus.
     *
     * Note, on X11 this will return the true available geometry only on systems
     * with one monitor and if window manager has set _NET_WORKAREA atom. In all
     * other cases this is equal to geometry(). This is a limitation in X11
     * window manager specification."
     */
    if (!pScreen) {
        // newLoc is off-screen so cannot be shown
        return;
    }
    QRect screen = pScreen->availableGeometry();
#else
    QRect screen = qApp->desktop()->availableGeometry(editorComponentRef_->parentWidget()->parentWidget());
#endif

    newLoc.setX(qMax(screen.left(), newLoc.x()));                                   //constrain the origin of the list to the leftmost pixel
    newLoc.setY(qMax(screen.top(), newLoc.y()));                                    //constrain the origin of the list to the topmost pixel
    newLoc.setX(qMin(screen.x() + screen.width() - menuRef_->width(), newLoc.x())); //ensure that the entire width of the list can be shown to the right
    if( newLoc.y() + menuRef_->height() > screen.bottom() ){                        //if the list could go below the bottom, draw above
        newLoc.setY(qMin(newLoc.y(), screen.bottom()) - menuRef_->height());        //positions the list above the word
    } else {
        newLoc.setY(newLoc.y() + renderer->lineHeight()); //places it below the line, as normal
    }
    menuRef_->move(newLoc.x(), newLoc.y());
}

/// intercepts hide() calls to inform the tooltip to hide as well
void TextEditorAutoCompleteComponent::hideEvent(QHideEvent *event)
{
    infoTipRef_->hide();
    event->isAccepted();
}

/// we need to intercept keypresses if the widget is visible
bool TextEditorAutoCompleteComponent::eventFilter(QObject *obj, QEvent *event)
{
    if( event->type() == QEvent::Close && obj == menuRef_) {
        hide();
        hideInfoTip();
        return QObject::eventFilter(obj, event);
    }
    if( obj == listWidgetRef_ && event->type()==QEvent::KeyPress ) {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);

        // text keys are allowed
        if( !key->text().isEmpty() ) {
            QChar nextChar = key->text().at(0);
            if( nextChar.isLetterOrNumber() ) {
              QApplication::sendEvent(editorComponentRef_, event);
              return true;
            }
        }

        // escape key
        switch( key->key() ) {
            case Qt::Key_Escape:
                menuRef_->close();
                canceled_ = true;
                return true; // stop event

            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Tab:
                if( listWidgetRef_->currentItem() && currentWord_ == listWidgetRef_->currentItem()->text() ) { // sends normal enter/return/tab if you've typed a full word
                    menuRef_->close();
                    QApplication::sendEvent(editorComponentRef_, event);
                    return true;
                } else if ( listWidgetRef_->currentItem() ) {
                    insertCurrentSelectedListItem();
                    menuRef_->close();
                    return true;
                }
                break;

            case Qt::Key_Backspace:
                QApplication::sendEvent(editorComponentRef_, event);
                return true;

            case Qt::Key_Shift: //ignore shift, don't hide
                QApplication::sendEvent(editorComponentRef_, event);
                return true;

            // forward special keys to list
            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_PageDown:
            case Qt::Key_PageUp:
                return false;
        }

        // default operation is to hide and continue the event
        menuRef_->close();
        QApplication::sendEvent(editorComponentRef_, event);
        return true;

    }
    return QObject::eventFilter(obj, event);
}


/// inserts the currently selected list item
void TextEditorAutoCompleteComponent::insertCurrentSelectedListItem()
{
    TextSelection* sel = controller()->textSelection();
    sel->moveCarets(- currentWord_.length());
    if( listWidgetRef_->currentItem() ) {
        controller()->replaceSelection(listWidgetRef_->currentItem()->text());
    }
}

void TextEditorAutoCompleteComponent::updateList()
{
    // fetch the current selection
    TextDocument* doc = controller()->textDocument();
    TextRange range = controller()->textSelection()->range(0);

    if (!doc->config()->autocompleteAutoShow()) {
        return;
    }

    // when the character after
    if(!shouldDisplayAutoComplete(range, currentWord_)) {
        if(isVisible()) {
            menuRef_->close();
        }
        return;
    }

    // fills the autocomplete list with the curent word
    if( fillAutoCompleteList(doc, range, currentWord_)) {
        menuRef_->popup(menuRef_->pos());

        editorComponentRef_->setFocus();

        // position the widget
        showInfoTip();
    } else {
        menuRef_->close();
    }
}

/// this event is called when a key pressed
void TextEditorAutoCompleteComponent::textKeyPressed()
{
    updateList();
}

/// processes backspaces
void TextEditorAutoCompleteComponent::backspacePressed()
{
    updateList();
}

void TextEditorAutoCompleteComponent::listItemClicked(QListWidgetItem* item)
{
    item->setSelected(true);
    showInfoTip();
}

void TextEditorAutoCompleteComponent::listItemDoubleClicked(QListWidgetItem*)
{
    insertCurrentSelectedListItem();
    menuRef_->close();
}

void TextEditorAutoCompleteComponent::selectItemOnHover(QModelIndex modelIndex)
{
    listWidgetRef_->selectionModel()->select(modelIndex,QItemSelectionModel::SelectCurrent);
}

AutoCompleteDelegate::AutoCompleteDelegate(TextEditorController *controller, QObject *parent) : QAbstractItemDelegate(parent)
{
    pixelSize = 12;
    controllerRef_ = controller;
}

TextEditorController *AutoCompleteDelegate::controller() const
{
    return controllerRef_;
}

void AutoCompleteDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    TextRenderer* renderer = controller()->textRenderer();
    TextTheme* themeRef_ = renderer->theme();

    painter->setPen( themeRef_->caretColor() );

    QFont font = controller()->textDocument()->config()->font();
    QFontMetrics fm(font);

    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, renderer->theme()->selectionColor());
    else
        painter->fillRect(option.rect, renderer->theme()->backgroundColor());
    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(themeRef_->foregroundColor());

    QString sLabel = index.data(Qt::DisplayRole).toString();
    QString sDetail = index.data(Qt::UserRole).toString();
    QString sDocumentation = index.data(Qt::WhatsThisRole).toString();
    QString sType = "void";

    if (sDetail.contains(" = ")) {
        sType = sDetail.split(" = ").value(0);
    }

    QRect typeRect = option.rect;
    QRect hyphenRect = option.rect;
    QRect nameRect = option.rect;
    QPen typePen = QPen(themeRef_->findHighlightForegroundColor());
    QPen namePen = QPen(themeRef_->foregroundColor());

    hyphenRect.setX(hyphenRect.x() + fm.horizontalAdvance(sLabel));
    typeRect.setX(nameRect.x() + nameRect.width() - fm.horizontalAdvance(sType) - 1);
    painter->setFont(font);
    painter->drawText(nameRect, sLabel);

    if (sType != "void")
    {
        painter->drawText(typeRect, sType);
    }
    painter->restore();
}

QSize AutoCompleteDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
    const QFont font = controller()->textDocument()->config()->font();
    QFontMetrics fm(font);
    return QSize(100, ( fm.height() ) );
}

FakeToolTip::FakeToolTip(TextEditorController *controller, QWidget *parent) :
    QWidget(parent, Qt::ToolTip | Qt::WindowStaysOnTopHint )
{
    setFocusPolicy(Qt::NoFocus);
    //setAttribute(Qt::WA_DeleteOnClose);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    tipText = new QTextDocument(this);

    tipText->setHtml("");

    controllerRef_ = controller;

    // Set the window and button text to the tooltip text color, since this
    // widget draws the background as a tooltip.
    QPalette p = palette();
    const QColor toolTipTextColor = p.color(QPalette::Inactive, QPalette::ToolTipText);
    p.setColor(QPalette::Inactive, QPalette::WindowText, toolTipTextColor);
    p.setColor(QPalette::Inactive, QPalette::ButtonText, toolTipTextColor);
    setPalette(p);

    setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, nullptr, this) / 255.0);
}

void FakeToolTip::setText(const QString text)
{
    TextRenderer* renderer = controller()->textRenderer();
    TextTheme* themeRef_ = renderer->theme();
    tipText->setDefaultStyleSheet("p {color:" + themeRef_->foregroundColor().name() + "}");
    tipText->setHtml(QString("<p>%1</p>").arg(text));
}

TextEditorController *FakeToolTip::controller()
{
    return controllerRef_;
}

void FakeToolTip::paintEvent(QPaintEvent*)
{
    QStyle *style = this->style();
    QPainter *p = new QPainter(this);
    QStyleOptionFrame *opt = new QStyleOptionFrame();
    QRect labelRect;
    TextRenderer* renderer = controller()->textRenderer();
    TextTheme* themeRef_ = renderer->theme();

    opt->initFrom(this);
    style->drawPrimitive(QStyle::PE_PanelTipLabel, opt, p);

    tipText->setDefaultFont(controller()->textDocument()->config()->font());

    labelRect = this->rect();

    labelRect.setX(labelRect.x() + 2);
    labelRect.setY(labelRect.y() + 2);
    labelRect.setWidth(labelRect.width() - 2);
    labelRect.setHeight(labelRect.height() - 2);

    p->fillRect(labelRect, themeRef_->backgroundColor());

    p->translate(-2, -2);

    p->setPen(themeRef_->foregroundColor());
    labelRect.setWidth(labelRect.width() + 1);
    labelRect.setHeight(labelRect.height() + 1);

    tipText->drawContents(p, labelRect);

    p->end();
    delete p;
    delete opt;
}

void FakeToolTip::resizeEvent(QResizeEvent *)
{
    QStyleHintReturnMask frameMask;
    QStyleOption option;
    option.initFrom(this);
}

}// edbee
