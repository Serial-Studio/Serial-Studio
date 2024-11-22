/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QObject>

namespace edbee {

class DynamicVariables;
class TextAutoCompleteProviderList;
class TextCodecManager;
class TextEditorCommandMap;
class TextEditorKeyMap;
class TextGrammarManager;
class TextKeyMapManager;
class TextScopeManager;
class TextThemeManager;

/// The texteditor manager,
/// It manages all singleton objects for the editor
/// It performs the initialization and shutdown code for all editors
class EDBEE_EXPORT Edbee : public QObject
{
    Q_OBJECT
private:
    Edbee();
    virtual ~Edbee();

public:
    static Edbee* instance();

    void setKeyMapPath( const QString& keyMapPath );
    void setGrammarPath( const QString& grammarPath );
    void setThemePath( const QString& themePath );

    void autoInit();

public slots:
    void init();
    void shutdown();
    void autoShutDownOnAppExit();

public:

    TextEditorKeyMap* defaultKeyMap();
    TextEditorCommandMap* defaultCommandMap();

    TextCodecManager* codecManager();
    TextScopeManager* scopeManager();
    TextGrammarManager* grammarManager();
    TextThemeManager* themeManager();
    TextKeyMapManager* keyMapManager();
    DynamicVariables* environmentVariables();
    TextAutoCompleteProviderList* autoCompleteProviderList();


protected:
    bool inited_;                               ///< This method is set to true if the manager is inited

    QString grammarPath_;                       ///< The path were to load all grammars from
    QString themePath_;                         ///< The path to load all themes from
    QString keyMapPath_;                        ///< The path to load all keymaps

    TextEditorCommandMap* defaultCommandMap_;   ///< The default command map

    TextCodecManager* codecManager_;            ///< The text-editor codec manager
    TextScopeManager* scopeManager_;            ///< The text-scope manager
    TextGrammarManager* grammarManager_;        ///< The grammar manager
    TextThemeManager* themeManager_;            ///< The text theme manager
    TextKeyMapManager* keyMapManager_;          ///< The keymap manager
    DynamicVariables* environmentVariables_;    ///< The (dynamic) environment variables
    TextAutoCompleteProviderList* autoCompleteProviderList_;   ///< The global autocomplete providers
};



} // edbee
