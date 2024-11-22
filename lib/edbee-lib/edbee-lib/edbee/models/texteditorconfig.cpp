/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "texteditorconfig.h"

#include "edbee/debug.h"

namespace edbee {


/// The default constructor
/// Fills this configuration object with some sane defaults
/// @param parent the parent references of this Qobject
TextEditorConfig::TextEditorConfig( QObject* parent )
    : QObject(parent)
    , changeInProgressLevel_(0)
    , changeCount_(0)
    , caretWidth_(2)
    , caretBlinkingRate_(700)   // 0 means no blinking (default = 700)
    , indentSize_(4)
    , useTabChar_(true)
    , smartTab_(true)
    , charGroups_()
    , whitespaces_("\n\t ")
    , whitespaceWithoutNewline_("\t ")
    , extraLineSpacing_(0)
    , useLineSeparator_(false)
    , lineSeparatorPen_( QColor(230,230,230), 1.5, Qt::DashLine )
    , undoGroupPerSpace_(true)
    , showCaretOffset_(true)
    , themeName_("Monokai")
    , scrollPastEnd_(false)
    , showWhitespaceMode_(HideWhitespaces)
    , renderBidiContolCharacters_(true)
    , autocompleteAutoShow_(true)
    , autocompleteMinimalCharacters_(0)
{
    charGroups_.append( QStringLiteral("./\\()\"'-:,.;<>~!@#$%^&*|+=[]{}`~?"));
}


/// empty constructor :)
TextEditorConfig::~TextEditorConfig()
{
}


/// use this method to start a group of changes.
/// Default behaviour is to emit a configChanged signal if a setting is changed
/// When you call beginChanges this method will make sure the config surpresses the signals
/// and only a signal is fired after the last endChange.
/// You can nest multiple beginChanges, only the last endChanges fires a signal
void TextEditorConfig::beginChanges()
{
    ++changeInProgressLevel_;
}


/// Ends the previous call of beginChanges. It will notify a change
/// when all items have been changes
void TextEditorConfig::endChanges()
{
    Q_ASSERT(changeInProgressLevel_ > 0 );
    --changeInProgressLevel_;
    if( changeInProgressLevel_ == 0 && changeCount_ > 0 ) {
        notifyChange();
    }
}


/// Returns the caret width in pixels
int TextEditorConfig::caretWidth() const
{
    return caretWidth_;
}


/// Sets the caret render width in pixels
/// @param width the caret width in pixels
void TextEditorConfig::setCaretWidth(int width)
{
    if( caretWidth_ != width ) {
        caretWidth_ = width;
        notifyChange();
    }
}


/// Returns the caret blinking rate (delay) (lower means faster )
/// The blinking delay is in milliseconds
int TextEditorConfig::caretBlinkingRate() const
{
    return  caretBlinkingRate_;
}


/// Sets the caret blinking rate (delay) (lower means faster )
/// @param rate The blinking delay is in milliseconds
void TextEditorConfig::setCaretBlinkRate(int rate)
{
    if( caretBlinkingRate_ != rate ) {
        caretBlinkingRate_ = rate;
        notifyChange();
    }
}


/// Returns the indent size in the number of characters
int TextEditorConfig::indentSize() const
{
    return indentSize_;
}


/// Sets the indentsize in characters
/// @param size the size of indentation in nr of characters
void TextEditorConfig::setIndentSize(int size)
{
    if( indentSize_ != size ) {
        indentSize_ = size;
        notifyChange();
    }
}


/// Should the tab-character be used?
bool TextEditorConfig::useTabChar() const
{
    return useTabChar_;
}


/// Should the tab-character be used?
/// @param enable enable or disable the use of tab characters
void TextEditorConfig::setUseTabChar(bool enable)
{
    if( useTabChar_ != enable ) {
        useTabChar_ = enable;
        notifyChange();
    }
}


/// Returns the state of smarttab mode
/// Smarttab enables the automatic addition of indents when inserting a newline.
bool TextEditorConfig::smartTab() const
{
    return smartTab_;
}


/// Sets the smarttab mode
/// @param enable should smarttab be enabled
void TextEditorConfig::setSmartTab(bool enable)
{
    if( smartTab_ != enable ) {
        smartTab_ = enable;
        notifyChange();
    }
}


/// Returns the characters groups
/// Character groups are groups that considered the 'same' kind of characters.
/// For example skipping to the next word skips the characters of the same group
/// Currently spaces and word/id characters are hardcoded groups.
const QStringList& TextEditorConfig::charGroups() const
{
    return charGroups_;
}


/// Sets the character groups
/// @see TextEditorConfig::charGroups
void TextEditorConfig::setCharGroups(const QStringList &items)
{
    if( charGroups_ != items ) {
        charGroups_ = items;
        notifyChange();
    }
}


/// Returns all white-space characters
const QString& TextEditorConfig::whitespaces() const
{
    return whitespaces_;
}


/// Sets the whitespace characters
void TextEditorConfig::setWhitespaces(const QString& value)
{
    if( whitespaces_ != value ) {
        whitespaces_ = value;
        notifyChange();
    }
}


/// Retuns the whitespace character without newlines
const QString& TextEditorConfig::whitespaceWithoutNewline() const
{
    return whitespaceWithoutNewline_;
}


/// Sets the characters that are considered whitespace. Excluding the newline character
/// @param value the list of characters that are considered whitespace characters
void TextEditorConfig::setWhitespaceWithoutNewline(const QString& value)
{
    if( whitespaceWithoutNewline_ != value ) {
        whitespaceWithoutNewline_ = value;
        notifyChange();
    }
}


/// Returns the extra line spacing in pixels
int TextEditorConfig::extraLineSpacing() const
{
    return extraLineSpacing_;
}


/// Sets the extra line spacing between lines in pixels
/// @param value the number of extra line-spacing between editor lines
void TextEditorConfig::setExtraLineSpacing(int value)
{
    if( extraLineSpacing_ != value ) {
        extraLineSpacing_ = value;
        notifyChange();
    }
}


/// Should we use a line seperator pen
/// @see TextEditorConfig::lineSeparatorPen
bool TextEditorConfig::useLineSeparator() const
{
    return useLineSeparator_;
}


/// Should we use a sline seperator?
/// @see TextEditorConfig::lineSeparatorPen
void TextEditorConfig::setUseLineSeparator(bool value)
{
    if( useLineSeparator_ != value ) {
        useLineSeparator_ = value;
        notifyChange();
    }
}


/// The line sperator pen to use.When the setting useLineSeparator is set
/// this pen is used to draw lines between the text-lines
/// @see TextEditorConfig::useLineSeparator
const QPen& TextEditorConfig::lineSeparatorPen() const
{
    return lineSeparatorPen_;
}


/// This method sets the line seperator pen that used to paint
/// line between the text-lines
/// @see TextEditorConfig::useLineSeparator
void TextEditorConfig::setLineSeperatorPen(const QPen& pen)
{
    if( lineSeparatorPen_ != pen ) {
        lineSeparatorPen_ = pen;
        notifyChange();
    }
}


/// Should a space result in a new 'undo-group'
/// The default behaviour is that pressing a space character results in the
/// closing of an undo group. (This groups the entered characters into 1 undo operation)
bool TextEditorConfig::undoGroupPerSpace() const
{
    return undoGroupPerSpace_;
}


/// Sets the undo group per space value
/// @see TextEditorConfig::undoGroupPerSpace
void TextEditorConfig::setUndoGroupPerSpace(bool enable)
{
    if( undoGroupPerSpace_ != enable ) {
        undoGroupPerSpace_ = enable;
        notifyChange();
    }
}


/// should the caret-offset be shown. The texteditor can signal a
/// statusbar text to a slot. This text can optionally contain the
/// current caret offset
bool TextEditorConfig::showCaretOffset() const
{
    return showCaretOffset_;
}


/// Enables / disables the passing of the character offset
/// @see TextEditorConfig::showCaretOffset
void TextEditorConfig::setShowCaretOffset(bool enable)
{
    if( showCaretOffset_ != enable ) {
        showCaretOffset_ = enable;
        notifyChange();
    }
}


/// returns the active theme name
/// (The theme name is the name without the file-extension of the file in the theme directory)
QString TextEditorConfig::themeName()
{
    return themeName_;
}


/// This method sets the active theme name
/// @param name the name of the active theme (this is the filename of the theme without the extension)
void TextEditorConfig::setThemeName(const QString& name)
{
    if( themeName_ != name ) {
        themeName_ = name;
        notifyChange();
    }
}


/// returns the current font to use
QFont TextEditorConfig::font() const
{
    return font_;
}


/// Changes the font that's used by the editor
void TextEditorConfig::setFont(const QFont& font)
{
    if( font_ != font ) {
        font_ = font;
        notifyChange();
    }
}


/// returns the scroll past end setting
/// @return
bool TextEditorConfig::scrollPastEnd() const
{
    return scrollPastEnd_;
}


/// Sets the scroll past end option.
/// When enabled the last line of the document is scrollable to the top of the window
/// @param enabled should scroll past end be enabled ?
void TextEditorConfig::setScrollPastEnd(bool enabled)
{
    if( scrollPastEnd_ != enabled ) {
        scrollPastEnd_ = enabled;
        notifyChange();
    }
}


/// Shows the whitespacemode
int TextEditorConfig::showWhitespaceMode() const
{
    return showWhitespaceMode_;
}


/// Sets the show the whitespace mode
/// @param mode the whitespace mode to set. Can be one of the enumeration values. (Or another integer if you build your own renderer)
void TextEditorConfig::setShowWhitespaceMode(int mode)
{
    if( showWhitespaceMode_ != mode ) {
        showWhitespaceMode_ = mode;
        notifyChange();
    }
}



/// Sets the show whitespace mode
/// @param str the whitespace mode "show" or "hide" for now. (When invalid hide is used)
void TextEditorConfig::setShowWhitespaceMode(const QString& str)
{
    if( str == "show" ) {
        if( showWhitespaceMode_ != ShowWhitespaces ) {
            showWhitespaceMode_ = ShowWhitespaces;
            notifyChange();
        }
    } else {
        if( showWhitespaceMode_ != HideWhitespaces ) {
            showWhitespaceMode_ = HideWhitespaces;
            notifyChange();
        }
    }
}

bool TextEditorConfig::renderBidiContolCharacters() const
{
    return renderBidiContolCharacters_;
}

/// Sets the render bidirectional control-character mode.
/// For showing possible security related characters:  https://www.trojansource.codes
/// Default is Enabled!
void TextEditorConfig::setRenderBidiContolCharacters(bool enabled)
{
    renderBidiContolCharacters_ = enabled;
}

/// Sets whether autocomplete comes up automatically, or only manually(manual trigger isn't implemented yet)
/// @see TextEditorConfig::autocompleteAutoShow
void TextEditorConfig::setAutocompleteAutoShow(bool enable)
{
    if( autocompleteAutoShow_ != enable ) {
        autocompleteAutoShow_ = enable;
        notifyChange();
    }
}

int TextEditorConfig::autocompleteMinimalCharacters() const
{
    return autocompleteMinimalCharacters_;
}

void TextEditorConfig::setAutocompleteMinimalCharacters(int amount)
{
    autocompleteMinimalCharacters_ = amount;
}

/// Show autocomplete automatically, or only manually(manual isn't implemented yet)
bool TextEditorConfig::autocompleteAutoShow() const
{
    return autocompleteAutoShow_;
}

/// This internal method is used to notify the listener that a change has happend
/// Thi smethod only emits a signal if there's no config group change busy
void TextEditorConfig::notifyChange()
{
    ++changeCount_;
    if( changeInProgressLevel_ == 0 ) {
        emit configChanged();
        changeCount_= 0;
    }
}


} // edbee
