/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QCache>
#include <QTextLayout>
#include <QTextCharFormat>

class QTextFormat;

namespace edbee {

class MultiLineScopedTextRange;
class ScopedTextRange;
class TextBufferChange;
class TextDocument;
class TextEditorController;
class Edbee;
class TextScopeList;
class TextScopeSelector;

/// The styles available in tmTheme files
//class TextStyle
//{
//    bool bold_, italic_, underline_;
//    QColor foreground_;
//    QColor background_;
//};

//=================================================

class EDBEE_EXPORT TextThemeRule {
public:
    TextThemeRule(const QString& name, const QString& selector, QColor foreground=QColor(), QColor background=QColor(), bool bold=false, bool italic=false, bool underline=false );
    virtual ~TextThemeRule();
    bool matchesScopeList(const TextScopeList *scopes );
    void fillFormat( QTextCharFormat* format );

    QString name() { return name_; }
    TextScopeSelector* scopeSelector() { return scopeSelector_; }
    QColor foregroundColor() { return foregroundColor_; }
    QColor backgroundColor() { return backgroundColor_; }
    bool bold() { return bold_; }
    bool italic() { return italic_; }
    bool underline() { return underline_; }

private:

    QString name_;                      ///< The human name of the setting
    TextScopeSelector* scopeSelector_;  ///< The scope selector
//    QString comment_;                   ///< The comment for this theme

    QColor foregroundColor_;            ///< The foreground-color
    QColor backgroundColor_;            ///< The background-color
    bool bold_;
    bool italic_;
    bool underline_;
};


//=================================================

/// This class defines a single theme
class EDBEE_EXPORT TextTheme : public QObject
{
public:
    TextTheme();
    virtual ~TextTheme();

    const QList<TextThemeRule*>& rules() { return themeRules_; }
    void giveThemeRule( TextThemeRule* rule );

    void fillFormatForTextScopeList(const TextScopeList *scopeList, QTextCharFormat* format );

    QString name() { return name_; }
    void setName( const QString& name ) { name_ = name; }
    QString uuid() { return uuid_; }
    void setUuid( const QString& uuid ) { uuid_ = uuid; }

    QColor backgroundColor() { return backgroundColor_; }
    void setBackgroundColor( const QColor& color ) { backgroundColor_ = color ; }
    QColor caretColor() { return caretColor_; }
    void setCaretColor( const QColor& color ) { caretColor_=color; }
    QColor foregroundColor() { return foregroundColor_; }
    void setForegroundColor( const QColor& color ) { foregroundColor_=color; }
    QColor invisiblesColor() { return invisiblesColor_; }
    void setInvisiblesColor( const QColor& color ) { invisiblesColor_=color; }
    QColor lineHighlightColor() { return lineHighlightColor_; }
    void setLineHighlightColor( const QColor& color ) { lineHighlightColor_=color; }
    QColor selectionColor() { return selectionColor_; }
    void setSelectionColor( const QColor& color ) { selectionColor_=color; }
    QColor findHighlightBackgroundColor() { return findHighlightBackgroundColor_; }
    void setFindHighlightBackgroundColor( const QColor& color ) { findHighlightBackgroundColor_=color; }
    QColor findHighlightForegroundColor() { return findHighlightForegroundColor_; }
    void setFindHighlightForegroundColor( const QColor& color ) { findHighlightForegroundColor_=color; }
    QColor selectionBorderColor() { return selectionBorderColor_; }
    void setSelectionBorderColor( const QColor& color ) { selectionBorderColor_=color; }
    QColor activeGuideColor() { return activeGuideColor_; }
    void setActiveGuideColor( const QColor& color ) { activeGuideColor_=color; }

    QColor bracketForegroundColor() { return bracketForegroundColor_;}
    void setBracketForegroundColor( const QColor& color ) { bracketForegroundColor_=color;}
    QString bracketOptions() { return bracketOptions_; }
    void setBracketOptions( const QString& str ) { bracketOptions_=str; }

    QColor bracketContentsForegroundColor() { return bracketContentsForegroundColor_;}
    void setBracketContentsForegroundColor( const QColor& color ) { bracketContentsForegroundColor_=color;}
    QString bracketContentsOptions() { return bracketContentsOptions_; }
    void setBracketContentsOptions(const QString& str ) { bracketContentsOptions_=str; }

    QString tagsOptions() { return tagsOptions_; }
    void setTagsOptions(const QString& str ) { tagsOptions_=str; }


private:

    QString name_;                        ///< The name of the theme
    QString uuid_;                        ///< The uuid


    // thTheme settings
    QColor backgroundColor_;               ///< The default background color    <key>background</key> <string>#272822</string>
    QColor caretColor_;                    ///< The default caret color         <key>caret</key> <string>#F8F8F0</string>
    QColor foregroundColor_;               ///< The default foreground color    <key>foreground</key> <string>#F8F8F2</string>
    QColor invisiblesColor_;               ///< The invisible color             <key>invisibles</key> <string>#3B3A32</string>
    QColor lineHighlightColor_;            ///< The highlight color             <key>lineHighlight</key> <string>#3E3D32</string>
    QColor selectionColor_;                ///< The selection color             <key>selection</key> <string>#49483E</string>
    QColor findHighlightBackgroundColor_;  ///< The find-highlight color        <key>findHighlight</key> <string>#FFE792</string>
    QColor findHighlightForegroundColor_;  ///< The find-highlight color        <key>findHighlightForeground</key> <string>#000000</string>
    QColor selectionBorderColor_;          ///< The selection border color      <key>selectionBorder</key> <string>#222218</string>
    QColor activeGuideColor_;              ///< The active guide color          <key>activeGuide</key> <string>#9D550FB0</string>

    QColor bracketForegroundColor_;        ///< The bracket foreground color    <key>bracketsForeground</key> <string>#F8F8F2A5</string>
    QString bracketOptions_;               ///< The bracket options             <key>bracketsOptions</key> <string>underline</string>

    QColor bracketContentsForegroundColor_;///< The content foreground color    <key>bracketContentsForeground</key> <string>#F8F8F2A5</string>
    QString bracketContentsOptions_;       ///< The bracket options             <key>bracketContentsOptions</key> <string>underline</string>

    QString tagsOptions_;                  ///< The tags options                <key>tagsOptions</key> <string>stippled_underline</string>


    // The selectos
    QList<TextThemeRule*> themeRules_;     ///< the scope selector

};



//================================


/// This class is used to return the style formats for rendering the texts
class EDBEE_EXPORT TextThemeStyler : public QObject
{
Q_OBJECT

public:
    TextThemeStyler( TextEditorController* controller );
    virtual ~TextThemeStyler();

    QVector<QTextLayout::FormatRange> getLineFormatRanges( int lineIdx );

    TextEditorController* controller() { return controllerRef_; }

//    void giveTextTheme( TextTheme* theme );
    void setThemeByName( const QString& themeName );
    QString themeName() const;

    void setTheme( TextTheme* theme );
    TextTheme* theme() const;

private:
    QTextCharFormat getTextScopeFormat(QVector<ScopedTextRange *> &activeRanges);
    void appendFormatRange(QVector<QTextLayout::FormatRange>& rangeList, int start, int end,  QVector<edbee::ScopedTextRange *> &activeRanges );

private slots:

    void textDocumentChanged(edbee::TextDocument* oldDocument, edbee::TextDocument* newDocument);
    void invalidateLayouts();
    void themePointerChanged( const QString& name, TextTheme* oldTheme, TextTheme* newTheme );


private:

    TextEditorController* controllerRef_;                                     ///< A reference to the controller
    QString themeName_;                                                       ///< The name of the active theme (when a 'custom' theme active)
    TextTheme* themeRef_;                                                     ///< The active theme


};


//================================


/// This class is used to manage load 'themes'.
/// This method loads only loads a theme if requested.
/// It will list all available theme when
class EDBEE_EXPORT TextThemeManager : public QObject
{
Q_OBJECT

protected:
    TextThemeManager();
    virtual ~TextThemeManager();

public:
    void clear();

    TextTheme* readThemeFile( const QString& fileName, const QString& name=QString() );
    void listAllThemes( const QString& themePath=QString() );
    int themeCount() { return themeNames_.size(); }
    QString themeName( int idx );
    TextTheme* theme( const QString& name );
    TextTheme* fallbackTheme() const { return fallbackTheme_; }
    QString lastErrorMessage() const;
    void setTheme( const QString& name, TextTheme* theme );

signals:
    void themePointerChanged( const QString& name, TextTheme* oldTheme, TextTheme* newTheme );


private:

    QString  themePath_;                           ///< The theme path
    QStringList themeNames_;                       ///< All themes
    QHash<QString,TextTheme*> themeMap_;           ///< A map with all (loaded) themes
    TextTheme* fallbackTheme_;                     ///< The fallback theme (this can be used if no themes are found)
    QString lastErrorMessage_;                     ///< The last error message

    friend class Edbee;

};


} // edbee
