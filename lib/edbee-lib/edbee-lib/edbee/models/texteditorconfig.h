/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QFont>
#include <QObject>
#include <QPen>
#include <QString>
#include <QStringList>

namespace edbee {

/// General configuration settings of the text editor
class EDBEE_EXPORT TextEditorConfig : public QObject
{
Q_OBJECT

public:

    /// an enumeration with the possible types of whitespace showing
    enum ShowWhitespaceMode {
        HideWhitespaces = 0,               ///< Don't show whitespaces
        ShowWhitespaces = 1                ///< Alsways show the whitespace
        /// TODO: In the future, only show when selected
    };



    TextEditorConfig( QObject* parent=0 );
    virtual ~TextEditorConfig();

    void beginChanges();
    void endChanges();

    int caretWidth() const;
    void setCaretWidth( int width );

    int caretBlinkingRate() const;
    void setCaretBlinkRate( int rate );

    int indentSize() const;
    void setIndentSize( int size );

    bool useTabChar() const;
    void setUseTabChar( bool enable );

    bool smartTab() const;
    void setSmartTab( bool enable );

    const QStringList& charGroups() const;
    void setCharGroups( const QStringList& items );

    const QString& whitespaces() const;
    void setWhitespaces( const QString& value );

    const QString& whitespaceWithoutNewline() const;
    void setWhitespaceWithoutNewline( const QString& );

    int extraLineSpacing() const;
    void setExtraLineSpacing( int value );

    bool useLineSeparator() const;
    void setUseLineSeparator( bool value );

    const QPen& lineSeparatorPen() const;
    void setLineSeperatorPen( const QPen& pen );

    bool undoGroupPerSpace() const;
    void setUndoGroupPerSpace( bool enable );

    bool showCaretOffset() const;
    void setShowCaretOffset( bool enable );

    QString themeName();
    void setThemeName( const QString& name );

    QFont font() const ;
    void setFont( const QFont& font );

    bool scrollPastEnd() const;
    void setScrollPastEnd( bool enabled );

    int showWhitespaceMode() const;
    void setShowWhitespaceMode( int mode );
    void setShowWhitespaceMode( const QString& str );

    bool renderBidiContolCharacters() const;
    void setRenderBidiContolCharacters( bool enabled );


    bool autocompleteAutoShow() const;
    void setAutocompleteAutoShow( bool enable );

    int autocompleteMinimalCharacters() const;
    void setAutocompleteMinimalCharacters( int amount );


signals:
    void configChanged();

protected:
    void notifyChange();

private:

    int changeInProgressLevel_;         ///< The change in progress level
    int changeCount_;                   ///< A flag that counts the number of changes

    int caretWidth_;                    ///< The caretWith in pixels
    int caretBlinkingRate_;             ///< The caret blinking rate
    int indentSize_;                    ///< The ident-size in characters
    bool useTabChar_;                   ///< Should the tab character be used?
    bool smartTab_;                     ///< Is smarttab enabled?

    QStringList charGroups_;            ///< Character groups
    QString whitespaces_;               ///< All whitespaces
    QString whitespaceWithoutNewline_;  ///< Whitespace characters without newline
    int extraLineSpacing_;              ///< The extra space to place between lines
    bool useLineSeparator_;             ///< Should we draw a line-seperator between lines?
    QPen lineSeparatorPen_;             ///< A line seperator pen
    bool undoGroupPerSpace_;            ///< An undogroup per space?
    bool showCaretOffset_;              ///< Show the caret offset?
    QString themeName_;                 ///< The active theme name
    QFont font_;                        ///< The font to used

    bool scrollPastEnd_;                ///< Should the last line of the document be  scrollable to the top of the window
    int showWhitespaceMode_;            ///< The current whitespace mode to make
    bool renderBidiContolCharacters_;   ///< Renders dangers control characters as red marks

    bool autocompleteAutoShow_;         ///< Show autocomplete automatically, or only when manually triggered
    int autocompleteMinimalCharacters_; ///< How manu characters need to be entered before autocomplete kicks in    
};

} // edbee
