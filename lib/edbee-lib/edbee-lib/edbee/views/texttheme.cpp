/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "texttheme.h"

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QPalette>
#include <QStack>
#include <QVector>

#include "edbee/io/tmthemeparser.h"
#include "edbee/models/textbuffer.h"
#include "edbee/models/textdocument.h"
#include "edbee/models/textdocumentscopes.h"
#include "edbee/texteditorcontroller.h"
#include "edbee/edbee.h"

#include "edbee/debug.h"

namespace edbee {


TextThemeRule::TextThemeRule(const QString& name, const QString& selector, QColor foreground, QColor background, bool bold, bool italic, bool underline)
    : name_( name )
    , scopeSelector_(0)
    , foregroundColor_( foreground )
    , backgroundColor_( background )
    , bold_(bold)
    , italic_(italic)
    , underline_(underline)
{
    scopeSelector_ = new TextScopeSelector(selector);
}

TextThemeRule::~TextThemeRule()
{
    delete scopeSelector_;
}

/// This method checks if the given scopelist matches the scope selector
bool TextThemeRule::matchesScopeList(const TextScopeList* scopes)
{
    return ( scopeSelector_->calculateMatchScore( scopes ) >= 0 ) ;
}

void TextThemeRule::fillFormat(QTextCharFormat* format)
{
    if( foregroundColor_.isValid() ) { format->setForeground(foregroundColor_ ); }
    if( backgroundColor_.isValid() ) { format->setBackground(backgroundColor_ ); }
    if( bold_ ) { format->setFontWeight( QFont::Bold ); }  //QFont::Black
    if( italic_) { format->setFontItalic(true); }
    if( underline_ ) { format->setFontUnderline(true); }
}

//=================================================


TextTheme::TextTheme()
    : name_("Default Theme")
    , uuid_("")
    , backgroundColor_( 0xffeeeeee )
    , caretColor_( 0xff000000  )
    , foregroundColor_( 0xff222222 )
    , lineHighlightColor_(0xff999999 )
    , selectionColor_( 0xff9999ff)

    // thTheme settings
//    , backgroundColor_(0xff272822)
//    , caretColor_(0xffF8F8F0)
//    , foregroundColor_(0xffF8F8F2)
//    , invisiblesColor_(0xff3B3A32)
//    , lineHighlightColor_(0xff3E3D32)
//    , selectionColor_(0xff49483E)
//    , findHighlightBackgroundColor_(0xffFFE792)
//    , findHighlightForegroundColor_(0xff000000)
//    , selectionBorderColor_(0xff222218)
//    , activeGuideColor_(0x9D550FB0)
//    , bracketForegroundColor_(0xF8F8F2A5)
//    , bracketOptions_("underline")
//    , bracketContentsForegroundColor_(0xF8F8F2A5)
//    , bracketContentsOptions_("underline")
//    , tagsOptions_("stippled_underline")

{
    QPalette pal = QApplication::palette();
    backgroundColor_ = pal.color( QPalette::Window);
    foregroundColor_ = pal.color( QPalette::WindowText );
    selectionColor_ = pal.color( QPalette::Highlight );
//    giveThemeRule( new TextThemeRule("Comment","comment", QColor("#75715E") ));
//    giveThemeRule( new TextThemeRule("String","string", QColor("#E6DB74") ));
}

TextTheme::~TextTheme()
{
    qDeleteAll(themeRules_);
}

/// The text theme
void TextTheme::giveThemeRule(TextThemeRule* rule)
{
    themeRules_.append(rule);
}

void TextTheme::fillFormatForTextScopeList( const TextScopeList* scopeList, QTextCharFormat* format)
{
//    format->setForeground( foregroundColor() );
//    format->setBackground( backgroundColor() );

    foreach( TextThemeRule* rule, themeRules_ ) {
        if( rule->matchesScopeList( scopeList ) ) {
            rule->fillFormat(format);
        }
    }

}


//=================================================


/// Constructs the theme styler
/// @param controller the controller to use this styler for
TextThemeStyler::TextThemeStyler( TextEditorController* controller )
    : controllerRef_( controller )
    , themeName_()
    , themeRef_(0)
{
    connect( controller, SIGNAL(textDocumentChanged(edbee::TextDocument*,edbee::TextDocument*)), SLOT(textDocumentChanged(edbee::TextDocument*,edbee::TextDocument*)) );
    connect( Edbee::instance()->themeManager(), SIGNAL(themePointerChanged(QString,TextTheme*,TextTheme*)), SLOT(themePointerChanged(QString,TextTheme*,TextTheme*)) );
    themeRef_ = Edbee::instance()->themeManager()->fallbackTheme();    
}


/// The current destructor
TextThemeStyler::~TextThemeStyler()
{
}


/// This method returns a reference to the given line format.
/// WARNING this reference is ONLY valid very shortly.. Another call to this
/// method can invalidates the previous result!!!
///
/// @param lineIdx the line index
/// @return the array of ranges
QVector<QTextLayout::FormatRange> TextThemeStyler::getLineFormatRanges( int lineIdx )
{
    TextDocumentScopes* scopes = controller()->textDocument()->scopes();

    // check if the range is in the case. When it is, use it
    QVector<QTextLayout::FormatRange> formatRangeList;

    // get all textranges on the given line
    ScopedTextRangeList* scopedRanges = scopes->scopedRangesAtLine(lineIdx);
    if( scopedRanges == 0 || scopedRanges->size() == 0 ) { return formatRangeList; }


    // build format ranges from these (nested) scope ranges
    //
    //  [ source.c                             ]
    //     [ string.c          ]   [keyword]
    //         [ escape. ]
    // =
    //  [ ][xx][#########][xxxx][ ][kkkkkkk][  ]
    //
    QStack<ScopedTextRange*> activeRanges;
    activeRanges.append( scopedRanges->at(0) );

    int lastOffset = 0; //lineStartOffset;
    for( int i=1, cnt=scopedRanges->size(); i<cnt; ++i ) {
        ScopedTextRange* range = scopedRanges->at(i);
        int min = range->min();  // find the minimum position

        // unwind the stack if required
        while( activeRanges.size() > 1 ) {
            ScopedTextRange* activeRange = activeRanges.last();
            int activeRangeMax = activeRange->max();

            // when the 'min' is behind the end of the textrange on the stack we need to pop the stack
            if( activeRangeMax <= min ) {
                appendFormatRange( formatRangeList, lastOffset, activeRangeMax-1, activeRanges );
                activeRanges.pop();
                lastOffset = activeRangeMax;
                Q_ASSERT( !activeRanges.empty() );
            } else {
                break;
            }
        }

        // add a new 'range' if a new one is started and there's a 'gap'
        if( lastOffset < min ) {
            appendFormatRange( formatRangeList, lastOffset, min-1, activeRanges );
            lastOffset = min;
        }

        // push the new range to the stack
        activeRanges.push_back( range );

    }

    // next we must unwind the stack
    while( !activeRanges.isEmpty() ) {
        ScopedTextRange* activeRange = activeRanges.last();
        int activeRangeMax = activeRange->max();
        if( lastOffset < activeRange->max() ) {
            appendFormatRange(formatRangeList, lastOffset, activeRangeMax-1, activeRanges );
            lastOffset = activeRange->max();
        }
        activeRanges.pop();
    }

    return formatRangeList;
}



/// Sets the active theme name
/// @param themeName
void TextThemeStyler::setThemeByName(const QString& themeName)
{
    // set the theme with the given name
    themeName_ = themeName;
    themeRef_= Edbee::instance()->themeManager()->theme(themeName_);

    // when no theme is found, fallback to the fallback theme
    if( !themeRef_) {
        qlog_warn() << "Theme not set:" << themeName_;
        themeName_ = "";
        themeRef_= Edbee::instance()->themeManager()->fallbackTheme();
    }
}


/// returns the current active theme name
/// This method returns "" for the fallback theme
/// It returns a NullString if the theme is specified manually (without the thememanager)
QString TextThemeStyler::themeName() const
{
    return themeName_;
}


/// Sets the active theme
/// When supplying 0 this theme applies the fallback theme
void TextThemeStyler::setTheme(TextTheme* theme)
{
    themeName_ = QString();
    themeRef_ = theme;
    if( !themeRef_) {
        themeName_ = "";
        themeRef_= Edbee::instance()->themeManager()->fallbackTheme();
    }
}


/// returns the active theme
TextTheme* TextThemeStyler::theme() const
{
    return themeRef_;
}



/// This method returns the character format for the given text scope
QTextCharFormat TextThemeStyler::getTextScopeFormat( QVector<ScopedTextRange*>& activeRanges )
{
//    ScopedTextRange* range = activeRanges.last();
    QTextCharFormat format;

    TextScopeList scopeList(activeRanges);
    theme()->fillFormatForTextScopeList( &scopeList, &format );
    return format;
}


/// helper function to create a format range
void TextThemeStyler::appendFormatRange(QVector<QTextLayout::FormatRange> &rangeList, int start, int end,  QVector<ScopedTextRange*>& activeRanges )
{
    // only append a format if the lexer style is different then default
    if( activeRanges.size() > 1  ) {
        QTextLayout::FormatRange formatRange;
        formatRange.start  = start;
        formatRange.length = end - start + 1;
        formatRange.format = getTextScopeFormat( activeRanges );
        rangeList.append( formatRange );
    }
}

void TextThemeStyler::textDocumentChanged(edbee::TextDocument *oldDocument, edbee::TextDocument *newDocument)
{
    Q_UNUSED(newDocument);
    Q_UNUSED(oldDocument);
}

/// invalidates all layouts
void TextThemeStyler::invalidateLayouts()
{
    //    formateRangeListCache_.clear();
}

void TextThemeStyler::themePointerChanged(const QString& name, TextTheme* oldTheme, TextTheme *newTheme)
{
    if( name == themeName_ ) {
        themeRef_ = newTheme;
    } else {
        if( oldTheme == themeRef_ ) {
            Q_ASSERT(false && "The old theme is deleted but it's not the same theme name. This shouldn't happen");
            // If it happens a solution is to set the fallback theme
        }
    }
}


//=================================================


/// Constructs the theme manager
/// And intialises a fallback theme
TextThemeManager::TextThemeManager()
    : fallbackTheme_(0)
{
    fallbackTheme_ = new TextTheme();
    fallbackTheme_->setForegroundColor(0xFF000000);
    fallbackTheme_->setBackgroundColor(0xFFFFFFFF);
}


/// destructs the theme maanger
TextThemeManager::~TextThemeManager()
{
    clear();
    delete fallbackTheme_;
}


/// clears all items
void TextThemeManager::clear()
{
    qDeleteAll(themeMap_);
    themeMap_.clear();
    themeNames_.clear();
}


/// This method loads all theme names from the given theme path (*.tmTheme files)
/// @param the new themePath. if the themepath is blank, the themes of the current path are reloaded
void TextThemeManager::listAllThemes(const QString& themePath)
{
    if( !themePath.isEmpty() ) { themePath_ = themePath; }
    if( !themePath_.isEmpty() ) {
        clear();
        QDir dir(themePath_);
        QFileInfoList fileInfoList = dir.entryInfoList(QStringList("*.tmTheme"),QDir::Files, QDir::Name);
        foreach( QFileInfo fi, fileInfoList ) {
            themeNames_.append( fi.baseName() );
        }
    }
}


/// Returns the theme name at the given index
/// @param idx the index of the theme to retrieve
QString TextThemeManager::themeName(int idx)
{
    return themeNames_.at(idx);
}


/// This method loads the given theme file.
/// The theme manager stays owner of the given theme
/// @param filename the filename of the theme to load
/// @param name the name of the theme (if the name isn't given the basenaem of the fileName is used (excluding the .tmTheme extension)
/// @return the loaded theme or 0 if the theme couldn' be loaded
TextTheme* TextThemeManager::readThemeFile( const QString& fileName, const QString& nameIn )
{
    lastErrorMessage_.clear();

    // check if the file exists
    QFile file(fileName);
    if( file.exists() && file.open(QIODevice::ReadOnly) ) {

        // parse the theme
        TmThemeParser parser;
        TextTheme* theme = parser.readContent(&file);
        if( theme ) {

            // when the name if blank extract it from the filename
            QString name = nameIn;
            if( name.isEmpty() ) {
                name = QFileInfo(fileName).completeBaseName();
            }

            setTheme(name,theme);
        } else {
            lastErrorMessage_ = QObject::tr("Error parsing theme %1:%2").arg(file.fileName()).arg( parser.lastErrorMessage());
        }
        file.close();
        return theme;
    } else {
        lastErrorMessage_ = QObject::tr("Error theme not found %1.").arg(file.fileName());
        return 0;
    }

}


/// This method gets or loads the theme.
/// (The auto loading feature only works if the themepath is supplied)
///
/// @param the name of the theme to load
/// @return the theme with the given name
TextTheme* TextThemeManager::theme(const QString& name)
{
    if( name.isEmpty() ) { return 0; }
    TextTheme* theme=themeMap_.value(name);
    if( !theme && !themePath_.isEmpty()) {
        QString filename = QStringLiteral("%1/%2.tmTheme").arg(themePath_).arg(name);
        theme = readThemeFile( filename );
        if( !theme ) {
            qlog_warn() << this->lastErrorMessage();

            // add the theme to the map
            theme = new TextTheme();
            themeMap_.insert(name, theme);
        }
    }
    return theme;
}


/// this method returns the last error message
QString TextThemeManager::lastErrorMessage() const
{
    return lastErrorMessage_;
}

void TextThemeManager::setTheme(const QString &name, TextTheme *theme)
{
    TextTheme* oldTheme = themeMap_.value(name);
    themeMap_.insert(name,theme);
    emit themePointerChanged(name, oldTheme, theme);
    delete oldTheme;
}

} // edbee
