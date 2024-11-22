/**
 * Copyright 2011-2012 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

namespace edbee {


/// A special class to perform line-ending detection
class EDBEE_EXPORT LineEnding {
public:

    enum Type {
        UnixType = 0,
        WindowsType = 1,
        MacClassicType = 2,
        TypeCount
    };


protected:

    explicit LineEnding( LineEnding::Type ending, const char* chars, const char* escaped_chars, const char* name );
    virtual ~LineEnding();

public:
    static LineEnding* types();
    static int typeCount() { return TypeCount; }
    static LineEnding* get( int idx );

    static LineEnding* unixType();
    static LineEnding* windowsType();
    static LineEnding* macClassicType();


    static LineEnding* detect( const QString& str, LineEnding* unkownEnding=0 );


    /// This method returns the type
    virtual LineEnding::Type type() const { return type_; }

    /// This method returns the chars
    virtual const char* chars() const { return charsRef_; }

    /// This method returns the escaped chars
    virtual const char* escapedChars() const { return escapedCharsRef_; }

    /// This method returns the name of the line ending
    virtual const char* name() const { return nameRef_; }


 //   virtual LineEnding detectLineEnding
//    virtual static QString lineEndingChars( LineEnding::Type endingType  );

private:

    LineEnding::Type type_;         ///< The type of the line ending
    const char* charsRef_;          ///< The characters reference
    const char* escapedCharsRef_;   ///< The textual display of the chars
    const char* nameRef_;           ///< The name of these line ending
};



} // edbee
