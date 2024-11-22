/**
 * Copyright 2011-2013 - Reliable Bits Software by Blommers IT. All Rights Reserved.
 * Author Rick Blommers
 */

#pragma once

#include "edbee/exports.h"

#include <QString>

namespace edbee {

/// The minimal engine we currently require for handling regexpt.
/// It may grow in the future
class EDBEE_EXPORT RegExpEngine {
public:
    virtual ~RegExpEngine() {}
    virtual QString pattern() = 0;
    virtual bool isValid() = 0;
    virtual QString error() = 0;
    virtual int indexIn( const QString& str, int offset ) = 0;
    virtual int indexIn( const QChar* str, int offset, int length ) = 0;
    virtual int lastIndexIn( const QString& str, int offset ) = 0;
    virtual int lastIndexIn( const QChar* str, int offset, int length ) = 0;

    virtual int pos( int nth = 0 ) const = 0;
    virtual int len( int nth = 0 ) const = 0;
    virtual QString cap( int nth = 0 ) const = 0;
};



/// A class for matching QStrings with the Oniguruma API
/// We need this Regular Expression library to be able to support tmLanguages fully
/// I tried to make this class as close as possible to the QRegExp library
class EDBEE_EXPORT RegExp {
public:
    enum Engine {
        EngineOniguruma = 1,
        EngineQRegExp = 2
//        QRegExp::RegExp	0	A rich Perl-like pattern matching syntax. This is the default.
//        QRegExp::RegExp2	3	Like RegExp, but with greedy quantifiers. (Introduced in Qt 4.2.)
//        QRegExp::Wildcard	1	This provides a simple pattern matching syntax similar to that used by shells (command interpreters) for "file globbing". See QRegExp wildcard matching.
//        QRegExp::WildcardUnix	4	This is similar to Wildcard but with the behavior of a Unix shell. The wildcard characters can be escaped with the character "\".
//        QRegExp::FixedString	2	The pattern is a fixed string. This is equivalent to using the RegExp pattern on a string in which all metacharacters are escaped using escape().
//        QRegExp::W3CXmlSchema11	5	The pattern is a regular expression as defined by the W3C XML Schema 1.1 specification.
    };

    enum Syntax {
        SyntaxDefault,          /// The default syntax
        SyntaxFixedString       /// A plain fixed string
    };



    RegExp( const QString& pattern, bool caseSensitive=true, Syntax syntax=SyntaxDefault, Engine engine=EngineOniguruma );
    virtual ~RegExp();

    static QString escape( const QString& str, Engine engine=EngineOniguruma );

    bool isValid() const;
    QString errorString() const ;
    QString pattern() const ;


    int	indexIn( const QString& str, int offset = 0 ); // const;
    int indexIn( const QChar* str, int offset, int length );
    int lastIndexIn( const QString& str, int offset=-1 );
    int lastIndexIn( const QChar* str, int offset, int length );
    int pos( int nth = 0 ) const;
    int len( int nth = 0 ) const;
    QString cap( int nth = 0) const;


    /// matched length is equal to pos-0-length
    int matchedLength() { return len(0); }

    //    int cap( int nth = 0 ) const;

private:
    RegExpEngine* d_;       ///< The private data member
};

} // edbee
