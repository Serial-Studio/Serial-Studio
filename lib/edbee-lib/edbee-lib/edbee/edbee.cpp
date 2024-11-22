/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "edbee.h"

#include <QApplication>
#include <QAccessible>

#include "edbee/models/textautocompleteprovider.h"
#include "edbee/models/dynamicvariables.h"
#include "edbee/models/textbuffer.h"
#include "edbee/models/texteditorcommandmap.h"
#include "edbee/models/texteditorkeymap.h"
#include "edbee/models/textdocumentscopes.h"
#include "edbee/models/textgrammar.h"
#include "edbee/util/textcodec.h"
#include "edbee/views/accessibletexteditorwidget.h"
#include "edbee/views/texttheme.h"


#include "edbee/debug.h"

namespace edbee {

/// The edbee instance singleton
static Edbee* theInstance=0;


/// The constructor
Edbee::Edbee()
    : inited_(false)
    , defaultCommandMap_(0)
    , codecManager_(0)
    , scopeManager_(0)
    , grammarManager_(0)
    , themeManager_(0)
    , keyMapManager_(0)
    , environmentVariables_(0)
    ,autoCompleteProviderList_(0)
{
}


/// The edbee destructors destroys the different managers
Edbee::~Edbee()
{
    delete autoCompleteProviderList_;
    delete environmentVariables_;
    delete keyMapManager_;
    delete themeManager_;
    delete grammarManager_;
    delete scopeManager_;
    delete codecManager_;
    delete defaultCommandMap_;
}


/// The singleton instance getter
Edbee* Edbee::instance()
{
    if( !theInstance ) { theInstance = new Edbee(); }
    return theInstance;
}


/// sets the path where to find the keymap files
/// @param keyMapPath the path with keymap files
void Edbee::setKeyMapPath( const QString& keyMapPath )
{
    keyMapPath_ = keyMapPath;
}


/// Sets the grammar path
/// @param grammarPath the path with the grammar files
void Edbee::setGrammarPath( const QString& grammarPath )
{
    grammarPath_ = grammarPath;
}


/// Sets the path where to find the theme files
/// @param themePath the path to find the themes
void Edbee::setThemePath( const QString& themePath )
{
    themePath_ = themePath;
}


/// This method automatically initializes the edbee library it this hasn't already been done
void Edbee::autoInit()
{
    if( !inited_ ) {
        init();
        autoShutDownOnAppExit();
    }
}



/// TODO: We need a way to load the (scoped) environment variables
/// for now we just add some variables for some common languages
static void initHardCodedDynamicScopes( DynamicVariables* env )
{
    QString tcs("TM_COMMENT_START");
    QString tce("TM_COMMENT_END");
    QString tcs2("TM_COMMENT_START_2");
    QString tce2("TM_COMMENT_END_2");
    QString tcs3("TM_COMMENT_START_3");
    QString tce3("TM_COMMENT_END_3");
    env->setAndGiveScopedSelector( tcs, "# ", "source.yaml");

    env->setAndGiveScopedSelector( tcs, "// ", "source.c, source.c++, source.objc, source.objc++");
    env->setAndGiveScopedSelector( tcs2, "/*", "source.c, source.c++, source.objc, source.objc++");
    env->setAndGiveScopedSelector( tce2, "*/", "source.c, source.c++, source.objc, source.objc++");

    env->setAndGiveScopedSelector( tcs, "-- ", "source.lua");
    env->setAndGiveScopedSelector( tcs2, "--[[", "source.lua");
    env->setAndGiveScopedSelector( tce2, "]]", "source.lua");

    env->setAndGiveScopedSelector( tcs, "/*", "source.css");
    env->setAndGiveScopedSelector( tce, "*/", "source.css");

    env->setAndGiveScopedSelector( tcs, "; ", "source.clojure");

    env->setAndGiveScopedSelector( tcs, "# ", "source.coffee");
    env->setAndGiveScopedSelector( tcs2, "###", "source.coffee");
    env->setAndGiveScopedSelector( tce2, "###", "source.coffee");

    env->setAndGiveScopedSelector( tcs, "<!-- ", "text.html");
    env->setAndGiveScopedSelector( tce, " -->", "text.html");

    env->setAndGiveScopedSelector( tcs, "<!-- ", "text.xml");
    env->setAndGiveScopedSelector( tce, " -->", "text.xml");

    env->setAndGiveScopedSelector( tcs, "// ", "source.java");
    env->setAndGiveScopedSelector( tcs2, "/*", "source.java");
    env->setAndGiveScopedSelector( tce2, "*/", "source.java");

    env->setAndGiveScopedSelector( tcs, "// ", "source.js, source.json");
    env->setAndGiveScopedSelector( tcs2, "/*", "source.js, source.json");
    env->setAndGiveScopedSelector( tce2, "*/", "source.js, source.json");

    env->setAndGiveScopedSelector( tcs, "// ", "source.php");
    env->setAndGiveScopedSelector( tcs2, "# ", "source.php");
    env->setAndGiveScopedSelector( tcs3, "/*", "source.php");
    env->setAndGiveScopedSelector( tce3, "*/", "source.php");

    env->setAndGiveScopedSelector( tcs, "# ", "source.perl");

    env->setAndGiveScopedSelector( tcs, "-# ", "text.haml");    // I hate the default '/'

    env->setAndGiveScopedSelector( tcs, "# ", "source.js, source.ruby");
    env->setAndGiveScopedSelector( tcs2, "=begin", "source.js, source.ruby");
    env->setAndGiveScopedSelector( tce2, "=end", "source.js, source.ruby");

    env->setAndGiveScopedSelector( tcs, "// ", "source.scss");
    env->setAndGiveScopedSelector( tcs2, "/*", "source.scss");
    env->setAndGiveScopedSelector( tce2, "*/", "source.scss");

    env->setAndGiveScopedSelector( tcs, "-- ", "source.sql");
    env->setAndGiveScopedSelector( tcs2, "/*", "source.sql");
    env->setAndGiveScopedSelector( tce2, "*/", "source.sql");

    env->setAndGiveScopedSelector( tcs, "# ", "source.shell");

}



/// initializes the engine on startup
void Edbee::init()
{
    defaultCommandMap_    = new TextEditorCommandMap();
    codecManager_         = new TextCodecManager();
    scopeManager_         = new TextScopeManager();
    themeManager_         = new TextThemeManager();
    grammarManager_       = new TextGrammarManager();
    keyMapManager_        = new TextKeyMapManager();
    environmentVariables_ = new DynamicVariables();
    autoCompleteProviderList_ = new TextAutoCompleteProviderList();

    qRegisterMetaType<edbee::TextBufferChange>("edbee::TextBufferChange");

    // register the AccessibileText interface
    QAccessible::installFactory(edbee::AccessibleTextEditorWidget::factory);

    // factory fill the default command map
    defaultCommandMap_->loadFactoryCommandMap();

    // load all grammar definitions
    if( !grammarPath_.isEmpty() ) {
        grammarManager_->readAllGrammarFilesInPath( grammarPath_ );
    }

    // load all themes
    if( !themePath_.isEmpty() ) {
       themeManager_->listAllThemes( themePath_ );
    }

    // load the keymaps or fallback to the factory keymap
    if( !keyMapPath_.isEmpty() ) {
        keyMapManager_->loadAllKeyMaps( keyMapPath_ );
    } else {
        keyMapManager_->loadFactoryKeyMap();
    }

    inited_ = true;

    // for now initialize the dynamic variables
    initHardCodedDynamicScopes(environmentVariables_);
}


/// destroys the texteditor manager and all related class
void Edbee::shutdown()
{
    delete theInstance;
    theInstance = 0;
}


/// Call this method to automatically shutdown the texteditor manager on shutdown
/// (This method listens to the qApp::aboutToQuit signal
void Edbee::autoShutDownOnAppExit()
{
    connect( qApp, SIGNAL(aboutToQuit()),this,SLOT(shutdown()) );
}


/// returns the default editor keymap
TextEditorKeyMap* Edbee::defaultKeyMap()
{
    return keyMapManager()->get("");
}


/// Returns the default editor command map. This is the default command map that's used
/// by the editor components
/// @return the default command map
TextEditorCommandMap* Edbee::defaultCommandMap()
{
    return defaultCommandMap_;
}


/// Returns the codec-manager for Edbee
TextCodecManager* Edbee::codecManager()
{
    Q_ASSERT(inited_);
    return codecManager_;
}


/// Returns the scope manager. The scope manager is a general class to share scope names and identifiers between different editors
TextScopeManager* Edbee::scopeManager()
{
    Q_ASSERT(inited_);
    return scopeManager_;
}


/// Returns the grammar manager used by all editors
/// The grammar manager is used for sharing the different grammar definitions between the different editors
TextGrammarManager* Edbee::grammarManager()
{
    Q_ASSERT(inited_);
    return grammarManager_;
}


/// Returns the theme manager
/// The theme manager is used to share themes between the different editors
TextThemeManager* Edbee::themeManager()
{
    Q_ASSERT(inited_);
    return themeManager_;
}


/// Returns the keymap manager
TextKeyMapManager* Edbee::keyMapManager()
{
    Q_ASSERT(inited_);
    return keyMapManager_;
}


/// Returns the dynamicvariables object
DynamicVariables* Edbee::environmentVariables()
{
    Q_ASSERT(inited_);
    return environmentVariables_;
}

TextAutoCompleteProviderList *Edbee::autoCompleteProviderList()
{
    Q_ASSERT(autoCompleteProviderList_);
    return autoCompleteProviderList_;
}


} // edbee
