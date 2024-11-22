/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include "textchangewithcaret.h"

#include "edbee/debug.h"

namespace edbee {

TextChangeWithCaret::TextChangeWithCaret(int offset, int length, const QString& text, int caret )
    : TextChange( offset, length, text )
    , caret_( caret )
{
}


/// returns the caret position
int TextChangeWithCaret::caret() const
{
    return caret_;
}


/// Sets the caret position
/// @param caret the caret to set
void TextChangeWithCaret::setCaret(int caret)
{
    caret_ = caret;
}


} // edbee
