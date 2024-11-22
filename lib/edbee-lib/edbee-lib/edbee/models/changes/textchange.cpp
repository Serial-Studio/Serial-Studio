/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textchange.h"

#include "edbee/models/textbuffer.h"
#include "edbee/models/textdocument.h"

#include "edbee/debug.h"

namespace edbee {


/// Constructs a single textchange
/// @param offset, the offset of the change
/// @param length, the length of the change
/// @param text , the new text
/// @param executed, a boolean (mainly used for testing) to mark this change as exected
TextChange::TextChange(int offset, int length, const QString& text)
    : offset_(offset)
    , length_(length)
    , text_(text)
{
}


/// undo's a single textchange
TextChange::~TextChange()
{
}


/// executes the given textchange
/// @param document the document to execute the change on
void TextChange::execute(TextDocument* document)
{
    replaceText(document);
}


/// Reverts the single textchange
/// @param document the document to execute the change on
void TextChange::revert(TextDocument* document)
{
    replaceText(document);
}


/// This method merges the old data with the new data
/// @apram change the data to merge with
void TextChange::mergeStoredData(AbstractRangedChange* change)
{
    TextChange* singleTextChange = dynamic_cast<TextChange*>(change);

    QString newText;
    newText.resize( getMergedStoredLength( change) );
    mergeStoredDataViaMemcopy( newText.data(), text_.data(), singleTextChange->text_.data(), change, sizeof(QChar) );
    /*
      QString newText;
      // we first need to 'take' the leading part of the new change
      if( change->offset() < offset() ) {
          newText.append( change->oldText(document).mid(0, offset() - change->offset() ) );
      }

      newText.append(oldText(document));

      // then we need to append the remainer
      int delta = offset()-change->offset();
      int remainerOffset = newLength() + delta;
      if( remainerOffset >= 0 ) {
          if( remainerOffset < change->oldLength() ) {
              //Q_ASSERT(false);    // need to figure out if this works
              newText.append( change->oldText( document  ).mid(remainerOffset ) );
          }
      }

    */
    text_ = newText;
}


/// This method gives the given change to this textchange. The changes will be merged
/// if possible. This method currently only works with executed changes!!!
///
/// @param document the document
/// @param textChange the textchange to mege
/// @return true on success else false
bool TextChange::giveAndMerge( TextDocument* document, Change* textChange)
{
    Q_UNUSED( document );
    TextChange* change = dynamic_cast<TextChange*>( textChange );
    if( change ) {
        return merge( change );
    }
    return false;
}


/// converts the change to a string
QString TextChange::toString()
{
//    return "SingleTextChange";
    return QStringLiteral("SingleTextChange:%1").arg(testString());
}


/// Return the offset
/// @return the offset of the change
int TextChange::offset() const
{
    return offset_;
}


/// set the new offset
/// @param offset the new offset
void TextChange::setOffset(int offset)
{
    offset_ = offset;
}


/// This is the length in the document
int TextChange::docLength() const
{
    return length_;
}


/// The content length is the length that's currently stored in memory.
int TextChange::storedLength() const
{
    return text_.size();
}


/// Set the length of the change
/// @param len sets the length of the change
void TextChange::setDocLength(int len)
{
    length_ = len;
}

/// The text currently stored in this textchange
QString TextChange::storedText() const
{
    return text_;
}


/// Sets the text of this change
/// @param text the new text
void TextChange::setStoredText(const QString& text)
{
    text_ = text;
}


/// Appends the text to this change
void TextChange::appendStoredText(const QString& text)
{
    text_.append( text );
}


/// This method returns the text currently in the document
const QString TextChange::docText(TextDocument* doc) const
{
    return doc->textPart( offset_, length_ );
}


/// This method returns a string used for testing
QString TextChange::testString()
{
    return QStringLiteral("%1:%2:%3").arg(offset_).arg(length_).arg(QString(text_).replace("\n","§"));
}


/// replaces the text and stores the 'old' content
/// @param document the document to change it for
void TextChange::replaceText(TextDocument* document)
{
    TextBuffer* buffer = document->buffer();
    QString old = buffer->textPart( offset_, length_ );

    buffer->replaceText( offset_, length_, text_ );
    length_ = text_.length();
    text_ = old;
}




} // edbee
