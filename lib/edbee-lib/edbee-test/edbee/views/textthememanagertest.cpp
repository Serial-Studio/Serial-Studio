#include "textthememanagertest.h"

#include "edbee/edbee.h"
#include "edbee/models/texteditorconfig.h"
#include "edbee/models/textdocument.h"
#include "edbee/views/textrenderer.h"
#include "edbee/views/texttheme.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/texteditorwidget.h"

#include "edbee/util/test.h"

#include <QDebug>
#include <QMainWindow>


namespace edbee {

void TextThemeManagerTest::reloadingThemesShouldNotifyAllPointerOwners()
{
    TextEditorWidget widget;
    TextDocument* doc = widget.textDocument();
    TextRenderer* renderer = widget.textRenderer();
    TextThemeManager* themeManager = Edbee::instance()->themeManager();

    // Add theheme to the theme maanger
    themeManager->setTheme("thetheme", new TextTheme() );

    // give the document the theme (and directly fetch the pointer)
    doc->config()->setThemeName("thetheme");
    TextTheme* oldTheme = renderer->theme();


    // Simulate the old (incorrect situation)
    //---------------------------------------
    // Simulate a reload theme in the manager, without a signal (this should fail and is the old situation)
    themeManager->blockSignals(true);
    TextTheme* blockedTheme = new TextTheme();
    Edbee::instance()->themeManager()->setTheme("thetheme", blockedTheme );
    testEqual( (qintptr)widget.textRenderer()->theme(), (qintptr)oldTheme );
    themeManager->blockSignals(false);

    // manually set the blocked to for safety.
    // The qmake sanitizer fails on next call with the invalid pointer reference ;) )
    // which is probably the cause of the crash of issue #26
    renderer->setThemeByName("thetheme");

    // Simulate reload theme (correct situation)
    //------------------------------------------
    TextTheme* newTheme = new TextTheme();
    Edbee::instance()->themeManager()->setTheme("thetheme", newTheme );
    testEqual( (qintptr)widget.textRenderer()->theme(), (qintptr)newTheme );
}

}
