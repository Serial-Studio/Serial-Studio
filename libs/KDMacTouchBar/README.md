# KDMacTouchBar
KDAB's Qt Widget for the Mac Touch Bar

## Introduction
The KDMacTouchBar class wraps the native NSTouchBar class.

KDMacTouchBar provides a Qt-based API for NSTouchBar. The touchbar displays
a number of QActions. Each QAction can have a text and an icon. Alternatively,
the QActions might be separators (with or without text) or QWidgetActions.

Add actions by calling addAction(). Alternatively, you can use one of the
convenience methods like addDialogButtonBox() or addTabBar() which provide
common use cases.

If an action with a associated menu is added, its menu items are added as
sub-touchbar. Showing sub-menus of this menu is not supported, due to macOS
system restrictions.

## Usage:
```
QMainWindow *mw = ...;
KDMacTouchBar *touchBar = new KDMacTouchBar(mw);
touchBar->addAction(actionNewFile);
touchBar->addSeparator();
touchBar->addAction(actionSaveFile);
```

## Licensing:
KD MacTouchBar is (C) 2019-2021, Klarälvdalens Datakonsult AB,
and is available under the terms of:

* the LGPL (see LICENSE.LGPL.txt for details)
* the KDAB commercial license, provided that you buy a license.
  please contact info@kdab.com if you are interested in buying commercial licenses.

## Get Involved:
KDAB will happily accept external contributions; however, **all**
contributions will require a signed Contributor License Agreement
(see docs/KDMacTouchBar-CopyrightAssignmentForm.pdf).

Contact info@kdab.com for more information.

Please submit your contributions or issue reports from our GitHub space at
https://github.com/KDAB/KDMacTouchBar

## About KDAB
KD MacTouchBar is supported and maintained by Klarälvdalens Datakonsult AB (KDAB).

The KDAB Group is the global No.1 software consultancy for Qt, C++ and
OpenGL applications across desktop, embedded and mobile platforms.

The KDAB Group provides consulting and mentoring for developing Qt applications
from scratch and in porting from all popular and legacy frameworks to Qt.
We continue to help develop parts of Qt and are one of the major contributors
to the Qt Project. We can give advanced or standard trainings anywhere
around the globe on Qt as well as C++, OpenGL, 3D and more.

Please visit https://www.kdab.com to meet the people who write code like this.

Stay up-to-date with KDAB product announcements:

* [KDAB Newsletter](https://news.kdab.com)
* [KDAB Blogs](https://www.kdab.com/category/blogs)
* [KDAB on Twitter](https://twitter.com/KDABQt)
