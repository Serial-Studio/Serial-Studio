/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#include <QString>
#include "lineending.h"

#include "edbee/debug.h"

namespace edbee {

/// This method returns the line endings
/// @param type the type to retrieve
LineEnding *LineEnding::types()
{
    static LineEnding types[] = {
        LineEnding( LineEnding::UnixType, "\n", "\\n", "Unix" ),
        LineEnding( LineEnding::WindowsType, "\r\n", "\\r\\n", "Windows" ),
        LineEnding( LineEnding::MacClassicType, "\r" , "\\r", "Mac Classic"),
    };
    return types;
}


/// This methode returns the line ending from the given type
LineEnding *LineEnding::get(int idx )
{
    Q_ASSERT( 0 <= idx && idx < TypeCount );
    return types() + idx;
}


/// Returns the unix line ending type
LineEnding*LineEnding::unixType()
{
    return get( UnixType );
}


// returns the windows line ending type
LineEnding*LineEnding::windowsType()
{
    return get( WindowsType );

}


/// Returns the Mac class line ending type
LineEnding*LineEnding::macClassicType()
{
    return get( MacClassicType );
}


/// Constructs a line ending type
/// @param type the type enum constant
/// @param chars the chars used for the line ending
/// @param escaped an escaped variant of the characters
/// @param name the name of the line ending
LineEnding::LineEnding(LineEnding::Type type, const char* chars, const char* escaped, const char* name )
    : type_( type )
    , charsRef_( chars )
    , escapedCharsRef_( escaped )
    , nameRef_( name )
{
}


LineEnding::~LineEnding()
{
}


/// This method detects the, line endings of a QString.
/// A byte array is tricky because the encoding could be multi-byte
///
/// @param str the string to find the line-endings.
/// @param unknown the type to return when the ending is not nown. (defaults to 0)
/// @return the found type or the unkown parameter if not found
///
LineEnding *LineEnding::detect(const QString& str, LineEnding *unkown )
{
    const int endLoopWhenCountReaches = 3;
    int macClassicCount = 0;
    int unixCount = 0;
    int winCount = 0;

    for( int i=0, cnt=str.length(); i<cnt; i++ ) {

        // retrieve the characters
        QChar c1 = str.at(i);
        QChar c2 = (i+1<cnt) ? str.at(i+1) : QChar(0);

        // detect the line-ending
        if( c1 == '\r' ) {
           if( c2 == '\n' ) {
               ++winCount;
               ++i;
               if( winCount >= endLoopWhenCountReaches ) break;
           } else  {
               ++macClassicCount;
               if( macClassicCount >= endLoopWhenCountReaches ) break;
           }
        }
        else if( c1 == '\n') {
            ++unixCount;
            if( unixCount >= endLoopWhenCountReaches ) break;
        }
    }
    if( macClassicCount > unixCount && macClassicCount > winCount ) return get( LineEnding::MacClassicType );
    if( winCount > unixCount ) return get( LineEnding::WindowsType );
    if( unixCount > 0) return get( LineEnding::UnixType );
    return unkown;
}


} // edbee
