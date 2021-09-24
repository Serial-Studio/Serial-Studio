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
#include "kdmactouchbar.h"

#include <QActionEvent>
#include <QApplication>
#include <QDialogButtonBox>
#include <QLayout>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QProxyStyle>
#include <QStack>
#include <QTabBar>
#include <QTimer>
#include <QWidgetAction>

#import <AppKit/AppKit.h>

QT_BEGIN_NAMESPACE

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
NSImage *qt_mac_create_nsimage(const QIcon &icon, int defaultSize = 0);
#else
//  defined in gui/painting/qcoregraphics.mm
@interface NSImage (QtExtras)
+ (instancetype)imageFromQIcon:(const QIcon &)icon;
@end
static NSImage *qt_mac_create_nsimage(const QIcon &icon)
{
    return [NSImage imageFromQIcon:icon];
}
#endif

static QString identifierForAction(QObject *action)
{
    return QStringLiteral("0x%1 %2")
        .arg((quintptr)action, QT_POINTER_SIZE * 2, 16, QLatin1Char('0'))
        .arg(action->objectName());
}


static QString removeMnemonics(const QString &original)
{
    QString returnText(original.size(), QChar('\0'));
    int finalDest = 0;
    int currPos = 0;
    int l = original.length();
    while (l) {
        if (original.at(currPos) == QLatin1Char('&')) {
            ++currPos;
            --l;
            if (l == 0)
                break;
        } else if (original.at(currPos) == QLatin1Char('(') && l >= 4 &&
                   original.at(currPos + 1) == QLatin1Char('&') &&
                   original.at(currPos + 2) != QLatin1Char('&') &&
                   original.at(currPos + 3) == QLatin1Char(')')) {
            /* remove mnemonics its format is "\s*(&X)" */
            int n = 0;
            while (finalDest > n && returnText.at(finalDest - n - 1).isSpace())
                ++n;
            finalDest -= n;
            currPos += 4;
            l -= 4;
            continue;
        }
        returnText[finalDest] = original.at(currPos);
        ++currPos;
        ++finalDest;
        --l;
    }
    returnText.truncate(finalDest);
    return returnText;
}

class TabBarAction : public QAction
{
public:
    TabBarAction(QTabBar *tabBar, QObject *parent)
        : QAction(parent)
    {
        setData(QVariant::fromValue<QObject *>(tabBar));
        connect(tabBar, &QTabBar::currentChanged, this, &TabBarAction::sendDataChanged);
        connect(tabBar, &QObject::destroyed, this, &QObject::deleteLater);
    }

    void sendDataChanged()
    {
        QActionEvent e(QEvent::ActionChanged, this);
        for (auto w : associatedWidgets())
            QApplication::sendEvent(w, &e);
    }
};

class ButtonBoxAction : public QAction
{
public:
    ButtonBoxAction(QDialogButtonBox *buttonBox, QObject *parent)
        : QAction(parent)
        , mButtonBox(buttonBox)
    {
        setData(QVariant::fromValue<QObject *>(buttonBox));

        checkStandardButtonAndTexts();

        auto timer = new QTimer(buttonBox);
        timer->start(200);
        connect(timer, &QTimer::timeout, this, &ButtonBoxAction::checkStandardButtonAndTexts);
        connect(buttonBox, &QObject::destroyed, this, &QObject::deleteLater);
    }

    void checkStandardButtonAndTexts()
    {
        QStringList buttonTexts;
        QPushButton *defaultButton = nullptr;
        for (auto b : mButtonBox->buttons()) {
            buttonTexts.append(b->text());
            if (auto pb = qobject_cast<QPushButton *>(b))
                if (pb->isDefault())
                    defaultButton = pb;
        }

        if (buttonTexts != mButtonTexts || defaultButton != mDefaultButton) {
            sendDataChanged();
            mButtonTexts = buttonTexts;
            mDefaultButton = defaultButton;
        }
    }

    void sendDataChanged()
    {
        QActionEvent e(QEvent::ActionChanged, this);
        for (auto w : associatedWidgets())
            QApplication::sendEvent(w, &e);
    }

    QDialogButtonBox *mButtonBox;
    QList<QAbstractButton *> mButtons;
    QPushButton *mDefaultButton = nullptr;
    QStringList mButtonTexts;
};

@interface QObjectPointer : NSObject {
}
@property (readonly) QObject *qobject;
@end

@implementation QObjectPointer
QObject *_qobject;
@synthesize qobject = _qobject;

- (id)initWithQObject:(QObject *)aQObject
{
    self = [super init];
    _qobject = aQObject;
    return self;
}

@end

class WidgetActionContainerWidget : public QWidget
{
public:
    WidgetActionContainerWidget(QWidget *widget, NSView *view)
        : w(widget)
        , v(view)
    {
        widget->setParent(this);
        widget->move(0, 0);
        if (!widget->testAttribute(Qt::WA_Resized))
            widget->resize(widget->sizeHint()
                               .boundedTo(QSize(widget->maximumWidth(), 30))
                               .expandedTo(widget->minimumSize()));
        setAttribute(Qt::WA_DontShowOnScreen);
        setAttribute(Qt::WA_QuitOnClose, false);
        ensurePolished();
        setVisible(true);
        QPixmap pm(1, 1);
        render(&pm, QPoint(), QRegion(),
               widget->autoFillBackground()
                   ? (QWidget::DrawWindowBackground | QWidget::DrawChildren)
                   : QWidget::DrawChildren);
    }

    QSize sizeHint() const { return w->size(); }

    void resizeEvent(QResizeEvent *event) { w->resize(event->size()); }

    bool event(QEvent *event)
    {
        if (event->type() == QEvent::UpdateRequest)
            [v setNeedsDisplay:YES];
        return QWidget::event(event);
    }

    QWidget *w;
    NSView *v;
};

@interface WidgetActionView : NSView
@property WidgetActionContainerWidget *widget;
@property QWidget *touchTarget;
@property QWidgetAction *action;
@property (readonly) NSSize intrinsicContentSize;
@end

@implementation WidgetActionView
@synthesize action;
@synthesize widget;
@synthesize touchTarget;

- (NSSize)intrinsicContentSize
{
    return NSMakeSize(widget->width(), widget->height());
}

- (id)initWithWidgetAction:(QWidgetAction *)wa
{
    self = [super init];
    action = wa;
    widget = new WidgetActionContainerWidget(action->requestWidget(nullptr), self);
    if (!widget->testAttribute(Qt::WA_Resized))
        widget->resize(widget->sizeHint()
                           .boundedTo(QSize(widget->maximumWidth(), 30))
                           .expandedTo(widget->minimumSize()));
    [self setNeedsDisplay:YES];
    return self;
}

- (void)dealloc
{
    widget->w->setParent(0);
    [super dealloc];
}

- (void)drawRect:(NSRect)frame
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    widget->w->resize(frame.size.width, frame.size.height);
    QPixmap pm(widget->w->size() * 2);
    pm.setDevicePixelRatio(2);
    pm.fill(Qt::transparent);
    widget->w->render(&pm, QPoint(), QRegion(),
                      widget->w->autoFillBackground()
                          ? (QWidget::DrawWindowBackground | QWidget::DrawChildren)
                          : QWidget::DrawChildren);
    CGImageRef cgImage = pm.toImage().toCGImage();
    NSImage *image = [[NSImage alloc] initWithCGImage:cgImage size:NSZeroSize];
    [image drawInRect:frame];
    [pool release];
}

- (void)touchesBeganWithEvent:(NSEvent *)event
{
    NSTouch *touch = [[event touchesMatchingPhase:NSTouchPhaseBegan inView:self] anyObject];
    const QPoint point = QPointF::fromCGPoint([touch locationInView:self]).toPoint();
    touchTarget = widget->childAt(point);
    if (touchTarget == nullptr)
        touchTarget = widget;
    QMouseEvent e(QEvent::MouseButtonPress, touchTarget->mapFrom(widget, point), Qt::LeftButton,
                  Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(touchTarget, &e);
}
- (void)touchesMovedWithEvent:(NSEvent *)event
{
    NSTouch *touch = [[event touchesMatchingPhase:NSTouchPhaseMoved inView:self] anyObject];
    const QPoint point = QPointF::fromCGPoint([touch locationInView:self]).toPoint();
    QMouseEvent e(QEvent::MouseButtonPress, touchTarget->mapFrom(widget, point), Qt::LeftButton,
                  Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(touchTarget, &e);
}
- (void)touchesEndedWithEvent:(NSEvent *)event
{
    NSTouch *touch = [[event touchesMatchingPhase:NSTouchPhaseEnded inView:self] anyObject];
    const QPoint point = QPointF::fromCGPoint([touch locationInView:self]).toPoint();
    QMouseEvent e(QEvent::MouseButtonRelease, touchTarget->mapFrom(widget, point), Qt::LeftButton,
                  Qt::LeftButton, Qt::NoModifier);
    qApp->sendEvent(touchTarget, &e);
}

@end

@interface DynamicTouchBarProviderDelegate : NSResponder <NSTouchBarDelegate>
@property (strong) NSMutableDictionary *items;
@end

@implementation DynamicTouchBarProviderDelegate
@synthesize items;

- (id)initWithItems:(NSMutableDictionary *)i
{
    self = [super init];
    self.items = i;
    return self;
}

- (NSTouchBarItem *)touchBar:(NSTouchBar *)touchBar
       makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier
{
    Q_UNUSED(touchBar);

    if ([self.items objectForKey:identifier] != nil)
        return [self.items objectForKey:identifier];

    return nil;
}
@end

@interface DynamicTouchBarProvider
    : NSResponder <NSTouchBarProvider, NSApplicationDelegate, NSWindowDelegate>
@property (strong) NSMutableDictionary *items;
@property (strong) NSMutableDictionary *commands;
@property (strong) NSObject *qtDelegate;
@property (strong, readonly) NSTouchBar *touchBar;
@property (strong) DynamicTouchBarProviderDelegate *delegate;
@property KDMacTouchBar *qMacTouchBar;
@property QStack<NSPopoverTouchBarItem *> openedPopOvers;
@end

@implementation DynamicTouchBarProvider
@synthesize items;
@synthesize commands;
@synthesize touchBar;
@synthesize delegate;
@synthesize qMacTouchBar;
@synthesize openedPopOvers;

- (id)initWithKDMacTouchBar:(KDMacTouchBar *)bar
{
    self = [super init];
    items = [[NSMutableDictionary alloc] init];
    commands = [[NSMutableDictionary alloc] init];
    qMacTouchBar = bar;
    delegate = [[DynamicTouchBarProviderDelegate alloc] initWithItems:items];
    return self;
}

- (void)addItem:(QAction *)action
{
    // Create custom button item
    NSString *identifier = identifierForAction(action).toNSString();
    NSTouchBarItem *item = nil;
    NSView *view = nil;

    if (auto wa = qobject_cast<QWidgetAction *>(action)) {
        NSPopoverTouchBarItem *i =
            [[[NSPopoverTouchBarItem alloc] initWithIdentifier:identifier] autorelease];
        item = i;
        view = [[WidgetActionView alloc] initWithWidgetAction:wa];
        i.collapsedRepresentation = view;
    } else if (auto tb = qobject_cast<QTabBar *>(action->data().value<QObject *>())) {
        NSCustomTouchBarItem *i =
            [[[NSCustomTouchBarItem alloc] initWithIdentifier:identifier] autorelease];
        item = i;
        NSMutableArray *labels = [[NSMutableArray alloc] init];
        view =
            [[NSSegmentedControl segmentedControlWithLabels:labels
                                               trackingMode:NSSegmentSwitchTrackingSelectOne
                                                     target:self
                                                     action:@selector(tabBarAction:)] autorelease];
        i.view = view;
    } else if (auto bb = qobject_cast<QDialogButtonBox *>(action->data().value<QObject *>())) {
        NSMutableArray *buttonItems = [[NSMutableArray alloc] init];

        for (int i = 0; i < bb->layout()->count(); ++i) {
            auto layoutItem = bb->layout()->itemAt(i);
            if (auto b = qobject_cast<QPushButton *>(layoutItem->widget())) {
                auto buttonIdentifier = identifierForAction(b).toNSString();
                NSCustomTouchBarItem *buttonItem = nil;
                NSButton *button = nil;
                if ([[items allKeys] containsObject:buttonIdentifier]) {
                    buttonItem = [items objectForKey:buttonIdentifier];
                    button = buttonItem.view;
                } else {
                    buttonItem = [[NSCustomTouchBarItem alloc] initWithIdentifier:buttonIdentifier];
                    button = [NSButton buttonWithTitle:removeMnemonics(b->text()).toNSString()
                                                target:self
                                                action:@selector(buttonAction:)];
                }
                if (b->isDefault())
                    [button setKeyEquivalent:@"\r"];
                button.title = removeMnemonics(b->text()).toNSString();
                buttonItem.view = button;
                [buttonItems addObject:buttonItem];
                [commands setObject:[[QObjectPointer alloc] initWithQObject:b]
                             forKey:[NSValue valueWithPointer:button]];
                [items setObject:buttonItem forKey:buttonIdentifier];
            }
        }

        item = [[NSGroupTouchBarItem groupItemWithIdentifier:identifier items:buttonItems]
            autorelease];
    } else if (action->isSeparator()) {
        NSPopoverTouchBarItem *i =
            [[[NSPopoverTouchBarItem alloc] initWithIdentifier:identifier] autorelease];
        item = i;
        view = [NSTextField labelWithString:action->text().toNSString()];
        i.collapsedRepresentation = view;
    } else {
        NSPopoverTouchBarItem *i =
            [[[NSPopoverTouchBarItem alloc] initWithIdentifier:identifier] autorelease];
        item = i;
        view = [[NSButton buttonWithTitle:removeMnemonics(action->text()).toNSString()
                                   target:self
                                   action:@selector(itemAction:)] autorelease];
        i.collapsedRepresentation = view;
    }

    if (view)
        [view retain];
    [item retain];

    [items setObject:item forKey:identifier];
    [commands setObject:[[QObjectPointer alloc] initWithQObject:action]
                 forKey:[NSValue valueWithPointer:view]];

    if (!qobject_cast<QDialogButtonBox *>(action->data().value<QObject *>()))
        [self changeItem:action];
    else
        [self makeTouchBar];
}

- (void)changeItem:(QAction *)action
{
    NSString *identifier = identifierForAction(action).toNSString();

    if (auto wa = qobject_cast<QWidgetAction *>(action)) {

    } else if (auto tb = qobject_cast<QTabBar *>(action->data().value<QObject *>())) {
        NSCustomTouchBarItem *item = [items objectForKey:identifier];
        NSSegmentedControl *control = item.view;
        control.segmentCount = tb->count();
        for (int i = 0; i < tb->count(); ++i) {
            [control setLabel:tb->tabText(i).toNSString() forSegment:i];
        }
        control.selectedSegment = tb->currentIndex();
    } else if (auto bb = qobject_cast<QDialogButtonBox *>(action->data().value<QObject *>())) {
        // unfortunately we cannot modify a NSGroupTouchBarItem, so we
        // have to recreate it from scratch :-(
        int index = qMacTouchBar->actions().indexOf(action);
        if (index == qMacTouchBar->actions().count() - 1) {
            qMacTouchBar->removeAction(action);
            qMacTouchBar->addAction(action);
        } else {
            QAction *before = qMacTouchBar->actions().at(index + 1);
            qMacTouchBar->removeAction(action);
            qMacTouchBar->insertAction(before, action);
        }
    } else if (action->isSeparator()) {
        NSPopoverTouchBarItem *item = [items objectForKey:identifier];
        NSTextField *field = item.collapsedRepresentation;
        field.stringValue = action->text().toNSString();
    } else {
        NSPopoverTouchBarItem *item = [items objectForKey:identifier];
        NSButton *button = item.collapsedRepresentation;
        button.imagePosition = NSImageLeft;
        button.enabled = action->isEnabled();
        button.buttonType =
            action->isCheckable() ? NSButtonTypePushOnPushOff : NSButtonTypeAccelerator;
        button.bordered = action->isSeparator() && !action->text().isEmpty() ? NO : YES;
        button.highlighted = !button.bordered;
        button.state = action->isChecked();
        button.hidden = !action->isVisible();
        button.image = qt_mac_create_nsimage(action->icon());
        button.title = removeMnemonics(action->text()).toNSString();
        switch (qMacTouchBar->touchButtonStyle())
        {
        case KDMacTouchBar::IconOnly:
            button.imagePosition = NSImageOnly;
            break;
        case KDMacTouchBar::TextOnly:
            button.imagePosition = NSNoImage;
            break;
        case KDMacTouchBar::TextBesideIcon:
            button.imagePosition = NSImageLeft;
            break;
        }
        item.showsCloseButton = action->menu() != nullptr;
        if (action->menu()) {
            item.popoverTouchBar = [[NSTouchBar alloc] init];
            item.popoverTouchBar.delegate = delegate;
            // Add ordered items array
            NSMutableArray *array = [[NSMutableArray alloc] init];
            for (auto action : action->menu()->actions()) {
                if (action->isVisible()) {
                    [self addItem:action];
                    if (action->isSeparator() && action->text().isEmpty())
                        [array addObject:NSTouchBarItemIdentifierFixedSpaceLarge];
                    else
                        [array addObject:identifierForAction(action).toNSString()];
                }
            }
            item.popoverTouchBar.defaultItemIdentifiers = array;
        }
    }

    [self makeTouchBar];
}

- (void)removeItem:(QAction *)action
{
    NSString *identifier = identifierForAction(action).toNSString();
    NSPopoverTouchBarItem *item = [items objectForKey:identifier];
    if (item == nil)
        return;
    QObjectPointer *command = [commands objectForKey:[NSValue valueWithPointer:item.view]];
    [commands removeObjectForKey:[NSValue valueWithPointer:item.view]];
    [items removeObjectForKey:identifier];
    [command release];

    [self makeTouchBar];
}

- (void)buttonAction:(id)sender
{
    QObjectPointer *qobjectPointer =
        (QObjectPointer *)[commands objectForKey:[NSValue valueWithPointer:sender]];
    // Missing entry in commands dict. Should not really happen, but does
    if (!qobjectPointer)
        return;

    QPushButton *button = qobject_cast<QPushButton *>(qobjectPointer.qobject);
    if (!button)
        return;

    button->click();
}

- (void)tabBarAction:(id)sender
{
    QObjectPointer *qobjectPointer =
        (QObjectPointer *)[commands objectForKey:[NSValue valueWithPointer:sender]];
    // Missing entry in commands dict. Should not really happen, but does
    if (!qobjectPointer)
        return;

    // Check for deleted QObject
    if (!qobjectPointer.qobject)
        return;

    QAction *action = static_cast<QAction *>(qobjectPointer.qobject);
    if (!action)
        return;
    QTabBar *tabBar = qobject_cast<QTabBar *>(action->data().value<QObject *>());
    if (!tabBar)
        return;

    NSString *identifier = identifierForAction(action).toNSString();
    NSCustomTouchBarItem *item = [items objectForKey:identifier];
    NSSegmentedControl *control = item.view;

    tabBar->setCurrentIndex(control.selectedSegment);
}

- (void)itemAction:(id)sender
{
    QObjectPointer *qobjectPointer =
        (QObjectPointer *)[commands objectForKey:[NSValue valueWithPointer:sender]];
    // Missing entry in commands dict. Should not really happen, but does
    if (!qobjectPointer)
        return;

    // Check for deleted QObject
    if (!qobjectPointer.qobject)
        return;

    QAction *action = static_cast<QAction *>(qobjectPointer.qobject);
    if (!action || action->isSeparator())
        return;
    if (!action->isEnabled())
        return;
    action->activate(QAction::Trigger);

    if (action->menu()) {
        NSString *identifier = identifierForAction(action).toNSString();
        NSPopoverTouchBarItem *item = [items objectForKey:identifier];
        [item showPopover:item];
        openedPopOvers.push(item);
    } else {
        while (!openedPopOvers.isEmpty()) {
            auto poppedItem = openedPopOvers.pop();
            [poppedItem dismissPopover:poppedItem];
        }
    }
}

- (void)clearItems
{
    [items removeAllObjects];
    [commands removeAllObjects];
}

- (NSTouchBar *)makeTouchBar
{
    // Create the touch bar with this instance as its delegate
    if (touchBar == nil) {
        touchBar = [[NSTouchBar alloc] init];
        touchBar.delegate = delegate;
    }

    // Add ordered items array
    NSMutableArray *array = [[NSMutableArray alloc] init];
    for (auto action : qMacTouchBar->actions()) {
        if (action->isVisible()) {
            if (action->isSeparator() && action->text().isEmpty())
                [array addObject:NSTouchBarItemIdentifierFixedSpaceLarge];
            else
                [array addObject:identifierForAction(action).toNSString()];
        }
    }
    touchBar.defaultItemIdentifiers = array;

    return touchBar;
}

- (void)installAsDelegateForWindow:(NSWindow *)window
{
    _qtDelegate = window.delegate; // Save current delegate for forwarding
    window.delegate = self;
}

- (void)installAsDelegateForApplication:(NSApplication *)application
{
    _qtDelegate = application.delegate; // Save current delegate for forwarding
    application.delegate = self;
}

- (BOOL)respondsToSelector:(SEL)aSelector
{
    // We want to forward to the qt delegate. Respond to selectors it
    // responds to in addition to selectors this instance resonds to.
    return [_qtDelegate respondsToSelector:aSelector] || [super respondsToSelector:aSelector];
}

- (void)forwardInvocation:(NSInvocation *)anInvocation
{
    // Forward to the existing delegate. This function is only called for selectors
    // this instance does not responds to, which means that the qt delegate
    // must respond to it (due to the respondsToSelector implementation above).
    [anInvocation invokeWithTarget:_qtDelegate];
}

@end

class KDMacTouchBar::Private
{
public:
    DynamicTouchBarProvider *touchBarProvider = nil;
    QAction *principialAction = nullptr;
    QAction *escapeAction = nullptr;
    KDMacTouchBar::TouchButtonStyle touchButtonStyle = TextBesideIcon;

    static bool automaticallyCreateMessageBoxTouchBar;
};

bool KDMacTouchBar::Private::automaticallyCreateMessageBoxTouchBar = false;

class AutomaticMessageBoxTouchBarStyle : public QProxyStyle
{
public:
    using QProxyStyle::QProxyStyle;

    void polish(QWidget *w) override
    {
        if (auto mb = qobject_cast<QMessageBox *>(w)) {
            if (KDMacTouchBar::isAutomacicallyCreatingMessageBoxTouchBar())
                new KDMacTouchBar(mb);
        }
        QProxyStyle::polish(w);
    }
};

/*!
  \class KDMacTouchBar
  \brief The KDMacTouchBar class wraps the native NSTouchBar class.

  KDMacTouchBar provides a Qt-based API for NSTouchBar. The touchbar displays
  a number of QActions. Each QAction can have a text and an icon. Alternatively,
  the QActions might be separators (with or without text) or QWidgetActions.

  Add actions by calling addAction(). Alternatively, you can use one of the
  convenience methods like addDialogButtonBox() or addTabBar() which provide
  common use cases.

  If an action with a associated menu is added, its menu items are added as
  sub-touchbar. Showing sub-menus of this menu is not supported, due to macOS
  system restrictions.

  Usage: (QtWidgets)
  \code
    QMainWindow *mw = ...;
    KDMacTouchBar *touchBar = new KDMacTouchBar(mw);
    touchBar->addAction(actionNewFile);
    touchBar->addSeparator();
    touchBar->addAction(actionSaveFile);
  \endcode
*/

/*!
\enum KDMacTouchBar::TouchButtonStyle
\value IconOnly Only display the icon.
\value TextOnly Only display the text.
\value TextBesideIcon The text appears beside the icon.
*/

/*!
    Constructs a KDMacTouchBar for the window of \a parent. If \a parent is
    nullptr, the KDMacTouchBar is shown as soon as this QApplication has focus,
    if no other window with an own KDMacTouchBar has focus.
*/
KDMacTouchBar::KDMacTouchBar(QWidget *parent)
    : QWidget(parent)
    , d(new Private)
{
    d->touchBarProvider = [[DynamicTouchBarProvider alloc] initWithKDMacTouchBar:this];
    if (parent) {
        NSView *view = reinterpret_cast<NSView *>(parent->window()->winId());
        [d->touchBarProvider installAsDelegateForWindow:[view window]];
    } else {
        [d->touchBarProvider installAsDelegateForApplication:[NSApplication sharedApplication]];
    }
}

/*!
    Constructs a KDMacTouchBar containing the QDialogButtonBox from inside of
    \a messageBox.
*/
KDMacTouchBar::KDMacTouchBar(QMessageBox *messageBox)
    : QWidget( messageBox)
    , d(new Private)
{
    d->touchBarProvider = [[DynamicTouchBarProvider alloc] initWithKDMacTouchBar:this];
    NSView *view = reinterpret_cast<NSView *>(messageBox->window()->winId());
    [d->touchBarProvider installAsDelegateForWindow:[view window]];
    setPrincipialAction(addMessageBox(messageBox));
}

/*!
    Destroys the touch bar.
*/
KDMacTouchBar::~KDMacTouchBar()
{
    [d->touchBarProvider release];
    delete d;
}

/*!
   This static convenience method controls, whether KDMacTouchBar will
   automatically create a touchbar for QMessageBox instances. The
   created touchbar contains the buttons of the message box.
   This enables to use the static QMessageBox method and still having
   a touchbar for them.

   \note When you enable this setting for the first time, KDMacTouchBar will
         install a QProxyStyle into the QApplication object to be able to create
         the KDMacTouchBar in its polish method. The installed QProxyStyle will
         use the existing application style as base style.
*/
void KDMacTouchBar::setAutomaticallyCreateMessageBoxTouchBar(bool automatic)
{
    static AutomaticMessageBoxTouchBarStyle *touchStyle = nullptr;
    if (automatic && !touchStyle) {
        qApp->setStyle(touchStyle = new AutomaticMessageBoxTouchBarStyle(qApp->style()));
    }
    Private::automaticallyCreateMessageBoxTouchBar = automatic;
}

/*!
   Returns whether KDMacTouchBar automatically creates a touchbar for
   QMessageBox instances.
*/
bool KDMacTouchBar::isAutomacicallyCreatingMessageBoxTouchBar()
{
    return Private::automaticallyCreateMessageBoxTouchBar;
}

/*!
    Adds a separator item to the end of the touch bar.
*/
QAction *KDMacTouchBar::addSeparator()
{
    auto action = new QAction(this);
    action->setSeparator(true);
    addAction(action);
    return action;
}

/*! \reimp */
bool KDMacTouchBar::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::ActionAdded:
        [d->touchBarProvider addItem:static_cast<QActionEvent *>(event)->action()];
        break;
    case QEvent::ActionChanged:
        [d->touchBarProvider changeItem:static_cast<QActionEvent *>(event)->action()];
        break;
    case QEvent::ActionRemoved:
        [d->touchBarProvider removeItem:static_cast<QActionEvent *>(event)->action()];
        break;
    default:
        break;
    }

    return QWidget::event(event);
}

/*!
    Adds a button group controlling a \a tabBar item to the end of the touch bar.
*/
QAction *KDMacTouchBar::addTabBar(QTabBar *tabBar)
{
    auto a = new TabBarAction(tabBar, this);
    addAction(a);
    return a;
}

/*!
   Removes \a tabBar from the touch bar.
*/
void KDMacTouchBar::removeTabBar(QTabBar *tabBar)
{
    for (auto a : actions()) {
        if (a->data().value<QObject *>() == tabBar) {
            removeAction(a);
            return;
        }
    }
}

/*!
   Adds the QDialogButtonBox of \a messageBox to the end of the touch bar.
*/
QAction *KDMacTouchBar::addMessageBox(QMessageBox *messageBox)
{
    return addDialogButtonBox(messageBox->findChild<QDialogButtonBox *>(QStringLiteral("qt_msgbox_buttonbox")));
}

/*!
   Removes the QDialogButtonBox of \a messageBox from the touch bar.
*/
void KDMacTouchBar::removeMessageBox(QMessageBox *messageBox)
{
    return removeDialogButtonBox(messageBox->findChild<QDialogButtonBox *>(QStringLiteral("qt_msgbox_buttonbox")));
}

/*!
   Adds a button group controlling \a buttonBox to the end of the touch bar.
*/
QAction *KDMacTouchBar::addDialogButtonBox(QDialogButtonBox *buttonBox)
{
    auto a = new ButtonBoxAction(buttonBox, this);
    addAction(a);
    return a;
}

/*!
   Removes the \a buttonBox from the touch bar.
*/
void KDMacTouchBar::removeDialogButtonBox(QDialogButtonBox *buttonBox)
{
    for (auto a : actions()) {
        if (a->data().value<QObject *>() == buttonBox) {
            removeAction(a);
            return;
        }
    }
}

/*!
   \property KDMacTouchBar::principialAction
   \brief the principial action of the touch bar

   The principial action of the touch bar is the QAction you want the system
   to center in the touch bar.

   You need to add the action to the touch bar before you can set it as principial
   action.
*/
void KDMacTouchBar::setPrincipialAction(QAction *action)
{
    d->principialAction = action;
    d->touchBarProvider.touchBar.principalItemIdentifier = action ? identifierForAction(action).toNSString() : nil;
}

QAction *KDMacTouchBar::principialAction() const
{
    return d->principialAction;
}

/*!
   \property KDMacTouchBar::escapeAction
   \brief the action used as system escape key action

   By setting a QAction as escapeAction, it is possible to replace the system
   escape key with a random action.

   You don't need to add the action to the touch bar before you can set it as
   escape action.
*/
void KDMacTouchBar::setEscapeAction(QAction *action)
{
    if (d->escapeAction == action)
        return;
    if (d->escapeAction)
        [d->touchBarProvider removeItem:d->escapeAction];
    d->escapeAction = action;
    if (d->escapeAction) {
        [d->touchBarProvider addItem:d->escapeAction];
        d->touchBarProvider.touchBar.escapeKeyReplacementItemIdentifier =
            identifierForAction(action).toNSString();
    } else
        d->touchBarProvider.touchBar.escapeKeyReplacementItemIdentifier = nil;
}

QAction *KDMacTouchBar::escapeAction() const
{
    return d->escapeAction;
}

/*!
   \property KDMacTouchBar::touchButtonStyle
   This property holds the style of touch bar buttons.

   This property defines the style of all touch buttons that are added as QActions.
   Added tab widgets, dialog button boxes, message boxes or QWidgetActions won't
   follow this style.

   The default is KDMacTouchBar::TextBesideIcon
   */
void KDMacTouchBar::setTouchButtonStyle(TouchButtonStyle touchButtonStyle)
{
    if (d->touchButtonStyle == touchButtonStyle)
        return;

    d->touchButtonStyle = touchButtonStyle;

    for (auto* action : actions())
        [d->touchBarProvider changeItem:action];
    if (d->escapeAction)
        [d->touchBarProvider changeItem:d->escapeAction];
}

KDMacTouchBar::TouchButtonStyle KDMacTouchBar::touchButtonStyle() const
{
    return d->touchButtonStyle;
}

/*!
   Removes all actions from the touch bar. Removes even the escapeAction.
   The principialAction is cleared.
*/
void KDMacTouchBar::clear()
{
    setEscapeAction(nullptr);
    setPrincipialAction(nullptr);
    for (auto* action : actions())
        removeAction(action);
}

QT_END_NAMESPACE
